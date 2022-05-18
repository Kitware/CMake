/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmXCOFF.h"

#include <algorithm>
#include <cstddef>

#include <cm/memory>

#include "cmsys/FStream.hxx"

#include "cmStringAlgorithms.h"

// Include the XCOFF format information system header.
#ifdef _AIX
#  define __XCOFF32__
#  define __XCOFF64__
#  include <xcoff.h>
#else
#  error "This source may be compiled only on AIX."
#endif

class cmXCOFFInternal
{
public:
  // Construct and take ownership of the file stream object.
  cmXCOFFInternal(cmXCOFF* external, std::unique_ptr<std::iostream> fin,
                  cmXCOFF::Mode mode)
    : External(external)
    , Stream(std::move(fin))
    , Mode(mode)
  {
  }

  // Destruct and delete the file stream object.
  virtual ~cmXCOFFInternal() = default;

  cmXCOFF::Mode GetMode() const { return this->Mode; }

  virtual cm::optional<cm::string_view> GetLibPath() = 0;

  virtual bool SetLibPath(cm::string_view libPath) = 0;
  virtual bool RemoveLibPath() = 0;

protected:
  // Data common to all ELF class implementations.

  // The external cmXCOFF object.
  cmXCOFF* External;

  // The stream from which to read.
  std::unique_ptr<std::iostream> Stream;

  cmXCOFF::Mode Mode;

  // Helper methods for subclasses.
  void SetErrorMessage(const char* msg) { this->External->ErrorMessage = msg; }
};

namespace {

struct XCOFF32
{
  using filehdr = struct filehdr;
  using aouthdr = struct aouthdr;
  using scnhdr = struct scnhdr;
  using ldhdr = struct ldhdr;
  static const std::size_t aouthdr_size = _AOUTHSZ_EXEC;
};
const unsigned char xcoff32_magic[] = { 0x01, 0xDF };

struct XCOFF64
{
  using filehdr = struct filehdr_64;
  using aouthdr = struct aouthdr_64;
  using scnhdr = struct scnhdr_64;
  using ldhdr = struct ldhdr_64;
  static const std::size_t aouthdr_size = _AOUTHSZ_EXEC_64;
};
const unsigned char xcoff64_magic[] = { 0x01, 0xF7 };

template <typename XCOFF>
class Impl : public cmXCOFFInternal
{
  static_assert(sizeof(typename XCOFF::aouthdr) == XCOFF::aouthdr_size,
                "aouthdr structure size matches _AOUTHSZ_EXEC macro");

  typename XCOFF::filehdr FileHeader;
  typename XCOFF::aouthdr AuxHeader;
  typename XCOFF::scnhdr LoaderSectionHeader;
  typename XCOFF::ldhdr LoaderHeader;

  std::streamoff LoaderImportFileTablePos = 0;
  std::vector<char> LoaderImportFileTable;

  bool Read(typename XCOFF::filehdr& x)
  {
    // FIXME: Add byte swapping if needed.
    return static_cast<bool>(
      this->Stream->read(reinterpret_cast<char*>(&x), sizeof(x)));
  }

  bool Read(typename XCOFF::aouthdr& x)
  {
    // FIXME: Add byte swapping if needed.
    return static_cast<bool>(
      this->Stream->read(reinterpret_cast<char*>(&x), sizeof(x)));
  }

  bool Read(typename XCOFF::scnhdr& x)
  {
    // FIXME: Add byte swapping if needed.
    return static_cast<bool>(
      this->Stream->read(reinterpret_cast<char*>(&x), sizeof(x)));
  }

  bool Read(typename XCOFF::ldhdr& x)
  {
    // FIXME: Add byte swapping if needed.
    return static_cast<bool>(
      this->Stream->read(reinterpret_cast<char*>(&x), sizeof(x)));
  }

  bool WriteLoaderImportFileTableLength(decltype(XCOFF::ldhdr::l_istlen) x)
  {
    // FIXME: Add byte swapping if needed.
    return static_cast<bool>(
      this->Stream->write(reinterpret_cast<char const*>(&x), sizeof(x)));
  }

public:
  Impl(cmXCOFF* external, std::unique_ptr<std::iostream> fin,
       cmXCOFF::Mode mode);

  cm::optional<cm::string_view> GetLibPath() override;
  bool SetLibPath(cm::string_view libPath) override;
  bool RemoveLibPath() override;
};

template <typename XCOFF>
Impl<XCOFF>::Impl(cmXCOFF* external, std::unique_ptr<std::iostream> fin,
                  cmXCOFF::Mode mode)
  : cmXCOFFInternal(external, std::move(fin), mode)
{
  if (!this->Read(this->FileHeader)) {
    this->SetErrorMessage("Failed to read XCOFF file header.");
    return;
  }
  if (this->FileHeader.f_opthdr != XCOFF::aouthdr_size) {
    this->SetErrorMessage("XCOFF auxiliary header missing.");
    return;
  }
  if (!this->Read(this->AuxHeader)) {
    this->SetErrorMessage("Failed to read XCOFF auxiliary header.");
    return;
  }
  if (this->AuxHeader.o_snloader == 0) {
    this->SetErrorMessage("XCOFF loader section missing.");
    return;
  }
  if (!this->Stream->seekg((this->AuxHeader.o_snloader - 1) *
                             sizeof(typename XCOFF::scnhdr),
                           std::ios::cur)) {
    this->SetErrorMessage("Failed to seek to XCOFF loader section header.");
    return;
  }
  if (!this->Read(this->LoaderSectionHeader)) {
    this->SetErrorMessage("Failed to read XCOFF loader section header.");
    return;
  }
  if ((this->LoaderSectionHeader.s_flags & STYP_LOADER) == 0) {
    this->SetErrorMessage("XCOFF loader section header missing STYP_LOADER.");
    return;
  }
  if (!this->Stream->seekg(this->LoaderSectionHeader.s_scnptr,
                           std::ios::beg)) {
    this->SetErrorMessage("Failed to seek to XCOFF loader header.");
    return;
  }
  if (!this->Read(this->LoaderHeader)) {
    this->SetErrorMessage("Failed to read XCOFF loader header.");
    return;
  }
  this->LoaderImportFileTablePos =
    this->LoaderSectionHeader.s_scnptr + this->LoaderHeader.l_impoff;
  if (!this->Stream->seekg(this->LoaderImportFileTablePos)) {
    this->SetErrorMessage(
      "Failed to seek to XCOFF loader import file id table.");
    return;
  }
  this->LoaderImportFileTable.resize(this->LoaderHeader.l_istlen);
  if (!this->Stream->read(this->LoaderImportFileTable.data(),
                          this->LoaderImportFileTable.size())) {
    this->SetErrorMessage("Failed to read XCOFF loader import file id table.");
    return;
  }
}

template <typename XCOFF>
cm::optional<cm::string_view> Impl<XCOFF>::GetLibPath()
{
  cm::optional<cm::string_view> result;
  auto end = std::find(this->LoaderImportFileTable.begin(),
                       this->LoaderImportFileTable.end(), '\0');
  if (end != this->LoaderImportFileTable.end()) {
    result = cm::string_view(this->LoaderImportFileTable.data(),
                             end - this->LoaderImportFileTable.begin());
  }
  return result;
}

template <typename XCOFF>
bool Impl<XCOFF>::SetLibPath(cm::string_view libPath)
{
  // The new LIBPATH must end in the standard AIX LIBPATH.
#define CM_AIX_LIBPATH "/usr/lib:/lib"
  std::string libPathBuf;
  if (libPath != CM_AIX_LIBPATH &&
      !cmHasLiteralSuffix(libPath, ":" CM_AIX_LIBPATH)) {
    libPathBuf = std::string(libPath);
    if (!libPathBuf.empty() && libPathBuf.back() != ':') {
      libPathBuf.push_back(':');
    }
    libPathBuf += CM_AIX_LIBPATH;
    libPath = libPathBuf;
  }
#undef CM_AIX_LIBPATH

  auto oldEnd = std::find(this->LoaderImportFileTable.begin(),
                          this->LoaderImportFileTable.end(), '\0');
  if (oldEnd == this->LoaderImportFileTable.end()) {
    this->SetErrorMessage("XCOFF loader import file id table is invalid.");
    return false;
  }
  if ((this->LoaderImportFileTable.begin() + libPath.size()) > oldEnd) {
    this->SetErrorMessage("XCOFF loader import file id table is too small.");
    return false;
  }

  {
    std::vector<char> ift;
    ift.reserve(this->LoaderImportFileTable.size());
    // Start with the new LIBPATH.
    ift.insert(ift.end(), libPath.begin(), libPath.end());
    // Add the rest of the original table.
    ift.insert(ift.end(), oldEnd, this->LoaderImportFileTable.end());
    // If the new table is shorter, zero out the leftover space.
    ift.resize(this->LoaderImportFileTable.size(), 0);
    this->LoaderHeader.l_istlen =
      static_cast<decltype(XCOFF::ldhdr::l_istlen)>(ift.size());
    this->LoaderImportFileTable = std::move(ift);
  }

  if (!this->Stream->seekp(this->LoaderSectionHeader.s_scnptr +
                             offsetof(typename XCOFF::ldhdr, l_istlen),
                           std::ios::beg)) {
    this->SetErrorMessage(
      "Failed to seek to XCOFF loader header import file id table length.");
    return false;
  }
  if (!this->WriteLoaderImportFileTableLength(this->LoaderHeader.l_istlen)) {
    this->SetErrorMessage(
      "Failed to write XCOFF loader header import file id table length.");
    return false;
  }
  if (!this->Stream->seekp(this->LoaderImportFileTablePos, std::ios::beg)) {
    this->SetErrorMessage(
      "Failed to seek to XCOFF loader import file id table.");
    return false;
  }
  if (!this->Stream->write(this->LoaderImportFileTable.data(),
                           this->LoaderImportFileTable.size())) {
    this->SetErrorMessage(
      "Failed to write XCOFF loader import file id table.");
    return false;
  }

  return true;
}

template <typename XCOFF>
bool Impl<XCOFF>::RemoveLibPath()
{
  return this->SetLibPath({});
}
}

//============================================================================
// External class implementation.

cmXCOFF::cmXCOFF(const char* fname, Mode mode)
{
  // Try to open the file.
  std::ios::openmode fmode = std::ios::in | std::ios::binary;
  if (mode == Mode::ReadWrite) {
    fmode |= std::ios::out;
  }
  auto f = cm::make_unique<cmsys::fstream>(fname, fmode);

  // Quit now if the file could not be opened.
  if (!f || !*f) {
    this->ErrorMessage = "Error opening input file.";
    return;
  }

  // Read the XCOFF magic number.
  unsigned char magic[2];
  if (!f->read(reinterpret_cast<char*>(magic), sizeof(magic))) {
    this->ErrorMessage = "Error reading XCOFF magic number.";
    return;
  }
  if (!f->seekg(0)) {
    this->ErrorMessage = "Error seeking to beginning of file.";
    return;
  }

  // Check the XCOFF type.
  if (magic[0] == xcoff32_magic[0] && magic[1] == xcoff32_magic[1]) {
    this->Internal = cm::make_unique<Impl<XCOFF32>>(this, std::move(f), mode);
  } else if (magic[0] == xcoff64_magic[0] && magic[1] == xcoff64_magic[1]) {
    this->Internal = cm::make_unique<Impl<XCOFF64>>(this, std::move(f), mode);
  } else {
    this->ErrorMessage = "File is not a XCOFF32 or XCOFF64 binary.";
  }
}

cmXCOFF::~cmXCOFF() = default;

cmXCOFF::cmXCOFF(cmXCOFF&&) noexcept = default;
cmXCOFF& cmXCOFF::operator=(cmXCOFF&&) noexcept = default;

bool cmXCOFF::Valid() const
{
  return this->Internal && this->ErrorMessage.empty();
}

cm::optional<cm::string_view> cmXCOFF::GetLibPath() const
{
  cm::optional<cm::string_view> result;
  if (this->Valid()) {
    result = this->Internal->GetLibPath();
  }
  return result;
}

bool cmXCOFF::SetLibPath(cm::string_view libPath)
{
  return this->Valid() && this->Internal->GetMode() == Mode::ReadWrite &&
    this->Internal->SetLibPath(libPath);
}

bool cmXCOFF::RemoveLibPath()
{
  return this->Valid() && this->Internal->GetMode() == Mode::ReadWrite &&
    this->Internal->RemoveLibPath();
}

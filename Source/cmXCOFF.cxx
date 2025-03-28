/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmXCOFF.h"

#include <algorithm>
#include <cstddef>

#include <cm/memory>
#include <cm/string_view>

#include "cmsys/FStream.hxx"

#include "cmStringAlgorithms.h"

// Include the XCOFF format information system header.
#ifdef _AIX
#  define __XCOFF32__
#  define __XCOFF64__
#  include <xcoff.h>
#  define __AR_BIG__
#  include <ar.h>
#else
#  error "This source may be compiled only on AIX."
#endif

// Function to align a number num with align_num bytes.
size_t align(size_t num, int align_num)
{
  align_num = 1 << (align_num);
  return (((num + align_num - 1) / align_num) * align_num);
}

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

enum class IsArchive
{
  No,
  Yes,
};

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

  bool Read(fl_hdr& x)
  {
    // FIXME: Add byte swapping if needed.
    return static_cast<bool>(
      this->Stream->read(reinterpret_cast<char*>(&x), sizeof(x)));
  }

  bool Read(ar_hdr& x)
  {
    // FIXME: Add byte swapping if needed.
    return static_cast<bool>(
      this->Stream->read(reinterpret_cast<char*>(&x), sizeof(x)));
  }

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
       cmXCOFF::Mode mode, IsArchive IsBigArchive, int nextEvenBytePos);

  cm::optional<cm::string_view> GetLibPath() override;
  bool SetLibPath(cm::string_view libPath) override;
  bool RemoveLibPath() override;

  // Needed for SetLibPath () to move in a archive while write.
  IsArchive is_big_archive;
  int nextEvenByte;
  int bytes_to_align;
};

template <typename XCOFF>
Impl<XCOFF>::Impl(cmXCOFF* external, std::unique_ptr<std::iostream> fin,
                  cmXCOFF::Mode mode, IsArchive IsBigArchive,
                  int nextEvenBytePos)
  : cmXCOFFInternal(external, std::move(fin), mode)
{
  this->is_big_archive = IsBigArchive;
  this->nextEvenByte = nextEvenBytePos;
  if (this->is_big_archive == IsArchive::Yes) {
    fl_hdr header;
    this->Stream->read(reinterpret_cast<char*>(&header), sizeof(fl_hdr));

    long long fstmoff = std::atoll(header.fl_fstmoff);
    this->Stream->seekg(fstmoff, std::ios::beg);

    ar_hdr arHeader;
    this->Stream->read(reinterpret_cast<char*>(&arHeader), sizeof(ar_hdr));

    // Move the pointer to next even byte after reading headers.
    this->Stream->seekg(this->nextEvenByte, std::ios::cur);
  }

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
  this->bytes_to_align =
    this->AuxHeader.o_algntext > this->AuxHeader.o_algndata
    ? this->AuxHeader.o_algntext
    : this->AuxHeader.o_algndata;
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
  if (is_big_archive == IsArchive::Yes) {
    size_t header_len = this->nextEvenByte + sizeof(fl_hdr) + sizeof(ar_hdr);
    size_t scnptrFromArchiveStart = this->LoaderSectionHeader.s_scnptr +
      align(header_len, this->bytes_to_align);
    if (!this->Stream->seekg(scnptrFromArchiveStart, std::ios::beg)) {
      this->SetErrorMessage("Failed to seek to XCOFF loader header.");
      return;
    }
  } else if (!this->Stream->seekg(this->LoaderSectionHeader.s_scnptr,
                                  std::ios::beg)) {
    this->SetErrorMessage("Failed to seek to XCOFF loader header.");
    return;
  }

  if (!this->Read(this->LoaderHeader)) {
    this->SetErrorMessage("Failed to read XCOFF loader header.");
    return;
  }
  if (is_big_archive == IsArchive::Yes) {
    size_t header_len = sizeof(fl_hdr) + sizeof(ar_hdr) + this->nextEvenByte;
    size_t scnptrFromArchiveStartPlusOff = this->LoaderSectionHeader.s_scnptr +
      this->LoaderHeader.l_impoff + align(header_len, this->bytes_to_align);
    this->LoaderImportFileTablePos = scnptrFromArchiveStartPlusOff;
  } else {
    this->LoaderImportFileTablePos =
      this->LoaderSectionHeader.s_scnptr + this->LoaderHeader.l_impoff;
  }
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
  // The new LIBPATH must contain standard AIX LIBPATH entries.
  std::string libPathBuf;
#define ENSURE_ENTRY(x)                                                       \
  if (libPath != x && !cmHasLiteralPrefix(libPath, x ":") &&                  \
      !cmHasLiteralSuffix(libPath, ":" x) &&                                  \
      libPath.find(":" x ":") == std::string::npos) {                         \
    libPathBuf = std::string(libPath);                                        \
    if (!libPathBuf.empty() && libPathBuf.back() != ':') {                    \
      libPathBuf.push_back(':');                                              \
    }                                                                         \
    libPathBuf += x;                                                          \
    libPath = libPathBuf;                                                     \
  }
  ENSURE_ENTRY("/usr/lib")
  ENSURE_ENTRY("/lib")
#undef ENSURE_ENTRY

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

  size_t scnptr;
  if (this->is_big_archive == IsArchive::Yes) {
    size_t header_len = sizeof(fl_hdr) + sizeof(ar_hdr) + this->nextEvenByte;
    scnptr = this->LoaderSectionHeader.s_scnptr +
      offsetof(typename XCOFF::ldhdr, l_istlen) +
      align(header_len, this->bytes_to_align);
  } else {
    scnptr = this->LoaderSectionHeader.s_scnptr +
      offsetof(typename XCOFF::ldhdr, l_istlen);
  }

  if (!this->Stream->seekp(scnptr, std::ios::beg)) {
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

IsArchive check_if_big_archive(const char* fname)
{
  int len = std::strlen(fname);
  if (len < 2) {
    return IsArchive::No;
  }

  if (std::strcmp(fname + len - 2, ".a") == 0) {
    return IsArchive::Yes;
  } else {
    return IsArchive::No;
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

  // To hold the length of the shared object name in the path.
  int nextEvenByte = 0;

  // Read archive name length.
  int archive_name_length = 0;
  // If a big archive, we will read the archive file headers first.
  // Then move to the next even byte to get the magic number.
  if (check_if_big_archive(fname) == IsArchive::Yes) {
    fl_hdr header;
    f->read(reinterpret_cast<char*>(&header), sizeof(fl_hdr));

    if (std::strncmp(header.fl_magic, AIAMAGBIG, SAIAMAG) != 0) {
      this->ErrorMessage = "Not a valid archive file or wrong format";

      return;
    }
    long long fstmoff = std::atoll(header.fl_fstmoff);
    f->seekg(fstmoff, std::ios::beg);

    ar_hdr arHeader;
    f->read(reinterpret_cast<char*>(&arHeader), sizeof(ar_hdr));

    {
      errno = 0;
      char* ar_namlen_endp;
      unsigned long ar_namlen =
        strtoul(arHeader.ar_namlen, &ar_namlen_endp, 10);
      if ((ar_namlen_endp != arHeader.ar_namlen) && (errno == 0)) {
        archive_name_length = static_cast<int>(ar_namlen);
      } else {
        this->ErrorMessage = "Error parsing archive name length.";
        return;
      }
    }

    // Round off to even byte.
    if (archive_name_length % 2 == 0) {
      nextEvenByte = archive_name_length;
    } else {
      nextEvenByte = archive_name_length + 1;
    }

    f->seekg(nextEvenByte, std::ios::cur);
  }

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
    this->Internal = cm::make_unique<Impl<XCOFF32>>(
      this, std::move(f), mode, check_if_big_archive(fname), nextEvenByte);
  } else if (magic[0] == xcoff64_magic[0] && magic[1] == xcoff64_magic[1]) {
    this->Internal = cm::make_unique<Impl<XCOFF64>>(
      this, std::move(f), mode, check_if_big_archive(fname), nextEvenByte);
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

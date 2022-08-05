/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmELF.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cmext/algorithm>

#include <cm3p/kwiml/abi.h>

#include "cmsys/FStream.hxx"

#include "cmelf/elf32.h"
#include "cmelf/elf64.h"
#include "cmelf/elf_common.h"

// Low-level byte swapping implementation.
template <size_t s>
struct cmELFByteSwapSize
{
};
static void cmELFByteSwap(char* data, cmELFByteSwapSize<2> /*unused*/)
{
  char one_byte;
  one_byte = data[0];
  data[0] = data[1];
  data[1] = one_byte;
}
static void cmELFByteSwap(char* data, cmELFByteSwapSize<4> /*unused*/)
{
  char one_byte;
  one_byte = data[0];
  data[0] = data[3];
  data[3] = one_byte;
  one_byte = data[1];
  data[1] = data[2];
  data[2] = one_byte;
}
static void cmELFByteSwap(char* data, cmELFByteSwapSize<8> /*unused*/)
{
  char one_byte;
  one_byte = data[0];
  data[0] = data[7];
  data[7] = one_byte;
  one_byte = data[1];
  data[1] = data[6];
  data[6] = one_byte;
  one_byte = data[2];
  data[2] = data[5];
  data[5] = one_byte;
  one_byte = data[3];
  data[3] = data[4];
  data[4] = one_byte;
}

// Low-level byte swapping interface.
template <typename T>
void cmELFByteSwap(T& x)
{
  cmELFByteSwap(reinterpret_cast<char*>(&x), cmELFByteSwapSize<sizeof(T)>());
}

class cmELFInternal
{
public:
  using StringEntry = cmELF::StringEntry;
  enum ByteOrderType
  {
    ByteOrderMSB,
    ByteOrderLSB
  };

  // Construct and take ownership of the file stream object.
  cmELFInternal(cmELF* external, std::unique_ptr<std::istream> fin,
                ByteOrderType order)
    : External(external)
    , Stream(std::move(fin))
    , ByteOrder(order)
  {
// In most cases the processor-specific byte order will match that
// of the target execution environment.  If we choose wrong here
// it is fixed when the header is read.
#if KWIML_ABI_ENDIAN_ID == KWIML_ABI_ENDIAN_ID_LITTLE
    this->NeedSwap = (this->ByteOrder == ByteOrderMSB);
#elif KWIML_ABI_ENDIAN_ID == KWIML_ABI_ENDIAN_ID_BIG
    this->NeedSwap = (this->ByteOrder == ByteOrderLSB);
#else
    this->NeedSwap = false; // Final decision is at runtime anyway.
#endif

    // We have not yet loaded the section info.
    this->DynamicSectionIndex = -1;
  }

  // Destruct and delete the file stream object.
  virtual ~cmELFInternal() = default;

  // Forward to the per-class implementation.
  virtual unsigned int GetNumberOfSections() const = 0;
  virtual unsigned long GetDynamicEntryPosition(int j) = 0;
  virtual cmELF::DynamicEntryList GetDynamicEntries() = 0;
  virtual std::vector<char> EncodeDynamicEntries(
    const cmELF::DynamicEntryList&) = 0;
  virtual StringEntry const* GetDynamicSectionString(unsigned int tag) = 0;
  virtual bool IsMips() const = 0;
  virtual void PrintInfo(std::ostream& os) const = 0;

  // Lookup the SONAME in the DYNAMIC section.
  StringEntry const* GetSOName()
  {
    return this->GetDynamicSectionString(DT_SONAME);
  }

  // Lookup the RPATH in the DYNAMIC section.
  StringEntry const* GetRPath()
  {
    return this->GetDynamicSectionString(DT_RPATH);
  }

  // Lookup the RUNPATH in the DYNAMIC section.
  StringEntry const* GetRunPath()
  {
    return this->GetDynamicSectionString(DT_RUNPATH);
  }

  // Return the recorded ELF type.
  cmELF::FileType GetFileType() const { return this->ELFType; }

  // Return the recorded machine.
  std::uint16_t GetMachine() const { return this->Machine; }

protected:
  // Data common to all ELF class implementations.

  // The external cmELF object.
  cmELF* External;

  // The stream from which to read.
  std::unique_ptr<std::istream> Stream;

  // The byte order of the ELF file.
  ByteOrderType ByteOrder;

  // The ELF file type.
  cmELF::FileType ELFType = cmELF::FileTypeInvalid;

  // The ELF architecture.
  std::uint16_t Machine;

  // Whether we need to byte-swap structures read from the stream.
  bool NeedSwap;

  // The section header index of the DYNAMIC section (-1 if none).
  int DynamicSectionIndex;

  // Helper methods for subclasses.
  void SetErrorMessage(const char* msg)
  {
    this->External->ErrorMessage = msg;
    this->ELFType = cmELF::FileTypeInvalid;
  }

  // Store string table entry states.
  std::map<unsigned int, StringEntry> DynamicSectionStrings;
};

// Configure the implementation template for 32-bit ELF files.
struct cmELFTypes32
{
  using ELF_Ehdr = Elf32_Ehdr;
  using ELF_Shdr = Elf32_Shdr;
  using ELF_Dyn = Elf32_Dyn;
  using ELF_Half = Elf32_Half;
  using tagtype = ::uint32_t;
  static const char* GetName() { return "32-bit"; }
};

// Configure the implementation template for 64-bit ELF files.
struct cmELFTypes64
{
  using ELF_Ehdr = Elf64_Ehdr;
  using ELF_Shdr = Elf64_Shdr;
  using ELF_Dyn = Elf64_Dyn;
  using ELF_Half = Elf64_Half;
  using tagtype = ::uint64_t;
  static const char* GetName() { return "64-bit"; }
};

// Parser implementation template.
template <class Types>
class cmELFInternalImpl : public cmELFInternal
{
public:
  // Copy the ELF file format types from our configuration parameter.
  using ELF_Ehdr = typename Types::ELF_Ehdr;
  using ELF_Shdr = typename Types::ELF_Shdr;
  using ELF_Dyn = typename Types::ELF_Dyn;
  using ELF_Half = typename Types::ELF_Half;
  using tagtype = typename Types::tagtype;

  // Construct with a stream and byte swap indicator.
  cmELFInternalImpl(cmELF* external, std::unique_ptr<std::istream> fin,
                    ByteOrderType order);

  // Return the number of sections as specified by the ELF header.
  unsigned int GetNumberOfSections() const override
  {
    return static_cast<unsigned int>(this->ELFHeader.e_shnum);
  }

  // Get the file position of a dynamic section entry.
  unsigned long GetDynamicEntryPosition(int j) override;

  cmELF::DynamicEntryList GetDynamicEntries() override;
  std::vector<char> EncodeDynamicEntries(
    const cmELF::DynamicEntryList&) override;

  // Lookup a string from the dynamic section with the given tag.
  StringEntry const* GetDynamicSectionString(unsigned int tag) override;

  bool IsMips() const override { return this->ELFHeader.e_machine == EM_MIPS; }

  // Print information about the ELF file.
  void PrintInfo(std::ostream& os) const override
  {
    os << "ELF " << Types::GetName();
    if (this->ByteOrder == ByteOrderMSB) {
      os << " MSB";
    } else if (this->ByteOrder == ByteOrderLSB) {
      os << " LSB";
    }
    switch (this->ELFType) {
      case cmELF::FileTypeInvalid:
        os << " invalid file";
        break;
      case cmELF::FileTypeRelocatableObject:
        os << " relocatable object";
        break;
      case cmELF::FileTypeExecutable:
        os << " executable";
        break;
      case cmELF::FileTypeSharedLibrary:
        os << " shared library";
        break;
      case cmELF::FileTypeCore:
        os << " core file";
        break;
      case cmELF::FileTypeSpecificOS:
        os << " os-specific type";
        break;
      case cmELF::FileTypeSpecificProc:
        os << " processor-specific type";
        break;
    }
    os << "\n";
  }

private:
  static_assert(sizeof(ELF_Dyn().d_un.d_val) == sizeof(ELF_Dyn().d_un.d_ptr),
                "ByteSwap(ELF_Dyn) assumes d_val and d_ptr are the same size");

  void ByteSwap(ELF_Ehdr& elf_header)
  {
    cmELFByteSwap(elf_header.e_type);
    cmELFByteSwap(elf_header.e_machine);
    cmELFByteSwap(elf_header.e_version);
    cmELFByteSwap(elf_header.e_entry);
    cmELFByteSwap(elf_header.e_phoff);
    cmELFByteSwap(elf_header.e_shoff);
    cmELFByteSwap(elf_header.e_flags);
    cmELFByteSwap(elf_header.e_ehsize);
    cmELFByteSwap(elf_header.e_phentsize);
    cmELFByteSwap(elf_header.e_phnum);
    cmELFByteSwap(elf_header.e_shentsize);
    cmELFByteSwap(elf_header.e_shnum);
    cmELFByteSwap(elf_header.e_shstrndx);
  }

  void ByteSwap(ELF_Shdr& sec_header)
  {
    cmELFByteSwap(sec_header.sh_name);
    cmELFByteSwap(sec_header.sh_type);
    cmELFByteSwap(sec_header.sh_flags);
    cmELFByteSwap(sec_header.sh_addr);
    cmELFByteSwap(sec_header.sh_offset);
    cmELFByteSwap(sec_header.sh_size);
    cmELFByteSwap(sec_header.sh_link);
    cmELFByteSwap(sec_header.sh_info);
    cmELFByteSwap(sec_header.sh_addralign);
    cmELFByteSwap(sec_header.sh_entsize);
  }

  void ByteSwap(ELF_Dyn& dyn)
  {
    cmELFByteSwap(dyn.d_tag);
    cmELFByteSwap(dyn.d_un.d_val);
  }

  bool FileTypeValid(ELF_Half et)
  {
    unsigned int eti = static_cast<unsigned int>(et);
    if (eti == ET_NONE || eti == ET_REL || eti == ET_EXEC || eti == ET_DYN ||
        eti == ET_CORE) {
      return true;
    }
    if (eti >= ET_LOOS && eti <= ET_HIOS) {
      return true;
    }
    if (eti >= ET_LOPROC && eti <= ET_HIPROC) {
      return true;
    }
    return false;
  }

  bool Read(ELF_Ehdr& x)
  {
    // Read the header from the file.
    if (!this->Stream->read(reinterpret_cast<char*>(&x), sizeof(x))) {
      return false;
    }

    // The byte order of ELF header fields may not match that of the
    // processor-specific data.  The header fields are ordered to
    // match the target execution environment, so we may need to
    // memorize the order of all platforms based on the e_machine
    // value.  As a heuristic, if the type is invalid but its
    // swapped value is okay then flip our swap mode.
    ELF_Half et = x.e_type;
    if (this->NeedSwap) {
      cmELFByteSwap(et);
    }
    if (!this->FileTypeValid(et)) {
      cmELFByteSwap(et);
      if (this->FileTypeValid(et)) {
        // The previous byte order guess was wrong.  Flip it.
        this->NeedSwap = !this->NeedSwap;
      }
    }

    // Fix the byte order of the header.
    if (this->NeedSwap) {
      this->ByteSwap(x);
    }
    return true;
  }
  bool Read(ELF_Shdr& x)
  {
    if (this->Stream->read(reinterpret_cast<char*>(&x), sizeof(x)) &&
        this->NeedSwap) {
      this->ByteSwap(x);
    }
    return !this->Stream->fail();
  }
  bool Read(ELF_Dyn& x)
  {
    if (this->Stream->read(reinterpret_cast<char*>(&x), sizeof(x)) &&
        this->NeedSwap) {
      this->ByteSwap(x);
    }
    return !this->Stream->fail();
  }

  bool LoadSectionHeader(ELF_Half i)
  {
    // Read the section header from the file.
    this->Stream->seekg(this->ELFHeader.e_shoff +
                        this->ELFHeader.e_shentsize * i);
    if (!this->Read(this->SectionHeaders[i])) {
      return false;
    }

    // Identify some important sections.
    if (this->SectionHeaders[i].sh_type == SHT_DYNAMIC) {
      this->DynamicSectionIndex = i;
    }
    return true;
  }

  bool LoadDynamicSection();

  // Store the main ELF header.
  ELF_Ehdr ELFHeader;

  // Store all the section headers.
  std::vector<ELF_Shdr> SectionHeaders;

  // Store all entries of the DYNAMIC section.
  std::vector<ELF_Dyn> DynamicSectionEntries;
};

template <class Types>
cmELFInternalImpl<Types>::cmELFInternalImpl(cmELF* external,
                                            std::unique_ptr<std::istream> fin,
                                            ByteOrderType order)
  : cmELFInternal(external, std::move(fin), order)
{
  // Read the main header.
  if (!this->Read(this->ELFHeader)) {
    this->SetErrorMessage("Failed to read main ELF header.");
    return;
  }

  // Determine the ELF file type.
  switch (this->ELFHeader.e_type) {
    case ET_NONE:
      this->SetErrorMessage("ELF file type is NONE.");
      return;
    case ET_REL:
      this->ELFType = cmELF::FileTypeRelocatableObject;
      break;
    case ET_EXEC:
      this->ELFType = cmELF::FileTypeExecutable;
      break;
    case ET_DYN:
      this->ELFType = cmELF::FileTypeSharedLibrary;
      break;
    case ET_CORE:
      this->ELFType = cmELF::FileTypeCore;
      break;
    default: {
      unsigned int eti = static_cast<unsigned int>(this->ELFHeader.e_type);
      if (eti >= ET_LOOS && eti <= ET_HIOS) {
        this->ELFType = cmELF::FileTypeSpecificOS;
        break;
      }
      if (eti >= ET_LOPROC && eti <= ET_HIPROC) {
        this->ELFType = cmELF::FileTypeSpecificProc;
        break;
      }
      std::ostringstream e;
      e << "Unknown ELF file type " << eti;
      this->SetErrorMessage(e.str().c_str());
      return;
    }
  }

  this->Machine = this->ELFHeader.e_machine;

  // Load the section headers.
  this->SectionHeaders.resize(this->ELFHeader.e_shnum);
  for (ELF_Half i = 0; i < this->ELFHeader.e_shnum; ++i) {
    if (!this->LoadSectionHeader(i)) {
      this->SetErrorMessage("Failed to load section headers.");
      return;
    }
  }
}

template <class Types>
bool cmELFInternalImpl<Types>::LoadDynamicSection()
{
  // If there is no dynamic section we are done.
  if (this->DynamicSectionIndex < 0) {
    return false;
  }

  // If the section was already loaded we are done.
  if (!this->DynamicSectionEntries.empty()) {
    return true;
  }

  // If there are no entries we are done.
  ELF_Shdr const& sec = this->SectionHeaders[this->DynamicSectionIndex];
  if (sec.sh_entsize == 0) {
    return false;
  }

  // Allocate the dynamic section entries.
  int n = static_cast<int>(sec.sh_size / sec.sh_entsize);
  this->DynamicSectionEntries.resize(n);

  // Read each entry.
  for (int j = 0; j < n; ++j) {
    // Seek to the beginning of the section entry.
    this->Stream->seekg(sec.sh_offset + sec.sh_entsize * j);
    ELF_Dyn& dyn = this->DynamicSectionEntries[j];

    // Try reading the entry.
    if (!this->Read(dyn)) {
      this->SetErrorMessage("Error reading entry from DYNAMIC section.");
      this->DynamicSectionIndex = -1;
      return false;
    }
  }
  return true;
}

template <class Types>
unsigned long cmELFInternalImpl<Types>::GetDynamicEntryPosition(int j)
{
  if (!this->LoadDynamicSection()) {
    return 0;
  }
  if (j < 0 || j >= static_cast<int>(this->DynamicSectionEntries.size())) {
    return 0;
  }
  ELF_Shdr const& sec = this->SectionHeaders[this->DynamicSectionIndex];
  return static_cast<unsigned long>(sec.sh_offset + sec.sh_entsize * j);
}

template <class Types>
cmELF::DynamicEntryList cmELFInternalImpl<Types>::GetDynamicEntries()
{
  cmELF::DynamicEntryList result;

  // Ensure entries have been read from file
  if (!this->LoadDynamicSection()) {
    return result;
  }

  // Copy into public array
  result.reserve(this->DynamicSectionEntries.size());
  for (ELF_Dyn& dyn : this->DynamicSectionEntries) {
    result.emplace_back(dyn.d_tag, dyn.d_un.d_val);
  }

  return result;
}

template <class Types>
std::vector<char> cmELFInternalImpl<Types>::EncodeDynamicEntries(
  const cmELF::DynamicEntryList& entries)
{
  std::vector<char> result;
  result.reserve(sizeof(ELF_Dyn) * entries.size());

  for (auto const& entry : entries) {
    // Store the entry in an ELF_Dyn, byteswap it, then serialize to chars
    ELF_Dyn dyn;
    dyn.d_tag = static_cast<tagtype>(entry.first);
    dyn.d_un.d_val = static_cast<tagtype>(entry.second);

    if (this->NeedSwap) {
      this->ByteSwap(dyn);
    }

    char* pdyn = reinterpret_cast<char*>(&dyn);
    cm::append(result, pdyn, pdyn + sizeof(ELF_Dyn));
  }

  return result;
}

template <class Types>
cmELF::StringEntry const* cmELFInternalImpl<Types>::GetDynamicSectionString(
  unsigned int tag)
{
  // Short-circuit if already checked.
  auto dssi = this->DynamicSectionStrings.find(tag);
  if (dssi != this->DynamicSectionStrings.end()) {
    if (dssi->second.Position > 0) {
      return &dssi->second;
    }
    return nullptr;
  }

  // Create an entry for this tag.  Assume it is missing until found.
  StringEntry& se = this->DynamicSectionStrings[tag];
  se.Position = 0;
  se.Size = 0;
  se.IndexInSection = -1;

  // Try reading the dynamic section.
  if (!this->LoadDynamicSection()) {
    return nullptr;
  }

  // Get the string table referenced by the DYNAMIC section.
  ELF_Shdr const& sec = this->SectionHeaders[this->DynamicSectionIndex];
  if (sec.sh_link >= this->SectionHeaders.size()) {
    this->SetErrorMessage("Section DYNAMIC has invalid string table index.");
    return nullptr;
  }
  ELF_Shdr const& strtab = this->SectionHeaders[sec.sh_link];

  // Look for the requested entry.
  for (auto di = this->DynamicSectionEntries.begin();
       di != this->DynamicSectionEntries.end(); ++di) {
    ELF_Dyn& dyn = *di;
    if (static_cast<tagtype>(dyn.d_tag) == static_cast<tagtype>(tag)) {
      // We found the tag requested.
      // Make sure the position given is within the string section.
      if (dyn.d_un.d_val >= strtab.sh_size) {
        this->SetErrorMessage("Section DYNAMIC references string beyond "
                              "the end of its string section.");
        return nullptr;
      }

      // Seek to the position reported by the entry.
      unsigned long first = static_cast<unsigned long>(dyn.d_un.d_val);
      unsigned long last = first;
      unsigned long end = static_cast<unsigned long>(strtab.sh_size);
      this->Stream->seekg(strtab.sh_offset + first);

      // Read the string.  It may be followed by more than one NULL
      // terminator.  Count the total size of the region allocated to
      // the string.  This assumes that the next string in the table
      // is non-empty, but the "chrpath" tool makes the same
      // assumption.
      bool terminated = false;
      char c;
      while (last != end && this->Stream->get(c) && !(terminated && c)) {
        ++last;
        if (c) {
          se.Value += c;
        } else {
          terminated = true;
        }
      }

      // Make sure the whole value was read.
      if (!(*this->Stream)) {
        this->SetErrorMessage("Dynamic section specifies unreadable RPATH.");
        se.Value = "";
        return nullptr;
      }

      // The value has been read successfully.  Report it.
      se.Position = static_cast<unsigned long>(strtab.sh_offset + first);
      se.Size = last - first;
      se.IndexInSection =
        static_cast<int>(di - this->DynamicSectionEntries.begin());
      return &se;
    }
  }
  return nullptr;
}

//============================================================================
// External class implementation.

const long cmELF::TagRPath = DT_RPATH;
const long cmELF::TagRunPath = DT_RUNPATH;
const long cmELF::TagMipsRldMapRel = DT_MIPS_RLD_MAP_REL;

cmELF::cmELF(const char* fname)
{
  // Try to open the file.
  auto fin = cm::make_unique<cmsys::ifstream>(fname, std::ios::binary);

  // Quit now if the file could not be opened.
  if (!fin || !*fin) {
    this->ErrorMessage = "Error opening input file.";
    return;
  }

  // Read the ELF identification block.
  char ident[EI_NIDENT];
  if (!fin->read(ident, EI_NIDENT)) {
    this->ErrorMessage = "Error reading ELF identification.";
    return;
  }
  if (!fin->seekg(0)) {
    this->ErrorMessage = "Error seeking to beginning of file.";
    return;
  }

  // Verify the ELF identification.
  if (!(ident[EI_MAG0] == ELFMAG0 && ident[EI_MAG1] == ELFMAG1 &&
        ident[EI_MAG2] == ELFMAG2 && ident[EI_MAG3] == ELFMAG3)) {
    this->ErrorMessage = "File does not have a valid ELF identification.";
    return;
  }

  // Check the byte order in which the rest of the file is encoded.
  cmELFInternal::ByteOrderType order;
  if (ident[EI_DATA] == ELFDATA2LSB) {
    // File is LSB.
    order = cmELFInternal::ByteOrderLSB;
  } else if (ident[EI_DATA] == ELFDATA2MSB) {
    // File is MSB.
    order = cmELFInternal::ByteOrderMSB;
  } else {
    this->ErrorMessage = "ELF file is not LSB or MSB encoded.";
    return;
  }

  // Check the class of the file and construct the corresponding
  // parser implementation.
  if (ident[EI_CLASS] == ELFCLASS32) {
    // 32-bit ELF
    this->Internal = cm::make_unique<cmELFInternalImpl<cmELFTypes32>>(
      this, std::move(fin), order);
  } else if (ident[EI_CLASS] == ELFCLASS64) {
    // 64-bit ELF
    this->Internal = cm::make_unique<cmELFInternalImpl<cmELFTypes64>>(
      this, std::move(fin), order);
  } else {
    this->ErrorMessage = "ELF file class is not 32-bit or 64-bit.";
    return;
  }
}

cmELF::~cmELF() = default;

bool cmELF::Valid() const
{
  return this->Internal && this->Internal->GetFileType() != FileTypeInvalid;
}

cmELF::FileType cmELF::GetFileType() const
{
  if (this->Valid()) {
    return this->Internal->GetFileType();
  }
  return FileTypeInvalid;
}

std::uint16_t cmELF::GetMachine() const
{
  if (this->Valid()) {
    return this->Internal->GetMachine();
  }
  return 0;
}

unsigned int cmELF::GetNumberOfSections() const
{
  if (this->Valid()) {
    return this->Internal->GetNumberOfSections();
  }
  return 0;
}

unsigned long cmELF::GetDynamicEntryPosition(int index) const
{
  if (this->Valid()) {
    return this->Internal->GetDynamicEntryPosition(index);
  }
  return 0;
}

cmELF::DynamicEntryList cmELF::GetDynamicEntries() const
{
  if (this->Valid()) {
    return this->Internal->GetDynamicEntries();
  }

  return cmELF::DynamicEntryList();
}

std::vector<char> cmELF::EncodeDynamicEntries(
  const cmELF::DynamicEntryList& dentries) const
{
  if (this->Valid()) {
    return this->Internal->EncodeDynamicEntries(dentries);
  }

  return std::vector<char>();
}

bool cmELF::GetSOName(std::string& soname)
{
  if (StringEntry const* se = this->GetSOName()) {
    soname = se->Value;
    return true;
  }
  return false;
}

cmELF::StringEntry const* cmELF::GetSOName()
{
  if (this->Valid() &&
      this->Internal->GetFileType() == cmELF::FileTypeSharedLibrary) {
    return this->Internal->GetSOName();
  }
  return nullptr;
}

cmELF::StringEntry const* cmELF::GetRPath()
{
  if (this->Valid() &&
      (this->Internal->GetFileType() == cmELF::FileTypeExecutable ||
       this->Internal->GetFileType() == cmELF::FileTypeSharedLibrary)) {
    return this->Internal->GetRPath();
  }
  return nullptr;
}

cmELF::StringEntry const* cmELF::GetRunPath()
{
  if (this->Valid() &&
      (this->Internal->GetFileType() == cmELF::FileTypeExecutable ||
       this->Internal->GetFileType() == cmELF::FileTypeSharedLibrary)) {
    return this->Internal->GetRunPath();
  }
  return nullptr;
}

bool cmELF::IsMIPS() const
{
  if (this->Valid()) {
    return this->Internal->IsMips();
  }
  return false;
}

void cmELF::PrintInfo(std::ostream& os) const
{
  if (this->Valid()) {
    this->Internal->PrintInfo(os);
  } else {
    os << "Not a valid ELF file.\n";
  }
}

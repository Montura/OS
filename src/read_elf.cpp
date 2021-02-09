#include <cstdint>
#include <fstream>
#include <vector>

#define ELF_NIDENT	16

// Program headers with type PT_LOAD must be loaded into the application memory during its loading
#define PT_LOAD		1

// ELF header
struct elf_hdr {
  std::uint8_t e_ident[ELF_NIDENT];
  std::uint16_t e_type;
  std::uint16_t e_machine;
  std::uint32_t e_version;
  std::uint64_t e_entry;
  std::uint64_t e_phoff;
  std::uint64_t e_shoff;
  std::uint32_t e_flags;
  std::uint16_t e_ehsize;
  std::uint16_t e_phentsize;
  std::uint16_t e_phnum;
  std::uint16_t e_shentsize;
  std::uint16_t e_shnum;
  std::uint16_t e_shstrndx;
} __attribute__((packed));

// ELF program header entry
struct elf_phdr {
  std::uint32_t p_type;
  std::uint32_t p_flags;
  std::uint64_t p_offset;
  std::uint64_t p_vaddr;
  std::uint64_t p_paddr;
  std::uint64_t p_filesz;
  std::uint64_t p_memsz;
  std::uint64_t p_align;
} __attribute__((packed));


// returns the address of main entry point
std::uintptr_t entry_point(const char *name) {
  std::ifstream file(name, std::ios::binary | std::ios::ate);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  if (file.read(buffer.data(), size)) {
    auto header = reinterpret_cast<elf_hdr*>(buffer.data());
    return header->e_entry;
  }

  return 0;
}

// returns the size of memory required to load the program
std::size_t space(const char *name) {
  std::ifstream file(name, std::ios::binary | std::ios::ate);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  char *const buf = buffer.data();
  if (file.read(buf, size)) {
    auto header = reinterpret_cast<elf_hdr*>(buf);

    const uint16_t program_header_entity_size = header->e_phentsize;
    const uint16_t program_header_count = header->e_phnum;

    char* phdr_start = buf + header->e_phoff;
    elf_phdr phdr {};

    uint64_t res = 0;
    for (int i = 0; i< program_header_count; ++i) {
      phdr = *(reinterpret_cast<elf_phdr*>(phdr_start + i * program_header_entity_size));

      if (phdr.p_type == PT_LOAD) {
        res += phdr.p_memsz;
      }
    }
    return res;
  }

  return 0;
}
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>

#include <memory.hh>

using namespace mem;

// method dump_hex that receives a segment identifier and prints
// the 32 bit in hexadecimal format

void
memory::load_binary(const std::string& binfile)
{
    using ifile_iter = std::istream_iterator<uint8_t>;

    std::ifstream ifile(binfile, std::ios::binary);

    if (ifile.is_open() == false) {
	std::cerr << "Unable to open "<< binfile << std::endl;
	std::exit(EXIT_FAILURE);
    }

    // copy the binary into memory
    // Stop eating new lines in binary mode!!!
    ifile.unsetf(std::ios::skipws);

    ifile.seekg(0, std::ios::end);
    const auto ifsize = ifile.tellg();
    ifile.seekg(0, std::ios::beg);

    _binary.reserve(ifsize);
    _binary.insert(_binary.begin(),
	    ifile_iter(ifile),
	    ifile_iter());
    ifile.close();


    // read elf header
    _ehdr = *reinterpret_cast<Elf32_Ehdr*>(_binary.data());

    //std::cout << "Machine type: " << _ehdr.e_machine << std::endl;

    // ensure riscv32
    if (_ehdr.e_machine != EM_RISCV) {
        std::cerr << "Invalid machine type: " << _ehdr.e_machine << std::endl;
        std::exit(EXIT_FAILURE);
    }


    // ensure the binary has a correct program table
    if (_ehdr.e_phnum == 0) {
        std::cerr << "No program header table found" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // entry point
    //Elf32_Addr _entry_addr = _ehdr.e_entry;

    // load sections in memory
    // load program header table
    for (int i = 0; i < _ehdr.e_phnum; ++i) {
        Elf32_Phdr phdr = *reinterpret_cast<Elf32_Phdr*>(_binary.data() + _ehdr.e_phoff + i * _ehdr.e_phentsize);
        _phdr.push_back(phdr);
    }

    //load segments in memory
    for (int i = 0; i < _ehdr.e_phnum; ++i) {
        const Elf32_Phdr& phdr = _phdr[i];

        if (phdr.p_type == PT_LOAD) {
            segment seg;
            seg._initial_address = phdr.p_vaddr;
            seg._content.insert(seg._content.begin(),
                    _binary.begin() + phdr.p_offset,
                    _binary.begin() + phdr.p_offset + phdr.p_filesz);
            _segments.push_back(seg);
        }
    }

    // read ELF program header table,
    // ... to be completed
}

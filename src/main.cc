#include <iostream>
#include <map>

#include <instructions.hh>
#include <memory.hh>
#include <processor.hh>

using namespace instrs;
using namespace mem;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Invalid Syntax: peRISCVcope <program>" << std::endl;
        exit(1);
    }

   memory mem;
   processor proc;

   //map para hacer la relación instrucción-función
   std::map <uint8_t, instr_emulation> dispatch_map = {
       {0b0000011, instrs::load},
       {0b0100011, instrs::store},
       {0b0010011, instrs::alui},
       {0b0110011, instrs::alur},
       {0b0110111, instrs::lui},
       {0b1101111, instrs::jal},
       {0b1100011, instrs::condbranch}
   };


   mem.load_binary(argv[1]);
   mem.dump_hex(1);

   // read the entry point
   // ...

   proc.write_pc(mem.entry_point());

   // set the stack pointer
   // the stack grows downward with the stack pointer always being 16-byte aligned
   proc.write_reg(processor::sp, memory::stack_top);

   address_t pc = 0xDEADBEEF, next_pc = 0xDEADBEEF;

   size_t exec_instrs = 0;

   // Initialize sp
    proc.write_reg(2, memory::stack_top);

   do
   {
       // main interpreter loop
        pc = proc.read_pc();
        uint32_t instr = mem.read<uint32_t>(pc);

        std::cout << "Reading instrucion" << std::endl;

        next_pc = dispatch_map[(instr & 0x7F)](mem, proc, instr);

        proc.write_pc(next_pc);
       exec_instrs++;
   } while (next_pc != pc); // look for while(1) in the code

   std::cout << "Number of executed instructions: " << exec_instrs << std::endl;
}

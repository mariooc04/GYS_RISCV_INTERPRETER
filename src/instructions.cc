#include <instructions.hh>
#include <memory.hh>
using namespace instrs;
using namespace mem;

// ToDo use the type instead of RISC-V funct3 field?
// assume little endian
// LW 0b010
template<>
void instrs::execute_load<0b010>(mem::memory& mem, processor & proc,
    mem::address_t addr, uint8_t rd)
{
  assert((addr & 0b11) == 0); // ensure alignment
  proc.write_reg(rd, mem.read<uint32_t>(addr));
}

// LBU 100
template<>
void instrs::execute_load<0b100>(mem::memory& mem, processor & proc,
    mem::address_t addr, uint8_t rd)
{
  // no sign extension
  uint32_t val = mem.read<uint8_t>(addr);
  proc.write_reg(rd, val);
}

// LB 000
template<>
void instrs::execute_load<0b000>(mem::memory& mem, processor & proc,
    mem::address_t addr, uint8_t rd)
{
  uint32_t val = sign_extend<uint32_t, sizeof(uint8_t)*8>(mem.read<uint8_t>(addr));
  proc.write_reg(rd, val);
}

// LH
template<>
void instrs::execute_load<0b001>(mem::memory& mem, processor & proc,
    mem::address_t addr, uint8_t rd)
{
  assert((addr & 0b1) == 0); // ensure alignment
  // perform sign extension
  uint32_t val = sign_extend<uint32_t,
           sizeof(uint16_t)*8>(mem.read<uint16_t>(addr));
  proc.write_reg(rd, val);
}

// LHU
template<>
void instrs::execute_load<0b101>(mem::memory& mem, processor & proc,
    mem::address_t addr, uint8_t rd)
{
  assert((addr & 0b1) == 0); // ensure alignment
  // perform sign extension
  uint32_t val = mem.read<uint16_t>(addr);
  proc.write_reg(rd, val);
}


uint32_t instrs::load(memory& mem, processor & proc, uint32_t bitstream) {

  i_instruction ii{bitstream};

  // compute src address
  address_t src = proc.read_reg(ii.rs1()) + ii.imm();

  // ToDo refactor with templates
  switch(ii.funct3()) {
    case 0b010: execute_load<0b010>(mem, proc, src, ii.rd()); break;
    case 0b000: execute_load<0b000>(mem, proc, src, ii.rd()); break;
    case 0b001: execute_load<0b001>(mem, proc, src, ii.rd()); break;
    case 0b100: execute_load<0b100>(mem, proc, src, ii.rd()); break;
    case 0b101: execute_load<0b101>(mem, proc, src, ii.rd()); break;
  }
  // return next instruction
  return proc.next_pc();
}

// SB
template<>
void instrs::execute_store<0b000>(mem::memory& mem, processor& proc,
    mem::address_t addr, uint8_t rs2)
{
  mem.write<uint8_t>(addr, static_cast<uint8_t>(proc.read_reg(rs2)));
}

// SH
template<>
void instrs::execute_store<0b001>(mem::memory& mem, processor& proc,
    mem::address_t addr, uint8_t rs2)
{
  mem.write<uint16_t>(addr, static_cast<uint16_t>(proc.read_reg(rs2)));
}

// SW
template<>
void instrs::execute_store<0b010>(mem::memory& mem, processor& proc,
    mem::address_t addr, uint8_t rs2)
{
  mem.write<uint32_t>(addr, proc.read_reg(rs2));
}

uint32_t instrs::store(memory& mem, processor & proc, uint32_t bitstream) {

  s_instruction si{bitstream};

  // compute dst address
  address_t src = proc.read_reg(si.rs1()) + si.imm();

  // ToDo refactor with templates
  switch(si.funct3()) {
    case 0b000: execute_store<0b000>(mem, proc, src, si.rs2()); break; // SB
    case 0b001: execute_store<0b001>(mem, proc, src, si.rs2()); break; // SH
    case 0b010: execute_store<0b010>(mem, proc, src, si.rs2()); break; // SW
  }
  // return next instruction
  return proc.next_pc();
}

// alu and immediate
uint32_t instrs::alui(memory&, processor & proc, uint32_t bitstream) {
  i_instruction ii{bitstream};

  uint32_t val = proc.read_reg(ii.rs1());
  switch(ii.funct3()) {
    case 0b001: val = val - ii.imm(); break;
    case 0b000: val = val + ii.imm(); break;
  }

  std::cout << "ALUI: " << val << std::endl;
  std::cout << "ALUI: " << (uint32_t)ii.rd() << std::endl;

  proc.write_reg(ii.rd(), val);


  return proc.next_pc();
}

// alu and register
uint32_t instrs::alur(memory&, processor & proc, uint32_t bitstream) {
  r_instruction ii{bitstream};

  uint32_t val1 = proc.read_reg(ii.rs1());
  uint32_t val2 = proc.read_reg(ii.rs2());
  uint32_t val = 0;
  switch(ii.funct3()) {
    case 0b000: val = val1 + val2; break;
    case 0b001: val = val1 - val2; break;
  }

  proc.write_reg(ii.rd(), val);

  return proc.next_pc();
}

// load upper immediate
uint32_t instrs::lui(memory&, processor& proc, uint32_t bitstream) {
  u_instruction ii{bitstream};

  proc.write_reg(ii.rd(), ii.imm());

  return proc.next_pc();
}

// jump and link
uint32_t instrs::jal(memory&, processor& proc, uint32_t bitstream) {
  j_instruction ii{bitstream};

  // compute src address
  address_t src = proc.next_pc() + ii.imm();

  // save return address
  proc.write_reg(1, proc.next_pc());

  // jump
  return src;
}

// TODO condbranch
uint32_t instrs::condbranch(memory&, processor& proc, uint32_t bitstream) {
  b_instruction ii{bitstream};

  uint32_t val1 = proc.read_reg(ii.rs1());
  uint32_t val2 = proc.read_reg(ii.rs2());
  uint32_t val = 0;
  switch(ii.funct3()) {
    case 0b000: val = val1 + val2; break;
    case 0b001: val = val1 - val2; break;
  }

  // Implementation of branch
  if (val == 0) {
    // compute src address
    address_t src = proc.next_pc() + ii.imm();

    // jump
    return src;
  }
  else {
    return proc.next_pc();
  }
}
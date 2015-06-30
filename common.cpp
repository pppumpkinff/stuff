#include "common.hpp"

bool is_branch(op_code opcode) {
  return ((opcode == beq_op) ||
          (opcode == bne_op));
}
bool is_branch_or_jump(op_code opcode) {
  return ((opcode == jump_op) ||
          (opcode == beq_op)  ||
          (opcode == bne_op));
}
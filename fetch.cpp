#include "fetch.hpp"

void fetch(global_data * data) {
  int btb_entry;
  int pc = data->program_counter % MEM_SIZE;
  instruction new_ins;
  
  if (data->ctl_signal.halt_fetched) {
    // don't fetch instructions after having fetched halt.
    return;
  }
  
  if (data->instruction_queue.size() >= INS_QUEUE_LENGTH) {
    // if the queue is full, do nothing.
    return;
  }

  if (data->ctl_signal.halt_fetched) {
    // if halt was fetched previously, stop fetching new instructions.
    return;
  }
  
  new_ins = parse_instruction_hex(data->memory[pc], data);
  
  if (new_ins.opcode == halt_op) {
    data->ctl_signal.halt_fetched = true;
  }
  
  // setting the new program counter
  if (is_branch_or_jump(new_ins.opcode)) {
    if (data->ctl_signal.speculation_made) {
      // do not perform another speculation if we are already speculating
      return;
    }
    // if it's a branch / jump instruction,
    // speculate on the destination using the branch target buffer
    btb_entry = data->program_counter % BTB_SIZE;
    data->ctl_signal.address_speculated = data->branch_target_buffer[btb_entry];
    data->ctl_signal.address_if_no_jump = data->program_counter + 1 % MEM_SIZE;
    data->ctl_signal.speculation_made = true;
    data->program_counter = data->branch_target_buffer[btb_entry] % MEM_SIZE;
  } else {
    // not a branch, increase PC by one.
    ++data->program_counter;
  }

  // add to instruction queue
  data->instruction_queue.push(new_ins);
}

instruction parse_instruction_hex(unsigned int integer, global_data * data) {
	struct instruction result;
	result.opcode	           = (op_code) take_bits_from_integer(integer, 31, 28);
	result.dst		           = take_bits_from_integer(integer, 27, 24);
	result.src0		           = take_bits_from_integer(integer, 23, 20);
	result.src1		           = take_bits_from_integer(integer, 19, 16);
	result.imm		           = take_bits_from_integer(integer, 15, 0);
  result.unique_identifier = data->ctl_signal.unique_identifier;
  ++data->ctl_signal.unique_identifier;
	// if we are in speculative mode, this instruction is speculative.
	result.speculative = data->ctl_signal.speculation_made;
	result.validity    = false;
  data->trace[result.unique_identifier].instruction_hex = integer;
  data->trace[result.unique_identifier].cycle_issued = -1;
  data->trace[result.unique_identifier].cycle_execute_start = -1;
  data->trace[result.unique_identifier].cycle_write_cdb = -1;
  data->trace[result.unique_identifier].cycle_commit = -1;

	return result;
}
unsigned int take_bits_from_integer(unsigned int integer, unsigned int upper, unsigned int lower) {
	unsigned int removed_top = integer << (32 - 1 - upper); // remove top redundant bits
	unsigned int removed_top_and_bottom = removed_top >> (32 - 1 - upper + lower);
	return removed_top_and_bottom;
}
#include "commit.hpp"

void commit(global_data * data) {
  int address;
  int pc_after_branch;
  float content;
  reservation_station_identifier result_location_1;
  reservation_station_identifier result_location_2;
  reservation_station_int * location_1_int;
  reservation_station_int * location_2_int;
  reservation_station_float * location_fp;
  instruction ins;
  mem_buffer * mem_buf;

  if (!data->reorder_buffer.empty()) {
    ins = data->reorder_buffer.front();
  } else {
    return;
  }
  mem_buf = find_mem_buffer_using_identifier(data, ins.unique_identifier);

  if (data->ctl_signal.busy_committing_to_mem) {
    ++data->ctl_signal.cycles_since_mem_start;
    if (data->ctl_signal.cycles_since_mem_start >= data->cfg.mem_delay) {
      data->ctl_signal.cycles_since_mem_start = 0;
      data->ctl_signal.busy_committing_to_mem = false;
      if (ins.opcode == load_op) {
        address = mem_buf->mem_address;
        memcpy(&(mem_buf->data), &(data->memory[address]), sizeof(float));
        mem_buf->data_t.type = not_used;
        mem_buf->valid = true;
        // write cdb will write the result to the register
      }
      if (ins.opcode == store_op) {
        address = mem_buf->mem_address;
        content = mem_buf->data;
        memcpy(&(data->memory[address]), &content, sizeof(float));
        mem_buf->busy = false;
      }
      data->trace[ins.unique_identifier].cycle_commit = data->clock_cycle;
      data->reorder_buffer.pop();
    }
    // if we are busy with a mem instruction, do not commit anything else.
    return;
  }
  if (ins.opcode == halt_op) {
    data->ctl_signal.halt_committed = true;
    data->reorder_buffer.pop();
    data->trace[ins.unique_identifier].cycle_commit = data->clock_cycle;
  }
  if ((ins.opcode == store_op) || (ins.opcode == load_op)) {
    if ((mem_buf->mem_address_t.type == not_used) &&
        (mem_buf->data_t.type == not_used)) {
      data->ctl_signal.busy_committing_to_mem = true;
      data->ctl_signal.cycles_since_mem_start = 0;
    }
  }
  if ((ins.opcode == beq_op) || (ins.opcode == bne_op)) {
    if (ins.opcode == beq_op) {
      result_location_1 = find_reservation_station_using_identifier(data, ins.unique_identifier, eq);
    } else {
      result_location_1 = find_reservation_station_using_identifier(data, ins.unique_identifier, neq);
    }
    result_location_2 = find_reservation_station_using_identifier(data, ins.unique_identifier, add);
    if ((result_location_1.type != not_used) && (result_location_2.type != not_used)) {
      // both results exist, we can figure out if the branch has indeed happened.
      location_1_int = &(data->int_reservation_station[result_location_1.index]);
      location_2_int = &(data->int_reservation_station[result_location_2.index]);
      
      // free up reservation stations for future use
      location_1_int->in_use = false;
      location_2_int->in_use = false;
      
      // if the test condition is true
      if (location_1_int->result == 1) {
        if (location_2_int->result == data->ctl_signal.address_speculated) {
          // speculation was correct, do nothing
        } else {
          // branch was indeed taken, but the address was wrong.
          pc_after_branch = data->ctl_signal.address_if_no_jump;
          data->branch_target_buffer[pc_after_branch - 1] = location_2_int->result % MEM_SIZE; // update BTB
          data->program_counter = location_2_int->result;
          flush_speculative_instructions(data);
        }
      } else {
        // branch was not taken (we assume taken by default and use
        // BTB to calculate the destination)
        pc_after_branch = data->ctl_signal.address_if_no_jump;
        data->program_counter = pc_after_branch;
        flush_speculative_instructions(data);
      }
      data->ctl_signal.speculation_made = false;
      data->reorder_buffer.pop();
      data->trace[ins.unique_identifier].cycle_commit = data->clock_cycle;
    } else {
      return;
    }
  }
  if (ins.opcode == jump_op) {
    result_location_1 = find_reservation_station_using_identifier(data, ins.unique_identifier, add);
    // if the reservation station is ready
    if (result_location_1.type != not_used) {
      if (location_2_int->result == data->ctl_signal.address_speculated) {
        // speculation was correct, update BTB
        data->branch_target_buffer[pc_after_branch - 1] = location_1_int->result % MEM_SIZE;
      } else {
        pc_after_branch = data->ctl_signal.address_if_no_jump;
        data->program_counter = pc_after_branch;
        flush_speculative_instructions(data);
      }
      data->ctl_signal.speculation_made = false;
      data->reorder_buffer.pop();
      data->trace[ins.unique_identifier].cycle_commit = data->clock_cycle;
    }
  }

  if (ins.opcode == mult_s_op) {
    result_location_1 = find_reservation_station_using_identifier(data, ins.unique_identifier, mul);
    if (result_location_1.type != not_used) {
      location_fp = &(data->fp_mul_reservation_station[result_location_1.index]);
      data->fp_register[ins.dst] = location_fp->result;
      data->reorder_buffer.pop();
      data->trace[ins.unique_identifier].cycle_commit = data->clock_cycle;
    }
  }
  if ((ins.opcode == add_s_op) ||
      (ins.opcode == add_op) ||
      (ins.opcode == addi_op)) {
    result_location_1 = find_reservation_station_using_identifier(data, ins.unique_identifier, add);
    if (result_location_1.type != not_used) {
      if (ins.opcode == add_s_op) {
        location_fp = &(data->fp_add_reservation_station[result_location_1.index]);
        data->fp_register[ins.dst] = location_fp->result;
      } else {
        location_1_int = &(data->int_reservation_station[result_location_1.index]);
        data->int_register[ins.dst] = location_1_int->result;
      }
      data->reorder_buffer.pop();
      data->trace[ins.unique_identifier].cycle_commit = data->clock_cycle;
    }
  }
  if ((ins.opcode == sub_s_op) ||
      (ins.opcode == sub_op)   ||
      (ins.opcode == subi_op)) {
    result_location_1 = find_reservation_station_using_identifier(data, ins.unique_identifier, sub);
    if (result_location_1.type != not_used) {
      if (ins.opcode == sub_s_op) {
        location_fp = &(data->fp_add_reservation_station[result_location_1.index]);
        data->fp_register[ins.dst] = location_fp->result;
      } else {
        location_1_int = &(data->int_reservation_station[result_location_1.index]);
        data->int_register[ins.dst] = location_1_int->result;
      }
      data->reorder_buffer.pop();
      data->trace[ins.unique_identifier].cycle_commit = data->clock_cycle;
    }
  }
}

void flush_speculative_instructions(global_data * data) {
  unsigned int i;
  reservation_station_float * iterator_fp;
  reservation_station_int * iterator_int;
  mem_buffer * iterator_mem;
  for (i = 0; i < data->cfg.add_nr_reservation; ++i) {
    iterator_fp = &(data->fp_add_reservation_station[i]);
    iterator_fp->in_use = false;
  }
  for (i = 0; i < data->cfg.mul_nr_reservation; ++i) {
    iterator_fp = &(data->fp_mul_reservation_station[i]);
    iterator_fp->in_use = false;
  }
  for (i = 0; i < data->cfg.int_nr_reservation; ++i) {
    iterator_int = &(data->int_reservation_station[i]);
    iterator_int->in_use = false;
  }
  for (i = 0; i < data->cfg.mem_nr_load_buffers; ++i) {
    iterator_mem = &(data->load_buffer[i]);
    iterator_mem->busy = false;
  }
  for (i = 0; i < data->cfg.mem_nr_store_buffers; ++i) {
    iterator_mem = &(data->store_buffer[i]);
    iterator_mem->busy = false;
  }
  popAll(data->instruction_queue);
  popAll(data->reorder_buffer);
}

reservation_station_identifier find_reservation_station_using_identifier(global_data * data, int uid, operation oper) {
  unsigned int i;
  int j = 0;
  reservation_station_identifier result;
  reservation_station_float * iterator_fp;
  reservation_station_int * iterator_int;
  for (i = 0; i < data->cfg.add_nr_reservation; ++i) {
    iterator_fp = &(data->fp_add_reservation_station[i]);
    if ((iterator_fp->unique_identifier == uid) && 
        (iterator_fp->in_use)                   && 
        (iterator_fp->op == oper)               &&
        (iterator_fp->valid)) {
      iterator_fp->in_use = false;
      result.type = fp_add_sub;
      result.index = i;
      return result;
    }
  }
  for (i = 0; i < data->cfg.mul_nr_reservation; ++i) {
    iterator_fp = &(data->fp_mul_reservation_station[i]);
    if ((iterator_fp->unique_identifier == uid) && 
        (iterator_fp->in_use)                   && 
        (iterator_fp->op == oper)               &&
        (iterator_fp->valid)) {
      iterator_fp->in_use = false;
      result.type = fp_mul;
      result.index = i;
      return result;
    }
  }
  for (i = 0; i < data->cfg.int_nr_reservation; ++i) {
    iterator_int = &(data->int_reservation_station[i]);
    if ((iterator_int->unique_identifier == uid) && 
        (iterator_int->in_use)                   && 
        (iterator_int->op == oper)               &&
        (iterator_int->valid)) {
      if (!iterator_int->do_not_write_to_cdb) {
        // if it's a branch or jump instruction, do not
        // mark the station as available - we need both stations
        // to be valid for it to work.
        iterator_int->in_use = false;
      }
      result.type = int_alu;
      result.index = i;
      return result;
    }
  }
  // if we didn't find one
  result.type = not_used;
  return result;
}

mem_buffer * find_mem_buffer_using_identifier(global_data * data, int uid) {
  unsigned int i;
  mem_buffer * iterator;
  for (i = 0; i < data->cfg.mem_nr_load_buffers; ++i) {
    iterator = &(data->load_buffer[i]);
    if (iterator->unique_identifier == uid) {
      return iterator;
    }
  }
  for (i = 0; i < data->cfg.mem_nr_store_buffers; ++i) {
    iterator = &(data->store_buffer[i]);
    if (iterator->unique_identifier == uid) {
      return iterator;
    }
  }
  return NULL;
}

template <class T> void popAll (std::queue<T> q) { 
  while (!q.empty()) {
    q.pop();
  }
}
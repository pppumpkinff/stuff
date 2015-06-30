#include "issue.hpp"

void issue(global_data * data) {
  operation oper;
  reservation_station_identifier identifier;
  reservation_station_identifier buffer_identifier;
  instruction ins;

  if (!data->instruction_queue.empty()) {
    ins = data->instruction_queue.front();
  } else {
    return;
  }
  oper = opcode_to_operation(ins.opcode);

  if (ins.opcode == halt_op) {
    data->reorder_buffer.push(ins);
    return;
  }

  if (rob_full(data) || no_room_on_buffers(data, ins)) {
    return;
  }

  identifier = find_and_use_empty_reservation_station(data, ins);
  if (identifier.type == not_used) {
    // no empty spot on the reservation station
    return;
  }

  if (ins.opcode == jump_op) {
    set_operation_int(data, identifier, ins, add);
    set_src0_pc(data, identifier, ins);
    set_imm_int(data, identifier, ins);
  }

  // branch instructions require address and condition calculation
  // so we must issue them to two reservation stations.
  if (is_branch(ins.opcode)) {
    // if we already issued the target address calculation
    if (data->ctl_signal.issued_branch_prev_cycle) {
      set_operation_int(data, identifier, ins, oper);
      set_src0_int(data, identifier, ins);
      set_src1_int(data, identifier, ins);
      // reset control signals
      data->ctl_signal.issued_branch_prev_cycle = false;
      data->instruction_queue.pop();
      data->reorder_buffer.push(ins);
      return;
    } else {
      // if we haven't issued the target address calculation
      set_operation_int(data, identifier, ins, add);
      data->trace[ins.unique_identifier].cycle_issued = data->clock_cycle;
      data->ctl_signal.issued_branch_prev_cycle = true;
      set_src0_pc(data, identifier, ins);
      set_imm_int(data, identifier, ins);
      // do not remove the instruction from the instruction queue.
      // we will issue the address calculation next cycle.
      return;
    }
  }

  if ((identifier.type == fp_add_sub) 
   || (identifier.type == fp_mul)) {
    set_operation_fp(data, identifier, ins, oper);
    set_src0_fp(data, identifier, ins);
    set_src1_fp(data, identifier, ins);
    reserve_dst(data, identifier, ins);
  }
  if (is_i_type_alu(ins)) {
    set_operation_int(data, identifier, ins, oper);
    set_src0_int(data, identifier, ins);
    set_imm_int(data, identifier, ins);
    reserve_dst(data, identifier, ins);
  }
  if (is_r_type_alu(ins)) {
    set_operation_int(data, identifier, ins, oper);
    set_src0_int(data, identifier, ins);
    set_src1_int(data, identifier, ins);
    reserve_dst(data, identifier, ins);
  }

  buffer_identifier = find_and_use_empty_buffer(data, ins);
  if (buffer_identifier.type == store_buffer) {
    set_src0_int(data, identifier, ins);
    set_imm_int(data, identifier, ins);
    data->store_buffer[buffer_identifier.index].mem_address_t = identifier;
    // current reservation station results in the mem address
    if (data->temp_fp_register[ins.dst].busy) {
      data->store_buffer[buffer_identifier.index].data_t = data->temp_fp_register[ins.src1].result_from_station;
    } else {
      data->store_buffer[buffer_identifier.index].data = data->temp_fp_register[ins.src1].value;
      data->store_buffer[buffer_identifier.index].data_t.type = not_used;
    }
  }
  if (buffer_identifier.type == load_buffer) {
    set_src0_int(data, identifier, ins);
    set_imm_int(data, identifier, ins);
    // current reservation station results in the mem address
    data->load_buffer[buffer_identifier.index].mem_address_t = identifier;
    data->temp_fp_register[ins.dst].result_from_station = buffer_identifier;
  }
  data->reorder_buffer.push(ins);
  data->instruction_queue.pop();
  data->trace[ins.unique_identifier].cycle_issued = data->clock_cycle;
}

operation opcode_to_operation(op_code opcode) {
  switch (opcode) {
  case load_op:
    return add;
  case store_op:
    return add;
  case jump_op:
    return add;
  case beq_op:
    return eq;
  case bne_op:
    return neq;
  case add_op:
    return add;
  case addi_op:
    return add;
  case sub_op:
    return sub;
  case subi_op:
    return sub;
  case add_s_op:
    return add;
  case sub_s_op:
    return sub;
  case mult_s_op:
    return mul;
  default:
    return add;
  }
}

void set_operation_fp(global_data * data, reservation_station_identifier id, instruction ins, operation oper) {
  reservation_station_float * fp_station;
  if (id.type == fp_mul) {
    fp_station = &(data->fp_mul_reservation_station[id.index]); 
  } else {
    fp_station = &(data->fp_add_reservation_station[id.index]); 
  }
  fp_station->op = oper;
}

void set_operation_int(global_data * data, reservation_station_identifier id, instruction ins, operation oper) {
  reservation_station_int * int_station = &(data->int_reservation_station[id.index]);
  int_station->op = oper;
  if (is_branch_or_jump(ins.opcode)) {
    int_station->do_not_write_to_cdb = true;
  } else {
    int_station->do_not_write_to_cdb = false;
  }
}

void set_src0_pc(global_data * data, reservation_station_identifier id, instruction ins) {
  reservation_station_int * int_station = &(data->int_reservation_station[id.index]);
  
  // no register renaming needed
  int_station->q_i.type = not_used;
  int_station->v_i = data->program_counter;

}
void set_src0_int(global_data * data, reservation_station_identifier id, instruction ins) {
  register_rename_station_int * prev = &(data->temp_int_register[ins.src0]);
  reservation_station_int * int_station = &(data->int_reservation_station[id.index]);
  if (prev->busy) {
    // register is in use
    int_station->q_i = prev->result_from_station;
  } else {
    // no register renaming needed
    int_station->q_i.type = not_used;
    int_station->v_i = data->temp_int_register[ins.src0].value;
  }
}
void set_src1_int(global_data * data, reservation_station_identifier id, instruction ins) {
  register_rename_station_int * prev = &(data->temp_int_register[ins.src1]);
  reservation_station_int * int_station = &(data->int_reservation_station[id.index]);
  if (prev->busy) {
    // register is in use
    int_station->q_j = prev->result_from_station;
  } else {
    // no register renaming needed
    int_station->q_j.type = not_used;
    int_station->v_j = data->temp_int_register[ins.src1].value;
  }
}
void set_imm_int(global_data * data, reservation_station_identifier id, instruction ins) {
  data->int_reservation_station[id.index].v_j = ins.imm;
  data->int_reservation_station[id.index].q_j.type = not_used;
}
void reserve_dst(global_data * data, reservation_station_identifier id, instruction ins) {
  if ((id.type == fp_add_sub) ||
      (id.type == fp_mul))       {
    // if it's a FP instruction
    // the destination is an FP register
    data->temp_fp_register[ins.dst].busy = true;
    data->temp_fp_register[ins.dst].valid = false;
    data->temp_fp_register[ins.dst].speculative = data->ctl_signal.speculation_made;
    data->temp_fp_register[ins.dst].result_from_station = id;
  } else {
    data->temp_int_register[ins.dst].busy = true;
    data->temp_int_register[ins.dst].valid = false;
    data->temp_int_register[ins.dst].speculative = data->ctl_signal.speculation_made;
    data->temp_int_register[ins.dst].result_from_station = id;
  }
}
void set_src0_fp(global_data * data, reservation_station_identifier id, instruction ins) {
  register_rename_station_float * prev = &(data->temp_fp_register[ins.src0]);
  reservation_station_float * fp_station;
  if (id.type == fp_mul) {
    fp_station = &(data->fp_mul_reservation_station[id.index]); 
  } else {
    fp_station = &(data->fp_add_reservation_station[id.index]); 
  }
  if (prev->busy) {
    // register is in use
    fp_station->q_i = prev->result_from_station;
  } else {
    // no register renaming needed
    fp_station->q_i.type = not_used;
    fp_station->v_i = data->temp_fp_register[ins.src0].value;
  }
}
void set_src1_fp(global_data * data, reservation_station_identifier id, instruction ins) {
  register_rename_station_float * prev = &(data->temp_fp_register[ins.src1]);
  reservation_station_float * fp_station;
  if (id.type == fp_mul) {
    fp_station = &(data->fp_mul_reservation_station[id.index]); 
  } else {
    fp_station = &(data->fp_add_reservation_station[id.index]); 
  }
  if (prev->busy) {
    // register is in use
    fp_station->q_j = prev->result_from_station;
  } else {
    // no register renaming needed
    fp_station->q_j.type = not_used;
    fp_station->v_j = data->temp_fp_register[ins.src1].value;
  }
}

bool no_room_on_buffers(global_data * data, instruction ins) {
  unsigned int i;
  if (ins.opcode == load_op) {
    for (i = 0; i < data->cfg.mem_nr_load_buffers; ++i) {
      if (!data->load_buffer[i].busy) {
        return false;
      }
    }
  } else if (ins.opcode == store_op) {
    for (i = 0; i < data->cfg.mem_nr_store_buffers; ++i) {
      if (!data->store_buffer[i].busy) {
        return false;
      }
    }
  } else {
    return false;
  }
  return true;
}
bool rob_full(global_data * data) {
  return (data->reorder_buffer.size() >= data->cfg.rob_entries);
}
reservation_station_identifier find_and_use_empty_reservation_station(global_data * data, instruction ins) {
  unsigned int i;
  reservation_station_identifier result;
  result.type = not_used; // if it is not set elsewhere, this indicates we failed to find a spot.
  if ((ins.opcode == halt_op) || (ins.opcode == jump_op)) {
    // for a halt or jump, we don't need a reservation station.
    return result;
  }
  if (ins.opcode == mult_s_op) {
    for (i = 0; i < data->cfg.mul_nr_reservation; ++i) {
      if (!data->fp_mul_reservation_station[i].in_use) {
        data->fp_mul_reservation_station[i].unique_identifier = ins.unique_identifier;
        result.index = i;
        result.type = fp_mul;
        data->fp_mul_reservation_station[i].in_use = true;
        data->fp_mul_reservation_station[i].valid = false;
        break;
      }
    }
  } else if ((ins.opcode == add_s_op) || (ins.opcode == sub_s_op)) {
    for (i = 0; i < data->cfg.add_nr_reservation ; ++i) {
      if (!data->fp_add_reservation_station[i].in_use) {
        data->fp_add_reservation_station[i].unique_identifier = ins.unique_identifier;
        result.index = i;
        result.type = fp_add_sub;
        data->fp_add_reservation_station[i].in_use = true;
        data->fp_add_reservation_station[i].valid = false;
        break;
      }
    }
  } else if (ins.opcode != halt_op) {
    for (i = 0; i < data->cfg.int_nr_reservation; ++i) {
      if (!data->int_reservation_station[i].in_use) {
        data->int_reservation_station[i].unique_identifier = ins.unique_identifier;
        result.index = i;
        result.type = int_alu;
        data->int_reservation_station[i].in_use = true;
        data->int_reservation_station[i].valid = false;
        data->int_reservation_station[i].in_execute = false;
        break;
      }
    }
  } else {
    // not an instruction that requires a reservation station
    // jump or halt.
    return result;
  }
  // failed to find an empty spot
  return result;
}
bool is_r_type_alu(instruction ins) {
  return ((ins.opcode == add_op) ||
          (ins.opcode == sub_op));
}
bool is_i_type_alu(instruction ins) {
  return ((ins.opcode == addi_op) || 
          (ins.opcode == subi_op));
}
reservation_station_identifier find_and_use_empty_buffer(global_data * data, instruction ins) {
  reservation_station_identifier result;
  unsigned int i;
  switch (ins.opcode) {
    case load_op:
      for (i = 0; i < data->cfg.mem_nr_load_buffers; ++i) {
        if (!data->load_buffer[i].busy) {
          result.type = load_buffer;
          result.index = i;
          data->load_buffer[i].busy = true;
          data->load_buffer[i].valid = false;
          data->load_buffer[i].unique_identifier = ins.unique_identifier;
          return result;
        }
      }
      break;
    case store_op:
      for (i = 0; i < data->cfg.mem_nr_load_buffers; ++i) {
        if (!data->store_buffer[i].busy) {
          result.type = store_buffer;
          result.index = i;
          data->store_buffer[i].busy = true;
          data->store_buffer[i].valid = false;
          data->store_buffer[i].unique_identifier = ins.unique_identifier;
          return result;
        }
      }
      break;
  }
  // if we didn't find an empty spot or it's not a load/store instruction
  result.type = not_used;
  return result;
}

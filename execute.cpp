#include "execute.hpp"

void execute(global_data * data) {
  execute_int_alu(data);
  execute_fp_add_sub(data);
  execute_fp_mul(data);
}
void execute_int_alu(global_data * data) {
  int unique_identifier;
  reservation_station_int * int_station;
  unsigned int i;
  unsigned int time_since_execute_start;
  for (i = 0; i < data->cfg.int_nr_reservation; ++i) {
    int_station = &(data->int_reservation_station[i]);
    unique_identifier = int_station->unique_identifier;

    // if both parameters are ready and we're not yet executing
    if ((int_station->q_i.type == not_used)     &&    // 1st parameter available
        (int_station->q_j.type == not_used)     &&    // 2nd parameter available
        (!int_station->in_execute)              &&    // didn't already start executing
        (int_station->in_use)                   &&    // valid instruction
        (!data->ctl_signal.started_execution_instruction_this_clock_cycle)  &&    // int alu not in use already
        (!int_station->valid)) {                      // we haven't completed executing before
      data->ctl_signal.started_execution_instruction_this_clock_cycle = true; // to avoid issuing more than one instruction per cycle
      data->trace[unique_identifier].cycle_execute_start = data->clock_cycle;
      int_station->in_execute = true;
      int_station->written_to_cdb = false;
    }

    // if we already started executing the instruction before and we're not done
    if ((int_station->in_execute) && (!int_station->valid)) {
      time_since_execute_start = data->clock_cycle - data->trace[unique_identifier].cycle_execute_start;
      // if it has been long enough since we started executing
      // the instruction that we can find the result
      if (time_since_execute_start >= data->cfg.int_delay) {
        data->ctl_signal.started_execution_instruction_this_clock_cycle = false;
        int_station->valid = true;
        int_station->in_execute = false;
        switch (int_station->op) {
        case add:
          int_station->result = int_station->v_i + int_station->v_j;
          break;
        case sub:
          int_station->result = int_station->v_i - int_station->v_j;
          break;
        case eq:
          int_station->result = int_station->v_i == int_station->v_j;
          break;
        case neq:
          int_station->result = int_station->v_i != int_station->v_j;
          break;
        }
      }
    }
  }
  data->ctl_signal.started_execution_instruction_this_clock_cycle = false;
}

void execute_fp_add_sub(global_data * data) {
  int unique_identifier;
  reservation_station_float * fp_station;
  unsigned int i;
  unsigned int time_since_execute_start;
  for (i = 0; i < data->cfg.add_nr_reservation; ++i) {
    fp_station = &(data->fp_add_reservation_station[i]);
    unique_identifier = fp_station->unique_identifier;

    // if both parameters are ready and we're not yet executing
    if ((fp_station->q_i.type == not_used)    &&    // 1st parameter available
        (fp_station->q_j.type == not_used)    &&    // 2nd parameter available
        (!fp_station->in_execute)             &&    // didn't already start executing
        (fp_station->in_use)                  &&    // valid instruction
        (!data->ctl_signal.fp_add_in_execute) &&    // fp add not in use already
        (!fp_station->valid)) {                     // we haven't completed executing before
      data->ctl_signal.fp_add_in_execute = true;
      data->trace[unique_identifier].cycle_execute_start = data->clock_cycle;
      fp_station->in_execute = true;
      fp_station->written_to_cdb = false;
    }

    // if we already started executing before
    if (fp_station->in_execute) {
      time_since_execute_start = data->clock_cycle - data->trace[unique_identifier].cycle_execute_start;
      // if it has been long enough since we started executing
      // the instruction that we can find the result
      if (time_since_execute_start >= data->cfg.add_delay) {
        data->ctl_signal.fp_add_in_execute = false;
        fp_station->valid = true;
        fp_station->in_execute = false;
        switch (fp_station->op) {
        case add:
          fp_station->result = fp_station->v_i + fp_station->v_j;
          break;
        case sub:
          fp_station->result = fp_station->v_i - fp_station->v_j;
          break;
        }
      }
    }
  }
}

void execute_fp_mul(global_data * data) {
  int unique_identifier;
  reservation_station_float * fp_station;
  unsigned int i;
  unsigned int time_since_execute_start;
  for (i = 0; i < data->cfg.mul_nr_reservation; ++i) {
    fp_station = &(data->fp_mul_reservation_station[i]);
    unique_identifier = fp_station->unique_identifier;

    // if both parameters are ready and we're not yet executing
    if ((fp_station->q_i.type == not_used)    &&    // 1st parameter available
        (fp_station->q_j.type == not_used)    &&    // 2nd parameter available
        (!fp_station->in_execute)             &&    // didn't already start executing
        (fp_station->in_use)                  &&    // valid instruction
        (!data->ctl_signal.fp_mul_in_execute) &&    // fp mul not in use already
        (!fp_station->valid)) {                     // we haven't completed executing before
      data->ctl_signal.fp_mul_in_execute = true;
      data->trace[unique_identifier].cycle_execute_start = data->clock_cycle;
      fp_station->in_execute = true;
      fp_station->written_to_cdb = false;
    }

    // if we already started executing before
    if (fp_station->in_execute) {
      time_since_execute_start = data->clock_cycle - data->trace[unique_identifier].cycle_execute_start;
      // if it has been long enough since we started executing
      // the instruction that we can find the result
      if (time_since_execute_start >= data->cfg.mul_delay) {
        data->ctl_signal.fp_mul_in_execute = false;
        fp_station->valid = true;
        fp_station->in_execute = false;
        fp_station->result = fp_station->v_i * fp_station->v_j;
      }
    }
  }
}
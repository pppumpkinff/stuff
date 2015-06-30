#include "write_cdb.hpp"

void write_cdb(global_data * data) {
  unsigned int i;
  int unique_identifier;
  reservation_station_float * fp_station;
  reservation_station_int * int_station;
  mem_buffer * load_station;
  reservation_station_identifier result_location;

  result_location.type = not_used;

  for (i = 0; i < data->cfg.int_nr_reservation; ++i) {
    int_station = &(data->int_reservation_station[i]);
    if ((int_station->valid) &&                // is the result valid
        (int_station->in_use) &&               // is this reservation station in use?
        (!int_station->written_to_cdb) &&      // we didn't write the result before
        (!int_station->do_not_write_to_cdb)) { // if this isn't a branch or jump reservation station
      int_station->written_to_cdb = true;
      unique_identifier = int_station->unique_identifier;
      data->trace[unique_identifier].cycle_write_cdb = data->clock_cycle;
      result_location.type = int_alu;
      result_location.index = i;
      find_station_needing_result(data, result_location);
      stop_renaming_result(data, result_location);
    }
  }
  for (i = 0; i < data->cfg.add_nr_reservation; ++i) {
    fp_station = &(data->fp_add_reservation_station[i]);
    if ((fp_station->valid) &&            // is the result valid
        (fp_station->in_use) &&           // is this reservation station in use?
        (!fp_station->written_to_cdb)) {  // we didn't write the result before
      fp_station->written_to_cdb = true;
      unique_identifier = fp_station->unique_identifier;
      data->trace[unique_identifier].cycle_write_cdb = data->clock_cycle;
      result_location.type = fp_add_sub;
      result_location.index = i;
      find_station_needing_result(data, result_location);
      stop_renaming_result(data, result_location);
    }
  }
  for (i = 0; i < data->cfg.mul_nr_reservation; ++i) {
    fp_station = &(data->fp_mul_reservation_station[i]);
    if ((fp_station->valid) &&            // is the result valid
        (fp_station->in_use) &&           // is this reservation station in use?
        (!fp_station->written_to_cdb)) {  // we didn't write the result before
      fp_station->written_to_cdb = true;
      unique_identifier = fp_station->unique_identifier;
      data->trace[unique_identifier].cycle_write_cdb = data->clock_cycle;
      result_location.type = fp_mul;
      result_location.index = i;
      find_station_needing_result(data, result_location);
      stop_renaming_result(data, result_location);
    }
  }
  for (i = 0; i < data->cfg.mem_nr_load_buffers; ++i) {
    load_station = &(data->load_buffer[i]);
    if ((load_station->busy) && (load_station->valid)) {
      result_location.type = load_buffer;
      result_location.index = i;
      find_station_needing_result(data, result_location);
      stop_renaming_result(data, result_location);
    }
  }
}

void find_station_needing_result(global_data * data, reservation_station_identifier result) {
  unsigned int i;
  int int_result;
  float fp_result;
  bool is_float;
  reservation_station_float * fp_station;
  reservation_station_int * int_station;

  switch (result.type) {
    case fp_add_sub:
      is_float = true;
      fp_result = data->fp_add_reservation_station[result.index].result;
      break;
    case fp_mul:
      is_float = true;
      fp_result = data->fp_mul_reservation_station[result.index].result;
      break;
    case int_alu:
      is_float = false;
      int_result = data->int_reservation_station[result.index].result;
      break;
    case load_buffer:
      is_float = true;
      fp_result = data->load_buffer[result.index].data;
      break;
    default:
      return;
  }
  if (is_float) {
    // check if an fp_add_sub instruction is waiting for the result
    for (i = 0; i < data->cfg.add_nr_reservation; ++i) {
      fp_station = &(data->fp_add_reservation_station[i]);
      if (compare_reservation_station_identifiers(fp_station->q_i, result)) {
        fp_station->v_i = fp_result;
        fp_station->q_i.type = not_used;
      }
      if (compare_reservation_station_identifiers(fp_station->q_j, result)) {
        fp_station->v_j = fp_result;
        fp_station->q_j.type = not_used;
      }
    }
    // check if an fp_mul instruction is waiting for the result
    for (i = 0; i < data->cfg.mul_nr_reservation; ++i) {
      fp_station = &(data->fp_mul_reservation_station[i]);
      if (compare_reservation_station_identifiers(fp_station->q_i, result)) {
        fp_station->v_i = fp_result;
        fp_station->q_i.type = not_used;
      }
      if (compare_reservation_station_identifiers(fp_station->q_j, result)) {
        fp_station->v_j = fp_result;
        fp_station->q_j.type = not_used;
      }
    }
    // check if a store instruction is waiting for the result
    for (i = 0; i < data->cfg.mem_nr_store_buffers; ++i) {
      if (compare_reservation_station_identifiers(data->store_buffer[i].data_t, result)) {
        data->store_buffer[i].data = fp_result;
        data->store_buffer[i].data_t.type = not_used;
      }

    }
  } else {
    // check if an int instruction is waiting for the result
    for (i = 0; i < data->cfg.int_nr_reservation; ++i) {
      int_station = &(data->int_reservation_station[i]);
      if (compare_reservation_station_identifiers(int_station->q_i, result)) {
        int_station->v_i = int_result;
        int_station->q_i.type = not_used;
      }
      if (compare_reservation_station_identifiers(int_station->q_j, result)) {
        int_station->v_j = int_result;
        int_station->q_j.type = not_used;
      }
    }
    // check if a load instruction is waiting for the result (for memory address calculation)
    for (i = 0; i < data->cfg.mem_nr_load_buffers; ++i) {
      if (compare_reservation_station_identifiers(data->load_buffer[i].mem_address_t, result)) {
        data->load_buffer[i].mem_address = int_result;
        data->load_buffer[i].mem_address_t.type = not_used;
      }
    }
    // check if a store instruction is waiting for the result (for memory address calculation)
    for (i = 0; i < data->cfg.mem_nr_store_buffers; ++i) {
      if (compare_reservation_station_identifiers(data->store_buffer[i].mem_address_t, result)) {
        data->store_buffer[i].mem_address = int_result;
        data->store_buffer[i].mem_address_t.type = not_used;
      }
    }
  }
}

bool compare_reservation_station_identifiers(reservation_station_identifier a, reservation_station_identifier b) {
  return ((a.type == b.type) && (a.index == b.index));
}

void stop_renaming_result(global_data * data, reservation_station_identifier result) {
  register_rename_station_int * rename_station_int;
  register_rename_station_float * rename_station_fp;
  int i;
  int int_val;
  float fp_val;
  switch (result.type) {
    case fp_add_sub:
      fp_val = data->fp_add_reservation_station[result.index].result;
      break;
    case fp_mul:
      fp_val = data->fp_mul_reservation_station[result.index].result;
      break;
    case int_alu:
      int_val = data->int_reservation_station[result.index].result;
      break;
  }

  for (i = 0; i < REGISTER_SIZE; ++i) {
    rename_station_int = &(data->temp_int_register[i]);
    if (compare_reservation_station_identifiers(rename_station_int->result_from_station, result)) {
      data->temp_int_register[i].busy = false;
      data->temp_int_register[i].value = int_val;
    }
    
    rename_station_fp = &(data->temp_fp_register[i]);
    if (compare_reservation_station_identifiers(rename_station_fp->result_from_station, result)) {
      data->temp_fp_register[i].busy = false;
      data->temp_fp_register[i].value = fp_val;
    }
  }
}
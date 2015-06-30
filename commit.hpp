#include "common.hpp"

enum res_type {
  int_res, fp_res
};

struct reservation_station_identifiers {
  reservation_station_identifier identifier[5]; // at most we can have two identifiers per instruction.
                                                // for example in the case of a branch instruction, we'll
                                                // have one for eq and one for address calculation
  int number_of_entries;
  bool all_ready_to_commit;
  res_type result_type[5];
  int results_int[5];
  float results_fp[5];
};

void commit(global_data * data);
mem_buffer * find_mem_buffer_using_identifier(global_data * data, int uid);

template <class T> void popAll (std::queue<T> q);
void flush_speculative_instructions(global_data * data);
reservation_station_identifier find_reservation_station_using_identifier(global_data * data, int uid, operation oper);

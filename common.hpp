#ifndef _COMMON_HPP
#define _COMMON_HPP

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <queue>
#include <map>

#define INS_QUEUE_LENGTH 16
#define MEM_SIZE 1024
#define MAX_MEM_STRING_LENGTH 50 // to make room for whitespace
#define BASE 16
#define REGISTER_SIZE 16
#define BTB_SIZE 16
#define LINE_LENGTH 64
#define NUM_OF_PARAM 10

enum operation {
  add, eq, neq, sub, mul
};
enum reservation_station_type {
  fp_add_sub, fp_mul, int_alu, load_buffer, store_buffer, not_used
};
struct reservation_station_identifier {
  reservation_station_type type;
  int index;
};
struct control_signals {
  bool halt_fetched;
  bool halt_committed;
  bool speculation_made;
  int address_speculated;
  int address_if_no_jump;
  bool started_execution_instruction_this_clock_cycle;
  bool fp_mul_in_execute;
  bool fp_add_in_execute;
  bool issued_branch_prev_cycle;
  unsigned int unique_identifier;
  bool busy_committing_to_mem;
  unsigned int cycles_since_mem_start;
};
struct config_data {
  unsigned int int_delay;
  unsigned int add_delay;
  unsigned int mul_delay;
  unsigned int mem_delay;
  unsigned int rob_entries;
  unsigned int add_nr_reservation;
  unsigned int mul_nr_reservation;
  unsigned int int_nr_reservation;
  unsigned int mem_nr_load_buffers;
  unsigned int mem_nr_store_buffers;
};
struct mem_buffer {
  int unique_identifier;
  bool valid;
  bool busy;
  int mem_address;
  reservation_station_identifier mem_address_t;
  float data;                              
  reservation_station_identifier data_t;
};
enum op_code {
  load_op = 0,
  store_op = 1,
  jump_op = 2,
  beq_op = 3,
  bne_op = 4,
  add_op = 5,
  addi_op = 6,
  sub_op = 7,
  subi_op = 8,
  add_s_op = 9,
  sub_s_op = 10,
  mult_s_op = 11,
  halt_op = 12
};
struct instruction {
  int unique_identifier;
  op_code opcode;
  int dst;
  int src0;
  int src1;
  int imm;
  bool validity;
  bool speculative;
};
struct reservation_station_int {
  int unique_identifier;              // unique identifier for this instruction
  operation op;                       // which operation to run (i.e. add, eq...)
  int result;                         // the outcome of the calculation
  int v_i;                            // first parameter
  int v_j;                            // second parameter
  reservation_station_identifier q_i; // which reservation station will yield v_i
  reservation_station_identifier q_j; // which reservation station will yield v_j
  bool valid;                         // is the result valid
  bool in_use;                        // are we using this reservation station slot?
  bool in_execute;                    // have we started executing the instruction in this reservation station?
  bool written_to_cdb;                // whether we have written to cdb yet (in order to avoid doing that more than once)
  bool do_not_write_to_cdb;           // make write_cdb not delete this reservation station (for branch and jump instructions)
};
struct reservation_station_float {
  int unique_identifier;              // unique identifier for this instruction
  operation op;                       // which operation to run (i.e. add, eq...)
  float result;                       // the outcome of the calculation
  float v_i;                          // first parameter
  float v_j;                          // second parameter
  reservation_station_identifier q_i; // which reservation station will yield v_i
  reservation_station_identifier q_j; // which reservation station will yield v_j
  bool valid;                         // is the result valid
  bool in_use;                        // are we using this reservation station slot?
  bool in_execute;                    // have we started executing the instruction in this reservation station?
  bool written_to_cdb;                // whether we have written to cdb yet (in order to avoid doing that more than once)
};
struct register_rename_station_float {
  float value;
  reservation_station_identifier result_from_station;
  bool valid;
  bool busy;
  bool speculative;
};
struct register_rename_station_int {
  int value;
  reservation_station_identifier result_from_station;
  bool valid;
  bool busy;
  bool speculative;
};
struct trace_data {
  int instruction_hex;
  int cycle_issued;
  int cycle_execute_start;
  int cycle_write_cdb;
  int cycle_commit;
};
struct global_data {
  control_signals ctl_signal;
  int memory[MEM_SIZE];
  int int_register[REGISTER_SIZE];
  float fp_register[REGISTER_SIZE];
  register_rename_station_int temp_int_register[REGISTER_SIZE];
  register_rename_station_float temp_fp_register[REGISTER_SIZE];
  int program_counter;
  int clock_cycle;
  config_data cfg;
  int branch_target_buffer[BTB_SIZE];
  reservation_station_float * fp_add_reservation_station;
  reservation_station_float * fp_mul_reservation_station;
  reservation_station_int * int_reservation_station;
  std::queue <instruction> instruction_queue;
  std::queue <instruction> reorder_buffer;
  std::map <int, trace_data> trace;
  mem_buffer * load_buffer;
  mem_buffer * store_buffer;
};

bool is_branch(op_code opcode);
bool is_branch_or_jump(op_code opcode);

#endif
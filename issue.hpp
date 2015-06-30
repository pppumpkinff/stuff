#include "common.hpp"

void issue(global_data * data);
operation opcode_to_operation(op_code opcode);
// set the operation on the reservation station (int reservation station)
void set_operation_int(global_data * data, reservation_station_identifier id, instruction ins, operation oper);
// set the operation on the reservation station (fp reservation station)
void set_operation_fp(global_data * data, reservation_station_identifier id, instruction ins, operation oper);
// set the first parameter to be the program counter (int reservation station)
void set_src0_pc(global_data * data, reservation_station_identifier id, instruction ins);
// set the first parameter to be src0 of the instruction on the reservation station (int reservation station)
// does register renaming
void set_src0_int(global_data * data, reservation_station_identifier id, instruction ins);
// set the first parameter to be src1 of the instruction on the reservation station (int reservation station)
// does register renaming
void set_src1_int(global_data * data, reservation_station_identifier id, instruction ins);
// set the first parameter to be imm of the instruction on the reservation station (int reservation station)
void set_imm_int(global_data * data, reservation_station_identifier id, instruction ins);
// adds the destination to the register renaming table (temp_int_register, temp_fp_register)
void reserve_dst(global_data * data, reservation_station_identifier id, instruction ins);
// set the first parameter to be src0 of the instruction on the reservation station (fp reservation station)
// does register renaming
void set_src0_fp(global_data * data, reservation_station_identifier id, instruction ins);
// set the first parameter to be src1 of the instruction on the reservation station (fp reservation station)
// does register renaming
void set_src1_fp(global_data * data, reservation_station_identifier id, instruction ins);
// checks if the relevant memory buffers are not full, for a load/store instruction.
// if it's not a memory instruction, returns true
bool no_room_on_buffers(global_data * data, instruction ins);
// checks for room on the reorder buffer
bool rob_full(global_data * data);
// finds an empty reservation station (returns identifier)
// sets a few parameters
reservation_station_identifier find_and_use_empty_reservation_station(global_data * data, instruction ins);
// find an empty memory buffer (return identifier)
// sets a few parameters
reservation_station_identifier find_and_use_empty_buffer(global_data * data, instruction ins);
bool is_i_type_alu(instruction ins);
bool is_r_type_alu(instruction ins);

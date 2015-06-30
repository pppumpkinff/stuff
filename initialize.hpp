#include "common.hpp"

// initialize all the values needed to run
void initialize(int argc, char **argv, global_data * data);
// set the cfg parameter to our global data structure
void apply_conf_parameter (const char *param, int val, global_data * data);
void initialize_reservation_stations(global_data * data);
// populate the memory based on the contents of memin.txt
void populate_memory(const char * filename, global_data * data);
void parse_config_file (const char *CfgFile, global_data * data);
void initialize_registers(global_data * data);
// take an instruction in hex format and put the correct values
// into an instruction data type
instruction parse_instruction_hex(unsigned int integer, global_data * data);
// results in an integer of the lower to upper bits of a 32 bit integer
unsigned int take_bits_from_integer(unsigned int integer, unsigned int upper, unsigned int lower);
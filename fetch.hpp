#include "common.hpp"

void fetch(global_data * data);
void initialize_registers(global_data * data);
// take an instruction in hex format and put the correct values
// into an instruction data type
instruction parse_instruction_hex(unsigned int integer, global_data * data);
// results in an integer of the lower to upper bits of a 32 bit integer
unsigned int take_bits_from_integer(unsigned int integer, unsigned int upper, unsigned int lower);
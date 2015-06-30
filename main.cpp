#include "main.hpp"

instruction parse_instruction_hex(unsigned int integer, global_data * data);
unsigned int take_bits_from_integer(unsigned int integer, unsigned int upper, unsigned int lower);
void main(int argc, char **argv) {
  global_data data;
  initialize(argc, argv, &data);

  while (!data.ctl_signal.halt_committed) {
    write_cdb(&data);
    commit(&data);
    execute(&data);
    issue(&data);
    fetch(&data);
    ++data.clock_cycle;
  }
  
  finalize(argc, argv, &data);
  
}

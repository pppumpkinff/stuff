#include "common.hpp"

// runs all the below functions.
void execute(global_data * data);

// a function to execute instructions pending
// on the int reservation station
void execute_int_alu(global_data * data);

// a function to execute instructions pending
// on the fp_add_sub reservation station
void execute_fp_add_sub(global_data * data);

// a function to execute instructions pending
// on the fp_mul reservation station
void execute_fp_mul(global_data * data);
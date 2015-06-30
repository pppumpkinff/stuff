#include "common.hpp"

void finalize(int argc, char **argv, global_data * data);
void write_memory2file(const char * filename, global_data * data);
void write_integer2file(const char * filename, global_data * data);
void write_fp2file(const char * filename, global_data * data);
void write_trace2file(const char * filename, global_data * data);

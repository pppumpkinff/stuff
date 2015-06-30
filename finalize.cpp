#include "finalize.hpp"

void finalize(int argc, char **argv, global_data * data) {
  write_memory2file(argv[3], data);
  write_integer2file(argv[4], data);
  write_fp2file(argv[5], data);
  write_trace2file(argv[6], data);

}

void write_memory2file(const char * filename, global_data * data) {
  // Description: dump memory to file

  //==variables==//
  FILE * ptr2file;
  int i = 0;
  ptr2file= fopen(filename, "w");

  //==code==//
  if (NULL==ptr2file ) {
    // failed to open file
    printf("Failed to open file %s to dump memory \n", filename);
    exit(1);
  }
  while (i < MEM_SIZE){
    fprintf(ptr2file, "%08x\n", data->memory[i]);
    ++i;
  }

  fclose(ptr2file);
  return;
}
void write_integer2file(const char * filename, global_data * data) {
  // Description: dump memory to file

  //==variables==//
  FILE * ptr2file;
  int i = 0;
  ptr2file= fopen(filename, "w");

  //==code==//
  if (NULL==ptr2file ) {
    // failed to open file
    printf("Failed to open file %s to dump memory \n", filename);
    exit(1);
  }
  while (i < REGISTER_SIZE){
    fprintf(ptr2file, "%d\n", data->int_register[i]);
    ++i;
  }

  fclose(ptr2file);
  return;
}

void write_fp2file(const char * filename, global_data * data) {
  // Description: dump memory to file

  //==variables==//
  FILE * ptr2file;
  int i = 0;
  ptr2file= fopen(filename, "w");

  //==code==//
  if (NULL==ptr2file ) {
    // failed to open file
    printf("Failed to open file %s to dump memory \n", filename);
    exit(1);
  }
  while (i < REGISTER_SIZE){
    fprintf(ptr2file, "%f\n", data->fp_register[i]);
    ++i;
  }

  fclose(ptr2file);
  return;
}
void write_trace2file(const char * filename, global_data * data) {
  // Description: dump memory to file

  //==variables==//
  FILE * ptr2file;
  int i = 0;
  ptr2file= fopen(filename, "w");

  //==code==//
  if (NULL==ptr2file ) {
    // failed to open file
    printf("Failed to open file %s to dump memory \n", filename);
    exit(1);
  }
  while (!data->trace.empty()){
    fprintf(ptr2file, "%08x %d %d %d %d\n", 
      data->trace[i].instruction_hex,
      data->trace[i].cycle_issued,
      data->trace[i].cycle_execute_start,
      data->trace[i].cycle_write_cdb,
      data->trace[i].cycle_commit);
    ++i;
  }

  fclose(ptr2file);
  return;
}
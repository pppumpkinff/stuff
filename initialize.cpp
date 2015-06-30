#include "initialize.hpp"

void initialize(int argc, char **argv, global_data * data) {
  int i;
  
  parse_config_file(argv[1], data);
  populate_memory(argv[2], data);
  data->ctl_signal.halt_fetched = false;
  data->ctl_signal.halt_committed = false;
  data->ctl_signal.speculation_made = false;
  data->ctl_signal.fp_add_in_execute = false;
  data->ctl_signal.fp_mul_in_execute = false;
  data->ctl_signal.started_execution_instruction_this_clock_cycle = false;
  data->ctl_signal.busy_committing_to_mem = false;
  data->ctl_signal.cycles_since_mem_start = 0;
  data->ctl_signal.unique_identifier = 0;
  data->ctl_signal.issued_branch_prev_cycle = false;
  data->program_counter = 0;
  data->clock_cycle = 0;
  for (i = 0; i < BTB_SIZE; ++i) {
    data->branch_target_buffer[i] = 0;
  }
  initialize_registers(data);
  initialize_reservation_stations(data);
}
void initialize_reservation_stations(global_data * data) {
  data->fp_add_reservation_station = (reservation_station_float *) calloc(data->cfg.add_nr_reservation, sizeof(reservation_station_float));
  data->fp_mul_reservation_station = (reservation_station_float *) calloc(data->cfg.mul_nr_reservation, sizeof(reservation_station_float));
  data->int_reservation_station = (reservation_station_int *) calloc(data->cfg.int_nr_reservation, sizeof(reservation_station_int));
  data->store_buffer = (mem_buffer *) calloc(data->cfg.mem_nr_store_buffers, sizeof(mem_buffer));
  data->load_buffer = (mem_buffer *) calloc(data->cfg.mem_nr_load_buffers, sizeof(mem_buffer));
}
void populate_memory(const char * filename, global_data * data) {
	FILE * ptr2file;
	int i = 0;
	char line[MAX_MEM_STRING_LENGTH];
	ptr2file = fopen(filename, "r");//read only 
	if (NULL == ptr2file) {
		// failed to open file
		printf("Failed to open file for memory initialization: %s\n",filename);
		exit(1);
	}
    
	while ((fgets(line, MAX_MEM_STRING_LENGTH, ptr2file) != NULL) && (i < MEM_SIZE)){
		data->memory[i] = strtoul(line, NULL, BASE);
		++i;
	}
  
	fclose(ptr2file);
	return;
}
void parse_config_file (const char *CfgFile, global_data * data) {
  // Description:Parsing the Config file

  //==Variables:==//
  FILE *Ptr2CfgFile;
  char *param;
  int val;
  int count;
  char Line[LINE_LENGTH];


  //==File Handles:==//
  Ptr2CfgFile = fopen( CfgFile, "r"); // read only

  //===CODE================//
  if (NULL == Ptr2CfgFile) {
    printf("Error! Could not open the config file: %s\n", CfgFile);
    exit(1); // stdlib.h is included in the header
  }


  for (count = 0; count < NUM_OF_PARAM ; count++) {
    fgets (Line, LINE_LENGTH,Ptr2CfgFile);
    param = strtok      (Line, "=");//get the first token it is parameter
    val = atoi  (strtok (NULL, "="));//get the second token-value and convert to integer
    apply_conf_parameter(param, val, data);
  }
  fclose(Ptr2CfgFile);
}
void apply_conf_parameter (const char *param, int val, global_data * data) {
  // Description: assignment of the parameter from the config to appropriate global variable

    //===CODE================//
  if (0 == strcmp(param, "int_delay "))
    data->cfg.int_delay = val;
  else if (0 == strcmp(param, "add_delay "))
    data->cfg.add_delay = val;
  else if (0 == strcmp(param, "mul_delay "))
    data->cfg.mul_delay = val;
  else if (0 == strcmp(param, "mem_delay "))
    data->cfg.mem_delay = val;
  else if (0 == strcmp(param, "rob_entries "))
    data->cfg.rob_entries = val;
  else if (0 == strcmp(param, "add_nr_reservation "))
    data->cfg.add_nr_reservation = val;
  else if (0 == strcmp(param, "mul_nr_reservation "))
    data->cfg.mul_nr_reservation = val;
  else if (0 == strcmp(param, "int_nr_reservation "))
    data->cfg.int_nr_reservation = val;
  else if (0 == strcmp(param, "mem_nr_load_buffers "))
    data->cfg.mem_nr_load_buffers = val;
  else if (0 == strcmp(param, "mem_nr_store_buffers "))
    data->cfg.mem_nr_store_buffers = val;
}
void initialize_registers(global_data * data) {
	int i;
	for (i = 0; i < REGISTER_SIZE; ++i) {
		data->fp_register[i] = (float) i;
		data->int_register[i] = 0;
    data->temp_int_register[i].value = 0;
		data->temp_int_register[i].busy = false;
		data->temp_int_register[i].valid = false;
    data->temp_fp_register[i].value = (float) i;
    data->temp_int_register[i].speculative = false;
		data->temp_fp_register[i].busy = false;
		data->temp_fp_register[i].valid = false;
    data->temp_fp_register[i].speculative = false;
	}
}
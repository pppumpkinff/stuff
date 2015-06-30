#include "common.hpp"

void write_cdb(global_data * data);
bool compare_reservation_station_identifiers(reservation_station_identifier a, reservation_station_identifier b);
void find_station_needing_result(global_data * data, reservation_station_identifier result);
void stop_renaming_result(global_data * data, reservation_station_identifier result);
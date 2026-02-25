#ifndef PARSE_DB_FUNCS_H
#define PARSE_DB_FUNCS_H

#include "cJSON.h"
#include <sqlite3.h>
// note : you have to install the cJSON package: sudo apt install libcjson-dev
//install sqlite3 : sudo apt install sqlite3

int create_db(); 

char* read_json (const char* file) ; 

void json_import_to_db(sqlite3* database, const char* file) ; 

#endif

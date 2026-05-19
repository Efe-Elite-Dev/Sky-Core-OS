#ifndef SKY_LOADER_H
#define SKY_LOADER_H
#include "globals.h"

void sky_handle_double_click(const char* filename);
void load_pe_executable(const char* path);
void unpack_deb_package(const char* path);
void execute_sky_bundle(const char* path);

#endif

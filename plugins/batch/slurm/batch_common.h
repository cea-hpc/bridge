
#ifndef __BATCH_COMMON_H
#define __BATCH_COMMON_H

int extract_field(char* statusline,char* header,char** field);

int extract_mem(char* statusline,char* header,uint32_t* i);

int extract_float(char* statusline,char* header,float* load);

int extract_uint32(char* statusline,char* header,uint32_t* i);

int extract_cores(char* line,uint32_t* i);

int convert_mem(char* field,uint32_t* mem);

int convert_time(char* field,time_t* time);

#endif

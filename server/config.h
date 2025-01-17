#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "hashtable.h"
#include "str_util.h"

#define IP "ip"
#define PORT "port"
#define THREAD_NUM "thread_num"
#define USER_HOME "USER_HOME"
void readConfig(const char* filename, HashTable * ht);

#endif

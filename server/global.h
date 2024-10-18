#ifndef GLOBAL_H  
#define GLOBAL_H  
#include "linked_list.h"
extern MYSQL* mysql; // 声明全局变量  
extern HashTable ht;
extern ListNode * userList;
extern int logfd;

#define GlobalFile "../globalfile/"
#define GlobalDir "../globalfile"
#endif

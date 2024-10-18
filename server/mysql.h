#ifndef __MYSQL_H__
#define __MYSQL_H__
#include <mysql/mysql.h>
#include<stdio.h>
#define QUERY_LEN 1024
MYSQL * disk_mysql_init_connect(MYSQL*);

MYSQL_RES * disk_mysql_query(MYSQL * mysql,const char* query);

void disk_mysql_insert(MYSQL * mysql,const char* insert_query);

void disk_mysql_update(MYSQL * mysql,const char* update_query);

void disk_mysql_delete(MYSQL * mysql,const char* delete_query);

void disk_mysql_print(MYSQL_RES * res);

void disk_mysqlCheckGlobal(MYSQL *mysql);

bool isEmpty(MYSQL * mysql,const char * query);

void disk_mysql_databaseinit(MYSQL *mysql);

void table_V_init(MYSQL* mysql);

#endif

#include "mysql.h"
#include "hashtable.h"
#include "global.h"
MYSQL * disk_mysql_init_connect(MYSQL * mysql){
    const char * host = (const char *)find(&ht,"mysql_host");
    const char * user = (const char *)find(&ht,"mysql_user");
    const char * passwd = (const char *)find(&ht,"mysql_passwd");
    const char * database = (const char *)find(&ht,"mysql_database");
    mysql = mysql_real_connect(mysql,host,user,passwd,database,
                               3306,NULL,0);
    if(!mysql){
        fprintf(stderr,"%s.\n",mysql_error(mysql));
        return NULL;
    }
    return mysql;
}

MYSQL_RES * disk_mysql_query(MYSQL * mysql,const char* query){
    int ret = mysql_real_query(mysql,query,strlen(query));
    if(ret !=0 ){
        printf("%d : %s",mysql_errno(mysql),mysql_error(mysql));
        return NULL;
    }
    MYSQL_RES * res = mysql_store_result(mysql);
    if(res == NULL){
        printf("fail store result\n");
    }
    return res;
}

void disk_mysql_print(MYSQL_RES * res){
    int rows = mysql_num_rows(res);
    int cols = mysql_num_fields(res);
    printf("GOT %d rows\n",rows);
    MYSQL_ROW  row ;
    MYSQL_FIELD* col;

    while((col = mysql_fetch_field(res))){
        //col = mysql_fetch_fields(res);
        printf("%-15s",col->name);
    }
    printf("\n");

    while((row = mysql_fetch_row(res)) !=NULL){
        for(int i =0;i<cols;i++){
            printf("%-15s",row[i]);
        }
        printf("\n");
    }
    mysql_free_result(res);
    return;
}


void disk_mysql_insert(MYSQL * mysql,const char* insert_query){
    int ret = mysql_real_query(mysql,insert_query,strlen(insert_query));
    if(ret){
        printf("%d,%s\n",mysql_errno(mysql),mysql_error(mysql));
    }
    //else{
    //    int rows = mysql_affected_rows(mysql);
    //    printf("Query Ok %d affected row\n",rows);
    //}
    return;
}

void disk_mysql_update(MYSQL * mysql,const char* insert_query){
    int ret = mysql_real_query(mysql,insert_query,strlen(insert_query));
    if(ret){
        printf("%d,%s\n",mysql_errno(mysql),mysql_error(mysql));
    }
    //else{
    //    int rows = mysql_affected_rows(mysql);
    //    printf("Query Ok %d affected row\n",rows);
    //}
    return;
}


void disk_mysql_delete(MYSQL * mysql,const char* delete_query){
    int ret = mysql_real_query(mysql,delete_query,strlen(delete_query));
    if(ret){
        printf("%d,%s\n",mysql_errno(mysql),mysql_error(mysql));
    }
    //else{
    //    int rows = mysql_affected_rows(mysql);
    //    printf("Query Ok %d affected row\n",rows);
    //}
    return;
}



bool isEmpty(MYSQL * mysql,const char * query){
    int ret = mysql_real_query(mysql,query,strlen(query));
    if(ret !=0 ){
        printf("%d : %s",mysql_errno(mysql),mysql_error(mysql));
    }
    MYSQL_RES * res = mysql_store_result(mysql);
    if(res == NULL){
        printf("fail store result\n");
    } 
    return mysql_num_rows(res) == 0;
}

void disk_mysqlCheckGlobal(MYSQL *mysql){
    char query[QUERY_LEN]={'\0'};
    char filename[128] = {0};
    //查询没有关联的全局文件
    sprintf(query," SELECT G1.id,G1.hash FROM Global_File_Table G1 "
            " WHERE G1.id NOT IN "
            " (SELECT DISTINCT G2.id "
            " FROM Virtual_File_Table V  "
            "INNER JOIN Global_File_Table G2 "
            "ON V.hash = G2.id)");
    if(isEmpty(mysql,query)){
        return ;
    }
    MYSQL_RES * res =disk_mysql_query(mysql,query);
    int rows = mysql_num_rows(res);
    MYSQL_ROW row;
    while((row = mysql_fetch_row(res)) !=NULL){
        memset(query,0,QUERY_LEN);
        sprintf(filename,"%s%s",GlobalFile,row[1]); 
        sprintf(query," DELETE FROM Global_File_Table WHERE id = '%s' ",
                row[0]);
        disk_mysql_delete(mysql,query);
        int ret = remove(filename);
        if(ret){
            perror("remove file");
        }
    }
}


void table_V_init(MYSQL* mysql){
    char query[QUERY_LEN]={'\0'};
    strcpy(query,"INSERT INTO Virtual_File_Table "
           "( parent_id, filename, filesize, type, path, owner_id, hash) "
           " VALUES ( 0, '/', 0, 'd', '/', (SELECT id FROM USER_INFO WHERE username = 'root'), 0)");
    int ret = mysql_real_query(mysql, query, strlen(query));
    if(ret) {
        printf("%d, %s\n", mysql_errno(mysql), mysql_error(mysql));
    } 
    //else {
    //    //写入成功的情况
    //    int rows = mysql_affected_rows(mysql);
    //    printf("INSERT OK, %d row affected.\n", rows);
    //}
}
void disk_mysql_databaseinit(MYSQL *mysql){
    char query[QUERY_LEN]={'\0'};
    sprintf(query,"SELECT id FROM USER_INFO WHERE username = 'root'");
    if(isEmpty(mysql,query)){
        strcpy(query,"INSERT INTO USER_INFO "
               " (id,username,salt,cryptpasswd,path_id) "
               " VALUES (0,'root','$5$fd4f1c06568f0f4a', "
               "'$5$fd4f1c06568f0f4a$FssNWgriQyYoNGHvb2pCOz7fL7I4PbmhIxaVEMC3Vy1',1)");
        int ret = mysql_real_query(mysql, query, strlen(query));

        if(ret ==0) {
            table_V_init(mysql);
            //     //写入成功的情况
            //     int rows = mysql_affected_rows(mysql);
            //     printf("INSERT OK, %d row affected.\n", rows);
        }
    }
}




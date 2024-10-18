#include "thread_pool.h"
#include "global.h"

void rmdirHelp(char * dirID);//递归删除目录辅助函数

void rmdirCommand(task_t * task,user_info_t * user){

    char query[QUERY_LEN]={'\0'};
    char filename[128] = {0};
    train_t t;
    memset(&t,0,sizeof(t));

    //判断传入是否为目录
    sprintf(query," SELECT id FROM Virtual_File_Table "
            " WHERE parent_id = '%s' AND filename = '%s' AND type = 'd' ",
            user->pwd_id,task->data);

    if(isEmpty(mysql,query)){
        strcpy(t.buff,"Not a directory");
        goto end1;
    }
    {
        MYSQL_RES * res =disk_mysql_query(mysql,query);
        MYSQL_ROW row = mysql_fetch_row(res);
        char dirID[124] = {'\0'};
        strcpy(dirID,row[0]);

        //使用辅助函数，递归删除目录及其文件
        rmdirHelp(dirID);

        //如果删除过程中mysql出现错误
        if(mysql_errno(mysql)){
            strcpy(t.buff,mysql_error(mysql));
            goto end1;
        }

        //删除没有关联的全局文件
        disk_mysqlCheckGlobal(mysql);
        strcpy(t.buff,"rmDir Success");
    }
end1:
    t.len = strlen(t.buff);
    t.type = CMD_TYPE_RMDIR;
    sendn(task->peerfd,&t,sizeof(train_t));
    return ;
}

//辅助函数
void  rmdirHelp(char * dirID){
    char query[QUERY_LEN] = {'\0'};

    //查询该目录下的内容
    sprintf(query,"SELECT id,type FROM Virtual_File_Table WHERE parent_id = '%s'"
            ,dirID);

    //空目录
    if(isEmpty(mysql,query)){
        goto end2;
    }

    {
        //包含目录
        MYSQL_RES * res = disk_mysql_query(mysql,query);
        MYSQL_ROW row;
        int rows = mysql_num_rows(res);
        memset(&row,0,sizeof(row));
        //循环读取内容
        for(int i = 0 ; i<rows ;i++){
            row = mysql_fetch_row(res);
            //目录 递归
            if(strcmp(row[1],"d") == 0 ){
                rmdirHelp(row[0]);
            }
            //文件则删除
            else if(strcmp(row[1],"f") ==0){
                memset(query,0,QUERY_LEN);
                sprintf(query," DELETE FROM Virtual_File_Table "
                        " WHERE id = '%s'"
                        ,row[0]);
                disk_mysql_delete(mysql,query);
            }
        }   
    }
end2:
    memset(query,0,QUERY_LEN);
    sprintf(query," DELETE FROM Virtual_File_Table WHERE id = '%s'"
            ,dirID);
    disk_mysql_delete(mysql,query);
    return ;
}


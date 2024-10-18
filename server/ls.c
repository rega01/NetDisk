/*
 *@author cmj created
 *
 *
 *
 */
#include "thread_pool.h"
#include "global.h"

//extern ListNode * userList;

void lsCommand(task_t * task,user_info_t * user)
{
    train_t t;
    memset(&t,0,sizeof(train_t));
    t.type = CMD_TYPE_LS;
    //查询用户文件的完整路径
    char query[QUERY_LEN]={'\0'};

    //1、获取当前目录下所有的文件
    sprintf(query," SELECT filename FROM Virtual_File_Table "
            " WHERE parent_id = '%s' AND owner_id ='%s' ORDER BY filename "
            ,user->pwd_id ,user->mysql_id);

    if(isEmpty(mysql,query)){
        strcpy(t.buff,"EMPTY DIRECTORY");
        goto end;
    }

    MYSQL_RES* res = disk_mysql_query(mysql,query);
    int rows = mysql_num_rows(res);
    MYSQL_ROW row ;
    memset(&row,0,sizeof(row));
    strcpy(t.buff,mysql_fetch_row(res)[0]);
    while((row = mysql_fetch_row(res)) !=NULL){
        sprintf(t.buff,"%s  %s",t.buff,row[0]);
    }
    //puts(t.buff);

end:
    t.len = strlen(t.buff);
    sendn(task->peerfd,&t,sizeof(train_t));
    return ;
}



/*
 *@author cmj created
 *
 */

#include "thread_pool.h"
#include "global.h"
//extern ListNode * userList;
void pwdCommand(task_t * task,user_info_t * user)
{
    train_t t;
    memset(&t,0,sizeof(train_t));
    t.type = CMD_TYPE_PWD;
    //查询用户文件的完整路径
    char fullPath[1024]={'\0'};
    char query[QUERY_LEN]={'\0'};
    sprintf(query,"SELECT v.path FROM USER_INFO u  "
            " LEFT JOIN Virtual_File_Table v on u.path_id = v.id "
            " WHERE u.username = '%s'",
            user->name);
    if(isEmpty(mysql,query)){
        printf("pwd:%s",mysql_error(mysql));
        strcpy(t.buff,mysql_error(mysql));
        goto end;
    }
    MYSQL_RES *res = disk_mysql_query(mysql,query);
    MYSQL_ROW row = mysql_fetch_row(res);
    strcpy(fullPath,row[0]);
    //printf("current : %s\n",fullPath);

    strcpy(t.buff,fullPath);
    //sprintf(t.buff,"%s\n%s",t.buff,fullPath);
end:
    t.len = strlen(t.buff);
    sendn(task->peerfd,&t,sizeof(train_t));
    return ;
}


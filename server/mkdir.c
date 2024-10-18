#include "thread_pool.h"
#include "global.h"

void mkdirCommand(task_t * task,user_info_t * user)
{
    printf("execute mkdir command.\n");

    //printf("username = {%s}\nuser salt = {%s}\nencrypted = {%s}\n"
    //       "userpwdid={%s}\n,mysqid={%s}\n",
    //       user->name,user->salt,user->encrypted,user->pwd_id,user->mysql_id);

    // 获取新目录  拼接完整路径
    char newDir[50]= {'\0'};
    strcpy(newDir,task->data);
    //printf("newDir = %s\n",newDir);
    
    char query[QUERY_LEN] = {'\0'};
    sprintf(query,"SELECT path FROM Virtual_File_Table "
            " WHERE id = '%s'",
            user->pwd_id);
    if(isEmpty(mysql,query)){
        printf("fail get path\n");
        return ; 
    }
    MYSQL_RES * res = disk_mysql_query(mysql,query);
    MYSQL_ROW row = mysql_fetch_row(res);

    char fullPath[1024]= {'\0'};
    sprintf(fullPath,"%s/%s",row[0],newDir);
    //printf("fullpath = %s\n",fullPath);
    
    //插入新目录
    memset(query,0,QUERY_LEN);
    sprintf(query,"INSERT INTO Virtual_File_Table "
            " (parent_id,filename,owner_id,hash,filesize,type,path) "
            " VALUES('%s','%s','%s','0','0','d','%s')",
            user->pwd_id,newDir,user->mysql_id,fullPath);
    disk_mysql_insert(mysql,query);

    //获取是否成功信息
    train_t t ;
    memset(&t,0,sizeof(t));
    if(mysql_errno(mysql)){
        strcpy(t.buff , mysql_error(mysql));
    }else {
        strcpy(t.buff,"mkdir success");
    }
    t.len = strlen(t.buff);
    t.type = CMD_TYPE_MKDIR;
    //将结果信息发生给客户端
    sendn(task->peerfd,&t,sizeof(train_t));
    return  ;

}

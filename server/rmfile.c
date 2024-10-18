#include "thread_pool.h"
#include "global.h"

void rmfileCommand(task_t * task,user_info_t * user){

    char filename[128] = {0};
    char userFullPath[1024]={'\0'};
    char filesha256[1024]={'\0'};
    char query[QUERY_LEN]={'\0'};
    char virtualId[20]={'\0'};
    train_t t;
    memset(&t,0,sizeof(t));
    
    //1、查询是否有该文件
    sprintf(query," SELECT V.id,G.hash "
                             " FROM Virtual_File_Table V "
                             " LEFT JOIN Global_File_Table G ON V.hash = G.id "
                             "WHERE V.parent_id = '%s' AND V.owner_id = '%s' "
                             "AND V.filename = '%s' AND V.type = 'f' "
                             ,user->pwd_id,user->mysql_id,task->data);
    //2、根据不同结果执行
    //2.1没有该文件
    if(isEmpty(mysql,query)){
        strcpy(t.buff,"NO SUCH FILE IN CURRENT DIRECTORY");
        goto end;
    }

    //2.2包含该文件，删除该文件的虚拟文件表记录
    MYSQL_RES * res = disk_mysql_query(mysql,query);
    MYSQL_ROW row = mysql_fetch_row(res);
    strcpy(virtualId,row[0]);
    //存储hsha256值，用于检查全局文件表
    strcpy(filesha256,row[1]);
    //删除虚拟文件表记录
    memset(query,0,QUERY_LEN);
    sprintf(query," DELETE FROM Virtual_File_Table WHERE id ='%s'"
            ,virtualId);
    disk_mysql_delete(mysql,query);

    disk_mysqlCheckGlobal(mysql);

    memset(&t,0,sizeof(t));
    strcpy(t.buff,"rmfile success");
end:
    t.len = strlen(t.buff);
    t.type = CMD_TYPE_RMFILE;
    sendn(task->peerfd,&t,sizeof(train_t));
    return ;
}

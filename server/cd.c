#include "thread_pool.h"
#include "user.h"
#include "global.h"

//extern ListNode* userList;
void cdCommand(task_t * task,user_info_t * user){
    train_t t;
    memset(&t,0,sizeof(t));
    char query[QUERY_LEN]={'\0'};
    //1、改变目录（1、.. 2、子目录 3、.)
    //1.1 cd ..
    if(strcmp(task->data,"..") ==0){
        char parent_id[50]={'\0'};
        //sprintf(query," SELECT parent_id FROM Virtual_File_Table  "
        //        " WHERE id = '%s' AND owner_id ='%s' ; "
        //        ,user->pwd_id,user->mysql_id);
        sprintf(query," SELECT parent_id FROM Virtual_File_Table  "
                " WHERE id = '%s' ; "
                ,user->pwd_id);
        //puts(query);
        if(isEmpty(mysql,query)){
            printf("%s",mysql_error(mysql));
            strcpy(t.buff,mysql_error(mysql));
            goto end;
        }
        MYSQL_RES * res = disk_mysql_query(mysql,query);
        MYSQL_ROW row = mysql_fetch_row(res);
        strcpy(parent_id,row[0]);

        if(strcmp(parent_id,"0") ==0){
            strcpy(t.buff,"already in root");
            puts("already in root");
            goto end;
        }

        //查询父目录的信息
        memset(query,0,sizeof(query));
        memset(res,0,sizeof(MYSQL_RES));
        memset(&row,0,sizeof(row));
        //sprintf(query,"SELECT id,path FROM Virtual_File_Table "
        //        " WHERE id = '%s' AND owner_id ='%s' ; ",parent_id,user->mysql_id);
        sprintf(query,"SELECT id,path FROM Virtual_File_Table "
                " WHERE id = '%s'  ; ",parent_id);
        if(isEmpty(mysql,query)){
            printf("%s",mysql_error(mysql));
            strcpy(t.buff,mysql_error(mysql));
            goto end;
        }
        res = disk_mysql_query(mysql,query);
        row = mysql_fetch_row(res);
        strcpy(t.buff,row[1]);

        //sprintf(t.buff,"%s\n%s",t.buff,row[1]);
        //更新新的当前目录
        strcpy(user->pwd_id,row[0]);
        memset(query,0,sizeof(query));
        sprintf(query,"UPDATE USER_INFO "
                " SET path_id = '%s' WHERE username = '%s';"
                ,user->pwd_id,user->name);
        disk_mysql_update(mysql,query);
    }
    else if(strcmp(task->data,".") !=0 ){
        sprintf(query," SELECT id FROM Virtual_File_Table "
                " WHERE parent_id ='%s' AND filename = '%s' AND  type = 'f' "
                ,user->pwd_id,task->data);
        if(!isEmpty(mysql,query)){
            sprintf(t.buff,"%s is FILE",task->data);
            goto end;
        }

        sprintf(query," SELECT id,path "
                " FROM Virtual_File_Table "
                " WHERE parent_id = '%s' AND filename = '%s'AND owner_id = '%s' AND type = 'd' ;"
                ,user->pwd_id,task->data,user->mysql_id);
        if(isEmpty(mysql,query)){
            sprintf(t.buff,"NO SUCH DIRECTORY :%s",task->data);
            goto end;
        }
        MYSQL_RES *res = disk_mysql_query(mysql,query);
        MYSQL_ROW row = mysql_fetch_row(res);
        strcpy(t.buff,row[1]);

        //更新新的当前目录
        strcpy(user->pwd_id,row[0]);
        memset(query,0,sizeof(query));
        sprintf(query,"UPDATE USER_INFO "
                " SET path_id = '%s' WHERE username = '%s' "
                ,user->pwd_id,user->name);
        disk_mysql_update(mysql,query);
    }else{ 
        strcpy(t.buff,"Already in current directory");
    }

end:
    t.len = strlen(t.buff);
    t.type = CMD_TYPE_CD;
    sendn(task->peerfd,&t,sizeof(train_t));
    return ;
}


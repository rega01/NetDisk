#include "thread_pool.h"
#include "global.h"
#include "sha256.h"
void putsCommand(task_t * task,user_info_t * user) {

    mkdir(GlobalDir,0777);

    train_t t;
    memset(&t,0,sizeof(train_t));
    t.type = CMD_TYPE_PUTS;

    char filename[128] = {0};
    //strcpy(filename, task->data);

    //char fullPath[1024]={'\0'};
    char userFullPath[1024]={'\0'};
    char filesha256[1024]={'\0'};
    char query[QUERY_LEN]={'\0'};

    //1、接收sha256值,并查询全局文件表是否包含
    recvn(task->peerfd,&t,sizeof(t));
    strcpy(filesha256,t.buff);
    //使用sha256值作为文件名存储
    sprintf(filename,"%s%s",GlobalFile,filesha256); 
    //printf("sha256 = {%s}\n",t.buff);

    //拼接全局文件表查询语句
    sprintf(query," SELECT id FROM Global_File_Table "
            " WHERE hash = '%s'"
            ,filesha256);

    memset(&t,0,sizeof(t));

    //2、反馈给客户端并执行对应操作
    //该文件不存在于服务器
    if(isEmpty(mysql,query)){
        t.type = CMD_TYPE_PUTS;
        sendn(task->peerfd,&t,sizeof(t));

        //2.1打开文件
        //printf("Upload file {%s}",filename);
        int fd = open(filename, O_CREAT|O_RDWR, 0644);
        if(fd < 0) {
            perror("open"); return;
        } 

        //2.2接收文件大小
        memset(&t,0,sizeof(t));
        unsigned long totalSize  =0;
        int ret =recvn(task->peerfd,&totalSize,sizeof(totalSize));

        //接收并写入文件
        char buff[1000] = {0};
        unsigned long left = totalSize;
        while(left > 0) {
            if(left < 1000) {
                ret = recvn(task->peerfd, buff, left);
            } else {
                ret = recvn(task->peerfd, buff, sizeof(buff));
            }
            if(ret < 0) {
                break;
            }
            ret = write(fd, buff, ret);
            left -= ret;
        }
        //printf("left = %lu\n",totalSize - left);
        close(fd);
        //校验文件是否完整，否则不插入至数据库
        char * filesha256temp =sha256_file(filename); 
        if(strcmp(filesha256,filesha256temp) !=0 ){
            memset(&t,0,sizeof(t));
            strcpy(t.buff,"Upload fail");
            //删除本地文件，并且不执行下面的sql语句
            int ret = remove(filename);
            free(filesha256temp);
            goto end;
        }
        free(filesha256temp);

        //插入全局文件表
        memset(query,0,QUERY_LEN);
        sprintf(query," INSERT INTO Global_File_Table(hash) "
                " VALUES('%s') "
                ,filesha256);
        disk_mysql_insert(mysql,query);
        //printf("error ={%s},errno={%d}\n",mysql_error(mysql),mysql_errno(mysql));


        //拼接用户完整路径
        memset(query,0,QUERY_LEN);
        sprintf(query," SELECT path FROM Virtual_File_Table "
                " WHERE id = '%s' ",user->pwd_id);
        if(isEmpty(mysql,query)){
            puts("NO CURRENT PATH");
            goto end;
        }       
        MYSQL_RES * res = disk_mysql_query(mysql,query);
        sprintf(userFullPath,"%s/%s",mysql_fetch_row(res)[0],task->data);

        //插入虚拟文件表
        memset(query,0,QUERY_LEN);
        sprintf(query," INSERT INTO Virtual_File_Table "
                " (parent_id,filename,owner_id,hash,fileSize,type,path) "
                " VALUES('%s','%s','%s', "
                " (SELECT id FROM Global_File_Table WHERE hash = '%s'), "
                "  '%zu', 'f','%s'  ) "
                , user->pwd_id,task->data,user->mysql_id,filesha256,totalSize,userFullPath);
        disk_mysql_insert(mysql,query);

        if(mysql_errno(mysql)){
            strcpy(t.buff,mysql_error(mysql));

        }else{
            strcpy(t.buff,"Server handle Success");

        }

    }
    //该文件在服务器中
    else{
        t.type = CMD_TYPE_PUTS_EXIST;
        sendn(task->peerfd,&t,sizeof(t));
        //查询文件是否已存在于当前目录
        memset(query,0,QUERY_LEN);
        sprintf(query," SELECT id FROM Virtual_File_Table "
                " WHERE owner_id = '%s' AND parent_id = '%s' AND filename ='%s' "
                ,user->mysql_id,user->pwd_id,task->data);
        if(!isEmpty(mysql,query)){
            strcpy(t.buff,"File Already exist in Curent Directory");
            goto end;
        }

        //查询文件大小
        int fileFd = open(filename,O_RDONLY);
        struct stat st;
        fstat(fileFd,&st);
        off_t totalSize = st.st_size;

        //拼接用户完整路径
        memset(query,0,QUERY_LEN);
        sprintf(query," SELECT path FROM Virtual_File_Table "
                " WHERE id = '%s' ",user->pwd_id);
        if(isEmpty(mysql,query)){
            puts("NO CURRENT PATH");
            goto end;
        }       
        MYSQL_RES * res = disk_mysql_query(mysql,query);
        sprintf(userFullPath,"%s/%s",mysql_fetch_row(res)[0],task->data);

        //插入虚拟文件表
        memset(query,0,QUERY_LEN);
        sprintf(query," INSERT INTO Virtual_File_Table "
                " (parent_id,filename,owner_id,hash,fileSize,type,path) "
                " VALUES('%s','%s','%s', "
                " (SELECT id FROM Global_File_Table WHERE hash = '%s'), "
                "  '%zu', 'f','%s'  ) "
                , user->pwd_id,task->data,user->mysql_id,filesha256,totalSize,userFullPath);
        disk_mysql_insert(mysql,query);

        if(mysql_errno(mysql)){
            strcpy(t.buff,mysql_error(mysql));
        }else{
            strcpy(t.buff,"Server already has file (handle Success)");
        }

    }
end:
    t.len = strlen(t.buff);
    sendn(task->peerfd,&t,sizeof(t));   
    addEpollReadfd(task->epfd, task->peerfd);
}

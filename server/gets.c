#include "thread_pool.h"
#include "global.h"

void getsCommand(task_t * task,user_info_t * user){
    printf("execute gets command.\n");
    char query[QUERY_LEN]={'\0'};
    //1.获取文件剩余大小
    unsigned long targetSize = 0;
    int ret = recvn(task->peerfd,&targetSize,
                    sizeof(unsigned long));

    //2、打开文件，查看所需的总大小并移动位置
    char filename[128] ={'\0'} ;//globalpath + hash
    char filesha256[1024]={'\0'};
    unsigned long leftSize =-1;
    //strcpy(filename,task->data);
    sprintf(query," SELECT G.hash FROM Global_File_Table G "
            " LEFT JOIN  Virtual_File_Table V ON G.id = V.hash "
            "   WHERE V.parent_id ='%s' AND V.filename = '%s' "
            ,user->pwd_id,task->data);
    if(isEmpty(mysql,query)){
        sendn(task->peerfd,&leftSize,sizeof(unsigned long));
        addEpollReadfd(task->epfd,task->peerfd);
        return;
    }
    //存储hash值
    MYSQL_RES * res = disk_mysql_query(mysql,query);
    strcpy(filesha256,mysql_fetch_row(res)[0]);
    sprintf(filename,"%s%s",GlobalFile,filesha256); 

    //打开文件，查看大小
    int fileFd = open(filename,O_RDWR);
    struct stat statbuff ;
    memset(&statbuff,0,sizeof(struct stat));
    ret = fstat(fileFd,&statbuff);

    //2、发送剩余大小
    leftSize = statbuff.st_size - targetSize ;
    ret = sendn(task->peerfd,&leftSize,sizeof(unsigned long));

    //3、传输文件
    //根据文件大小选择不同传输方式
    char * method [3]={"USE senfile","USE splice","USE train"};
    train_t t;
    memset(&t,0,sizeof(train_t));
    //printf("targetSize=%lu\n",targetSize);
    lseek(fileFd,targetSize,SEEK_SET);

    //大于100M小于2G使用sendfile
    if((unsigned  long)leftSize > 100*1024*1024 && leftSize < (unsigned  long)1024*1024*1024*2){
        strcpy(t.buff,method[0]);
        t.len = strlen(t.buff);
        sendn(task->peerfd,&t,sizeof(t));
        sendfile(task->peerfd,fileFd,NULL,leftSize);
        //printf("send %lu\n",leftSize);
    }
    //大于2G使用splice
    else if(leftSize > (unsigned long)1024*1024*1024*2){
        strcpy(t.buff,method[1]);
        t.len = strlen(t.buff);
        sendn(task->peerfd,&t,sizeof(t));
        int pipefd[2];
        pipe(pipefd);
        while(leftSize>0){
            int ret = splice(fileFd,NULL,pipefd[1],NULL,
                             4096,SPLICE_F_MORE);
            ret = splice(pipefd[0],NULL,task->peerfd,NULL,
                         ret,SPLICE_F_MORE);
            leftSize -= ret;
        }
    }
    //小于100M使用小火车
    else if (leftSize >0){
        strcpy(t.buff,method[2]);
        t.len = strlen(t.buff);
        sendn(task->peerfd,&t,sizeof(t));
        while(leftSize >0){
            memset(&t,0,sizeof(train_t));
            int readnum  = (unsigned long)leftSize > sizeof(t.buff) 
                ? sizeof(t.buff) : leftSize;
            ret =  read(fileFd,t.buff,readnum);
            t.len = ret ;
            ret = sendn(task->peerfd,t.buff,t.len);
            leftSize -= t.len;
        }}

    //判断文件是否传输完整
    memset(&t,0,sizeof(t));
    strcpy(t.buff,filesha256);
    t.len = strlen(t.buff);
    t.type = CMD_TYPE_GETS;
    sendn(task->peerfd,&t,sizeof(t));  close(fileFd);
    //puts("transfer over");
    close(fileFd);
end:
    //t.len = strlen(t.buff);
    //sendn(task->peerfd,&t,sizeof(t));  close(fileFd);
    addEpollReadfd(task->epfd,task->peerfd);
}



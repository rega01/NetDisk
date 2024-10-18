#include <stdio.h>
#include "client.h"
#include <myownHead.h>
#include "sha256.h"


void getsCommand(int sockfd,train_t * pt){
    //1、尝试打开文件，然后发送给服务端本地的大小
    char filename[124] ={'\0'};
    strcpy(filename,pt->buff);
    int tryOpenfd = open(filename,O_RDWR);
    int ret = 0;
    unsigned long targetSize = 0;
   // printf("%lu\n",targetSize);
    struct stat statbuf;
    if(tryOpenfd !=-1) {
        memset(&statbuf,0,sizeof(struct stat));
        ret = fstat(tryOpenfd,&statbuf);
        ERROR_CHECK(ret ,-1,"fstat");
        targetSize = statbuf.st_size;
    }
    // 发送给服务端当前已有所需大小
    ret = sendn(sockfd,&targetSize,sizeof(unsigned long));

    //2、获取文件剩余大小
    unsigned long leftSize = 0;
    ret = recvn(sockfd,&leftSize,sizeof(unsigned long));
    if(leftSize ==0){
        printf("File already downland\n");
        return ;
    }else if(leftSize ==(unsigned long) -1){
        printf("NO SUCH FILE IN SERVER\n");
        return ;
    }
    //3、传输文件
    train_t methodt ;
    memset (&methodt,0,sizeof(train_t));
    recvn(sockfd,&methodt,sizeof(train_t));
    puts(methodt.buff);

    int filefd =-1;
    //文件存在就使用旧文件符
    if(tryOpenfd != -1){
        close(tryOpenfd);
        filefd = open(pt->buff,O_WRONLY);
        lseek(filefd,targetSize,SEEK_SET);
    }else {
        filefd = open(pt->buff,O_RDWR | O_CREAT |O_TRUNC,0666);
    }

    unsigned long recvSize = 0;
    while(recvSize < leftSize){
        train_t newt;
        memset(&newt,0,sizeof(train_t));
        unsigned long recvnum = (unsigned long)(leftSize - recvSize) > sizeof(newt.buff)
            ? sizeof(newt.buff):(leftSize-recvSize);
        recvn(sockfd,newt.buff,recvnum);
        ret = write(filefd,newt.buff,recvnum);
        recvSize += ret;
        printf("pace=%5.2f%%\r",(double)100.0*recvSize/leftSize);
        fflush(stdout);
    }
    printf("pace=100.00%%\n");
    close(tryOpenfd);
    close(filefd);

    //校验文件是否完整
    train_t t;
    memset(&t,0,sizeof(t));
    recvn(sockfd,&t,sizeof(t));
    char*filesha256temp =sha256_file(filename); 
    if(strcmp(t.buff,filesha256temp) ==0 ){
        puts("File Recv completely");
    }
    else{
        puts("File Recv WRONG TRY AGAIN");
    }
    free(filesha256temp);
}


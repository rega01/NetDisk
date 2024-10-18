#include <stdio.h>
#include "client.h"
#include <myownHead.h>
#include "sha256.h"

void putsCommand(int sockfd, train_t * pt)
{
    char filename[20] = {0};
    train_t t;
    memset(&t,0,sizeof(t));
    strcpy(filename, pt->buff);

    //1、获取文件的sha256值，发送给服务端
    char filesha256[128]= {'\0'};
    char*filesha256temp =sha256_file(filename); 
    strcpy(filesha256,filesha256temp);
    //puts(filesha256temp);
    //puts(filesha256);
    strcpy(t.buff,filesha256);
    t.len = strlen(t.buff);
    t.type = CMD_TYPE_PUTS;
    sendn(sockfd,&t,sizeof(t));

    //2、接收文件是否存在信息
    memset(&t,0,sizeof(t));
    recvn(sockfd,&t,sizeof(t));

    //根据不同结果执行
    //服务器没有该文件，需要上传
    if(t.type == CMD_TYPE_PUTS){
        //2.1打开文件
        int fd = open(filename, O_RDWR);
        if(fd < 0) {
            free(filesha256temp);
            perror("open"); return;
        }
        //2.2获取文件大小 并发送
        struct stat st;
        memset(&st, 0, sizeof(st));
        fstat(fd, &st);
        printf("file length: %ld\n", st.st_size);
        //发送文件大小
        memset(&t,0,sizeof(t));
        unsigned long totalSize = st.st_size;
        sendn(sockfd, &totalSize, sizeof(totalSize));

        //发送内容
        unsigned long cur = 0;
        char buff[1000] = {0};
        int ret = 0;
        while(cur < totalSize) {
            memset(buff, 0, sizeof(buff));
            ret = read(fd, buff, sizeof(buff));
            if(ret == 0) {
                break;
            }
            ret = sendn(sockfd, buff, ret);
            cur +=  ret;
            printf("Upload file : %5.2f%%\r",(double)100.0*cur/totalSize);
            fflush(stdout);
        }
        //发送完成
        printf("Upload file : 100.00%%\n");
        printf("file send over.\n");
        close(fd);
        memset(&t,0,sizeof(t));

    }

    //服务器已有文件

    memset(&t,0,sizeof(t));
    recvn(sockfd,&t,sizeof(t));
    puts(t.buff);

    free(filesha256temp);
}

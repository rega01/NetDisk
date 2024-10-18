#include "client.h"
#include "str_util.h"
#include "sha256.h"
#include <stdio.h>
#include <unistd.h>
//void gets_big(int sockfd,int filefd,int leftSize);

int tcpConnect(const char * ip, unsigned short port)
{
    //1. 创建TCP的客户端套接字
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd < 0) {
        perror("socket");
        return -1;
    }

    //2. 指定服务器的网络地址
    struct sockaddr_in serveraddr;
    //初始化操作,防止内部有脏数据
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;//指定IPv4
    serveraddr.sin_port = htons(port);//指定端口号
                                                 //指定IP地址
    serveraddr.sin_addr.s_addr = inet_addr(ip);

    //3. 发起建立连接的请求
    int ret = connect(clientfd, (const struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if(ret < 0) {
        perror("connect");
        close(clientfd);
        return -1;
    }
    return clientfd;
}

static int userLogin1(int sockfd, train_t *t);
static int userLogin2(int sockfd, train_t *t);
static int registerSalt(int sockfd, train_t *t);

int userLogin(int sockfd)
{
    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    userLogin1(sockfd, &t);
    return 0;
}

static int userLogin1(int sockfd, train_t *pt)
{
    /* printf("userLogin1.\n"); */
    train_t t;
    memset(&t, 0, sizeof(t));
    while(1) {
        puts(USER_NAME);
        char user[20]= {0};
        int ret = read(STDIN_FILENO, user, sizeof(user));
        user[ret - 1] = '\0';
        t.len = strlen(user);
        t.type = TASK_LOGIN_SECTION1;
        strncpy(t.buff, user, t.len);
        ret = sendn(sockfd, &t, sizeof(t));
        /* printf("login1 send %d bytes.\n", ret); */

        //接收信息
        memset(&t, 0, sizeof(t));
        ret = recvn(sockfd, &t, sizeof(t));
        /* printf("length: %d\n", t.len); */
        if(t.type == TASK_LOGIN_SECTION1_RESP_ERROR) {
            //无效用户名, 进入注册
            printf("user name not exist.\n"
                   "resigter\n");
            memset(&t, 0, sizeof(t));
            t.type =TASK_REGISTER_USERNAME;
            strcpy(t.buff,user);
            t.len = strlen(t.buff);
            ret = sendn(sockfd, &t, sizeof(t));
            //printf("t.len={%d},t.type={%d},t.buff={%s}\n",
            //      t.len,t.type,t.buff);

            //接收盐值
            memset(&t, 0, sizeof(t));
            ret = recv(sockfd,&t , sizeof(t),MSG_WAITALL);
            //printf("{%s},{%d}\n",t.buff,t.len);
            memcpy(pt, &t, sizeof(t));
            registerSalt(sockfd,pt);
            return 0;
        }
        //用户名正确
        puts("right username");
        memcpy(pt, &t, sizeof(t));
        userLogin2(sockfd, pt);
        break;
    }
    return 0;
}

static int userLogin2(int sockfd, train_t * pt)
{
     printf("userLogin2.\n"); 
    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    while(1) {
        char * passwd = getpass(PASSWORD);
        /* printf("password: %s\n", passwd); */
        char * encrytped = crypt(passwd, pt->buff);
        /* printf("encrytped: %s\n", encrytped); */
        t.len = strlen(encrytped);
        t.type = TASK_LOGIN_SECTION2;
        strncpy(t.buff, encrytped, t.len);
        ret = sendn(sockfd, &t, sizeof(t));
        /* printf("userLogin2 send %d bytes.\n", ret); */

        memset(&t, 0, sizeof(t));
        ret = recvn(sockfd, &t, sizeof(t));
        /* printf("2 length: %d\n", t.len); */
        if(t.type == TASK_LOGIN_SECTION2_RESP_ERROR) {
            //密码不正确
            printf("sorry, password is not correct.\n");
            continue;
        } else {
            printf("Login Success.\n");
            printf("please input a command.\n");
            fprintf(stderr, "%s", t.buff);
            break;
        } 
    }
    return 0;
}
static int registerSalt(int sockfd, train_t *pt){
    char * salt = (char*)calloc(1,65*sizeof(char));
    strcpy(salt,pt->buff);
    char*passwd = getpass("type the register passwd:");
    char*encryptcod = crypt(passwd,salt);
    train_t t;
    memset(&t,0,sizeof(t));
    t.type = TASK_REGISTER_ENCRYPTEDCODE;
    strcpy(t.buff,encryptcod);
    t.len = strlen(t.buff);
    printf( "t.len={%d},t.type={%d},t.buff={%s}\n",
            t.len,t.type,t.buff);
    sendn(sockfd,&t,sizeof(t));
    //接收是否创建成功
    memset(&t,0,sizeof(t));
    recv(sockfd,&t,sizeof(t),MSG_WAITALL);
    if(t.type != TASK_REGISTER_RESP_OK){
        printf("register fail\n");
        return -1;
    }
    printf("register success\n");
    puts("please relogin");
    memset(pt,0,sizeof(train_t));
    free(salt);
    userLogin1(sockfd,pt);
    return 0;
}

//其作用：确定接收len字节的数据
int recvn(int sockfd, void * buff, int len)
{
    int left = len;//还剩下多少个字节需要接收
    char * pbuf = buff;
    int ret = -1;
    while(left > 0) {
        ret = recv(sockfd, pbuf, left, 0);
        if(ret == 0) {
            break;
        } else if(ret < 0) {
            perror("recv");
            return -1;
        }

        left -= ret;
        pbuf += ret;
    }
    //当退出while循环时，left的值等于0
    return len - left;
}

//作用: 确定发送len字节的数据
int sendn(int sockfd, const void * buff, int len)
{
    int left = len;
    const char * pbuf = buff;
    int ret = -1;
    while(left > 0) {
        ret = send(sockfd, pbuf, left, 0);
        if(ret < 0) {
            perror("send");
            return -1;
        }

        left -= ret;
        pbuf += ret;
    }
    return len - left;
}

//解析命令
int parseCommand(const char * pinput, int len, train_t * pt)
{
    char * pstrs[10] = {0};
    int cnt = 0;
    splitString(pinput, " ", pstrs, 10, &cnt);
    pt->type =(CmdType) getCommandType(pstrs[0]);
    //暂时限定命令行格式为：
    //1. cmd
    //2. cmd content
    if(cnt > 1) {
        pt->len = strlen(pstrs[1]);
        strncpy(pt->buff, pstrs[1], strlen(pstrs[1]));
    }
    return 0;
}

int getCommandType(const char * str)
{
    if(!strcmp(str, "pwd")) 
        return CMD_TYPE_PWD;
    else if(!strcmp(str, "ls"))
        return CMD_TYPE_LS;
    else if(!strcmp(str, "cd"))
        return CMD_TYPE_CD;
    else if(!strcmp(str, "mkdir"))
        return CMD_TYPE_MKDIR;
    else if(!strcmp(str,"rmdir"))
        return CMD_TYPE_RMDIR;
    else if(!strcmp(str, "rmfile"))
        return CMD_TYPE_RMFILE;
    else if(!strcmp(str, "rm"))
        return CMD_TYPE_RMFILE;
    else if(!strcmp(str, "puts"))
        return CMD_TYPE_PUTS;
    else if(!strcmp(str, "gets"))
        return CMD_TYPE_GETS;
    else if(!strcmp(str, "exit"))
        return CLIENT_EXIT;
    else
        return CMD_TYPE_NOTCMD;
}



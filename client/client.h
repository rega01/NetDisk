#pragma once

#define USER_NAME "login as :"
#define PASSWORD "Password:"

typedef enum {
    CMD_TYPE_PWD=1,
    CMD_TYPE_LS,
    CMD_TYPE_CD,
    CMD_TYPE_MKDIR,
    CMD_TYPE_RMDIR,
    CMD_TYPE_RMFILE,
    CMD_TYPE_PUTS,
    CMD_TYPE_PUTS_EXIS,
    CMD_TYPE_GETS,
    CMD_TYPE_NOTCMD,  //不是命令

    TASK_LOGIN_SECTION1 = 100,
    TASK_LOGIN_SECTION1_RESP_OK,
    TASK_LOGIN_SECTION1_RESP_ERROR,
    TASK_LOGIN_SECTION2,
    TASK_LOGIN_SECTION2_RESP_OK,
    TASK_LOGIN_SECTION2_RESP_ERROR,

    TASK_REGISTER_USERNAME,
    TASK_REGISTER_SALT,
    TASK_REGISTER_ENCRYPTEDCODE,
    TASK_REGISTER_RESP_OK,
    TASK_REGISTER_ERROR,

    CLIENT_EXIT = 200,
    CLIENT_FORCE_EXIT,
}CmdType;


typedef struct 
{
    int len;//记录内容长度
    CmdType type;//消息类型
    char buff[1000];//记录内容本身
}train_t;

int tcpConnect(const char * ip, unsigned short port);
int recvn(int sockfd, void * buff, int len);
int sendn(int sockfd, const void * buff, int len);

int userLogin(int sockfd);

int parseCommand(const char * input, int len, train_t * pt);

//判断一个字符串是什么命令
int getCommandType(const char * str);
//执行上传文件操作
void putsCommand(int sockfd, train_t * pt);

void getsCommand(int sockfd, train_t * pt);


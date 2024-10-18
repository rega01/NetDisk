#ifndef __USER_H__
#define __USER_H__

#define SALT_LEN 8

typedef enum {
    STATUS_LOGOFF = 0,
    STATUS_LOGIN
}LoginStatus;

typedef struct {
    int sockfd;//套接字文件描述符
    LoginStatus status;//登录状态
    char name[20];//用户名(客户端传递过来的)
    char salt[65];
    char encrypted[100];//从/etc/shadow文件中获取的加密密文
    char pwd_id[128];//用户当前路径
    char mysql_id[20];

}user_info_t;

void loginCheck1(user_info_t * user);
void loginCheck2(user_info_t * user, const char * encrypted);
char* generateSalt();

void Register1(user_info_t * user); 
void Register2(user_info_t * user, const char * encrypted); 

//用户初始化函数
void userInit(user_info_t* user);
#endif


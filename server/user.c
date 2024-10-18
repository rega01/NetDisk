#include "user.h"
#include "thread_pool.h"
#include <stdio.h>
#include <string.h>
#include <shadow.h>
#include "global.h"
//extern HashTable ht;
//extern MYSQL * mysql;

static void get_setting(char *setting,char *passwd)
{
    int i,j;
    //取出salt,i 记录密码字符下标，j记录$出现次数
    for(i = 0,j = 0; passwd[i] && j != 4; ++i) {
        if(passwd[i] == '$')
            ++j;
    }
    strncpy(setting, passwd, i);
}

void loginCheck1(user_info_t * user)
{
    printf("loginCheck1.\n");
    train_t t;
    int ret;
    memset(&t, 0, sizeof(t));
    char query[256]={[0]='\0'};
    printf("logcheck1= %s\n",user->name);;
    sprintf( query, 
             "SELECT id FROM USER_INFO WHERE username = '%s';"
             ,user->name);
    //printf("query = %s",query);;
    bool empty = isEmpty(mysql,query);
    if(empty) {// 用户不存在的情况下
        puts("register");
        t.len = 0;   
        t.type = TASK_LOGIN_SECTION1_RESP_ERROR;
        ret = sendn(user->sockfd, &t, sizeof(t));
        printf(" send %d bytes.\n", ret);
        return;
    }
    //用户存在的情况下
    memset(query,0,sizeof(query));
    sprintf( query, 
             "SELECT id,salt,cryptpasswd ,path_id "
             "FROM USER_INFO WHERE username = '%s';"
             ,user->name);
    if(isEmpty(mysql,query)){
        printf("%s\n",mysql_error(mysql));
    }
    MYSQL_RES * res= disk_mysql_query(mysql,query);
    MYSQL_ROW row ;
    row  = mysql_fetch_row(res);
    strcpy(user->mysql_id,row[0]);
    strcpy(user->salt,row[1]);
    strcpy(user->encrypted,row[2]);
    strcpy(user->pwd_id,row[3]);

    //保存加密密文
    char setting[100];
    strcpy(setting,row[1]);
    t.len = strlen(setting);
    t.type = TASK_LOGIN_SECTION1_RESP_OK;
    strncpy(t.buff, setting, t.len);
    //发送setting
    //printf("t.len={%d},t.type={%d},t.buff={%s}\n",
    //       t.len,t.type,t.buff);
    ret = sendn(user->sockfd, &t, sizeof(t));
    //printf("check1 send %d bytes.\n", ret);
}

void loginCheck2(user_info_t * user, const char * encrypted)
{
    /* printf("loginCheck2.\n"); */
    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    if(strcmp(user->encrypted, encrypted) == 0) {
        //登录成功
        user->status = STATUS_LOGIN;//更新用户登录成功的状态
        t.type = TASK_LOGIN_SECTION2_RESP_OK;
        t.len = strlen("/server/$ ");// 暂定将 /server/ 作为pwd传递给client 
        strcpy(t.buff, "/server/$ ");
        //strcpy(user->pwd_id,getcwd(NULL,0));
        ret = sendn(user->sockfd, &t, sizeof(t));
        printf("Login success.\n");
    } else {
        //登录失败, 密码错误
        t.type = TASK_LOGIN_SECTION2_RESP_ERROR;
        printf("Login failed.\n");
        ret = sendn(user->sockfd, &t, sizeof(t));
    }
    //printf("check2 send %d bytes.\n", ret);
    return;
}

char* generateSalt(){
    char*saltHead =(char *)"$5$";
    unsigned char saltTail[SALT_LEN] = {'\0'};
    if(RAND_bytes(saltTail,SALT_LEN) !=1){
        printf("Fail generate salt\n");
        return NULL;
    }
    char* salt =(char *)calloc(sizeof(saltHead)+SALT_LEN,sizeof(char));
    strcat(salt,saltHead);
    for(int i=0;i<SALT_LEN;i++){
        sprintf(salt,"%s%02x",salt,saltTail[i]);
    }
    return salt;
}

void Register1(user_info_t * user){
    char * salt = generateSalt();

    strcpy(user->salt,salt);
    //puts(user->salt);
    train_t t;
    memset(&t,0,sizeof(train_t));
    t.type=TASK_REGISTER_SALT;
    strcpy(t.buff,salt);
    //puts(t.buff);
    t.len = strlen(t.buff);
    sendn(user->sockfd,&t,sizeof(t));
    free(salt);
    return;
}

void Register2(user_info_t * user, const char * encrypted){
    //todo store the encrypedcode
    strcpy(user->encrypted,encrypted) ;

    userInit(user);
    train_t t;
    memset(&t,0,sizeof(t));
    t.type = TASK_REGISTER_RESP_OK;
    sendn(user->sockfd,&t,sizeof(t));
}

void userInit(user_info_t* user){
    char query[QUERY_LEN]={[0]='\0'};

    //printf("user->name=%s\n",user->name);
    //1、插入新用户
    sprintf( query, 
             "INSERT INTO USER_INFO(username,salt,cryptpasswd)"
             "VALUES('%s','%s','%s');"
             ,user->name,user->salt,user->encrypted);
    disk_mysql_insert(mysql,query);

    memset(query,0,sizeof(query));
    sprintf( query, 
             "SELECT id FROM USER_INFO WHERE username = '%s'"
             ,user->name);
    MYSQL_RES * res = disk_mysql_query(mysql,query);
    MYSQL_ROW row;
    row = mysql_fetch_row(res);
    strcpy(user->mysql_id,row[0]);
    printf("user : %s mysql_id %s\n",user->name,user->mysql_id);

    //2、为用户创建目录
    memset(query,0,sizeof(query));
    char path[1024]= {'\0'};
    sprintf(path,"/%s",user->name);
    strcpy(user->pwd_id,path);
    //创建虚拟表插入语句
    sprintf(query,"INSERT INTO Virtual_File_Table"
            "(parent_id,filename,owner_id,hash,filesize,type,path)"
            "VALUES('1','%s',(SELECT id FROM USER_INFO WHERE username = '%s')"
            ",'0','0','d','%s')",
            user->name,user->name,user->pwd_id);
    disk_mysql_insert(mysql,query);

    //3、更新用户表的当前目录
    memset(query,0,sizeof(query));
    sprintf(path,"/%s",user->name);
    strcpy(user->pwd_id,path);
    //创建虚拟表插入语句
    sprintf(query,"UPDATE USER_INFO SET path_id = "
            " (SELECT id FROM Virtual_File_Table WHERE owner_id = '%s') "
            "WHERE username = '%s'"
            ,user->mysql_id,user->name);
    disk_mysql_insert(mysql,query);
}



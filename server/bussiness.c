#include "thread_pool.h"
#include "global.h"
//外部变量(userList是在main.c文件中定义的)
//主线程调用:处理客户端发过来的消息
void handleMessage(int sockfd, int epfd, task_queue_t * que)
{
    //消息格式：cmd content
    //1.1 获取消息长度
    train_t t;
    memset(&t,0,sizeof(t));
    int ret = recvn(sockfd, &t, sizeof(t));
    //printf( "message:t.len={%d},t.type={%d},t.buff={%s}\n",
    //        t.len,t.type,t.buff);
    int length = t.len;
    if(ret == 0 || t.type == CLIENT_EXIT) {
        goto end;
    }
    {
        //1.2 获取消息类型
        int cmdType = t.type;

        task_t *ptask = (task_t*)calloc(1, sizeof(task_t));
        ptask->peerfd = sockfd;
        ptask->epfd = epfd;
        ptask->type=(CmdType) cmdType;
        strcpy(ptask->data,t.buff);
        //printf("ptaskdata = %s\n",ptask->data);
        if(length > 0) {
            if(ret > 0) {
                //往线程池中添加任务
                if(ptask->type == CMD_TYPE_PUTS) {
                    //是上传文件任务，就暂时先从epoll中删除监听
                    delEpollReadfd(epfd, sockfd);
                }if(ptask->type == CMD_TYPE_GETS) {
                    //是下载文件任务，就暂时先从epoll中删除监听
                    delEpollReadfd(epfd, sockfd);
                }
                taskEnque(que, ptask);
            }
        } else if(length == 0){
            taskEnque(que, ptask);
        }}
end:
    if(ret == 0|| t.type == CLIENT_EXIT) {//连接断开的情况
        ListNode * pNode = userList;
        user_info_t * user = NULL;
        while(pNode != NULL) {
            user_info_t * user_temp = (user_info_t *)pNode->val;
            if(user_temp->sockfd == sockfd) {
                user = user_temp;
                break;
            }
            pNode = pNode ->next;
        }
        char*timeMessage =getCurrentTime();
        char logMessage[1024] ={'\0'};
        sprintf(logMessage,"%s :user(%s) exit\n",timeMessage,user->name);
        write(logfd,logMessage,strlen(logMessage));
        free(timeMessage);

        printf("\nconn %d is closed.\n", sockfd);
        delEpollReadfd(epfd, sockfd);
        close(sockfd);
        deleteNode2(&userList, sockfd);//删除用户信息
    }
}

//注意：此函数可以根据实际的业务逻辑，进行相应的扩展
//子线程调用
void doTask(task_t * task)
{
    assert(task);
    ListNode * pNode = userList;
    user_info_t * user = NULL;
    while(pNode != NULL) {
        user_info_t * user_temp = (user_info_t *)pNode->val;
        if(user_temp->sockfd == task->peerfd) {
            user = user_temp;
            break;
        }
        pNode = pNode ->next;
    }
    char*timeMessage =getCurrentTime();
    char logMessage[1024] ={'\0'};
    sprintf(logMessage,"%s:user(%s) execute CMD{%d} parameter{%s}\n"
            ,timeMessage,user->name,task->type,task->data);
    write(logfd,logMessage,strlen(logMessage));
    free(timeMessage);

    switch(task->type) {
    case CMD_TYPE_PWD:  
        pwdCommand(task,user);   break;
    case CMD_TYPE_CD:
        cdCommand(task,user);    break;
    case CMD_TYPE_LS:
        lsCommand(task,user);    break;
    case CMD_TYPE_MKDIR:
        mkdirCommand(task,user);  break;
    case CMD_TYPE_RMDIR:
        rmdirCommand(task,user);  break;
    case CMD_TYPE_RMFILE:
        rmfileCommand(task,user); break;
    case CMD_TYPE_NOTCMD:
        notCommand(task,user);   break;
    case CMD_TYPE_PUTS:
        putsCommand(task,user);   break;
    case CMD_TYPE_GETS:
        getsCommand(task,user);   break;
    case TASK_LOGIN_SECTION1:
        userLoginCheck1(task); break;
    case TASK_LOGIN_SECTION2:
        userLoginCheck2(task); break;
    case TASK_REGISTER_USERNAME:
        userRegister1(task); break;
    case TASK_REGISTER_ENCRYPTEDCODE:
        userRegister2(task); break;
    }
}

//每一个具体任务的执行，交给一个成员来实现


void notCommand(task_t * task,user_info_t * user)
{
    train_t t;
    memset(&t,0,sizeof(t));
    strcpy(t.buff,"wrong cmd");
    t.len = strlen(t.buff);
    sendn(task->peerfd,&t,sizeof(t));
    printf("execute not command.\n");
}



void userLoginCheck1(task_t * task) {
    printf("userLoginCheck1.\n");
    ListNode * pNode = userList;
    while(pNode != NULL) {
        user_info_t * user = (user_info_t *)pNode->val;
        if(user->sockfd == task->peerfd) {
            //拷贝用户名
            strcpy(user->name, task->data);
            loginCheck1(user);
            return;
        }
        pNode = pNode->next;
    }
}

void userLoginCheck2(task_t * task) {
    printf("userLoginCheck2.\n");
    ListNode * pNode = userList;
    while(pNode != NULL) {
        user_info_t * user = (user_info_t *)pNode->val;
        if(user->sockfd == task->peerfd) {
            //拷贝加密密文
            loginCheck2(user, task->data);
            return;
        }
        pNode = pNode->next;
    }
}
void userRegister1(task_t * task){
    printf("userRegister1:generateSalt\n");
    ListNode * pNode = userList;
    while(pNode != NULL) {
        user_info_t * user = (user_info_t *)pNode->val;
        if(user->sockfd == task->peerfd) {
            //拷贝用户名
            strcpy(user->name, task->data);
            Register1(user);
            return;
        }
        pNode = pNode->next;
    }
}
void userRegister2(task_t * task){
    printf("userRegister2:store encryptedCode\n");
    ListNode * pNode = userList;
    while(pNode != NULL) {
        user_info_t * user = (user_info_t *)pNode->val;
        if(user->sockfd == task->peerfd) {
            Register2(user,task->data);
            return;
        }
        pNode = pNode->next;
    }
}

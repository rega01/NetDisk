#include <stdio.h>
#include <time.h>
#include <stdlib.h>

char * getCurrentTime(void);
//int main()
//{
//    char *currentTime = getCurrentTime();
//    printf("curtn time %s\n",currentTime);
//    return 0;
//}


char * getCurrentTime(void){
     char * buff =(char *)calloc(1, 20*sizeof(char));  // 用于存储格式化的>
     time_t t;
     struct tm *tm_info;

     // 获取当前时间
     time( &t);

     // 将时间转换为本地时间
     tm_info = localtime(&t);

     // 格式化时间为"YYYY-MM-DD HH:MM:SS"的形式
     strftime(buff, 20,"%Y-%m-%d %H:%M:%S",tm_info);
     return buff;
 }


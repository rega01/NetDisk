#include "sha256.h"

char * sha256_file(const char * filename){
    SHA256_CTX * ctx= (SHA256_CTX*)calloc(1,sizeof(SHA256_CTX));
    int ret = SHA256_Init(ctx);
    int fd = open(filename,O_RDONLY);
    struct stat st;
    fstat(fd,&st);
    unsigned long totalSize = st.st_size;
    char buff[1280]={0};
    unsigned long readSize = totalSize;
    puts("Calculating sha256");
    while(readSize > 0){
        int readnum = readSize > sizeof(buff) ? sizeof(buff):readSize;
        int ret = read(fd,buff,readnum);
        SHA256_Update(ctx,buff,ret);
        readSize -= ret;
    }
    puts("Calculate Finish");
    unsigned char md[SHA256_DIGEST_LENGTH] = {0};
    ret = SHA256_Final(md,ctx);
    char*hash=(char*)calloc(65,sizeof(char));
    store_hash(md,hash);
    return hash;
}

void store_hash(unsigned char md[SHA256_DIGEST_LENGTH],char hash[65]) {
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hash+i*2,"%02x",md[i]);
    }
    return ;
}
// USAGE :
//int main()
//{
//    const char * filename ="Makefile";
//    char *hash =NULL;
//    hash = sha256_file(filename);
//    puts(hash);
//    return 0;
//}

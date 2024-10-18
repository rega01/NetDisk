#ifndef __SHA256_H__
#define __SHA256_H__
#include<openssl/sha.h>
#include <stdio.h>
#include <unistd.h>
#include "str_util.h"
#include <crypt.h>

char * sha256_file(const char * filename);

void store_hash(unsigned char md[SHA256_DIGEST_LENGTH],char hash[65]);







#endif

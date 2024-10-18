SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c, %.o, $(SRCS))
BIN:=client
CC:=gcc
FLAGS:=-g -w  -lcrypt -lcrypto

$(BIN):$(OBJS)
	$(CC) -o $@ $^ $(FLAGS) 
%.o:%.c
	$(CC) -c $^ -o $@ $(FLAGS)
.PHONY :clean  
clean:
	rm -rf $(OBJS) $(BIN)


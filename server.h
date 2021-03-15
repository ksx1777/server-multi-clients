#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <err.h>
#include <time.h>
#include <pthread.h>

#define TRUE  0x1
#define FALSE 0x0

typedef struct{
	char *name;
	char *help;	
} new_arg_list;

typedef struct{
	char *ip;
	short port;
	int max_conn;
} new_socket_conf;

int slen(char *s, char null_chr);
int scmp(char *s1, char *s2);
int stoi(char s[]);
char *get_argument(int argc, char **arg_value, char *arg_name);
int socket_bind(char *ip, short port, int queue_length);
int init_new_conn(int s, int conn_num);
void help(void);
void *mset(void *t, int v, size_t len);

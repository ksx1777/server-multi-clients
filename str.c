#include "server.h"

int slen(char *s, char null_chr){
	for(int i = 0; ;i++) if (s[i] == null_chr) return i;
}

int scmp(char *s1, char *s2){
	if(slen(s1, 0x0) != slen(s2, 0x0)) return FALSE;
	for(int i = 0; i < slen(s1, 0x0) ;i++) if(s1[i] != s2[i]) return FALSE;
	return TRUE;
}

int stoi(char s[]){
	int res = 0; int sign = 1; int i = 0;
	if(s[0] == '-') {
		sign = -1;
		i++;
	} for (; s[i] != '\0'; ++i) res = res * 10 + s[i] - '0';
	return sign*res;
}

void *mset(void *t, int v, size_t len){
	unsigned char *ptr = t;
	while( len-- > 0) *ptr++ = v;
	return t;
}

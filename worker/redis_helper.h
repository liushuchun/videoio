#ifndef __REDIS_HELPER__
#define __REDIS_HELPER__
#include <stdio.h>
#include <hircluster.h>

int redis_connect(redisClusterContext *cc, char* redis_address);
void redis_disconnect(redisClusterContext *cc);
int redis_get(redisClusterContext *cc, char* key, char* value);
int redis_set(redisClusterContext *cc, char* key, char* value);

#endif
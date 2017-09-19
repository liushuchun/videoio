
#include "redis_helper.h"

int redis_connect(redisClusterContext *cc, char* redis_address)
{
    cc = redisClusterContextInit();
    redisClusterSetOptionAddNodes(cc, redis_address);
    redisClusterConnect2(cc);
    if(cc == NULL || cc->err)
    {
        printf("connect error : %s\n", cc == NULL ? "NULL" : cc->errstr);
        return -1;
    }
    return 0;
}

void redis_disconnect(redisClusterContext *cc)
{
    redisClusterFree(cc);
}

int redis_set(redisClusterContext *cc, char* key, char* value)
{
    redisReply *reply = (redisReply*)redisClusterCommand(cc, "set %s %s", key, value);
    if(reply == NULL)
    {
        printf("reply is null[%s]\n", cc->errstr);
        redisClusterFree(cc);
        return -1;
    }
    freeReplyObject(reply);
    return 0;
}

int redis_get(redisClusterContext *cc, char* key, char* value)
{
    redisReply *reply = (redisReply*)redisClusterCommand(cc, "get %s %s", key, value);
    if(reply == NULL)
    {
        printf("reply is null[%s]\n", cc->errstr);
        redisClusterFree(cc);
        return -1;
    }
    freeReplyObject(reply);
    return 0;
}
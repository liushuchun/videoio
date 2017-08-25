#include "redis_helper.h"


int main()
{
    char *key="key-a";
    char *field="field-1";
    char *key1="key1";
    char *value1="value-1";
    char *key2="key1";
    char *value2="value-2";
    redisClusterContext *cc;

    int iRet = redis_connect(cc, "127.0.0.1:34501,127.0.0.1:34502,127.0.0.1:34503");
    if(iRet!=0)
    {
        printf("connect error");
        return -1;
    }

    do{
		iRet = redis_set(cc, key1, value1);
	    if(iRet!=0)
	    {
	        printf("redis_set error");
	        return -1;
	    }
		iRet = redis_get(cc, key1, value2);
	    if(iRet!=0)
	    {
	        printf("redis_get error");
	        return -1;
	    }
    }while(0);

    redis_disconnect(cc);

    return 0;
}

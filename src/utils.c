#include <string.h>
#include "utils.h"

int GridType_setRedisString(RedisModuleString** source, char** destination)
{
    if (*source)
    {
        size_t len;
        const char* str = RedisModule_StringPtrLen(*source, &len);
        if (len == 0)
        {
            *destination = NULL;
        }
        else
        {
            *destination = RedisModule_Alloc(len + 1);
            if (!*destination)
                return REDISMODULE_ERR;
            memcpy((void*)*destination, (void*)str, len + 1);
        }
    }
    else
    {
        *destination = NULL;
    }

    return REDISMODULE_OK;
}

int GridType_resetRedisString(RedisModuleString** source, char** destination)
{
    if (*destination)
        RedisModule_Free(*destination);

    return GridType_setRedisString(source, destination);
}

int GridType_getRangeValue(RedisModuleCtx *ctx, RedisModuleString **argv, int argi, long long max_value, long long* range_value, const char* type_errmsg, const char* bounds_errmsg)
{
    if (RedisModule_StringToLongLong(argv[argi], range_value) != REDISMODULE_OK)
    {
        RedisModule_ReplyWithError(ctx, type_errmsg);
        return REDISMODULE_ERR;
    }
    if (*range_value < 0)
        *range_value = max_value + *range_value;
    if (*range_value < 0 || *range_value >= max_value)
    {
        RedisModule_ReplyWithError(ctx, bounds_errmsg);
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

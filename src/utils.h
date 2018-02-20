#ifndef __UTILS_H
#define __UTILS_H

#include "redismodule.h"

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a <= _b ? _a : _b; })

int GridType_setRedisString(RedisModuleString** source, char** destination);
int GridType_resetRedisString(RedisModuleString** source, char** destination);
int GridType_getRangeValue(RedisModuleCtx *ctx, RedisModuleString **argv, int argi, long long max_value, long long* range_value, const char* type_errmsg, const char* bounds_errmsg);

#endif //  __UTILS_H
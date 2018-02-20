/* Copyright (c) 2018, Rob Blackbourn
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

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

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
#include "array_grid.h"

void ArrayGrid_clearRedisStrings(char **start, char **end)
{
    for (char **p = start; p < end; ++p)
    {
        if (*p)
        {
            RedisModule_Free(*p);
            *p = NULL;
        }
    }
}

int ArrayGrid_copyRedisStrings(RedisModuleString** source, char **start, char **end)
{
    for (char **p = start; p < end; ++p, ++source)
    {
        if (GridType_setRedisString(source, p) != REDISMODULE_OK)
        {
            ArrayGrid_clearRedisStrings(start, p);
            return REDISMODULE_ERR;
        }
    }

    return REDISMODULE_OK;
}

char **ArrayGrid_copyAndAllocRedisStrings(RedisModuleString **source, size_t len)
{
    char **destination = (char**)RedisModule_Alloc(sizeof(char*) * len);
    if (!destination)
        return NULL;

    if (ArrayGrid_copyRedisStrings(source, destination, destination + len) != REDISMODULE_OK)
    {
        RedisModule_Free(destination);
        return NULL;
    }

    return destination;
}

struct ArrayGrid *ArrayGrid_createObject(size_t rows, size_t columns, RedisModuleString **source)
{
    struct ArrayGrid *o = (struct ArrayGrid *)RedisModule_Alloc(sizeof(struct ArrayGrid));
    if (!o)
        return NULL;
        
    size_t len = rows * columns;
    o->start = ArrayGrid_copyAndAllocRedisStrings(source, len);
    if (!o->start)
    {
        RedisModule_Free(o);
        return NULL;
    }

    o->rows = rows;
    o->columns = columns;
    o->end = o->start + len;

    return o;
}

void ArrayGrid_releaseObject(struct ArrayGrid *o)
{
    ArrayGrid_clearRedisStrings(o->start, o->end);
    RedisModule_Free(o->start);
    RedisModule_Free(o);
}

int ArrayGrid_setObject(struct ArrayGrid *o, long long row_start, long long row_end, long long column_start, long long column_end, RedisModuleString **source)
{
    long long row_sign = row_start < row_end ? 1 : -1;
    long long column_sign = column_start < column_end ? 1 : -1;

    for (long long r = row_start; r != row_end + row_sign; r += row_sign)
    {
        char **p = o->start + r * o->columns + column_start;

        for (long long c = column_start; c != column_end + column_sign; c += column_sign, p += column_sign, ++source)
        {
            if (GridType_resetRedisString(source, p) != REDISMODULE_OK)
                return REDISMODULE_ERR;
        }
    }

    return REDISMODULE_OK;
}

int ArrayGrid_resizeAndCopyObject(struct ArrayGrid *o, size_t rows, size_t columns)
{
    if (rows == o->rows && columns == o->columns)
        return REDISMODULE_OK;

    size_t len = rows * columns;
    char **start = (char**)RedisModule_Alloc(sizeof(char*) * len);
    if (!start)
        return REDISMODULE_ERR;
    char **end = start + len;
    
    // Copy from the old to the new
    size_t min_rows = min(rows, o->rows);
    size_t min_columns = min(columns, o->columns);
    char** dest_end = start + min_rows * columns;
    size_t trim_count = min_columns * sizeof(char*);
    for (char **dest = start, **source = o->start; dest < dest_end; dest += columns, source += o->columns)
    {
        memcpy(dest, source, trim_count);
    }

    if (rows < o->rows)
        ArrayGrid_clearRedisStrings(o->start + rows * o->columns, o-> end);

    if (columns < o->columns)
    {
        char **trim_end = o->start + min_rows * o->columns;
        for (char **p1 = o->start + columns, **p2 = o->start + o->columns; p1 < trim_end; p1 += o->columns, p2 += columns)
        {
            ArrayGrid_clearRedisStrings(p1, p2);
        }
    }

    // Null out any new rows
    if (rows > o->rows)
    {
        memset(start + o->rows * columns, 0, (rows - o->rows) * columns * sizeof(char*));
    }

    // Null out the end of columns
    if (columns > o->columns)
    {
        size_t count = (columns - o->columns) * sizeof(char*);
        for (char **p = start + o->columns; p < end; p += columns)
        {
            memset(p, 0, count);
        }
    }

    RedisModule_Free(o->start);
    o->rows = rows;
    o->columns = columns;
    o->start = start;
    o->end = end;

    return REDISMODULE_OK;
}

int ArrayGrid_resizeAndReplaceObject(struct ArrayGrid *o, size_t rows, size_t columns, RedisModuleString **source)
{
    if (rows == o->rows && columns == o->columns)
        return ArrayGrid_setObject(o, 0, rows - 1, 0, columns - 1, source);

    size_t len = rows * columns;

    char **values = (char**)RedisModule_Alloc(sizeof(char*) * len);
    if (!values)
        return REDISMODULE_ERR;

    if (ArrayGrid_copyRedisStrings(source, values, values + len) != REDISMODULE_OK)
    {
        RedisModule_Free(values);
        return REDISMODULE_ERR;
    }

    ArrayGrid_clearRedisStrings(o->start, o->end);
    RedisModule_Free(o->start);

    o->rows = rows;
    o->columns = columns;
    o->start = values;
    o->end = values + len;

    return REDISMODULE_OK;
}

void ArrayGrid_rangeObject(RedisModuleCtx *ctx, struct ArrayGrid *o, long long row_start, long long row_end, long long column_start, long long column_end)
{
    long long rows = 1 + (max(row_start, row_end) - min(row_start, row_end));
    long long columns = 1 + (max(column_start, column_end) - min(column_start, column_end));
    RedisModule_ReplyWithArray(ctx, (long)(rows * columns));

    long long row_sign = row_start < row_end ? 1 : -1;
    long long column_sign = column_start < column_end ? 1 : -1;

    for (long long r = row_start; r != row_end + row_sign; r += row_sign)
    {
        char **p = o->start + r * o->columns + column_start;

        for (long long c = column_start; c != column_end + column_sign; c += column_sign, p += column_sign)
        {
            RedisModule_ReplyWithSimpleString(ctx, *p ? *p : "");
        }
    }
}

int ArrayGrid_getRangeValues(RedisModuleCtx *ctx, struct ArrayGrid *o, RedisModuleString **argv, long long *row_start, long long *row_end, long long *column_start, long long *column_end)
{
    int are_ranges_ok  =
        GridType_getRangeValue(ctx, argv, 2, (long long)o->rows, row_start, "Start row must be an integer", "Start row outside the bounds of the grid") == REDISMODULE_OK &&
        GridType_getRangeValue(ctx, argv, 3, (long long)o->rows, row_end, "End row must be an integer", "End row outside the bounds of the grid") == REDISMODULE_OK &&
        GridType_getRangeValue(ctx, argv, 4, (long long)o->columns, column_start, "Start column must be an integer", "Start column outside the bounds of the grid") == REDISMODULE_OK &&
        GridType_getRangeValue(ctx, argv, 5, (long long)o->columns, column_end, "End column must be an integer", "End column outside the bounds of the grid") == REDISMODULE_OK;
    return are_ranges_ok ? REDISMODULE_OK : REDISMODULE_ERR;
}

int ArrayGrid_getShape(RedisModuleCtx *ctx, struct ArrayGrid *o)
{
    RedisModule_ReplyWithArray(ctx, (long)2);
    RedisModule_ReplyWithLongLong(ctx, (long long)o->rows);
    RedisModule_ReplyWithLongLong(ctx, (long long)o->columns);
    return REDISMODULE_OK;
}

int ArrayGrid_dump(RedisModuleCtx *ctx, struct ArrayGrid *o)
{
    RedisModule_ReplyWithArray(ctx, (long) (2 + o->rows * o->columns));

    RedisModule_ReplyWithLongLong(ctx, (long long)o->rows);
    RedisModule_ReplyWithLongLong(ctx, (long long)o->columns);

    for (char **p = o->start; p < o->end; ++p)
    {
        if (*p)
            RedisModule_ReplyWithSimpleString(ctx, *p);
        else
            RedisModule_ReplyWithNull(ctx);
    }
    
    return REDISMODULE_OK;
}

void ArrayGrid_rdbSave(RedisModuleIO *rdb, struct ArrayGrid *o)
{
    RedisModule_SaveUnsigned(rdb, (uint64_t)o->rows);
    RedisModule_SaveUnsigned(rdb, (uint64_t)o->columns);
    for (char **p = o->start; p < o->end; ++p)
    {
        if (*p)
            RedisModule_SaveStringBuffer(rdb, *p, strlen(*p) + 1);
        else
            RedisModule_SaveStringBuffer(rdb, "", 1);
    }
}

struct ArrayGrid *ArrayGrid_rdbLoad(RedisModuleIO *rdb)
{
    size_t rows = (size_t) RedisModule_LoadUnsigned(rdb);
    size_t columns = (size_t) RedisModule_LoadUnsigned(rdb);
    size_t len = rows * columns;
    char **start = (char**) RedisModule_Alloc(sizeof(char*) * len);
    char **end = start + len;
    for (char **p = start; p < end; ++p)
    {
        size_t l;
        *p = RedisModule_LoadStringBuffer(rdb, &l);
    }

    struct ArrayGrid *o = (struct ArrayGrid*) RedisModule_Alloc(sizeof(struct ArrayGrid));
    o->rows = rows;
    o->columns = columns;
    o->start = start;
    o->end = end;
    return o;
}

void ArrayGrid_aofRewrite(RedisModuleIO *aof, RedisModuleString *key, struct ArrayGrid *o) 
{
    RedisModuleCtx *ctx = RedisModule_GetContextFromIO(aof);

    size_t len = o->columns * o->rows;
    RedisModuleString **start = (RedisModuleString**)RedisModule_Alloc(sizeof(RedisModuleString*) * len);
    RedisModuleString **p = start, **end = start + len;

    for (char **s = o->start; s < o->end; ++s, ++p)
    {
        if (*s)
            *p = RedisModule_CreateString(ctx, *s, strlen(*s));
        else
            *p = RedisModule_CreateString(ctx, "", 0);
    }

    RedisModule_EmitAOF(aof, "GRID.DIM","sllv", key, (long long)o->rows, (long long)o->columns, start, len);

    for (p = start; p < end; ++p)
        RedisModule_FreeString(ctx, *p);
    RedisModule_Free(start);
}

size_t ArrayGrid_memUsage(const struct ArrayGrid *o) 
{
    size_t usage = sizeof(*o) + sizeof(char**) * o->rows * o->columns;
    for (char **p = o->start; p < o->end; ++p)
        usage += *p ? strlen(*p) + 1 : 0;
    return usage;
}

void ArrayGrid_digest(RedisModuleDigest *md, struct ArrayGrid *o)
{
    RedisModule_DigestAddLongLong(md, o->rows);
    RedisModule_DigestAddLongLong(md, o->columns);
    for (char **p = o->start; p < o->end; ++p)
        RedisModule_DigestAddStringBuffer(md, (unsigned char*)*p, strlen((const char*)*p));
    RedisModule_DigestEndSequence(md);
}

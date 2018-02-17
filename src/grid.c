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

#include "redismodule.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define GRIDMODULE_ERRORMSG__ROWNOTINT "WRONGTYPE Row should be an int"
#define GRIDMODULE_ERRORMSG__COLNOTINT "WRONGTYPE Column should be an int"

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a <= _b ? _a : _b; })

static RedisModuleType *GridType;

struct GridTypeObject 
{
    size_t rows;
    size_t columns;
    char** start;
    char** end;
};

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

void GridType_clearRedisStrings(char **start, char **end)
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

int GridType_copyRedisStrings(RedisModuleString** source, char** start, char** end)
{
    for (char** p = start; p < end; ++p, ++source)
    {
        if (GridType_setRedisString(source, p) != REDISMODULE_OK)
        {
            GridType_clearRedisStrings(start, p);
            return REDISMODULE_ERR;
        }
    }

    return REDISMODULE_OK;
}

char** GridType_copyAndAllocRedisStrings(RedisModuleString** source, size_t len)
{
    char** destination = (char**)RedisModule_Alloc(sizeof(char*) * len);
    if (!destination)
        return NULL;

    if (GridType_copyRedisStrings(source, destination, destination + len) != REDISMODULE_OK)
    {
        RedisModule_Free(destination);
        return NULL;
    }

    return destination;
}

struct GridTypeObject *GridType_createObject(size_t rows, size_t columns, RedisModuleString** source) 
{
    struct GridTypeObject *o;
    o = RedisModule_Alloc(sizeof(struct GridTypeObject));
    if (!o)
        return NULL;
        
    size_t len = rows * columns;
    o->start = GridType_copyAndAllocRedisStrings(source, len);
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

void GridType_releaseObject(struct GridTypeObject *o) 
{
    GridType_clearRedisStrings(o->start, o->end);
    RedisModule_Free(o->start);
    RedisModule_Free(o);
}

int GridType_setObject(struct GridTypeObject *o, long long row_start, long long row_end, long long column_start, long long column_end, RedisModuleString **source)
{
    long long row_sign = row_start < row_end ? 1 : -1;
    long long column_sign = column_start < column_end ? 1 : -1;

    for (long long r = row_start; r != row_end + row_sign; r += row_sign)
    {
        char** p = o->start + r * o->columns + column_start;

        for (long long c = column_start; c != column_end + column_sign; c += column_sign, p += column_sign, ++source)
        {
            if (GridType_resetRedisString(source, p) != REDISMODULE_OK)
                return REDISMODULE_ERR;
        }
    }

    return REDISMODULE_OK;
}

int GridType_dimObject(RedisModuleKey *key, size_t rows, size_t columns, RedisModuleString **source)
{
    struct GridTypeObject *o = GridType_createObject((size_t)rows, (size_t)columns, source);
    RedisModule_ModuleTypeSetValue(key, GridType, o);
    return REDISMODULE_OK;
}

int GridType_resizeAndCopyObject(struct GridTypeObject *o, size_t rows, size_t columns)
{
    if (rows == o->rows && columns == o->columns)
        return REDISMODULE_OK;

    size_t len = rows * columns;
    char **start = (char**)RedisModule_Alloc(sizeof(char*) * len);
    if (!start)
        return REDISMODULE_ERR;
    char**end = start + len;
    
    // Copy from the old to the new
    size_t min_rows = min(rows, o->rows);
    size_t min_columns = min(columns, o->columns);
    char** dest_end = start + min_rows * columns;
    size_t trim_count = min_columns * sizeof(char*);
    for (char** dest = start, **source = o->start; dest < dest_end; dest += columns, source += o->columns)
    {
        memcpy(dest, source, trim_count);
    }

    if (rows < o->rows)
        GridType_clearRedisStrings(o->start + rows * o->columns, o-> end);

    if (columns < o->columns)
    {
        char** trim_end = o->start + min_rows * o->columns;
        for (char** p1 = o->start + columns, **p2 = o->start + o->columns; p1 < trim_end; p1 += o->columns, p2 += columns)
        {
            GridType_clearRedisStrings(p1, p2);
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
        for (char** p = start + o->columns; p < end; p += columns)
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

int GridType_resizeAndReplaceObject(struct GridTypeObject *o, size_t rows, size_t columns, RedisModuleString **source)
{
    if (rows == o->rows && columns == o->columns)
        return GridType_setObject(o, 0, rows - 1, 0, columns - 1, source);

    size_t len = rows * columns;

    char** values = (char**)RedisModule_Alloc(sizeof(char*) * len);
    if (!values)
        return REDISMODULE_ERR;

    if (GridType_copyRedisStrings(source, values, values + len) != REDISMODULE_OK)
    {
        RedisModule_Free(values);
        return REDISMODULE_ERR;
    }

    GridType_clearRedisStrings(o->start, o->end);
    RedisModule_Free(o->start);

    o->rows = rows;
    o->columns = columns;
    o->start = values;
    o->end = values + len;

    return REDISMODULE_OK;
}

int GridType_redimObject(RedisModuleKey *key, size_t rows, size_t columns, RedisModuleString **source)
{
    struct GridTypeObject *o = RedisModule_ModuleTypeGetValue(key);

    if (rows == 0 || columns == 0)
        return RedisModule_DeleteKey(key);
    else if (source)
        return GridType_resizeAndReplaceObject(o, rows, columns, source);
    else 
        return GridType_resizeAndCopyObject(o, rows, columns);
}

int GridType_reshapeObject(RedisModuleCtx *ctx, int type, RedisModuleKey *key, size_t rows, size_t columns, RedisModuleString **source)
{
    if (type == REDISMODULE_KEYTYPE_EMPTY)
        return GridType_dimObject(key, rows, columns, source);
    else if (RedisModule_ModuleTypeGetType(key) == GridType)
        return GridType_redimObject(key, rows, columns, source);
    else
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
}

void GridType_rangeObject(RedisModuleCtx *ctx, struct GridTypeObject *o, long long row_start, long long row_end, long long column_start, long long column_end)
{
    long long rows = 1 + (max(row_start, row_end) - min(row_start, row_end));
    long long columns = 1 + (max(column_start, column_end) - min(column_start, column_end));
    RedisModule_ReplyWithArray(ctx, (long)(rows * columns));

    long long row_sign = row_start < row_end ? 1 : -1;
    long long column_sign = column_start < column_end ? 1 : -1;

    for (long long r = row_start; r != row_end + row_sign; r += row_sign)
    {
        char** p = o->start + r * o->columns + column_start;

        for (long long c = column_start; c != column_end + column_sign; c += column_sign, p += column_sign)
        {
            RedisModule_ReplyWithSimpleString(ctx, *p ? *p : "");
        }
    }
}

/* Commands */

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

int GridType_DimCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    // GRID.DIM KEY ROWS COLS [R0-C0, R0-C1,,, ... ]
    if (argc < 4)
        return RedisModule_WrongArity(ctx);

    long long rows;
    if (RedisModule_StringToLongLong(argv[2], &rows) != REDISMODULE_OK)
    {
        return RedisModule_ReplyWithError(ctx, "WRONGTYPE Row should be an int");
    }
    if (rows < 0)
    {
        return RedisModule_ReplyWithError(ctx, "Rows must be gretaer than 0");
    }
    if ((unsigned long long)rows > SIZE_MAX)
    {
        return RedisModule_ReplyWithError(ctx, "Rows to large");
    }

    long long columns;
    if (RedisModule_StringToLongLong(argv[3], &columns) != REDISMODULE_OK)
    {
        return RedisModule_ReplyWithError(ctx, "WRONGTYPE Row should be an int");
    }
    if (columns < 0)
    {
        return RedisModule_ReplyWithError(ctx, "Columns must be grater than 0");
    }
    if ((unsigned long long)columns > SIZE_MAX)
    {
        return RedisModule_ReplyWithError(ctx, "Columns too large");
    }

    long long len = rows * columns;
    if ((unsigned long long)len > SIZE_MAX)
        return RedisModule_ReplyWithError(ctx, "Grid too large");
    
    if (argc > 4 && (argc - 4) != len)
        return RedisModule_ReplyWithError(ctx, "ARGCOUNT Invalid number of values for grid");

    RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ|REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    int status = GridType_reshapeObject(ctx, type, key, (size_t)rows, (size_t)columns, argc - 4 > 0 ? argv + 4 : NULL);
    RedisModule_CloseKey(key);

    RedisModule_ReplyWithSimpleString(ctx, "OK");

    return status;
}

int GridType_SetCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    // GRID.SET KEY ROW-START ROW-END COLUMN-START COLUMN-END ROW0-COL0 ... ROWN-COLN
    if (argc < 6)
        return RedisModule_WrongArity(ctx);

    RedisModuleKey *key = RedisModule_OpenKey(ctx,argv[1], REDISMODULE_READ|REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type == REDISMODULE_KEYTYPE_EMPTY)
        return RedisModule_ReplyWithError(ctx, "Empty key");
    if (type != REDISMODULE_KEYTYPE_MODULE || RedisModule_ModuleTypeGetType(key) != GridType)
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);

    struct GridTypeObject *o = RedisModule_ModuleTypeGetValue(key);

    long long row_start, row_end, column_start, column_end;
    int are_ranges_ok  =
        GridType_getRangeValue(ctx, argv, 2, (long long)o->rows, &row_start, "Start row must be an integer", "Start row outside the bounds of the grid") == REDISMODULE_OK &&
        GridType_getRangeValue(ctx, argv, 3, (long long)o->rows, &row_end, "End row must be an integer", "End row outside the bounds of the grid") == REDISMODULE_OK &&
        GridType_getRangeValue(ctx, argv, 4, (long long)o->columns, &column_start, "Start column must be an integer", "Start column outside the bounds of the grid") == REDISMODULE_OK &&
        GridType_getRangeValue(ctx, argv, 5, (long long)o->columns, &column_end, "End column must be an integer", "End column outside the bounds of the grid") == REDISMODULE_OK;
    if (!are_ranges_ok)
        return REDISMODULE_ERR;

    long long rows = 1 + (max(row_start, row_end) - min(row_start, row_end));
    long long columns = 1 + (max(column_start, column_end) - min(column_start, column_end));
    long long len = rows * columns;

    if (len != argc - 6)
        return RedisModule_ReplyWithError(ctx, "Invalid number of values");

    if (GridType_setObject(o, row_start, row_end, column_start, column_end, argv + 6) != REDISMODULE_OK)
    {
        return RedisModule_ReplyWithError(ctx, "Failed to set one or more items in the grid");
    }

    RedisModule_ReplyWithSimpleString(ctx, "OK");

    return REDISMODULE_OK;
}

int GridType_RangeCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    // GRID.RANGE KEY START-ROW END-ROW SART-COLUMN END-COLUMN
    if (argc < 6)
        return RedisModule_WrongArity(ctx);

    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    RedisModuleKey *key = RedisModule_OpenKey(ctx,argv[1], REDISMODULE_READ|REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type == REDISMODULE_KEYTYPE_EMPTY)
        return RedisModule_ReplyWithError(ctx, "Empty key");
    if (type != REDISMODULE_KEYTYPE_MODULE || RedisModule_ModuleTypeGetType(key) != GridType)
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);

    struct GridTypeObject *o = RedisModule_ModuleTypeGetValue(key);

    long long row_start, row_end, column_start, column_end;
    int are_ranges_ok  =
        GridType_getRangeValue(ctx, argv, 2, (long long)o->rows, &row_start, "Start row must be an integer", "Start row outside the bounds of the grid") == REDISMODULE_OK &&
        GridType_getRangeValue(ctx, argv, 3, (long long)o->rows, &row_end, "End row must be an integer", "End row outside the bounds of the grid") == REDISMODULE_OK &&
        GridType_getRangeValue(ctx, argv, 4, (long long)o->columns, &column_start, "Start column must be an integer", "Start column outside the bounds of the grid") == REDISMODULE_OK &&
        GridType_getRangeValue(ctx, argv, 5, (long long)o->columns, &column_end, "End column must be an integer", "End column outside the bounds of the grid") == REDISMODULE_OK;
    if (!are_ranges_ok)
        return REDISMODULE_ERR;

    GridType_rangeObject(ctx, o, row_start, row_end, column_start, column_end);

    return REDISMODULE_OK;
}

int GridType_ShapeCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    // GRID.SHAPE KEY
    if (argc != 2)
        return RedisModule_WrongArity(ctx);

    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    RedisModuleKey *key = RedisModule_OpenKey(ctx,argv[1], REDISMODULE_READ|REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type == REDISMODULE_KEYTYPE_EMPTY)
        return RedisModule_ReplyWithError(ctx, "Empty key");
    if (type != REDISMODULE_KEYTYPE_MODULE || RedisModule_ModuleTypeGetType(key) != GridType)
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);

    struct GridTypeObject *o = RedisModule_ModuleTypeGetValue(key);

    RedisModule_ReplyWithArray(ctx, (long)2);
    RedisModule_ReplyWithLongLong(ctx, (long long)o->rows);
    RedisModule_ReplyWithLongLong(ctx, (long long)o->columns);

    return REDISMODULE_OK;
}

int GridType_DumpCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    // GRID.DUMP KEY
    if (argc != 2)
        return RedisModule_WrongArity(ctx);

    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    RedisModuleKey *key = RedisModule_OpenKey(ctx,argv[1], REDISMODULE_READ|REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type == REDISMODULE_KEYTYPE_EMPTY)
        return RedisModule_ReplyWithError(ctx, "Empty key");
    if (type != REDISMODULE_KEYTYPE_MODULE || RedisModule_ModuleTypeGetType(key) != GridType)
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);

    struct GridTypeObject *o = RedisModule_ModuleTypeGetValue(key);

    RedisModule_ReplyWithArray(ctx, (long) (2 + o->rows * o->columns));

    RedisModule_ReplyWithLongLong(ctx, (long long)o->rows);
    RedisModule_ReplyWithLongLong(ctx, (long long)o->columns);

    for (char** p = o->start; p < o->end; ++p)
        RedisModule_ReplyWithSimpleString(ctx, *p ? *p : "");

    return REDISMODULE_OK;
}

/* Type Methods */

void GridType_Free(void *value) 
{
    GridType_releaseObject((struct GridTypeObject*)value);
}

void GridType_RdbSave(RedisModuleIO *rdb, void *value) 
{
    struct GridTypeObject *o = value;
    RedisModule_SaveUnsigned(rdb, (uint64_t)o->rows);
    RedisModule_SaveUnsigned(rdb, (uint64_t)o->columns);
    for (char** value = o->start; value < o->end; ++value)
        RedisModule_SaveStringBuffer(rdb, *value, strlen(*value) + 1);
}

void *GridType_RdbLoad(RedisModuleIO *rdb, int encver) 
{
    if (encver != 0)
    {
        /* RedisModule_Log("warning","Can't load data with version %d", encver);*/
        return NULL;
    }

    size_t rows = (size_t) RedisModule_LoadUnsigned(rdb);
    size_t columns = (size_t) RedisModule_LoadUnsigned(rdb);
    size_t len = rows * columns;
    char** values = (char**) RedisModule_Alloc(sizeof(char*)*len);
    char** end = values + len;
    for (char** p = values; p < end; ++p)
    {
        size_t l;
        *p = RedisModule_LoadStringBuffer(rdb, &l);
    }

    struct GridTypeObject *o = (struct GridTypeObject*) RedisModule_Alloc(sizeof(struct GridTypeObject));
    o->rows = rows;
    o->columns = columns;
    o->start = values;
    o->end = end;
    return o;
}

#define AOF_BUFSIZ 100

void GridType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) 
{
    struct GridTypeObject *o = value;
    RedisModule_EmitAOF(aof, "GRID.DIM","sll", key, (long long)o->rows, (long long)o->columns);
    char** p = o->start;
    for (size_t r = 0; r < o->rows; ++r)
    {
        for (size_t c = 0; c < o->columns; ++c, ++p)
        {
            RedisModule_EmitAOF(aof, "GRID.SET","sllllc", key, (long long)r, (long long)r, (long long)c, (long long)c, *p ? *p : "");
        }
    }
}

size_t GridType_MemUsage(const void *value) 
{
    // TODO: Not right.
    const struct GridTypeObject *o = value;
    size_t usage = sizeof(*o);
    for (char** p = o->start; p < o->end; ++p)
        usage += *p ? strlen(*p) + 1 : 0;
    return usage;
}

void GridType_Digest(RedisModuleDigest *md, void *value) 
{
    struct GridTypeObject *o = value;
    RedisModule_DigestAddLongLong(md, o->rows);
    RedisModule_DigestAddLongLong(md, o->columns);
    for (char** p = o->start; p < o->end; ++p)
        RedisModule_DigestAddStringBuffer(md, (unsigned char*)*p, strlen((const char*)*p));
    RedisModule_DigestEndSequence(md);
}

/* Initialisation */

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    REDISMODULE_NOT_USED(argv);
    REDISMODULE_NOT_USED(argc);

    if (RedisModule_Init(ctx, "GRID", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    RedisModuleTypeMethods tm = {
        .version = REDISMODULE_TYPE_METHOD_VERSION,
        .rdb_load = GridType_RdbLoad,
        .rdb_save = GridType_RdbSave,
        .aof_rewrite = GridType_AofRewrite,
        .mem_usage = GridType_MemUsage,
        .free = GridType_Free,
        .digest = GridType_Digest
    };

    GridType = RedisModule_CreateDataType(ctx, "GRID-RTB_", 0, &tm);
    if (GridType == NULL)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "GRID.DIM", GridType_DimCommand, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "GRID.SET", GridType_SetCommand, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "GRID.RANGE", GridType_RangeCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "GRID.SHAPE", GridType_ShapeCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "GRID.DUMP", GridType_DumpCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    return REDISMODULE_OK;
}

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
#include "utils.h"
#include "array_grid.h"
#include "row_grid.h"

#define GRIDMODULE_ERRORMSG__ROWNOTINT "WRONGTYPE Row should be an int"
#define GRIDMODULE_ERRORMSG__COLNOTINT "WRONGTYPE Column should be an int"

#define STORAGE_TYPE_ARRAY 0x01
#define STORAGE_TYPE_ROW 0x02

static RedisModuleType *GridType;

static int current_storage_type = STORAGE_TYPE_ROW;

struct GridTypeObject 
{
    unsigned char storage_type;

    union {
        struct ArrayGrid *array_grid;
        struct RowGrid *row_grid;
    };
};

struct GridTypeObject *GridType_createObject(unsigned char storage_type, size_t rows, size_t columns, RedisModuleString** source) 
{
    struct GridTypeObject *o;
    o = RedisModule_Alloc(sizeof(struct GridTypeObject));
    o->storage_type = storage_type;
    if (storage_type & STORAGE_TYPE_ARRAY)
        o->array_grid = ArrayGrid_createObject(rows, columns, source);
    else
        o->row_grid = RowGrid_createObject(rows, columns, source);
    return o;
}

void GridType_releaseObject(struct GridTypeObject *o) 
{
    if (o->storage_type & STORAGE_TYPE_ARRAY)
        ArrayGrid_releaseObject(o->array_grid);
    else
        RowGrid_releaseObject(o->row_grid);
    RedisModule_Free(o);
}

int GridType_setObject(struct GridTypeObject *o, long long row_start, long long row_end, long long column_start, long long column_end, RedisModuleString **source)
{
    if (o->storage_type == STORAGE_TYPE_ARRAY)
        return ArrayGrid_setObject(o->array_grid, row_start, row_end, column_start, column_end, source);
    else
        return RowGrid_setObject(o->row_grid, row_start, row_end, column_start, column_end, source);
}

int GridType_dimObject(RedisModuleKey *key, unsigned char storage_type, size_t rows, size_t columns, RedisModuleString **source)
{
    struct GridTypeObject *o = GridType_createObject(storage_type, (size_t)rows, (size_t)columns, source);
    RedisModule_ModuleTypeSetValue(key, GridType, o);
    return REDISMODULE_OK;
}

int GridType_resizeAndCopyObject(struct GridTypeObject *o, size_t rows, size_t columns)
{
    if (o->storage_type & STORAGE_TYPE_ARRAY)
        return ArrayGrid_resizeAndCopyObject(o->array_grid, rows, columns);
    else
        return RowGrid_resizeAndCopyObject(o->row_grid, rows, columns);
}

int GridType_resizeAndReplaceObject(struct GridTypeObject *o, size_t rows, size_t columns, RedisModuleString **source)
{
    if (o->storage_type & STORAGE_TYPE_ARRAY)
        return ArrayGrid_resizeAndReplaceObject(o->array_grid, rows, columns, source);
    else
        return RowGrid_resizeAndReplaceObject(o->row_grid, rows, columns, source);
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

int GridType_reshapeObject(RedisModuleCtx *ctx, RedisModuleKey *key, int type, unsigned char storage_type, size_t rows, size_t columns, RedisModuleString **source)
{
    if (type == REDISMODULE_KEYTYPE_EMPTY)
        return GridType_dimObject(key, storage_type, rows, columns, source);
    else if (RedisModule_ModuleTypeGetType(key) == GridType)
        return GridType_redimObject(key, rows, columns, source);
    else
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
}

void GridType_rangeObject(RedisModuleCtx *ctx, struct GridTypeObject *o, long long row_start, long long row_end, long long column_start, long long column_end)
{
    if (o->storage_type & STORAGE_TYPE_ARRAY)
        ArrayGrid_rangeObject(ctx, o->array_grid, row_start, row_end, column_start, column_end);
    else
        RowGrid_rangeObject(ctx, o->row_grid, row_start, row_end, column_start, column_end);
}

int GridType_dump(RedisModuleCtx *ctx, struct GridTypeObject* o)
{
    if (o->storage_type & STORAGE_TYPE_ARRAY)
        return ArrayGrid_dump(ctx, o->array_grid);
    else
        return RowGrid_dump(ctx, o->row_grid);
}

/* Commands */

int GridType_getRangeValues(RedisModuleCtx *ctx, struct GridTypeObject *o, RedisModuleString **argv, long long *row_start, long long *row_end, long long *column_start, long long *column_end)
{
    if (o->storage_type & STORAGE_TYPE_ARRAY)
        return ArrayGrid_getRangeValues(ctx, o->array_grid, argv, row_start, row_end, column_start, column_end);
    else
        return RowGrid_getRangeValues(ctx, o->row_grid, argv, row_start, row_end, column_start, column_end);
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
    int status = GridType_reshapeObject(ctx, key, type, current_storage_type, (size_t)rows, (size_t)columns, argc - 4 > 0 ? argv + 4 : NULL);
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
    int are_ranges_ok  = GridType_getRangeValues(ctx, o, argv + 2, &row_start, &row_end, &column_start, &column_end);
    if (are_ranges_ok != REDISMODULE_OK)
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
    int are_ranges_ok  = GridType_getRangeValues(ctx, o, argv + 2, &row_start, &row_end, &column_start, &column_end);
    if (are_ranges_ok != REDISMODULE_OK)
        return REDISMODULE_ERR;

    GridType_rangeObject(ctx, o, row_start, row_end, column_start, column_end);

    return REDISMODULE_OK;
}

int GridType_getShape(RedisModuleCtx *ctx, struct GridTypeObject* o)
{
    return ArrayGrid_getShape(ctx, o->array_grid);
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

    return GridType_getShape(ctx, o);
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

    return GridType_dump(ctx, o);
}

/* Type Methods */

void GridType_Free(void *value) 
{
    GridType_releaseObject((struct GridTypeObject*)value);
}

void GridType_RdbSave(RedisModuleIO *rdb, void *value) 
{
    struct GridTypeObject *o = value;

    if (o->storage_type == STORAGE_TYPE_ARRAY)
        ArrayGrid_rdbSave(rdb, o->array_grid);
    else
        RowGrid_rdbSave(rdb, o->row_grid);
}

void *GridType_RdbLoad(RedisModuleIO *rdb, int encver) 
{
    if (encver != 0)
    {
        /* RedisModule_Log("warning","Can't load data with version %d", encver);*/
        return NULL;
    }

    struct GridTypeObject *o = (struct GridTypeObject*) RedisModule_Alloc(sizeof(struct GridTypeObject));
    o->storage_type = current_storage_type;
    if (o->storage_type & STORAGE_TYPE_ARRAY)
        o->array_grid = ArrayGrid_rdbLoad(rdb);
    else
        o->row_grid = RowGrid_rdbLoad(rdb);
    return o;
}

void GridType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) 
{
    struct GridTypeObject *o = value;
    if (o->storage_type & STORAGE_TYPE_ARRAY)
        ArrayGrid_aofRewrite(aof, key, o->array_grid);
    else
        RowGrid_aofRewrite(aof, key, o->row_grid);
}

size_t GridType_MemUsage(const void *value) 
{
    const struct GridTypeObject *o = value;
    if (o->storage_type & STORAGE_TYPE_ARRAY)
        return ArrayGrid_memUsage(o->array_grid);
    else
        return RowGrid_memUsage(o->row_grid);
}

void GridType_Digest(RedisModuleDigest *md, void *value) 
{
    struct GridTypeObject *o = value;
    if (o->storage_type & STORAGE_TYPE_ARRAY)
        ArrayGrid_digest(md, o->array_grid);
    else
        RowGrid_digest(md, o->row_grid);
}

/* Initialisation */

unsigned char GridType_getStorageType(RedisModuleCtx *ctx,RedisModuleString **argv, int argc)
{
    for (RedisModuleString**p = argv, **end = argv + argc; p < end; ++p)
    {
        size_t len;
        const char* s = RedisModule_StringPtrLen(*p, &len);
        if (len != 0 && strcmp("STORAGE=ARRAY", s) == 0)
        {
            RedisModule_Log(ctx, "notice", "Setting storage to ARRAY");
            return STORAGE_TYPE_ARRAY;
        }
    }

    RedisModule_Log(ctx, "notice", "Setting storage to ROW");

    return STORAGE_TYPE_ROW;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (RedisModule_Init(ctx, "GRID", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    current_storage_type = GridType_getStorageType(ctx, argv, argc);

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

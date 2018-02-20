#ifndef __ARRAY_GRID_H
#define  __ARRAY_GRID_H

#include "redismodule.h"

struct ArrayGrid {
    size_t rows;
    size_t columns;
    char** start;
    char** end;
};

struct ArrayGrid *ArrayGrid_createObject(size_t rows, size_t columns, RedisModuleString** source);
void ArrayGrid_releaseObject(struct ArrayGrid *o);
int ArrayGrid_setObject(struct ArrayGrid *o, long long row_start, long long row_end, long long column_start, long long column_end, RedisModuleString **source);
int ArrayGrid_resizeAndCopyObject(struct ArrayGrid *o, size_t rows, size_t columns);
int ArrayGrid_resizeAndReplaceObject(struct ArrayGrid *o, size_t rows, size_t columns, RedisModuleString **source);
void ArrayGrid_rangeObject(RedisModuleCtx *ctx, struct ArrayGrid *o, long long row_start, long long row_end, long long column_start, long long column_end);
int ArrayGrid_getRangeValues(RedisModuleCtx *ctx, struct ArrayGrid *o, RedisModuleString **argv, long long *row_start, long long *row_end, long long *column_start, long long *column_end);
int ArrayGrid_getShape(RedisModuleCtx *ctx, struct ArrayGrid* o);
int ArrayGrid_dump(RedisModuleCtx *ctx, struct ArrayGrid* o);
void ArrayGrid_rdbSave(RedisModuleIO *rdb, struct ArrayGrid *o);
struct ArrayGrid* ArrayGrid_rdbLoad(RedisModuleIO *rdb);
void ArrayGrid_aofRewrite(RedisModuleIO *aof, RedisModuleString *key, struct ArrayGrid *o);
size_t ArrayGrid_memUsage(const struct ArrayGrid *o);
void ArrayGrid_digest(RedisModuleDigest *md, struct ArrayGrid *o);

#endif //  __ARRAY_GRID_H

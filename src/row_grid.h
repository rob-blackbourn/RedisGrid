#ifndef __ROW_GRID_H
#define  __ROW_GRID_H

#include "redismodule.h"

struct RowGrid {
    size_t rows;
    size_t columns;
    char*** rstart;
    char*** rend;
};

struct RowGrid *RowGrid_createObject(size_t rows, size_t columns, RedisModuleString** source);
void RowGrid_releaseObject(struct RowGrid *o);
int RowGrid_setObject(struct RowGrid *o, long long row_start, long long row_end, long long column_start, long long column_end, RedisModuleString **source);
int RowGrid_resizeAndCopyObject(struct RowGrid *o, size_t rows, size_t columns);
int RowGrid_resizeAndReplaceObject(struct RowGrid *o, size_t rows, size_t columns, RedisModuleString **source);
void RowGrid_rangeObject(RedisModuleCtx *ctx, struct RowGrid *o, long long row_start, long long row_end, long long column_start, long long column_end);
int RowGrid_getRangeValues(RedisModuleCtx *ctx, struct RowGrid *o, RedisModuleString **argv, long long *row_start, long long *row_end, long long *column_start, long long *column_end);
int RowGrid_getShape(RedisModuleCtx *ctx, struct RowGrid* o);
int RowGrid_dump(RedisModuleCtx *ctx, struct RowGrid* o);
void RowGrid_rdbSave(RedisModuleIO *rdb, struct RowGrid *o);
struct RowGrid* RowGrid_rdbLoad(RedisModuleIO *rdb);
void RowGrid_aofRewrite(RedisModuleIO *aof, RedisModuleString *key, struct RowGrid *o);
size_t RowGrid_memUsage(const struct RowGrid *o);
void RowGrid_digest(RedisModuleDigest *md, struct RowGrid *o);

#endif // __ROW_GRID_H

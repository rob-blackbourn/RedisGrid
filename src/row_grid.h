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

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

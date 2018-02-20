#include <string.h>

#include "utils.h"
#include "row_grid.h"

void RowGrid_clearRow(char **cstart, char **cend)
{
    for (char **c = cstart; c < cend; ++c)
    {
        if (*c)
        {
            RedisModule_Free(*c);
            *c = NULL;
        }
    }
}

void RowGrid_clearRows(char ***rstart, char ***rend, size_t columns)
{
    for (char ***r = rstart; r < rend; ++r)
        RowGrid_clearRow(*r, *r + columns);
}

int RowGrid_copyRow(RedisModuleString** source, char **cstart, char **cend)
{
    if (!cstart)
        return REDISMODULE_ERR;

    for (char **c = cstart; c < cend; ++c, ++source)
    {
        if (GridType_setRedisString(source, c) != REDISMODULE_OK)
        {
            RowGrid_clearRow(cstart, c);
            return REDISMODULE_ERR;
        }
    }

    return REDISMODULE_OK;
}

int RowGrid_copyRedisStrings(RedisModuleString **source, char ***rstart, char ***rend, size_t columns)
{
    for (char ***r = rstart; r < rend; ++r, source += columns)
    {
        if (RowGrid_copyRow(source, *r, *r + columns) != REDISMODULE_OK)
        {
            RowGrid_clearRows(rstart, r, columns);
            return REDISMODULE_ERR;
        }
    }

    return REDISMODULE_OK;
}

char ***RowGrid_copyAndAllocRedisStrings(RedisModuleString **source, size_t rows, size_t columns)
{
    char ***rstart = (char***)RedisModule_Alloc(sizeof(char**) * rows);
    if (!rstart)
        return NULL;

    for (char ***r = rstart, ***rend = rstart + rows; r < rend; ++r, source += columns)
    {
        *r = (char**)RedisModule_Alloc(sizeof(char*) * columns);
        if (RowGrid_copyRow(source, *r, *r + columns) != REDISMODULE_OK)
        {
            RowGrid_clearRows(rstart, r, columns);
            RedisModule_Free(rstart);
            return NULL;
        }
    }

    return rstart;
}

struct RowGrid *RowGrid_createObject(size_t rows, size_t columns, RedisModuleString **source)
{
    struct RowGrid *o = (struct RowGrid *)RedisModule_Alloc(sizeof(struct RowGrid));
    if (!o)
        return NULL;

    o->rstart = RowGrid_copyAndAllocRedisStrings(source, rows, columns);
    if (!o->rstart)
    {
        RedisModule_Free(o);
        return NULL;
    }

    o->rows = rows;
    o->columns = columns;
    o->rend = o->rstart + rows;

    return o;
}

void RowGrid_releaseObject(struct RowGrid *o)
{
    RowGrid_clearRows(o->rstart, o->rend, o->columns);
    for (char ***r = o->rstart; r < o->rend; ++r)
        RedisModule_Free(*r);
    RedisModule_Free(o->rstart);
    RedisModule_Free(o);
}

int RowGrid_setObject(struct RowGrid *o, long long row_start, long long row_end, long long column_start, long long column_end, RedisModuleString **source)
{
    long long row_sign = row_start < row_end ? 1 : -1;
    long long column_sign = column_start < column_end ? 1 : -1;

    for (long long r = row_start; r != row_end + row_sign; r += row_sign)
    {
        for (long long c = column_start; c != column_end + column_sign; c += column_sign, ++source)
        {
            if (GridType_resetRedisString(source, &o->rstart[r][c]) != REDISMODULE_OK)
                return REDISMODULE_ERR;
        }
    }

    return REDISMODULE_OK;
}

int RowGrid_resizeAndCopyObject(struct RowGrid *o, size_t rows, size_t columns)
{
    // If there are fewer rows in the new grid clear the old rows of data and free them.
    for (char ***r = o->rstart + rows; r < o->rend; ++r)
    {
        RowGrid_clearRow(*r, *r + o->columns);
        RedisModule_Free(*r);
    }

    if (columns != o->columns)
    {
        // We have already delt with surpless rows so calculate the row end.
        char *** rend =o->rstart + min(rows, o->rows);

        // if there are less columns in the new grid clear the surpless data.
        if (columns < o->columns)
        {
            for (char ***r = o->rstart; r < rend; ++r)
                RowGrid_clearRow(*r + columns, *r + o->columns);
        }

        // Resize the columns
        for (char ***r = o->rstart; r < rend; ++r)
            *r = (char**)RedisModule_Realloc(*r, sizeof(char*) * columns);

        // If the new columns are longer initialize the memory.
        if (columns > o->columns)
        {
            size_t trim_len = sizeof(char*) * (columns - o->columns);
            for (char ***r = o->rstart; r < rend; ++r)
                memset(*r + o->columns, 0, trim_len);
            
        }

        o->columns = columns;
    }

    // If the rows are a different size, resize them
    if (rows != o->rows)
    {
        o->rstart = (char***)RedisModule_Realloc(o->rstart, sizeof(char**) * rows);
        o->rend = o->rstart + rows;
        
        // If there are more rows allocate columns
        if (rows > o->rows)
        {
            for (char ***r = o->rstart + o->rows; r < o->rend; ++r)
                *r = (char**)RedisModule_Calloc(columns, sizeof(char*));
        }

        o->rows = rows;
    }

    return REDISMODULE_OK;
}

int RowGrid_resizeAndReplaceObject(struct RowGrid *o, size_t rows, size_t columns, RedisModuleString **source)
{
    if (rows == o->rows && columns == o->columns)
        return RowGrid_setObject(o, 0, rows - 1, 0, columns - 1, source);

    // Clear the existing values.
    RowGrid_clearRows(o->rstart, o->rend, o->columns);

    // if there are fewer rows in the new grid free them.
    for (char ***r = o->rstart + rows; r < o->rend; ++r)
        RedisModule_Free(*r);

    if (columns != o->columns)
    {
        // We have already delt with surpless rows so calculate the row end.
        char *** rend =o->rstart + min(rows, o->rows);

        // Resize the columns
        for (char ***r = o->rstart; r < rend; ++r)
            *r = (char**)RedisModule_Realloc(*r, sizeof(char*) * columns);

        // If the new columns are longer initialize the memory.
        if (columns > o->columns)
        {
            size_t trim_len = sizeof(char*) * (columns - o->columns);
            for (char***r = o->rstart; r < rend; ++r)
                memset(*r + o->columns, 0, trim_len);
        }

        o->columns = columns;
    }

    // If the rows are a different size, resize them
    if (rows != o->rows)
    {
        o->rstart = (char***)RedisModule_Realloc(o->rstart, sizeof(char**) * rows);
        o->rend = o->rstart + rows;
        
        // If there are more rows allocate columns
        if (rows > o->rows)
        {
            for (char ***r = o->rstart + o->rows; r < o->rend; ++r)
                *r = (char**)RedisModule_Calloc(columns, sizeof(char*));
        }

        o->rows = rows;
    }

    RowGrid_copyRedisStrings(source, o->rstart, o->rend, o->columns);

    return REDISMODULE_OK;
}

void RowGrid_rangeObject(RedisModuleCtx *ctx, struct RowGrid *o, long long row_start, long long row_end, long long column_start, long long column_end)
{
    long long rows = 1 + (max(row_start, row_end) - min(row_start, row_end));
    long long columns = 1 + (max(column_start, column_end) - min(column_start, column_end));
    RedisModule_ReplyWithArray(ctx, (long)(rows * columns));

    long long row_sign = row_start < row_end ? 1 : -1;
    long long column_sign = column_start < column_end ? 1 : -1;

    for (long long r = row_start; r != row_end + row_sign; r += row_sign)
    {
        for (long long c = column_start; c != column_end + column_sign; c += column_sign)
        {
            char *s = o->rstart[r][c];
            if (s)
                RedisModule_ReplyWithSimpleString(ctx, s);
            else
                RedisModule_ReplyWithNull(ctx);
        }
    }
}

int RowGrid_getRangeValues(RedisModuleCtx *ctx, struct RowGrid *o, RedisModuleString **argv, long long *row_start, long long *row_end, long long *column_start, long long *column_end)
{
    int are_ranges_ok  =
        GridType_getRangeValue(ctx, argv, 2, (long long)o->rows, row_start, "Start row must be an integer", "Start row outside the bounds of the grid") == REDISMODULE_OK &&
        GridType_getRangeValue(ctx, argv, 3, (long long)o->rows, row_end, "End row must be an integer", "End row outside the bounds of the grid") == REDISMODULE_OK &&
        GridType_getRangeValue(ctx, argv, 4, (long long)o->columns, column_start, "Start column must be an integer", "Start column outside the bounds of the grid") == REDISMODULE_OK &&
        GridType_getRangeValue(ctx, argv, 5, (long long)o->columns, column_end, "End column must be an integer", "End column outside the bounds of the grid") == REDISMODULE_OK;
    return are_ranges_ok ? REDISMODULE_OK : REDISMODULE_ERR;
}

int RowGrid_getShape(RedisModuleCtx *ctx, struct RowGrid *o)
{
    RedisModule_ReplyWithArray(ctx, (long)2);
    RedisModule_ReplyWithLongLong(ctx, (long long)o->rows);
    RedisModule_ReplyWithLongLong(ctx, (long long)o->columns);
    return REDISMODULE_OK;
}

int RowGrid_dump(RedisModuleCtx *ctx, struct RowGrid *o)
{
    RedisModule_ReplyWithArray(ctx, (long) (2 + o->rows * o->columns));

    RedisModule_ReplyWithLongLong(ctx, (long long)o->rows);
    RedisModule_ReplyWithLongLong(ctx, (long long)o->columns);

    for (char ***r = o->rstart; r < o->rend; ++r)
    {
        for (char **c = *r, **cend = *r + o->columns; c < cend; ++c)
        {
            if (*c)
                RedisModule_ReplyWithSimpleString(ctx, *c);
            else
                RedisModule_ReplyWithNull(ctx);
        }
    }
    
    return REDISMODULE_OK;
}

void RowGrid_rdbSave(RedisModuleIO *rdb, struct RowGrid *o)
{
    RedisModule_SaveUnsigned(rdb, (uint64_t)o->rows);
    RedisModule_SaveUnsigned(rdb, (uint64_t)o->columns);
    for (char ***r = o->rstart; r  < o->rend; ++r)
    {
        for (char **c = *r, **cend = *r + o->columns; c < cend; ++c)
        {
            if (*c)
                RedisModule_SaveStringBuffer(rdb, *c, strlen(*c) + 1);
            else
                RedisModule_SaveStringBuffer(rdb, "", 1);
        }
    }
}

struct RowGrid* RowGrid_rdbLoad(RedisModuleIO *rdb)
{
    size_t rows = (size_t) RedisModule_LoadUnsigned(rdb);
    size_t columns = (size_t) RedisModule_LoadUnsigned(rdb);
    char ***rstart = (char***) RedisModule_Alloc(sizeof(char**) * rows);
    char ***rend = rstart + rows;
    for (char ***r = rstart; r < rend; ++r)
    {
        *r = (char**)RedisModule_Alloc(sizeof(char*) * columns);
        for (char **c = *r, **cend = *r + columns; c < cend; ++c)
        {
            size_t l;
            *c = RedisModule_LoadStringBuffer(rdb, &l);
        }
    }

    struct RowGrid *o = (struct RowGrid*) RedisModule_Alloc(sizeof(struct RowGrid));
    o->rows = rows;
    o->columns = columns;
    o->rstart = rstart;
    o->rend = rend;
    return o;
}

void RowGrid_aofRewrite(RedisModuleIO *aof, RedisModuleString *key, struct RowGrid *o) 
{
    RedisModuleCtx *ctx = RedisModule_GetContextFromIO(aof);

    size_t len = o->columns * o->rows;
    RedisModuleString **start = (RedisModuleString**)RedisModule_Alloc(sizeof(RedisModuleString*) * len);
    RedisModuleString **p = start, **end = start + len;

    for (char ***r = o->rstart; r < o->rend; ++r)
    {
        for (char **c = *r, **cend = *r + o->columns; c < cend; ++c, ++p)
        {
            if (*c)
                *p = RedisModule_CreateString(ctx, *c, strlen(*c));
            else
                *p = RedisModule_CreateString(ctx, "", 0);
        }
    }

    RedisModule_EmitAOF(aof, "GRID.DIM","sllv", key, (long long)o->rows, (long long)o->columns, start, len);

    for (p = start; p < end; ++p)
        RedisModule_FreeString(ctx, *p);
    RedisModule_Free(start);
}

size_t RowGrid_memUsage(const struct RowGrid *o) 
{
    size_t usage = sizeof(*o) + sizeof(char***) * o->rows + o->rows * o->columns * sizeof(char**);

    for (char ***r = o->rstart; r < o->rend; ++r)
    {
        for (char **c = *r, **cend = *r + o->columns; c < cend; ++c)
        {
            usage += *c ? strlen(*c) + 1 : 0;
        }
    }

    return usage;
}

void RowGrid_digest(RedisModuleDigest *md, struct RowGrid *o)
{
    RedisModule_DigestAddLongLong(md, o->rows);
    RedisModule_DigestAddLongLong(md, o->columns);

    for (char ***r = o->rstart; r < o->rend; ++r)
    {
        for (char **c = *r, **cend = *r + o->columns; c < cend; ++c)
        {
            if (*c)
                RedisModule_DigestAddStringBuffer(md, (unsigned char*)*c, strlen((const char*)*c) + 1);
            else
                RedisModule_DigestAddStringBuffer(md, (unsigned char*)"", 1);
        }
    }

    RedisModule_DigestEndSequence(md);
}

using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using StackExchange.Redis;
using StackExchange.Redis.Data;

namespace StackExchange.Redis.Data
{
    public static class RedisGrid
    {
        public static bool GridDim<T>(this IDatabase db, RedisKey key, T[,] grid)
        {
            var rows = grid.GetLength(0);
            var columns = grid.GetLength(1);
            var args = new object[3 + rows * columns];
            args[0] = key;
            args[1] = rows;
            args[2] = columns;
            Copy(grid, args, 3);
            return (string)db.Execute("GRID.DIM", args) == "OK";
        }

        public static async Task<bool> GridDimAsync<T>(this IDatabase db, RedisKey key, T[,] grid)
        {
            var rows = grid.GetLength(0);
            var columns = grid.GetLength(1);
            var args = new object[3 + rows * columns];
            args[0] = key;
            args[1] = rows;
            args[2] = columns;
            Copy(grid, args, 3);
            return (string)await db.ExecuteAsync("GRID.DIM", args) == "OK";
        }

        public static bool GridDim(this IDatabase db, RedisKey key, int rows, int columns, params object[] optionalArgs)
        {
            var args = new List<object> { key, rows, columns };
            if (optionalArgs != null && optionalArgs.Length > 0)
                args.AddRange(optionalArgs);
            return (string)db.Execute("GRID.DIM", args) == "OK";
        }

        public static async Task<bool> GridDimAsync(this IDatabase db, RedisKey key, int rows, int columns, params object[] optionalArgs)
        {
            var args = new List<object> { key, rows, columns };
            if (optionalArgs != null && optionalArgs.Length > 0)
                args.AddRange(optionalArgs);
            return (string)await db.ExecuteAsync("GRID.DIM", args).ConfigureAwait(false) == "OK";
        }

        public static int[] GridShape(this IDatabase db, RedisKey key)
        {
            var shape = (RedisValue[])db.Execute("GRID.SHAPE", key);
            return new[] {(int) shape[0], (int) shape[1]};
        }

        public static async Task<int[]> GridShapeAsync(this IDatabase db, RedisKey key)
        {
            var shape = (RedisValue[])await db.ExecuteAsync("GRID.SHAPE", key);
            return new[] { (int)shape[0], (int)shape[1] };
        }

        public static RedisValue[] GridRange(this IDatabase db, RedisKey key, int rowStart, int rowEnd, int columnStart, int columnEnd)
        {
            return (RedisValue[])db.Execute("GRID.RANGE", rowStart, rowEnd, columnStart, columnEnd);
        }

        public static async Task<RedisValue[]> GridRangeAsync(this IDatabase db, RedisKey key, int rowStart, int rowEnd, int columnStart, int columnEnd)
        {
            return (RedisValue[])await db.ExecuteAsync("GRID.RANGE", rowStart, rowEnd, columnStart, columnEnd);
        }

        public static bool GridSet<T>(this IDatabase db, RedisKey key, int rowStart, int columnStart, T[,] items)
        {
            var rows = items.GetLength(0);
            var columns = items.GetLength(1);
            var args = new object[5 + rows * columns];
            args[0] = key;
            args[1] = rowStart;
            args[2] = rowStart + rows - 1;
            args[3] = columnStart;
            args[4] = columnStart + columns - 1;
            Copy(items, args, 5);
            return (string)db.Execute("GRID.SET", args) == "OK";
        }

        public static async Task<bool> GridSetAsync<T>(this IDatabase db, RedisKey key, int rowStart, int columnStart, T[,] items)
        {
            var rows = items.GetLength(0);
            var columns = items.GetLength(1);
            var args = new object[5 + rows * columns];
            args[0] = key;
            args[1] = rowStart;
            args[2] = rowStart + rows - 1;
            args[3] = columnStart;
            args[4] = columnStart + columns - 1;
            Copy(items, args, 5);
            return (string)await db.ExecuteAsync("GRID.SET", args) == "OK";
        }

        public static bool GridSet(this IDatabase db, RedisKey key, int rowStart, int rowEnd, int columnStart, int columnEnd, params object[] items)
        {
            var args = new object[5 + items.Length];
            args[0] = key;
            args[1] = rowStart;
            args[2] = rowEnd;
            args[3] = columnStart;
            args[4] = columnEnd;
            Array.Copy(items, 0, args, 5, items.Length);
            return (string)db.Execute("GRID.SET", args) == "OK";
        }

        public static async Task<bool> GridSetAsync(this IDatabase db, RedisKey key, int rowStart, int rowEnd, int columnStart, int columnEnd, params object[] items)
        {
            var args = new object[5 + items.Length];
            args[0] = key;
            args[1] = rowStart;
            args[2] = rowEnd;
            args[3] = columnStart;
            args[4] = columnEnd;
            Array.Copy(items, 0, args, 5, items.Length);
            return (string)await db.ExecuteAsync("GRID.SET", args) == "OK";
        }

        public static RedisValue[,] GridDump(this IDatabase db, RedisKey key)
        {
            var response = (RedisValue[])db.Execute("GRID.DUMP", key);
            var rows = (int)response[0];
            var columns = (int)response[1];
            var grid = new RedisValue[rows, columns];
            Copy(response, 2, grid);
            return grid;
        }

        public static async Task<RedisValue[,]> GridDumpAsync(this IDatabase db, RedisKey key)
        {
            var response = (RedisValue[])await db.ExecuteAsync("GRID.DUMP", key);
            var rows = (int)response[0];
            var columns = (int)response[1];
            var grid = new RedisValue[rows, columns];
            Copy(response, 2, grid);
            return grid;
        }

        public static void Copy(RedisValue[] source, int offset, RedisValue[,] destination)
        {
            for (int i = offset, r = 0; r < destination.GetLength(0); ++r)
            for (int c = 0; c < destination.GetLength(1); ++c, ++i)
                destination[r, c] = source[i];
        }

        public static void Copy(RedisValue[] source, int offset, string[,] destination)
        {
            for (int i = offset, r = 0; r < destination.GetLength(0); ++r)
            for (int c = 0; c < destination.GetLength(1); ++c, ++i)
                destination[r, c] = source[i];
        }

        public static void Copy<T>(T[,] source, object[] destination, int offset)
        {
            var rows = source.GetLength(0);
            var columns = source.GetLength(1);
            for (int i = offset, r = 0; r < rows; ++r)
                for (int c = 0; c < columns; ++c, ++i)
                    destination[i] = source[r, c];
        }

        public static string[,] AsStringGrid(this RedisValue[,] source)
        {
            var rows = source.GetLength(0);
            var columns = source.GetLength(1);
            var grid = new string[source.GetLength(0), source.GetLength(1)];
            for (var r = 0; r < rows; ++r)
            for (var c = 0; c < columns; ++c)
                grid[r, c] = source[r, c];
            return grid;
        }

        public static DataFrame AsDataFrame(this RedisValue[,] source)
        {
            return new DataFrame(
                Enumerable
                    .Range(0, source.GetLength(0))
                    .Select(r =>
                        AsVector(
                            source[r, 0],
                            source[r, 1],
                            Enumerable.Range(2, source.GetLength(1) - 2).Select(c => source[r, c]))));
        }

        private static IVector AsVector(string name, string dtype, IEnumerable<RedisValue> source)
        {
            if (dtype.StartsWith("float"))
                return new Vector<double?>(source.Select(x => (string)x == "nan" ? null : (double?)x), name);

            if (dtype.StartsWith("int") || dtype.StartsWith("uint"))
            {
                switch (dtype)
                {
                    case "int8":
                    case "uint8":
                    case "int16":
                    case "uint16":
                    case "int32":
                    case "uint32":
                        return new Vector<int?>(source.Select(x => (string)x == "nan" ? null : (int?)x), name);
                    default:
                        return new Vector<long?>(source.Select(x => (string)x == "nan" ? null : (long?)x), name);
                }
            }

            if (dtype.StartsWith("datetime"))
                return new Vector<DateTime?>(source.Select(x => (string)x == "nat" ? (DateTime?)null : DateTime.Parse(x)), name);

            if (dtype.StartsWith("timedelta"))
                return new Vector<TimeSpan>(source.Select(x => TimeSpan.FromSeconds((double)x)), name);

            if (dtype.StartsWith("bool"))
                return new Vector<bool?>(source.Select(x => (string)x == "nan" ? null : (bool?)x), name);

            if (dtype.StartsWith("byte"))
                return new Vector<byte>(source.Select(x => (byte)x), name);

            return new Vector<string>(source.Select(x => (string)x), name);
        }
    }
}

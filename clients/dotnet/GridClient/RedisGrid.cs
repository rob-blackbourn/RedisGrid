using StackExchange.Redis.Data;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Threading.Tasks;

namespace StackExchange.Redis
{
    public static class RedisGrid
    {
        public static bool GridDim(this IDatabase db, RedisKey key, string[,] grid)
        {
            var rows = grid.GetLength(0);
            var columns = grid.GetLength(1);
            var args = new object[3 + rows * columns];
            args[0] = key;
            args[1] = rows;
            args[2] = columns;
            Flatten(grid, args, 3);
            return (string)db.Execute("GRID.DIM", args) == "OK";
        }

        public static async Task<bool> GridDimAsync(this IDatabase db, RedisKey key, string[,] grid)
        {
            var rows = grid.GetLength(0);
            var columns = grid.GetLength(1);
            var args = new object[3 + rows * columns];
            args[0] = key;
            args[1] = rows;
            args[2] = columns;
            Flatten(grid, args, 3);
            return (string)await db.ExecuteAsync("GRID.DIM", args) == "OK";
        }

        public static bool GridDim(this IDatabase db, RedisKey key, int rows, int columns, params string[] elements)
        {
            var args = new List<object> { key, rows, columns };
            if (elements != null && elements.Length > 0)
                args.AddRange(elements);
            return (string)db.Execute("GRID.DIM", args) == "OK";
        }

        public static async Task<bool> GridDimAsync(this IDatabase db, RedisKey key, int rows, int columns, params string[] elements)
        {
            var args = new List<object> { key, rows, columns };
            if (elements != null && elements.Length > 0)
                args.AddRange(elements);
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
            return (RedisValue[])db.Execute("GRID.RANGE", key, rowStart, rowEnd, columnStart, columnEnd);
        }

        public static async Task<RedisValue[]> GridRangeAsync(this IDatabase db, RedisKey key, int rowStart, int rowEnd, int columnStart, int columnEnd)
        {
            return (RedisValue[])await db.ExecuteAsync("GRID.RANGE", key, rowStart, rowEnd, columnStart, columnEnd);
        }

        public static bool GridSet(this IDatabase db, RedisKey key, int rowStart, int columnStart, string[,] items)
        {
            var rows = items.GetLength(0);
            var columns = items.GetLength(1);
            var args = new object[5 + rows * columns];
            args[0] = key;
            args[1] = rowStart;
            args[2] = rowStart + rows - 1;
            args[3] = columnStart;
            args[4] = columnStart + columns - 1;
            Flatten(items, args, 5);
            return (string)db.Execute("GRID.SET", args) == "OK";
        }

        public static async Task<bool> GridSetAsync(this IDatabase db, RedisKey key, int rowStart, int columnStart, string[,] items)
        {
            var rows = items.GetLength(0);
            var columns = items.GetLength(1);
            var args = new object[5 + rows * columns];
            args[0] = key;
            args[1] = rowStart;
            args[2] = rowStart + rows - 1;
            args[3] = columnStart;
            args[4] = columnStart + columns - 1;
            Flatten(items, args, 5);
            return (string)await db.ExecuteAsync("GRID.SET", args) == "OK";
        }

        public static bool GridSet(this IDatabase db, RedisKey key, int rowStart, int rowEnd, int columnStart, int columnEnd, params string[] items)
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

        public static async Task<bool> GridSetAsync(this IDatabase db, RedisKey key, int rowStart, int rowEnd, int columnStart, int columnEnd, params string[] items)
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
            return MakeRedisValueGrid(response, 2, rows, columns);
        }

        public static async Task<RedisValue[,]> GridDumpAsync(this IDatabase db, RedisKey key)
        {
            var response = (RedisValue[])await db.ExecuteAsync("GRID.DUMP", key);
            var rows = (int)response[0];
            var columns = (int)response[1];
            return MakeRedisValueGrid(response, 2, rows, columns);
        }

        private static RedisValue[,] MakeRedisValueGrid(RedisValue[] source, int offset, int rows, int columns)
        {
            var grid = new RedisValue[rows, columns];
            for (int i = offset, r = 0; r < grid.GetLength(0); ++r)
                for (var c = 0; c < grid.GetLength(1); ++c, ++i)
                    grid[r, c] = source[i];
            return grid;
        }

        private static void Flatten(string[,] source, object[] destination, int offset)
        {
            var rows = source.GetLength(0);
            var columns = source.GetLength(1);
            for (int i = offset, r = 0; r < rows; ++r)
                for (var c = 0; c < columns; ++c, ++i)
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

        public static string[,] AsStringGrid(this RedisValue[] source, int rows, int columns)
        {
            var grid = new string[rows, columns];
            for (int r = 0, i = 0; r < rows; ++r)
            for (var c = 0; c < columns; ++c, ++i)
                grid[r, c] = source[i];
            return grid;
        }

        public static string[] AsStringArray(this RedisValue[] source)
        {
            return source.Select(x => x.ToString()).ToArray();
        }

        public static DataFrame AsDataFrame(this RedisValue[,] source)
        {
            return new DataFrame(
                Enumerable
                    .Range(0, source.GetLength(0))
                    .Select(r =>
                        AsSeries(
                            source[r, 0],
                            source[r, 1],
                            Enumerable.Range(2, source.GetLength(1) - 2).Select(c => source[r, c]))));
        }

        private static ISeries AsSeries(string name, string dtype, IEnumerable<RedisValue> source)
        {
            if (dtype.StartsWith("float"))
                return new Series<double?>(source.Select(x => (string)x == "nan" ? null : (double?)x), name);

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
                        return new Series<int?>(source.Select(x => (string)x == "nan" ? null : (int?)x), name);
                    default:
                        return new Series<long?>(source.Select(x => (string)x == "nan" ? null : (long?)x), name);
                }
            }

            if (dtype.StartsWith("datetime"))
                return new Series<DateTime?>(source.Select(x => (string)x == "nat" ? (DateTime?)null : DateTime.Parse(x)), name);

            if (dtype.StartsWith("timedelta"))
                return new Series<TimeSpan>(source.Select(x => TimeSpan.FromSeconds((double)x)), name);

            if (dtype.StartsWith("bool"))
                return new Series<bool?>(source.Select(x => (string)x == "nan" ? null : (bool?)x), name);

            if (dtype.StartsWith("byte"))
                return new Series<byte>(source.Select(x => (byte)x), name);

            return new Series<string>(source.Select(x => (string)x), name);
        }

        public static string[,] AsStringGrid(this DataFrame dataFrame)
        {
            var grid = new string[dataFrame.Columns.Count, 2 + dataFrame.Count];
            for (var i = 0; i < dataFrame.Columns.Count; ++i)
            {
                var series = dataFrame[i];
                grid[i, 0] = series.Name ?? string.Empty;
                grid[i, 1] = series.DataType();
                for (var j = 0; j < series.Count; ++j)
                    grid[i, 2 + j] = series[j].ToRedisString();
            }
            return grid;
        }

        private static string DataType(this ISeries series)
        {
            switch (Type.GetTypeCode(Nullable.GetUnderlyingType(series.Type) ?? series.Type))
            {
                case TypeCode.Boolean:
                    return "bool";
                case TypeCode.Byte:
                    return "byte";
                case TypeCode.Char:
                    return "int8";
                case TypeCode.DateTime:
                    return "datetime64[ns]";
                case TypeCode.DBNull:
                    return "object";
                case TypeCode.Decimal:
                    return "float64";
                case TypeCode.Double:
                    return "float64";
                case TypeCode.Empty:
                    return "object";
                case TypeCode.Int16:
                    return "int16";
                case TypeCode.Int32:
                    return "int32";
                case TypeCode.Int64:
                    return "int64";
                case TypeCode.Object:
                    return "object";
                case TypeCode.SByte:
                    return "int32";
                case TypeCode.Single:
                    return "float32";
                case TypeCode.String:
                    return "object";
                case TypeCode.UInt16:
                    return "uint16";
                case TypeCode.UInt32:
                    return "uint32";
                case TypeCode.UInt64:
                    return "uint64";
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }

        private static string ToRedisString(this object value)
        {
            if (value == null)
                return string.Empty;
            if (value is string s)
                return s;
            if (value is DateTime time)
                return time.ToString("yyyy-MM-ddTHH:mm:ss.fff");
            if (value is TimeSpan span)
                return span.TotalSeconds.ToString(CultureInfo.InvariantCulture);

            return value.ToString();
        }
    }
}

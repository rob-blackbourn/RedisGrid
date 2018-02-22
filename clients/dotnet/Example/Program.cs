using System;
using System.Diagnostics;
using StackExchange.Redis;
using StackExchange.Redis.Data;

namespace Example
{
    class Program
    {
        static void Main(string[] args)
        {
            // var redis = ConnectionMultiplexer.Connect("localhost");
            var redis = ConnectionMultiplexer.Connect("localhost:7000,localhost:7001,localhost:7002,localhost:7003,localhost:7004,localhost:7005");
            var db = redis.GetDatabase();

            RoundTripGrid(db);
            RoundTripDataFrame(db);
        }

        static void RoundTripGrid(IDatabase db)
        {
            // Store a grid.
            var key = Guid.NewGuid().ToString();
            var source = new[,] { { "1", "2" }, { "3", "4" }, { "5", "6" } };
            db.GridDim(key, source);

            // Fetch it back.
            var grid = db.GridDump(key).AsStringGrid();

            // Check it's shape.
            Debug.Assert(source.Rank == grid.Rank);
            for (var i = 0; i < source.Rank; ++i)
            {
                Debug.Assert(source.GetLowerBound(i) == grid.GetLowerBound(i));
                Debug.Assert(source.GetUpperBound(i) == grid.GetUpperBound(i));
            }

            // Check it's values.
            for (var i = 0; i < 3; ++i)
            for (var j = 0; j < 2; ++j)
                Debug.Assert(source[i, j] == grid[i, j]);

            // Delete it.
            db.GridDim(key, 0, 0);

            // is it deleted?
            try
            {
                db.GridDump(key).AsStringGrid();
                Debug.Assert(false);
            }
            catch (RedisServerException error)
            {
                Debug.Assert(error.Message == "Empty key");
            }
        }

        static void RoundTripDataFrame(IDatabase db)
        {
            // Store a grid.
            var key = Guid.NewGuid().ToString();
            var source = new DataFrame(
                new Series<string>(new[] {"Tom", "Dick", "Harry"}, "Name"),
                new Series<DateTime?>(new DateTime?[] {new DateTime(1973, 1, 1), new DateTime(1989, 12, 31), new DateTime(2000, 2, 28)}, "DateOfBirth"),
                new Series<double?>(new double?[] {1.83, 1.79, 1.66}, "Height"));

            db.GridDim(key, source.AsStringGrid());

            // Fetch it back.
            var grid = db.GridDump(key).AsDataFrame();

            // Check it's shape.
            Debug.Assert(source.Columns.Count == grid.Columns.Count);
            for (var i = 0; i < source.Columns.Count; ++i)
            {
                Debug.Assert(source[i].Count == grid[i].Count);
                Debug.Assert(source[i].Count == grid[i].Count);
                Debug.Assert(source[i].Name == grid[i].Name);
                Debug.Assert(source[i].Type == grid[i].Type);
            }

            // Check it's values.
            for (var c = 0; c < source.Columns.Count; ++c)
            for (var r = 0; r < source.Count; ++r)
                Debug.Assert(source[c, r] == grid[c, r]);

            // Delete it.
            db.GridDim(key, 0, 0);

            // is it deleted?
            try
            {
                db.GridDump(key).AsStringGrid();
                Debug.Assert(false);
            }
            catch (RedisServerException error)
            {
                Debug.Assert(error.Message == "Empty key");
            }
        }
    }
}

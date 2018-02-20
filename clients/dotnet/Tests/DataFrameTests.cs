using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using StackExchange.Redis;
using StackExchange.Redis.Data;

namespace Tests
{
    [TestClass]
    public class DataFrameTests
    {
        [TestMethod]
        public void ShouldRoundTrip()
        {
            var redis = ConnectionMultiplexer.Connect("10.11.153.125");
            var db = redis.GetDatabase();

            // Store a grid.
            var key = Guid.NewGuid().ToString();
            var source = new DataFrame(
                new Series<string>(new[] { "Tom", "Dick", "Harry" }, "Name"),
                new Series<DateTime?>(new DateTime?[] { new DateTime(1973, 1, 1), new DateTime(1989, 12, 31), new DateTime(2000, 2, 28) }, "DateOfBirth"),
                new Series<double?>(new double?[] { 1.83, 1.79, 1.66 }, "Height"));

            db.GridDim(key, source.AsGrid());

            // Fetch it back.
            var grid = db.GridDump(key).AsDataFrame();

            // Check it's shape.
            Assert.AreEqual(source.Columns.Count, grid.Columns.Count);
            for (var i = 0; i < source.Columns.Count; ++i)
            {
                Assert.AreEqual(source[i].Count, grid[i].Count);
                Assert.AreEqual(source[i].Count, grid[i].Count);
                Assert.AreEqual(source[i].Name, grid[i].Name);
                Assert.AreEqual(source[i].Type, grid[i].Type);
            }

            // Check it's values.
            for (var c = 0; c < source.Columns.Count; ++c)
            for (var r = 0; r < source.Count; ++r)
                Assert.AreEqual(source[c, r], grid[c, r]);

            // Delete it.
            db.GridDim(key, 0, 0);

            // is it deleted?
            try
            {
                db.GridDump(key).AsStringGrid();
                Assert.Fail("Should throw on previous call");
            }
            catch (RedisServerException error)
            {
                Assert.AreEqual(error.Message, "Empty key");
            }
        }
    }
}

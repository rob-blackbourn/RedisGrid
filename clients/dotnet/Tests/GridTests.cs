using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using StackExchange.Redis;

namespace Tests
{
    [TestClass]
    public class GridTests
    {
        [TestMethod]
        public void ShouldRoundTrip()
        {
            var redis = ConnectionMultiplexer.Connect("10.11.153.125");
            var db = redis.GetDatabase();

            // Store a grid.
            var key = Guid.NewGuid().ToString();
            var source = new[,] { { "1", "2" }, { "3", "4" }, { "5", "6" } };
            db.GridDim(key, source);

            // Fetch it back.
            var grid = db.GridDump(key).AsStringGrid();

            // Check it's shape.
            Assert.AreEqual(source.Rank, grid.Rank);
            for (var i = 0; i < source.Rank; ++i)
            {
                Assert.AreEqual(source.GetLowerBound(i), grid.GetLowerBound(i));
                Assert.AreEqual(source.GetUpperBound(i), grid.GetUpperBound(i));
            }

            // Check it's values.
            for (var i = 0; i < 3; ++i)
            for (var j = 0; j < 2; ++j)
                Assert.AreEqual(source[i, j], grid[i, j]);

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

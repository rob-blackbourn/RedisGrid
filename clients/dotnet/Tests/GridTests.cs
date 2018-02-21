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
            var source = GridExtensions.CreateOrdinalGrid(2, 3);
            db.GridDim(key, source);

            // Fetch it back.
            var grid = db.GridDump(key).AsStringGrid();

            Assert.IsTrue(source.Equals(grid));

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

        [TestMethod]
        public void ShouldResize()
        {
            var redis = ConnectionMultiplexer.Connect("10.11.153.125");
            var db = redis.GetDatabase();

            // Grow the rows
            Resize(db, 2, 3, 4, 3);
            // Grow the columns
            Resize(db, 2, 3, 2, 5);
            // Grow the rows and columns.
            Resize(db, 2, 3, 4, 5);
            // Shrink the rows
            Resize(db, 5, 4, 3, 4);
            // Shrink the columns
            Resize(db, 5, 4, 5, 2);
            // Shrink the rows and columns
            Resize(db, 5, 4, 3, 2);
            // Grow a large grid
            Resize(db, 500, 600, 550, 650);
            // Shrink a large grid
            Resize(db, 500, 600, 450, 550);
        }

        [TestMethod]
        public void ShouldGetRange()
        {
            var redis = ConnectionMultiplexer.Connect("10.11.153.125");
            var db = redis.GetDatabase();

            // Create and store a grid.
            var key = Guid.NewGuid().ToString();
            var source = GridExtensions.CreateOrdinalGrid(4, 5);
            db.GridDim(key, source);

            var fullGrid = db.GridRange(key, 0, 3, 0, 4).AsStringGrid(4, 5);
            Assert.IsTrue(GridExtensions.Equals(source, fullGrid));

            var fullGrid2 = db.GridRange(key, 0, -1, 0, -1).AsStringGrid(4, 5);
            Assert.IsTrue(GridExtensions.Equals(source, fullGrid2));

            var partGrid = db.GridRange(key, 0, 2, 0, 3).AsStringGrid(3, 4);
            Assert.IsTrue(GridExtensions.Equals(partGrid, new[,] { { "0", "1", "2", "3" }, { "5", "6", "7", "8" }, { "10", "11", "12", "13" } }));

            var partGrid2 = db.GridRange(key, 2, 0, 3, 0).AsStringGrid(3, 4);
            Assert.IsTrue(GridExtensions.Equals(partGrid2, new[,] { { "13", "12", "11", "10" }, { "8", "7", "6", "5" }, { "3", "2", "1", "0" } }));

            // Delete it.
            db.GridDim(key, 0, 0);
        }

        [TestMethod]
        public void ShouldSetRange()
        {
            var redis = ConnectionMultiplexer.Connect("10.11.153.125");
            var db = redis.GetDatabase();

            // Create and store a grid.
            var key = Guid.NewGuid().ToString();
            var source = GridExtensions.CreateOrdinalGrid(3, 4);
            db.GridDim(key, source);

            db.GridSet(key, 1, 1, new[,] {{"-1", "-2"}, {"-3", "-4"}});

            // Fetch it back.
            var grid = db.GridDump(key).AsStringGrid();
            Assert.IsTrue(GridExtensions.Equals(grid, new[,] { { "0", "1", "2", "3" }, { "4", "-1", "-2", "7" }, { "8", "-3", "-4", "11" } }));

            // Delete it.
            db.GridDim(key, 0, 0);
        }

        [TestMethod]
        public void ShouldGetShape()
        {
            var redis = ConnectionMultiplexer.Connect("10.11.153.125");
            var db = redis.GetDatabase();

            // Create and store a grid.
            var key = Guid.NewGuid().ToString();
            var source = GridExtensions.CreateOrdinalGrid(3, 4);
            db.GridDim(key, source);

            var bounds = db.GridShape(key);
            Assert.AreEqual(3, bounds[0]);
            Assert.AreEqual(4, bounds[1]);

            // Delete it.
            db.GridDim(key, 0, 0);
        }

        public void Resize(IDatabase db, int startRows, int startColumns, int endRows, int endColumns)
        {
            // Create and store a grid.
            var key = Guid.NewGuid().ToString();
            var source = GridExtensions.CreateOrdinalGrid(startRows, startColumns);
            db.GridDim(key, source);

            // Change the shape
            db.GridDim(key, endRows, endColumns);

            // Fetch it back.
            var grid = db.GridDump(key).AsStringGrid();

            // Check the shape.
            Assert.AreEqual(endRows, grid.GetRowCount());
            Assert.AreEqual(endColumns, grid.GetColumnCount());

            // Check the original values are still there
            for (var r = 0; r < Math.Min(startRows, endRows); ++r)
            for (var c = 0; c < Math.Min(startColumns, endColumns); ++c)
                Assert.AreEqual(source[r, c], grid[r, c]);

            // Check the new values are null.
            for (var r = 0; r < endRows; ++r)
            for (var c = 0; c < endColumns; ++c)
                if ((r >= startRows && r < endRows) || (c >= startColumns && c < endColumns))
                    Assert.IsNull(grid[r, c]);

            // Delete it.
            db.GridDim(key, 0, 0);
        }
    }
}

using System;

namespace Tests
{
    public static class GridExtensions
    {
        public static bool Equals<T>(this T[,] a, T[,] b)
        {
            // Check it's shape.
            if (a.Rank != b.Rank)
                return false;

            for (var i = 0; i < a.Rank; ++i)
            {
                if (a.GetLowerBound(i) != b.GetLowerBound(i))
                    return false;
                if (a.GetUpperBound(i) != b.GetUpperBound(i))
                    return false;
            }

            // Check it's values.
            for (var i = a.GetLowerBound(0); i < a.GetUpperBound(0); ++i)
                for (var j = a.GetLowerBound(1); j < a.GetUpperBound(1); ++j)
                    if (!Equals(a[i, j], b[i, j]))
                        return false;

            return true;
        }

        public static int GetRowCount(this string[,] grid)
        {
            return 1 + grid.GetUpperBound(0) - grid.GetLowerBound(0);
        }

        public static int GetColumnCount(this string[,] grid)
        {
            return 1 + grid.GetUpperBound(1) - grid.GetLowerBound(1);
        }

        public static string[,] CreateOrdinalGrid(int rows, int columns)
        {
            var grid = new String[rows, columns];
            for (int r = 0, i = 0; r < rows; ++r)
            for (var c = 0; c < columns; ++c, ++i)
                grid[r, c] = i.ToString();
            return grid;
        }
    }
}

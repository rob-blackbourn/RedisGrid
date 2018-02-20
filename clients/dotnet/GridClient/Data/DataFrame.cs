using System;
using System.Collections.Generic;
using System.Linq;
using StackExchange.Redis.Collections.Generic;

namespace StackExchange.Redis.Data
{
    /// <summary>
    /// An implementation of a DataFrame.
    /// </summary>
    public class DataFrame : IEquatable<DataFrame>
    {
        /// <summary>
        /// Construct a new data frame.
        /// </summary>
        /// <param name="columns">The columns from which the dataframe is constructed.</param>
        public DataFrame(params IVector[] columns)
            : this(null, null, columns)
        {
        }

        /// <summary>
        /// Construct a new data frame.
        /// </summary>
        /// <param name="columns">The columns from which the dataframe is constructed.</param>
        public DataFrame(IEnumerable<IVector> columns)
            : this(null, null, columns.ToArray())
        {
        }

        /// <summary>
        /// Construct a new data frame.
        /// </summary>
        /// <param name="name">The name of the data frame.</param>
        /// <param name="rowHeaders">The row headers</param>
        /// <param name="columns">The columns</param>
        public DataFrame(string name = null, IEnumerable<string> rowHeaders = null, params IVector[] columns)
            : this(name, rowHeaders?.ToList(), columns)
        {
        }

        /// <summary>
        /// Construct a new DataFRame.
        /// </summary>
        /// <param name="name">The name of the dataframe</param>
        /// <param name="rowHeaders">The row gheaders for the data frame</param>
        /// <param name="columns">The columns from which the dataframe is constructed.</param>
        public DataFrame(string name = null, List<string> rowHeaders = null, params IVector[] columns)
            : this(name, rowHeaders, columns.ToList())
        {
        }

        /// <summary>
        /// Construct a new DataFRame.
        /// </summary>
        /// <param name="name">The name of the dataframe</param>
        /// <param name="rowHeaders">The row gheaders for the data frame</param>
        /// <param name="columns">The columns from which the dataframe is constructed.</param>
        public DataFrame(string name = null, List<string> rowHeaders = null, List<IVector> columns = null)
        {
            Name = name;
            RowHeaders = rowHeaders;
            Columns = columns ?? new List<IVector>();
        }

        /// <summary>
        /// The name of the DataFrame.
        /// </summary>
        public string Name { get; set; }

        private List<string> _rowHeaders;

        /// <summary>
        /// The row headers.
        /// </summary>
        public List<string> RowHeaders
        {
            get => _rowHeaders;
            set => _rowHeaders = EnsureRowHeadersAreNullable(value ?? new List<string>());
        }

        /// <summary>
        /// The column vectors.
        /// </summary>
        public List<IVector> Columns { get; }

        /// <summary>
        /// A row enumeration
        /// </summary>
        public IEnumerable<KeyValuePair<string, object[]>> Rows => Enumerable.Range(0, Count).Select(r => KeyValuePair.Create(RowHeaders.Count == 0 ? null : RowHeaders[r], Columns.Select(column => column[r]).ToArray()));

        /// <summary>
        /// Add a new row.
        /// </summary>
        /// <param name="row">The row to add</param>
        /// <param name="header">The row header</param>
        public void Add(object[] row, string header = null)
        {
            if (row == null)
                throw new ArgumentNullException(nameof(row));
            if (row.Length != Columns.Count)
                throw new ArgumentException("There must be the same number of items in the row as there are columns in the data frame.", nameof(row));

            EnsureColumnsAreSameLength();

            var r = Count;
            for (var c = 0; c < Columns.Count; ++c)
                Columns[c].Add(row[c]);

            if (header != null)
                RowHeaders[r] = header;
        }

        /// <summary>
        /// Insert a new row into the data frame.
        /// </summary>
        /// <param name="index">The index of the row.</param>
        /// <param name="row">The row to insert</param>
        /// <param name="header">The optional row header</param>
        public void Insert(int index, object[] row, string header = null)
        {
            if (index < 0 || index > Count)
                throw new ArgumentOutOfRangeException();
            if (row == null)
                throw new ArgumentNullException(nameof(row));
            if (row.Length != Columns.Count)
                throw new ArgumentException("There must be the same number of items in the row as there are columns in the data frame.", nameof(row));

            EnsureColumnsAreSameLength();

            for (var c = 0; c < Columns.Count; ++c)
                Columns[c].Insert(index, row[c]);

            if (header != null)
                RowHeaders.Insert(index, header);
        }

        /// <summary>
        /// Access a cell in the DataFrame.
        /// </summary>
        /// <param name="c">The index of the column</param>
        /// <param name="r">The index of the row</param>
        /// <returns>A cell in the data frame</returns>
        public object this[int c, int r]
        {
            get => Columns[c][r];
            set => Columns[c][r] = value;
        }

        /// <summary>
        /// Access a cell in the DataFrame.
        /// </summary>
        /// <param name="c">The name of the column</param>
        /// <param name="r">The index of the row</param>
        /// <returns>A cell in the data frame</returns>
        public object this[string c, int r]
        {
            get => this[c][r];
            set => this[c][r] = value;
        }

        /// <summary>
        /// Access a column inn the DataFrame.
        /// </summary>
        /// <param name="index">The index of the column</param>
        /// <returns>The column vector.</returns>
        public IVector this[int index]
        {
            get => Columns[index];
            set => Columns[index] = value;
        }

        /// <summary>
        /// Access a column inn the DataFrame.
        /// </summary>
        /// <param name="name">The name of the column</param>
        /// <returns>The column vector.</returns>
        public IVector this[string name]
        {
            get { return Columns.FirstOrDefault(x => x.Name == name); }
            set
            {
                var index = Columns.IndexOf(x => x.Name == name);
                if (index < 0)
                    Columns.Add(value);
                else
                    Columns[index] = value;
            }
        }

        /// <summary>
        /// The number of rows in the dataframe.
        /// </summary>
        public int Count => Columns.Count == 0 ? 0 : Columns.Max(c => c.Count);

        private void EnsureColumnsAreSameLength()
        {
            var n = Count;

            foreach (var column in Columns)
                while (column.Count < n)
                    column.Add(null);

            if (RowHeaders.Count == 0)
                return;

            while (RowHeaders.Count < n)
                RowHeaders.Add(null);
        }

        private List<string> EnsureRowHeadersAreNullable(List<string> rowHeaders)
        {
            return rowHeaders;
        }

        /// <inheritdoc />
        public bool Equals(DataFrame other)
        {
            if (ReferenceEquals(null, other)) return false;
            if (ReferenceEquals(this, other)) return true;
            return string.Equals(Name, other.Name) &&
                _rowHeaders.SequenceEqual(other._rowHeaders) &&
                Columns.SequenceEqual(other.Columns);
        }

        /// <inheritdoc />
        public override bool Equals(object obj)
        {
            if (ReferenceEquals(null, obj)) return false;
            if (ReferenceEquals(this, obj)) return true;
            if (obj.GetType() != this.GetType()) return false;
            return Equals((DataFrame) obj);
        }

        /// <inheritdoc />
        public override int GetHashCode()
        {
            unchecked
            {
                var hashCode = (_rowHeaders != null ? _rowHeaders.GetHashCode() : 0);
                hashCode = (hashCode * 397) ^ (Name != null ? Name.GetHashCode() : 0);
                hashCode = (hashCode * 397) ^ (Columns != null ? Columns.GetHashCode() : 0);
                return hashCode;
            }
        }
    }
}

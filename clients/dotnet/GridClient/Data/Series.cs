using System;
using System.Collections;
using System.Collections.Generic;
using System.Data;
using System.Linq;

namespace StackExchange.Redis.Data
{
    /// <summary>
    /// An implementation of a strongly typed series.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public class Series<T> : ISeries<T>
    {
        private readonly List<T> _values;

        /// <summary>
        /// Construct a strongly typed series.
        /// </summary>
        /// <param name="values">The values with which to populated the series.</param>
        /// <param name="name">The name of the series.</param>
        /// <param name="isNullable">An optional boolean specifying the nullability</param>
        public Series(IEnumerable<T> values, string name = null, bool? isNullable = null)
            : this(values.ToList(), name, isNullable)
        {            
        }

        /// <summary>
        /// Construct a stringly typed series.
        /// </summary>
        /// <param name="values">The values to wrap with this series.</param>
        /// <param name="name">The optional name of the series.</param>
        /// <param name="isNullable">An optional boolean specifying the nullability</param>
        public Series(List<T> values, string name = null, bool? isNullable = null)
            : this(name, isNullable, values)
        {
        }

        /// <summary>
        /// Construct a strongly typed series.
        /// </summary>
        /// <param name="name">The optional name of the series.</param>
        /// <param name="isNullable">An optional boolean specifying the nullability</param>
        /// <param name="values">The values to wrap with this series.</param>
        public Series(string name = null, bool? isNullable = null, List<T> values = null)
        {
            if (isNullable == true && typeof(T).IsValueType && Nullable.GetUnderlyingType(typeof(T)) == null)
                throw new ArgumentException("A value type class cannot be nullable");

            Name = name;
            Type = typeof(T);
            IsNullable = isNullable ?? Nullable.GetUnderlyingType(typeof(T)) != null || typeof(T).IsClass;
            _values = values ?? new List<T>();
        }

        /// <inheritdoc />
        public string Name { get; set; }

        /// <inheritdoc />
        public Type Type { get; }

        /// <inheritdoc />
        public bool IsNullable { get; }

        /// <inheritdoc />
        public bool Remove(T item)
        {
            return _values.Remove(item);
        }

        /// <inheritdoc />
        void ICollection.CopyTo(Array array, int index)
        {
            for (int i = 0; i < _values.Count; ++i)
                array.SetValue(_values[i], index + i);
        }

        /// <inheritdoc />
        public int Count => _values.Count;

        /// <inheritdoc />
        object ICollection.SyncRoot => _values;

        /// <inheritdoc />
        bool ICollection.IsSynchronized => false;

        /// <inheritdoc />
        public bool IsReadOnly => false;

        /// <inheritdoc />
        bool IList.IsFixedSize => false;

        /// <inheritdoc />
        void IList.Remove(object value)
        {
            _values.Remove((T) value);
        }

        /// <inheritdoc />
        public void RemoveAt(int index)
        {
            _values.RemoveAt(index);
        }

        /// <inheritdoc />
        object IList.this[int index]
        {
            get => this[index];
            set => this[index] = (T)value;
        }

        /// <inheritdoc />
        public T this[int index]
        {
            get => _values[index];
            set => _values[index] = value;
        }

        /// <inheritdoc />
        public int IndexOf(T item)
        {
            return _values.IndexOf(item);
        }

        /// <inheritdoc />
        public void Insert(int index, T value)
        {
            if (index < 0 || index >= _values.Count)
                throw new ArgumentOutOfRangeException(nameof(index), $"Index {index} must be between 0 and Count");

            _values.Insert(index, value);
        }

        /// <inheritdoc />
        int IList.IndexOf(object value)
        {
            return _values.IndexOf((T) value);
        }

        /// <inheritdoc />
        void IList.Insert(int index, object value)
        {
            Insert(index, (T)value);
        }

        /// <inheritdoc />
        public void Add(T value)
        {
            if (!IsNullable && (object)value == null)
                throw new NoNullAllowedException();
            _values.Add(value);
        }

        /// <inheritdoc />
        int IList.Add(object value)
        {
            if (!IsNullable && value == null)
                throw new NoNullAllowedException();
            Add((T) value);
            return Count - 1;
        }

        /// <inheritdoc />
        bool IList.Contains(object value)
        {
            return _values.Contains((T) value);
        }

        /// <inheritdoc />
        public void Clear()
        {
            _values.Clear();
        }

        /// <inheritdoc />
        public bool Contains(T item)
        {
            return _values.Contains(item);
        }

        /// <inheritdoc />
        public void CopyTo(T[] array, int arrayIndex)
        {
            _values.CopyTo(array, arrayIndex);
        }

        /// <inheritdoc />
        public IEnumerator<T> GetEnumerator()
        {
            return _values.GetEnumerator();
        }

        /// <inheritdoc />
        IEnumerator IEnumerable.GetEnumerator()
        {
            return ((IEnumerable) _values).GetEnumerator();
        }

        public bool Equals(Series<T> other)
        {
            return Equals((ISeries<T>) other);
        }

        public bool Equals(ISeries<T> other)
        {
            return other != null &&
                string.Equals(Name, other.Name) && 
                Type == other.Type && 
                IsNullable == other.IsNullable &&
                _values.SequenceEqual(other);
        }

        public override bool Equals(object obj)
        {
            if (ReferenceEquals(null, obj)) return false;
            if (ReferenceEquals(this, obj)) return true;
            if (obj.GetType() != GetType()) return false;
            return Equals((Series<T>) obj);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                var hashCode = (_values != null ? _values.GetHashCode() : 0);
                hashCode = (hashCode * 397) ^ (Name != null ? Name.GetHashCode() : 0);
                hashCode = (hashCode * 397) ^ (Type != null ? Type.GetHashCode() : 0);
                hashCode = (hashCode * 397) ^ IsNullable.GetHashCode();
                return hashCode;
            }
        }

        public override string ToString()
        {
            return $"Name=\"{Name}\", Type={Type}, IsNullable={IsNullable}, Count={Count}";
        }
    }
}
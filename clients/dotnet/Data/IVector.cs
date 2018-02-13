using System;
using System.Collections;
using System.Collections.Generic;

namespace StackExchange.Redis.Data
{
    /// <summary>
    /// The interface for a weakly typed vector.
    /// </summary>
    public interface IVector : IList
    {
        /// <summary>
        /// The optional name of the vector.
        /// </summary>
        string Name { get; set; }

        /// <summary>
        /// The type of the vector.
        /// </summary>
        Type Type { get; }

        /// <summary>
        /// If true, the vector is nullable. Note that the underlying type must support the nullability.
        /// </summary>
        bool IsNullable { get; }
    }

    /// <summary>
    /// The interface for a strongly type vector.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public interface IVector<T> : IVector, IList<T>, IEquatable<IVector<T>>
    {
    }
}
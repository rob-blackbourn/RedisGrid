using System;
using System.Collections;
using System.Collections.Generic;

namespace StackExchange.Redis.Data
{
    /// <summary>
    /// The interface for a weakly typed series.
    /// </summary>
    public interface ISeries : IList
    {
        /// <summary>
        /// The optional name of the series.
        /// </summary>
        string Name { get; set; }

        /// <summary>
        /// The type of the series.
        /// </summary>
        Type Type { get; }

        /// <summary>
        /// If true, the series is nullable. Note that the underlying type must support the nullability.
        /// </summary>
        bool IsNullable { get; }
    }

    /// <summary>
    /// The interface for a strongly type series.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public interface ISeries<T> : ISeries, IList<T>, IEquatable<ISeries<T>>
    {
    }
}
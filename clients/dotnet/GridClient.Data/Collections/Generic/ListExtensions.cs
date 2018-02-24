using System;
using System.Collections.Generic;

namespace StackExchange.Redis.Collections.Generic
{
    /// <summary>
    /// Extension methods for generic lists.
    /// </summary>
    public static class ListExtensions
    {
        /// <summary>
        /// Find the index of an item in a list.
        /// </summary>
        /// <typeparam name="T">The type of the elements in the list.</typeparam>
        /// <param name="source">The list to be indexed.</param>
        /// <param name="predicate">The function tp be applied to determine the index.</param>
        /// <returns>If the item was found the index was returned, otherwise -1.</returns>
        public static int IndexOf<T>(this IReadOnlyList<T> source,  Func<T, bool> predicate)
        {
            for (var i = 0; i < source.Count; ++i)
                if (predicate(source[i]))
                    return i;
            return -1;
        }
    }
}
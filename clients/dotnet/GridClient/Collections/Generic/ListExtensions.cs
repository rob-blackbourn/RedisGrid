using System;
using System.Collections.Generic;

namespace StackExchange.Redis.Collections.Generic
{
    public static class ListExtensions
    {
        public static int IndexOf<T>(this IReadOnlyList<T> source,  Func<T, bool> predicate)
        {
            for (var i = 0; i < source.Count; ++i)
                if (predicate(source[i]))
                    return i;
            return -1;
        }
    }
}
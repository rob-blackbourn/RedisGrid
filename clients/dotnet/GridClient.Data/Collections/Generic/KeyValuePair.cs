using System.Collections.Generic;

namespace StackExchange.Redis.Collections.Generic
{
    /// <summary>
    /// A helper class for key value pairs.
    /// </summary>
    public static class KeyValuePair
    {
        /// <summary>
        /// Create a key value pair.
        /// </summary>
        /// <typeparam name="TKey">The type of the key.</typeparam>
        /// <typeparam name="TValue">The type of the value.</typeparam>
        /// <param name="key">The key.</param>
        /// <param name="value">The value.</param>
        /// <returns>A key value pair.</returns>
        public static KeyValuePair<TKey, TValue> Create<TKey, TValue>(TKey key, TValue value)
        {
            return new KeyValuePair<TKey, TValue>(key, value);
        }
    }
}

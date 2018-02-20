using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using StackExchange.Redis.ComponentModel;

namespace StackExchange.Redis.Data
{
    public static class VectorExtensions
    {
        public static Vector<T> ToVector<T>(this List<T> source, string name = null, bool? isNullable = null)
        {
            return new Vector<T>(name, isNullable, source);
        }

        public static Vector<T> ToVector<T>(this IEnumerable<T> source, string name = null, bool? isNullable = null)
        {
            return new Vector<T>(name, isNullable, source?.ToList());
        }

        public static IVector CreateVector<TSource, TResult>(this IEnumerable<TSource> source, Expression<Func<TSource,TResult>> selectorExpression, bool? isNullable = null)
        {
            var propertyName = PropertySupport.ExtractPropertyName(selectorExpression);
            var selector = selectorExpression.Compile();
            return CreateVector(source, typeof(TResult), propertyName, x => selector((TSource)x), isNullable);
        }

        public static IVector CreateVector<T>(this IEnumerable<T> source, string propertyName, bool? isNullable = null)
        {
            var sourceType = typeof(T);
            var property = sourceType.GetProperty(propertyName);
            if (property == null)
                throw new ArgumentException($"Failed to find property \"{propertyName}\" for type {sourceType}", nameof(propertyName));

            return CreateVector(source, property.PropertyType, property.Name, property.GetValue, isNullable);
        }

        public static IVector CreateVector(this System.Collections.IEnumerable source, Type propertyType, string name, Func<object,object> selector, bool? isNullable = null)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));
            if (propertyType == null)
                throw new ArgumentNullException(nameof(propertyType));
            if (selector == null)
                throw new ArgumentNullException(nameof(selector));

            var types = new[] { propertyType };
            var vectorType = typeof(Vector<>);
            var genericVectorType = vectorType.MakeGenericType(types);
            var enumerableType = typeof(List<>);
            var genericEnumerableType = enumerableType.MakeGenericType(types);
            var constructor = genericVectorType.GetConstructor(new[] { typeof(string), typeof(bool?), genericEnumerableType });
            if (constructor == null)
                throw new TypeAccessException($"Failed to find constructor for Vector<{propertyType}>");

            var vector = (IVector)constructor.Invoke(new object[] { name, isNullable, null });
            foreach (var value in source)
                vector.Add(selector(value));

            return vector;
        }
    }
}

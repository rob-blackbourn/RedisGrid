using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using StackExchange.Redis.ComponentModel;

namespace StackExchange.Redis.Data
{
    public static class SeriesExtensions
    {
        /// <summary>
        /// Create a series from a list.
        /// </summary>
        /// <typeparam name="T">The type of the series.</typeparam>
        /// <param name="source">The series data.</param>
        /// <param name="name">An optional series name.</param>
        /// <param name="isNullable">Optionally specify nullability for object types.</param>
        /// <returns>A series.</returns>
        public static Series<T> ToSeries<T>(this List<T> source, string name = null, bool? isNullable = null)
        {
            return new Series<T>(name, isNullable, source);
        }

        /// <summary>
        /// Create a series from an enumerable.
        /// </summary>
        /// <typeparam name="T">The type of the series.</typeparam>
        /// <param name="source">The series data.</param>
        /// <param name="name">An optional series name.</param>
        /// <param name="isNullable">Optionally specify nullability for object types.</param>
        /// <returns>A series.</returns>
        public static Series<T> ToSeries<T>(this IEnumerable<T> source, string name = null, bool? isNullable = null)
        {
            return new Series<T>(name, isNullable, source?.ToList());
        }

        /// <summary>
        /// Create a series from an enumerable with a projection function.
        /// </summary>
        /// <typeparam name="TSource">The type of the source data.</typeparam>
        /// <typeparam name="TResult">The type of the series.</typeparam>
        /// <param name="source">An enumerable of data from which the series should be built.</param>
        /// <param name="selectorExpression">A projection function.</param>
        /// <param name="isNullable">Optionally specify nullability for object types.</param>
        /// <returns>A series.</returns>
        public static ISeries CreateSeries<TSource, TResult>(this IEnumerable<TSource> source, Expression<Func<TSource,TResult>> selectorExpression, bool? isNullable = null)
        {
            var propertyName = PropertySupport.ExtractPropertyName(selectorExpression);
            var selector = selectorExpression.Compile();
            return CreateSeries(source, typeof(TResult), propertyName, x => selector((TSource)x), isNullable);
        }

        /// <summary>
        /// Create a series from an enumerable specifying a string property name for each element of the source enumerable.
        /// </summary>
        /// <typeparam name="T">The type of the source data.</typeparam>
        /// <param name="source">The data from which the series is built.</param>
        /// <param name="propertyName">The property name of the series elements from which the data will be taken.</param>
        /// <param name="isNullable">Optionally specify nullability for object types.</param>
        /// <returns>A series.</returns>
        public static ISeries CreateSeries<T>(this IEnumerable<T> source, string propertyName, bool? isNullable = null)
        {
            var sourceType = typeof(T);
            var property = sourceType.GetProperty(propertyName);
            if (property == null)
                throw new ArgumentException($"Failed to find property \"{propertyName}\" for type {sourceType}", nameof(propertyName));

            return CreateSeries(source, property.PropertyType, property.Name, property.GetValue, isNullable);
        }

        public static ISeries CreateSeries(this System.Collections.IEnumerable source, Type propertyType, string name, Func<object,object> selector, bool? isNullable = null)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));
            if (propertyType == null)
                throw new ArgumentNullException(nameof(propertyType));
            if (selector == null)
                throw new ArgumentNullException(nameof(selector));

            var types = new[] { propertyType };
            var seriesType = typeof(Series<>);
            var genericSeriesType = seriesType.MakeGenericType(types);
            var enumerableType = typeof(List<>);
            var genericEnumerableType = enumerableType.MakeGenericType(types);
            var constructor = genericSeriesType.GetConstructor(new[] { typeof(string), typeof(bool?), genericEnumerableType });
            if (constructor == null)
                throw new TypeAccessException($"Failed to find constructor for Series<{propertyType}>");

            var series = (ISeries)constructor.Invoke(new object[] { name, isNullable, null });
            foreach (var value in source)
                series.Add(selector(value));

            return series;
        }
    }
}

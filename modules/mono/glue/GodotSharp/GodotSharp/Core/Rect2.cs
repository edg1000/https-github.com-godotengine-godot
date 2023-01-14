using System;
using System.Runtime.InteropServices;

namespace Godot
{
    /// <summary>
    /// 2D axis-aligned bounding box. Rect2 consists of a position, a size, and
    /// several utility functions. It is typically used for fast overlap tests.
    /// </summary>
    [Serializable]
    [StructLayout(LayoutKind.Sequential)]
    public struct Rect2 : IEquatable<Rect2>
    {
        private Vector2 _position;
        private Vector2 _size;

        /// <summary>
        /// Beginning corner. Typically has values lower than <see cref="End"/>.
        /// </summary>
        /// <value>Directly uses a private field.</value>
        public Vector2 Position
        {
            readonly get { return _position; }
            set { _position = value; }
        }

        /// <summary>
        /// Size from <see cref="Position"/> to <see cref="End"/>. Typically all components are positive.
        /// If the size is negative, you can use <see cref="Abs"/> to fix it.
        /// </summary>
        /// <value>Directly uses a private field.</value>
        public Vector2 Size
        {
            readonly get { return _size; }
            set { _size = value; }
        }

        /// <summary>
        /// Ending corner. This is calculated as <see cref="Position"/> plus <see cref="Size"/>.
        /// Setting this value will change the size.
        /// </summary>
        /// <value>
        /// Getting is equivalent to <paramref name="value"/> = <see cref="Position"/> + <see cref="Size"/>,
        /// setting is equivalent to <see cref="Size"/> = <paramref name="value"/> - <see cref="Position"/>
        /// </value>
        public Vector2 End
        {
            readonly get { return _position + _size; }
            set { _size = value - _position; }
        }

        /// <summary>
        /// Returns a <see cref="Rect2"/> with equivalent position and size, modified so that
        /// the top-left corner is the origin and width and height are positive.
        /// </summary>
        /// <returns>The modified <see cref="Rect2"/>.</returns>
        public readonly Rect2 Abs()
        {
            Vector2 end = End;
            Vector2 topLeft = new Vector2(Mathf.Min(_position.x, end.x), Mathf.Min(_position.y, end.y));
            return new Rect2(topLeft, _size.Abs());
        }

        /// <summary>
        /// Returns the intersection of this <see cref="Rect2"/> and <paramref name="b"/>.
        /// If the rectangles do not intersect, an empty <see cref="Rect2"/> is returned.
        /// </summary>
        /// <param name="b">The other <see cref="Rect2"/>.</param>
        /// <returns>
        /// The intersection of this <see cref="Rect2"/> and <paramref name="b"/>,
        /// or an empty <see cref="Rect2"/> if they do not intersect.
        /// </returns>
        public readonly Rect2 Intersection(Rect2 b)
        {
            Rect2 newRect = b;

            if (!Intersects(newRect))
            {
                return new Rect2();
            }

            newRect._position.x = Mathf.Max(b._position.x, _position.x);
            newRect._position.y = Mathf.Max(b._position.y, _position.y);

            Vector2 bEnd = b._position + b._size;
            Vector2 end = _position + _size;

            newRect._size.x = Mathf.Min(bEnd.x, end.x) - newRect._position.x;
            newRect._size.y = Mathf.Min(bEnd.y, end.y) - newRect._position.y;

            return newRect;
        }

        /// <summary>
        /// Returns <see langword="true"/> if this <see cref="Rect2"/> is finite, by calling
        /// <see cref="Mathf.IsFinite"/> on each component.
        /// </summary>
        /// <returns>Whether this vector is finite or not.</returns>
        public bool IsFinite()
        {
            return _position.IsFinite() && _size.IsFinite();
        }

        /// <summary>
        /// Returns <see langword="true"/> if this <see cref="Rect2"/> completely encloses another one.
        /// </summary>
        /// <param name="b">The other <see cref="Rect2"/> that may be enclosed.</param>
        /// <returns>
        /// A <see langword="bool"/> for whether or not this <see cref="Rect2"/> encloses <paramref name="b"/>.
        /// </returns>
        public readonly bool Encloses(Rect2 b)
        {
            return b._position.x >= _position.x && b._position.y >= _position.y &&
               b._position.x + b._size.x < _position.x + _size.x &&
               b._position.y + b._size.y < _position.y + _size.y;
        }

        /// <summary>
        /// Returns this <see cref="Rect2"/> expanded to include a given point.
        /// </summary>
        /// <param name="to">The point to include.</param>
        /// <returns>The expanded <see cref="Rect2"/>.</returns>
        public readonly Rect2 Expand(Vector2 to)
        {
            Rect2 expanded = this;

            Vector2 begin = expanded._position;
            Vector2 end = expanded._position + expanded._size;

            if (to.x < begin.x)
            {
                begin.x = to.x;
            }
            if (to.y < begin.y)
            {
                begin.y = to.y;
            }

            if (to.x > end.x)
            {
                end.x = to.x;
            }
            if (to.y > end.y)
            {
                end.y = to.y;
            }

            expanded._position = begin;
            expanded._size = end - begin;

            return expanded;
        }

        /// <summary>
        /// Returns the area of the <see cref="Rect2"/>.
        /// See also <see cref="HasArea"/>.
        /// </summary>
        /// <returns>The area.</returns>
        public readonly real_t GetArea()
        {
            return _size.x * _size.y;
        }

        /// <summary>
        /// Returns the center of the <see cref="Rect2"/>, which is equal
        /// to <see cref="Position"/> + (<see cref="Size"/> / 2).
        /// </summary>
        /// <returns>The center.</returns>
        public readonly Vector2 GetCenter()
        {
            return _position + (_size * 0.5f);
        }

        /// <summary>
        /// Returns a copy of the <see cref="Rect2"/> grown by the specified amount
        /// on all sides.
        /// </summary>
        /// <seealso cref="GrowIndividual(real_t, real_t, real_t, real_t)"/>
        /// <seealso cref="GrowSide(Side, real_t)"/>
        /// <param name="by">The amount to grow by.</param>
        /// <returns>The grown <see cref="Rect2"/>.</returns>
        public readonly Rect2 Grow(real_t by)
        {
            Rect2 g = this;

            g._position.x -= by;
            g._position.y -= by;
            g._size.x += by * 2;
            g._size.y += by * 2;

            return g;
        }

        /// <summary>
        /// Returns a copy of the <see cref="Rect2"/> grown by the specified amount
        /// on each side individually.
        /// </summary>
        /// <seealso cref="Grow(real_t)"/>
        /// <seealso cref="GrowSide(Side, real_t)"/>
        /// <param name="left">The amount to grow by on the left side.</param>
        /// <param name="top">The amount to grow by on the top side.</param>
        /// <param name="right">The amount to grow by on the right side.</param>
        /// <param name="bottom">The amount to grow by on the bottom side.</param>
        /// <returns>The grown <see cref="Rect2"/>.</returns>
        public readonly Rect2 GrowIndividual(real_t left, real_t top, real_t right, real_t bottom)
        {
            Rect2 g = this;

            g._position.x -= left;
            g._position.y -= top;
            g._size.x += left + right;
            g._size.y += top + bottom;

            return g;
        }

        /// <summary>
        /// Returns a copy of the <see cref="Rect2"/> grown by the specified amount
        /// on the specified <see cref="Side"/>.
        /// </summary>
        /// <seealso cref="Grow(real_t)"/>
        /// <seealso cref="GrowIndividual(real_t, real_t, real_t, real_t)"/>
        /// <param name="side">The side to grow.</param>
        /// <param name="by">The amount to grow by.</param>
        /// <returns>The grown <see cref="Rect2"/>.</returns>
        public readonly Rect2 GrowSide(Side side, real_t by)
        {
            Rect2 g = this;

            g = g.GrowIndividual(Side.Left == side ? by : 0,
                    Side.Top == side ? by : 0,
                    Side.Right == side ? by : 0,
                    Side.Bottom == side ? by : 0);

            return g;
        }

        /// <summary>
        /// Returns <see langword="true"/> if the <see cref="Rect2"/> has
        /// area, and <see langword="false"/> if the <see cref="Rect2"/>
        /// is linear, empty, or has a negative <see cref="Size"/>.
        /// See also <see cref="GetArea"/>.
        /// </summary>
        /// <returns>
        /// A <see langword="bool"/> for whether or not the <see cref="Rect2"/> has area.
        /// </returns>
        public readonly bool HasArea()
        {
            return _size.x > 0.0f && _size.y > 0.0f;
        }

        /// <summary>
        /// Returns <see langword="true"/> if the <see cref="Rect2"/> contains a point,
        /// or <see langword="false"/> otherwise.
        /// </summary>
        /// <param name="point">The point to check.</param>
        /// <returns>
        /// A <see langword="bool"/> for whether or not the <see cref="Rect2"/> contains <paramref name="point"/>.
        /// </returns>
        public readonly bool HasPoint(Vector2 point)
        {
            if (point.x < _position.x)
                return false;
            if (point.y < _position.y)
                return false;

            if (point.x >= _position.x + _size.x)
                return false;
            if (point.y >= _position.y + _size.y)
                return false;

            return true;
        }

        /// <summary>
        /// Returns <see langword="true"/> if the <see cref="Rect2"/> overlaps with <paramref name="b"/>
        /// (i.e. they have at least one point in common).
        ///
        /// If <paramref name="includeBorders"/> is <see langword="true"/>,
        /// they will also be considered overlapping if their borders touch,
        /// even without intersection.
        /// </summary>
        /// <param name="b">The other <see cref="Rect2"/> to check for intersections with.</param>
        /// <param name="includeBorders">Whether or not to consider borders.</param>
        /// <returns>A <see langword="bool"/> for whether or not they are intersecting.</returns>
        public readonly bool Intersects(Rect2 b, bool includeBorders = false)
        {
            if (includeBorders)
            {
                if (_position.x > b._position.x + b._size.x)
                {
                    return false;
                }
                if (_position.x + _size.x < b._position.x)
                {
                    return false;
                }
                if (_position.y > b._position.y + b._size.y)
                {
                    return false;
                }
                if (_position.y + _size.y < b._position.y)
                {
                    return false;
                }
            }
            else
            {
                if (_position.x >= b._position.x + b._size.x)
                {
                    return false;
                }
                if (_position.x + _size.x <= b._position.x)
                {
                    return false;
                }
                if (_position.y >= b._position.y + b._size.y)
                {
                    return false;
                }
                if (_position.y + _size.y <= b._position.y)
                {
                    return false;
                }
            }

            return true;
        }

        /// <summary>
        /// Returns a larger <see cref="Rect2"/> that contains this <see cref="Rect2"/> and <paramref name="b"/>.
        /// </summary>
        /// <param name="b">The other <see cref="Rect2"/>.</param>
        /// <returns>The merged <see cref="Rect2"/>.</returns>
        public readonly Rect2 Merge(Rect2 b)
        {
            Rect2 newRect;

            newRect._position.x = Mathf.Min(b._position.x, _position.x);
            newRect._position.y = Mathf.Min(b._position.y, _position.y);

            newRect._size.x = Mathf.Max(b._position.x + b._size.x, _position.x + _size.x);
            newRect._size.y = Mathf.Max(b._position.y + b._size.y, _position.y + _size.y);

            newRect._size -= newRect._position; // Make relative again

            return newRect;
        }

        /// <summary>
        /// Constructs a <see cref="Rect2"/> from a position and size.
        /// </summary>
        /// <param name="position">The position.</param>
        /// <param name="size">The size.</param>
        public Rect2(Vector2 position, Vector2 size)
        {
            _position = position;
            _size = size;
        }

        /// <summary>
        /// Constructs a <see cref="Rect2"/> from a position, width, and height.
        /// </summary>
        /// <param name="position">The position.</param>
        /// <param name="width">The width.</param>
        /// <param name="height">The height.</param>
        public Rect2(Vector2 position, real_t width, real_t height)
        {
            _position = position;
            _size = new Vector2(width, height);
        }

        /// <summary>
        /// Constructs a <see cref="Rect2"/> from x, y, and size.
        /// </summary>
        /// <param name="x">The position's X coordinate.</param>
        /// <param name="y">The position's Y coordinate.</param>
        /// <param name="size">The size.</param>
        public Rect2(real_t x, real_t y, Vector2 size)
        {
            _position = new Vector2(x, y);
            _size = size;
        }

        /// <summary>
        /// Constructs a <see cref="Rect2"/> from x, y, width, and height.
        /// </summary>
        /// <param name="x">The position's X coordinate.</param>
        /// <param name="y">The position's Y coordinate.</param>
        /// <param name="width">The width.</param>
        /// <param name="height">The height.</param>
        public Rect2(real_t x, real_t y, real_t width, real_t height)
        {
            _position = new Vector2(x, y);
            _size = new Vector2(width, height);
        }

        /// <summary>
        /// Returns <see langword="true"/> if the
        /// <see cref="Rect2"/>s are exactly equal.
        /// Note: Due to floating-point precision errors, consider using
        /// <see cref="IsEqualApprox"/> instead, which is more reliable.
        /// </summary>
        /// <param name="left">The left rect.</param>
        /// <param name="right">The right rect.</param>
        /// <returns>Whether or not the rects are exactly equal.</returns>
        public static bool operator ==(Rect2 left, Rect2 right)
        {
            return left.Equals(right);
        }

        /// <summary>
        /// Returns <see langword="true"/> if the
        /// <see cref="Rect2"/>s are not equal.
        /// Note: Due to floating-point precision errors, consider using
        /// <see cref="IsEqualApprox"/> instead, which is more reliable.
        /// </summary>
        /// <param name="left">The left rect.</param>
        /// <param name="right">The right rect.</param>
        /// <returns>Whether or not the rects are not equal.</returns>
        public static bool operator !=(Rect2 left, Rect2 right)
        {
            return !left.Equals(right);
        }

        /// <summary>
        /// Returns <see langword="true"/> if this rect and <paramref name="obj"/> are equal.
        /// </summary>
        /// <param name="obj">The other object to compare.</param>
        /// <returns>Whether or not the rect and the other object are exactly equal.</returns>
        public override readonly bool Equals(object obj)
        {
            return obj is Rect2 other && Equals(other);
        }

        /// <summary>
        /// Returns <see langword="true"/> if this rect and <paramref name="other"/> are equal.
        /// </summary>
        /// <param name="other">The other rect to compare.</param>
        /// <returns>Whether or not the rects are exactly equal.</returns>
        public readonly bool Equals(Rect2 other)
        {
            return _position.Equals(other._position) && _size.Equals(other._size);
        }

        /// <summary>
        /// Returns <see langword="true"/> if this rect and <paramref name="other"/> are approximately equal,
        /// by running <see cref="Vector2.IsEqualApprox(Vector2)"/> on each component.
        /// </summary>
        /// <param name="other">The other rect to compare.</param>
        /// <returns>Whether or not the rects are approximately equal.</returns>
        public readonly bool IsEqualApprox(Rect2 other)
        {
            return _position.IsEqualApprox(other._position) && _size.IsEqualApprox(other.Size);
        }

        /// <summary>
        /// Serves as the hash function for <see cref="Rect2"/>.
        /// </summary>
        /// <returns>A hash code for this rect.</returns>
        public override readonly int GetHashCode()
        {
            return _position.GetHashCode() ^ _size.GetHashCode();
        }

        /// <summary>
        /// Converts this <see cref="Rect2"/> to a string.
        /// </summary>
        /// <returns>A string representation of this rect.</returns>
        public override readonly string ToString()
        {
            return $"{_position}, {_size}";
        }

        /// <summary>
        /// Converts this <see cref="Rect2"/> to a string with the given <paramref name="format"/>.
        /// </summary>
        /// <returns>A string representation of this rect.</returns>
        public readonly string ToString(string format)
        {
            return $"{_position.ToString(format)}, {_size.ToString(format)}";
        }
    }
}

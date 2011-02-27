#ifndef     _VECTOR2_H_
#define     _VECTOR2_H_

/**
 *  Representation of a vector in 2d space.
 *  @brief  2x1 Vector class.
 */
template <class T>
class Vector2
{
public:
    /**
     *  Constructor.
     */
    Vector2() {}

    /**
     *  Constructor accepting initial values.
     *  @param  fX  A T representing the x-axis.
     *  @param  fY  A T representing the y-axis.
     */
    Vector2( const T& pX, const T& pY ):
        x(pX), y(pY)
    {
    }

    /**
     *  Copy constructor.
     *  @param  pOther A vector to be copied.
     */
    Vector2( const Vector2& pOther ) :
        x(pOther.x), y(pOther.y)
    {
    }

    /**
     *  Set this vector to the content of another vector.
     *  @brief  Assignement operator.
     *  @param  pOther   A vector to be copied.
     *  @return Reference to this vector object.
     */
    const Vector2& operator = ( const Vector2& pOther )
    {
        x = pOther.x;    y = pOther.y;
        return *this;
    }

    /**
     *  Test if this vector is equal to another vector.
     *  @brief  Equal operator.
     *  @param  pOther   A vector to be compared to.
     *  @return A boolean, \b true if the two vector are equal (all members are equals), or \b false otherwise.
     */
    bool operator == ( const Vector2& pOther ) const
    {
        return ( x == pOther.x && y == pOther.y );
    }

    /**
     *  Test if this vector is not equal to another vector.
     *  @brief  Not equal operator.
     *  @param  pOther   A vector to be compared to.
     *  @return A boolean, \b true if the two vector are not equal (all members are not equal), or \b false otherwise.
     */
    bool operator != ( const Vector2& pOther ) const
    {
        return !( (*this) == pOther );
    }

    /**
     *  Invert the sign of the vector.
     *  @brief  Unary minus operator.
     *  @return A new vector, same as this one but with the signs of all the elements inverted.
     */
    Vector2 operator - () const
    {
        return Vector2( -x, -y );
    }

    /**
     *  Add a vector to this vector.
     *  @brief  Addition operator.
     *  @param  pOther   a vector, to be added to this vector.
     *  @return A new vector, the result of the addition of the two vector.
     */
    Vector2 operator + ( const Vector2& pOther ) const
    {
        return Vector2( x + pOther.x, y + pOther.y );
    }

    /**
     *  Substract a vector to this vector.
     *  @brief  Substraction operator.
     *  @param  pOther   a vector, to be substracted to this vector.
     *  @return A new vector, the result of the substraction of the two vector.
     */
    Vector2 operator - ( const Vector2& pOther ) const
    {
        return Vector2( x - pOther.x, y - pOther.y );
    }

    /**
     *  Divide this vector by a scale factor.
     *  @brief  Division operator for a scalar.
     *  @param  pScale  value to divide this vector by.
     *  @return A new vector, the result of the division.
     */
    Vector2 operator / ( const T& pScale ) const
    {
        return Vector2( x / pScale, y / pScale );
    }

    /**
     *  Multiply this vector by a scalar.
     *  @brief  Multiplication operator for a scalar.
     *  @param  pScale  The vector will be multiplied by this value.
     *  @return A new vector which is the result of the operation.
     */
    Vector2 operator * ( const T& pScale ) const
    {
        return Vector2( x * pScale, y * pScale );
    }

    /**
     *  Add the content of another vector to this vector.
     *  @brief  Addition assignment operator for a vector.
     *  @param  pOther  The vector to add to this vector.
     *  @return A const reference to this vector.
     */
    const Vector2& operator += ( const Vector2& pOther )
    {
        x += pOther.x;   y += pOther.y;
        return *this;
    }

    /**
     *  Substract the content of another vector to this vector.
     *  @brief  Substraction assigment operator for a vector.
     *  @param  pOther  The vector to substract from this vector.
     *  @return A const reference to this vector.
     */
    const Vector2& operator -= ( const Vector2& pOther )
    {
        x -= pOther.x;   y -= pOther.y;
        return *this;
    }

    /**
     *  Divide this vector by a factor.
     *  @brief  Division assigment operator for a scalar.
     *  @param  pScale  Value to be used for the division.
     *  @return A const reference to this vector.
     */
    const Vector2& operator /= ( const T& pScale )
    {
        x /= pScale;    y /= pScale;
        return *this;
    }

    /**
     *  Multiply this vector by a factor.
     *  @brief  Multiplication assigment operator for a scalar.
     *  @param  pScale  Value to be used for the multiplication
     *  @return A const reference to this vector.
     */
    const Vector2& operator *= ( const T& pScale )
    {
        x *= pScale;    y *= pScale;
        return *this;
    }

    /**
     *  Access vector elements by their indexes.
     *  @brief  Function call operator used to access vector elements.
     *  @param  pIndex  Index of the element to obtain.
     *  @return A reference to the element at index pIndex.
     */
    T& operator () ( const int& pIndex )
    {
        return elem[pIndex];
    }

    /**
     *  Access to the internal array of the vector.
     *  @brief Indirection operator (const).
     *  @return Internal array used to store the vector values.
     */
    operator const T*() const
    {
        return elem;
    }

    /**
     *  Access to the internal array of the vector.
     *  @brief Indirection operator.
     *  @return Internal array used to store the vector values.
     */
    operator T*()
    {
        return elem;
    }

    /**
     *  Normalize the vector (divide by its length).
     *  @brief  Normalize the vector.
     *  @return Reference to the vector.
     */
    const Vector2& Normalize()
    {
        T length = GetLength();
        GD_ASSERT( length != 0 );

        x /= length;   y /= length;
        return *this;
    }

    /**
     *  Create a normalized copy of this vector (Divide by its length).
     *  @brief Create a normalized copy of this vector.
     *  @return A normalized copy of the vector.
     */
    Vector2 GetNormalized() const
    {
        GD_ASSERT( GetLength() != 0 );
        return ( (*this) / GetLength() );
    }

	/**
     *  Returns true if the vector is normalized, false otherwise.
     */
    bool IsNormalized() const
    {
        return GetLength() == 1;
    }

    /**
     *  Get the length of this vector.
     *  @return The length of this vector.
     */
    T GetLength() const
    {
        return Maths::Sqrt( x*x + y*y );
    }

    /**
     *  Get the squared length of this vector.
     *  @return The squared length of this vector.
     */
    T GetLengthSqr() const
    {
        return x*x + y*y;
    }

    /**
     *  Get the distance between two vectors.
     *  @param  pOther  The other vector.
     *  @return The distance between the two vectors.
     */
    T GetDistanceFrom( const Vector2& pOther ) const
    {
        return Vector2( pOther - *this ).GetLength();
    }

    /**
     *  Check if two vector are equals using an epsilon.
     *  @param  pOther  The other vector.
     *  @param  pEps    The epsilon value.
     *  @return \bTrue if the vectors values fit in the epsilon range.
     */
    bool EqualEps( const Vector2& pOther, T pEps = EEdef_EPSILON ) const
    {
        return  x > (pOther.x - pEps) && x < (pOther.x + pEps) &&
                y > (pOther.y - pEps) && y < (pOther.y + pEps);
    }

    union
    {
        T elem[2];          //!< This vector as a 2 elements array.
        struct
        {
            T x;            //!< X component of the vector.
            T y;            //!< Y component of the vector.
        };
    };

    static const Vector2 X_AXIS;    //!< The X axis.
    static const Vector2 Y_AXIS;    //!< The Y axis.
    static const Vector2 ZERO;      //!< A null vector.
};


typedef Vector2<int>	 Vector2i;
typedef Vector2<float>   Vector2f;
typedef Vector2<double>  Vector2d;


// Constants
template<class T> const Vector2<T> Vector2<T>::X_AXIS( T(1), T(0) );
template<class T> const Vector2<T> Vector2<T>::Y_AXIS( T(0), T(1) );
template<class T> const Vector2<T> Vector2<T>::ZERO  ( T(0), T(0) );


#endif  //  _VECTOR2_H_

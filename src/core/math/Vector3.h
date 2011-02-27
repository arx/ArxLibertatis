#ifndef     _VECTOR3_H_
#define     _VECTOR3_H_

#define cross   ^
#define dot     |

/**
 *  Representation of a vector in 3d space.
 *  @brief  3x1 Vector class.
 */
template <class T>
class Vector3
{
public:
    /**
     *  Constructor.
     */
    Vector3() {}

    /**
     *  Constructor accepting initial values.
     *  @param  fX  A T representing the x-axis.
     *  @param  fY  A T representing the y-axis.
     *  @param  fZ  A T representing the z-axis.
     */
    Vector3( const T& pX, const T& pY, const T& pZ ):
        x(pX), y(pY), z(pZ)
    {
    }

    /**
     *  Copy constructor.
     *  @param  pOther A vector to be copied.
     */
    Vector3( const Vector3& pOther ) :
        x(pOther.x), y(pOther.y), z(pOther.z)
    {
    }

    /**
     *  Set this vector to the content of another vector.
     *  @brief  Assignement operator.
     *  @param  pOther   A vector to be copied.
     *  @return Reference to this vector object.
     */
    const Vector3& operator = ( const Vector3& pOther )
    {
        x = pOther.x;    y = pOther.y;    z = pOther.z;
        return *this;
    }

    /**
     *  Test if this vector is equal to another vector.
     *  @brief  Equal operator.
     *  @param  pOther   A vector to be compared to.
     *  @return A boolean, \b true if the two vector are equal (all members are equals), or \b false otherwise.
     */
    bool operator == ( const Vector3& pOther ) const
    {
        return ( x == pOther.x && y == pOther.y && z == pOther.z );
    }

    /**
     *  Test if this vector is not equal to another vector.
     *  @brief  Not equal operator.
     *  @param  pOther   A vector to be compared to.
     *  @return A boolean, \b true if the two vector are not equal (all members are not equal), or \b false otherwise.
     */
    bool operator != ( const Vector3& pOther ) const
    {
        return !( (*this) == pOther );
    }

    bool operator < ( const Vector3& pOther ) const
    {
        return ( x < pOther.x && y < pOther.y && z < pOther.z );
    }

    bool operator > ( const Vector3& pOther ) const
    {
        return ( x > pOther.x && y > pOther.y && z > pOther.z );
    }

    /**
     *  Invert the sign of the vector.
     *  @brief  Unary minus operator.
     *  @return A new vector, same as this one but with the signs of all the elements inverted.
     */
    Vector3 operator - () const
    {
        return Vector3( -x, -y, -z );
    }

    /**
     *  Add a vector to this vector.
     *  @brief  Addition operator.
     *  @param  pOther   a vector, to be added to this vector.
     *  @return A new vector, the result of the addition of the two vector.
     */
    Vector3 operator + ( const Vector3& pOther ) const
    {
        return Vector3( x + pOther.x, y + pOther.y, z + pOther.z );
    }

    /**
     *  Substract a vector to this vector.
     *  @brief  Substraction operator.
     *  @param  pOther   a vector, to be substracted to this vector.
     *  @return A new vector, the result of the substraction of the two vector.
     */
    Vector3 operator - ( const Vector3& pOther ) const
    {
        return Vector3( x - pOther.x, y - pOther.y, z - pOther.z );
    }

    /**
     *  Divide this vector by a scale factor.
     *  @brief  Division operator for a scalar.
     *  @param  pScale  value to divide this vector by.
     *  @return A new vector, the result of the division.
     */
    Vector3 operator / ( const T& pScale ) const
    {
        return Vector3( x / pScale, y / pScale, z / pScale );
    }

    /**
     *  Multiply this vector by a scalar.
     *  @brief  Multiplication operator for a scalar.
     *  @param  pScale  The vector will be multiplied by this value.
     *  @return A new vector which is the result of the operation.
     */
    Vector3 operator * ( const T& pScale ) const
    {
        return Vector3( x * pScale, y * pScale, z * pScale );
    }

    /**
     *  Add the content of another vector to this vector.
     *  @brief  Addition assignment operator for a vector.
     *  @param  pOther  The vector to add to this vector.
     *  @return A const reference to this vector.
     */
    const Vector3& operator += ( const Vector3& pOther )
    {
        x += pOther.x;   y += pOther.y;   z += pOther.z;
        return *this;
    }

    /**
     *  Substract the content of another vector to this vector.
     *  @brief  Substraction assigment operator for a vector.
     *  @param  pOther  The vector to substract from this vector.
     *  @return A const reference to this vector.
     */
    const Vector3& operator -= ( const Vector3& pOther )
    {
        x -= pOther.x;   y -= pOther.y;   z -= pOther.z;
        return *this;
    }

    /**
     *  Divide this vector by a factor.
     *  @brief  Division assigment operator for a scalar.
     *  @param  pScale  Value to be used for the division.
     *  @return A const reference to this vector.
     */
    const Vector3& operator /= ( const T& pScale )
    {
        x /= pScale;    y /= pScale;    z /= pScale;
        return *this;
    }

    /**
     *  Multiply this vector by a factor.
     *  @brief  Multiplication assigment operator for a scalar.
     *  @param  pScale  Value to be used for the multiplication
     *  @return A const reference to this vector.
     */
    const Vector3& operator *= ( const T& pScale )
    {
        x *= pScale;    y *= pScale;    z *= pScale;
        return *this;
    }

    /**
     *  Compute the cross product of two vectors.
     *  @brief  Cross product.
     *  @param  pOther  vector to compute the cross product.
     *  @return A new vector that is the result of the cross product of the two vectors.
     */
    Vector3 operator cross ( const Vector3& pOther ) const
    {
        return Vector3( y*pOther.z - z*pOther.y, z*pOther.x - x*pOther.z, x*pOther.y - y*pOther.x );
    }

    /**
     *  Compute the dot product of two vectors.
     *  @brief  Dot product of two vectors.
     *  @param  pOther  vector to compute the dot product.
     *  @return Result of the dot product of the two vectors.
     */
    T operator dot ( const Vector3& pOther ) const
    {
        return ( x*pOther.x + y*pOther.y + z*pOther.z );
    }

    /**
     *  Access vector elements by their indexes.
     *  @brief  Subscript operator used to access vector elements (const).
     *  @param  pIndex  Index of the element to obtain.
     *  @return A reference to the element at index pIndex.
     */
    const T& operator () ( const int& pIndex ) const
    {
        return elem[pIndex];
    }

    /**
     *  Access vector elements by their indexes.
     *  @brief  Subscript operator used to access vector elements.
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
    const Vector3& Normalize()
    {
        T scalar = GetLength();
        if(scalar > 0)
            scalar = 1 / scalar;
        else
            scalar = 0;

        x *= scalar;   y *= scalar;   z *= scalar;
        return *this;
    }

    /**
     *  Create a normalized copy of this vector (Divide by its length).
     *  @brief Create a normalized copy of this vector.
     *  @return A normalized copy of the vector.
     */
    Vector3 GetNormalized() const
    {
        T scalar = GetLength();
        if(scalar > 0)
            scalar = 1.0f / scalar;
        else
            scalar = 0;

        return (*this) * scalar;
    }

	/**
     *  Returns true if the vector is normalized, false otherwise.
     */
    bool IsNormalized() const
    {
        return GetLength() == 1;
    }

    /**
     *  Set this vector to a random vector.
     *  @brief  Randomize this vector.
     *  @return A const reference to this vector, now randomized.
     */
    const Vector3& Randomize()
    {
        z = Maths::Rand( -1.0f, 1.0f );
        T a = Maths::Rand( 0.0f, Maths::PI_2 );
        T r = Maths::Sqrt( 1.0f - z*z );
        x = r * Maths::Cos(a);
        y = r * Maths::Sin(a);

        return *this;
    }

    /**
     *  Get a random vector.
     *  @return A new random vector.
     */
    static Vector3 GetRandomVector()
    {
        T z = Maths::Rand( -1.0, 1.0 );
        T a = Maths::Rand( 0.0f, Maths::PI_2 );
        T r = Maths::Sqrt(1.0f - z*z);
        return Vector3(r * Maths::Cos(a), r * Maths::Sin(a), z);
    }

    /**
     *  Build a vector using 2 angles in the x and y planes.
     *  @param  pAngleX     Rotation around the X axis.
     *  @param  pAngleY     Rotation around the Y axis.
     *  @return This vector pointing in the right direction.
     */
    const Vector3& FromAngle( T pAngleX, T pAngleY )
    {
        x = Maths::Cos(Maths::ToRadians(pAngleY));
        y = Maths::Sin(Maths::ToRadians(pAngleX));
        z = Maths::Sin(Maths::ToRadians(pAngleY));

        return Normalize();
    }

    /**
     *  Get the length of this vector.
     *  @return The length of this vector.
     */
    T GetLength() const
    {
        return Maths::Sqrt( x*x + y*y + z*z );
    }

    /**
     *  Get the squared length of this vector.
     *  @return The squared length of this vector.
     */
    T GetLengthSqr() const
    {
        return x*x + y*y + z*z;
    }

    /**
     *  Get the distance between two vectors.
     *  @param  pOther  The other vector.
     *  @return The distance between the two vectors.
     */
    T GetDistanceFrom( const Vector3& pOther ) const
    {
        return Vector3( pOther - *this ).GetLength();
    }

    /**
     *  Get the angle, in radian, between two vectors.
     *  @param  pOther  The other vector.
     *  @return The angle between the two vectors, in radians.
     */
    T GetAngleBetween( const Vector3& pOther ) const
    {
        return Maths::ACos( (*this dot pOther ) / (GetLength()*pOther.GetLength()) );
    }

    /**
     *  Check if two vector are equals using an epsilon.
     *  @param  pOther  The other vector.
     *  @param  pEps    The epsilon value.
     *  @return \bTrue if the vectors values fit in the epsilon range.
     */
    bool EqualEps( const Vector3& pOther, T pEps = Maths::EPSILON ) const
    {
        return  x > (pOther.x - pEps) && x < (pOther.x + pEps) &&
                y > (pOther.y - pEps) && y < (pOther.y + pEps) &&
                z > (pOther.z - pEps) && z < (pOther.z + pEps);
    }

    union
    {
        T elem[3];          //!< This vector as a 3 elements array.
        struct
        {
            T x;            //!< X component of the vector.
            T y;            //!< Y component of the vector.
            T z;            //!< Z component of the vector.
        };
    };

    static const Vector3 X_AXIS;    //!< The X axis.
    static const Vector3 Y_AXIS;    //!< The Y axis.
    static const Vector3 Z_AXIS;    //!< The Z axis.
    static const Vector3 ZERO;      //!< A null vector.
};


typedef Vector3<s32>	Vector3i;
typedef Vector3<float>  Vector3f;
typedef Vector3<double> Vector3d;


// Constants
template<class T> const Vector3<T> Vector3<T>::X_AXIS( T(1), T(0), T(0) );
template<class T> const Vector3<T> Vector3<T>::Y_AXIS( T(0), T(1), T(0) );
template<class T> const Vector3<T> Vector3<T>::Z_AXIS( T(0), T(0), T(1) );
template<class T> const Vector3<T> Vector3<T>::ZERO  ( T(0), T(0), T(0) );


#endif  //  _VECTOR3_H_

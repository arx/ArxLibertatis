
#ifndef ARX_PLATFORM_MATH_VECTOR3_H
#define ARX_PLATFORM_MATH_VECTOR3_H

#include <limits>
#include <cmath>

#include "platform/Platform.h"

#define cross ^
#define dot |

/*!
 * Representation of a vector in 3d space.
 * @brief 3x1 Vector class.
 */
template <class T>
class Vector3 {
	
public:
	
	/*!
	 * Constructor.
	 */
	Vector3() {}
	
	/*!
	 * Constructor accepting initial values.
	 * @param fX A T representing the x-axis.
	 * @param fY A T representing the y-axis.
	 * @param fZ A T representing the z-axis.
	 */
	Vector3(T pX, T pY, T pZ) : x(pX), y(pY), z(pZ) { }
	
	/*!
	 * Copy constructor.
	 * @param other A vector to be copied.
	 */
	Vector3(const Vector3 & other) : x(other.x), y(other.y), z(other.z) { }
	
	/*!
	 * Set this vector to the content of another vector.
	 * @brief Assignement operator.
	 * @param other A vector to be copied.
	 * @return Reference to this vector object.
	 */
	const Vector3 & operator=(const Vector3 & other) {
		x = other.x, y = other.y, z = other.z;
		return *this;
	}
	
	/*!
	 * Test if this vector is equal to another vector.
	 * @brief Equal operator.
	 * @param other A vector to be compared to.
	 * @return A boolean, \b true if the two vector are equal(all members are equals), or \b false otherwise.
	 */
	bool operator==(const Vector3 & other) const {
		return (x == other.x && y == other.y && z == other.z);
	}
	
	/*!
	 * Test if this vector is not equal to another vector.
	 * @brief Not equal operator.
	 * @param other A vector to be compared to.
	 * @return A boolean, \b true if the two vector are not equal(all members are not equal), or \b false otherwise.
	 */
	bool operator!=(const Vector3 & other) const {
		return !((*this) == other);
	}
	
	bool operator<(const Vector3 & other) const {
		return (x < other.x && y < other.y && z < other.z);
	}
	
	bool operator>(const Vector3 & other) const {
		return (x > other.x && y > other.y && z > other.z);
	}
	
	/*!
	 * Invert the sign of the vector.
	 * @brief Unary minus operator.
	 * @return A new vector, same as this one but with the signs of all the elements inverted.
	 */
	Vector3 operator-() const {
		return Vector3(-x, -y, -z);
	}
	
	/*!
	 * Add a vector to this vector.
	 * @brief Addition operator.
	 * @param other a vector, to be added to this vector.
	 * @return A new vector, the result of the addition of the two vector.
	 */
	Vector3 operator+(const Vector3 & other) const {
		return Vector3(x + other.x, y + other.y, z + other.z);
	}
	

	Vector3 operator*(const Vector3 & other) const {
		return Vector3(x * other.x, y * other.y, z * other.z);
	}
	
	/*!
	 * Substract a vector to this vector.
	 * @brief Substraction operator.
	 * @param other a vector, to be substracted to this vector.
	 * @return A new vector, the result of the substraction of the two vector.
	 */
	Vector3 operator-(const Vector3 & other) const {
		return Vector3(x - other.x, y - other.y, z - other.z);
	}
	
	/*!
	 * Divide this vector by a scale factor.
	 * @brief Division operator for a scalar.
	 * @param scale value to divide this vector by.
	 * @return A new vector, the result of the division.
	 */
	Vector3 operator/(T scale) const {
		return Vector3(x / scale, y / scale, z / scale);
	}
	
	/*!
	 * Multiply this vector by a scalar.
	 * @brief Multiplication operator for a scalar.
	 * @param scale The vector will be multiplied by this value.
	 * @return A new vector which is the result of the operation.
	 */
	Vector3 operator*(T scale) const {
		return Vector3(x * scale, y * scale, z * scale);
	}
	
	/*!
	 * Add the content of another vector to this vector.
	 * @brief Addition assignment operator for a vector.
	 * @param other The vector to add to this vector.
	 * @return A const reference to this vector.
	 */
	const Vector3 & operator+=(const Vector3 & other) {
		x += other.x, y += other.y, z += other.z;
		return *this;
	}
	
	const Vector3 & operator*=(const Vector3 & other) {
		x *= other.x, y *= other.y, z *= other.z;
		return *this;
	}
	
	/*!
	 * Substract the content of another vector to this vector.
	 * @brief Substraction assigment operator for a vector.
	 * @param other The vector to substract from this vector.
	 * @return A const reference to this vector.
	 */
	const Vector3 & operator-=(const Vector3 & other) {
		x -= other.x, y -= other.y, z -= other.z;
		return *this;
	}
	
	/*!
	 * Divide this vector by a factor.
	 * @brief Division assigment operator for a scalar.
	 * @param scale Value to be used for the division.
	 * @return A const reference to this vector.
	 */
	const Vector3 & operator/=(T scale) {
		x /= scale, y /= scale, z /= scale;
		return *this;
	}
	
	/*!
	 * Multiply this vector by a factor.
	 * @brief Multiplication assigment operator for a scalar.
	 * @param scale Value to be used for the multiplication
	 * @return A const reference to this vector.
	 */
	const Vector3 & operator *=(T scale) {
		x *= scale, y *= scale, z *= scale;
		return *this;
	}
	
	/*!
	 * Compute the cross product of two vectors.
	 * @brief Cross product.
	 * @param other vector to compute the cross product.
	 * @return A new vector that is the result of the cross product of the two vectors.
	 */
	Vector3 operator cross(const Vector3 & other) const {
		return Vector3(y*other.z - z*other.y, z*other.x - x*other.z, x*other.y - y*other.x);
	}
	
	/*!
	 * Compute the dot product of two vectors.
	 * @brief Dot product of two vectors.
	 * @param other vector to compute the dot product.
	 * @return Result of the dot product of the two vectors.
	 */
	T operator dot(const Vector3 & other) const {
		return (x*other.x + y*other.y + z*other.z);
	}
	
	/*!
	 * Access vector elements by their indexes.
	 * @brief Subscript operator used to access vector elements(const).
	 * @param pIndex Index of the element to obtain.
	 * @return A reference to the element at index pIndex.
	 */
	T operator()(const int& pIndex) const {
		return elem[pIndex];
	}
	
	/*!
	 * Access vector elements by their indexes.
	 * @brief Subscript operator used to access vector elements.
	 * @param pIndex Index of the element to obtain.
	 * @return A reference to the element at index pIndex.
	 */
	T & operator()(const int& pIndex) {
		return elem[pIndex];
	}
	
	/*!
	 * Access to the internal array of the vector.
	 * @brief Indirection operator(const).
	 * @return Internal array used to store the vector values.
	 */
	operator const T*() const {
		return elem;
	}
	
	/*!
	 * Access to the internal array of the vector.
	 * @brief Indirection operator.
	 * @return Internal array used to store the vector values.
	 */
	operator T*() {
		return elem;
	}
	
	/*!
	 * Normalize the vector(divide by its length).
	 * @brief Normalize the vector.
	 * @return The old magnitude.
	 */
	T normalize() {
		
		T magnitude = length();
		T scalar;
		if(magnitude > 0) {
			scalar = 1 / magnitude;
		} else {
			scalar = 0;
		}
		
		x *= scalar; y *= scalar; z *= scalar;
		
		return magnitude;
	}
	
	/*!
	 * Create a normalized copy of this vector(Divide by its length).
	 * @brief Create a normalized copy of this vector.
	 * @return A normalized copy of the vector.
	 */
	Vector3 getNormalized() const {
		
		T scalar = length();
		if(scalar > 0) {
			scalar = 1.0f / scalar;
		} else {
			scalar = 0;
		}
		
		return (*this) * scalar;
	}
	
	/*!
	 * Returns true if the vector is normalized, false otherwise.
	 */
	bool normalized() const {
		return lengthSqr() == 1;
	}
	
	/*!
	 * Build a vector using 2 angles in the x and y planes.
	 * @param pAngleX Rotation around the X axis, in randians.
	 * @param pAngleY Rotation around the Y axis, in radians.
	 * @return This vector pointing in the right direction.
	 */
	static Vector3 fromAngle(T pAngleX, T pAngleY) {
		
		Vector3 res(std::cos(pAngleY), std::sin(pAngleX), std::sin(pAngleY));
		res.normalize();
		
		return res;
	}
	
	/*!
	 * Get the length of this vector.
	 * @return The length of this vector.
	 */
	T length() const {
		return std::sqrt(lengthSqr());
	}
	
	/*!
	 * Get the squared length of this vector.
	 * @return The squared length of this vector.
	 */
	T lengthSqr() const {
		return x*x + y*y + z*z;
	}
	
	/*!
	 * Get the distance between two vectors.
	 * @param other The other vector.
	 * @return The distance between the two vectors.
	 */
	T distanceFrom(const Vector3 & other) const {
		return Vector3(other - *this).length();
	}
	
	T distanceFromSqr(const Vector3 & other) const {
		return Vector3(other - *this).lengthSqr();
	}
	
	/*!
	 * Get the angle, in radian, between two vectors.
	 * @param other The other vector.
	 * @return The angle between the two vectors, in radians.
	 */
	T angleBetween(const Vector3 & other) const {
		return std::acos((*this dot other) /(length()*other.length()));
	}
	
	/*!
	 * Check if two vector are equals using an epsilon.
	 * @param other The other vector.
	 * @param pEps The epsilon value.
	 * @return \bTrue if the vectors values fit in the epsilon range.
	 */
	bool equalEps(const Vector3 & other, T pEps = std::numeric_limits<T>::epsilon()) const {
		return x > (other.x - pEps) && x < (other.x + pEps) && y > (other.y - pEps) && y < (other.y + pEps) && z > (other.z - pEps) && z < (other.z + pEps);
	}
	
	union {
		T elem[3]; //!< This vector as a 3 elements array.
		struct {
			T x; //!< X component of the vector.
			T y; //!< Y component of the vector.
			T z; //!< Z component of the vector.
		};
	};
	
	static const Vector3 X_AXIS; //!< The X axis.
	static const Vector3 Y_AXIS; //!< The Y axis.
	static const Vector3 Z_AXIS; //!< The Z axis.
	static const Vector3 ZERO; //!< A null vector.
	
};

// Constants
template<class T> const Vector3<T> Vector3<T>::X_AXIS(T(1), T(0), T(0));
template<class T> const Vector3<T> Vector3<T>::Y_AXIS(T(0), T(1), T(0));
template<class T> const Vector3<T> Vector3<T>::Z_AXIS(T(0), T(0), T(1));
template<class T> const Vector3<T> Vector3<T>::ZERO(T(0), T(0), T(0));

template<class T>
inline T dist(const Vector3<T> & a, const Vector3<T> & b) {
	return a.distanceFrom(b);
}

template<class T>
inline T distSqr(const Vector3<T> & a, const Vector3<T> & b) {
	return a.distanceFromSqr(b);
}

template<class T>
inline bool closerThan(const Vector3<T> & a, const Vector3<T> & b, T d) {
	return (distSqr(a, b) < (d * d));
}

template<class T>
inline bool fartherThan(const Vector3<T> & a, const Vector3<T> & b, T d) {
	return (distSqr(a, b) > (d * d));
}

typedef Vector3<s32> Vec3i;
typedef Vector3<float> Vec3f;
typedef Vector3<double> Vec3d;

#endif // ARX_PLATFORM_MATH_VECTOR3_H

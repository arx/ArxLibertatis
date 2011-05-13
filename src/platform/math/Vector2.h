
#ifndef ARX_PLATFORM_MATH_VECTOR2_H
#define ARX_PLATFORM_MATH_VECTOR2_H

#include <limits>
#include <cmath>

#include "platform/Platform.h"


/*!
 * Representation of a vector in 2d space.
 * @brief 2x1 Vector class.
 */
template <class T>
class Vector2 {
	
public:
	/*!
	 * Constructor.
	 */
	Vector2() {}
	
	/*!
	 * Constructor accepting initial values.
	 * @param fX A T representing the x-axis.
	 * @param fY A T representing the y-axis.
	 */
	Vector2(T pX, T pY) : x(pX), y(pY) { }
	
	/*!
	 * Copy constructor.
	 * @param other A vector to be copied.
	 */
	Vector2(const Vector2 & other) : x(other.x), y(other.y) { }
	
	/*!
	 * Set this vector to the content of another vector.
	 * @brief Assignement operator.
	 * @param other A vector to be copied.
	 * @return Reference to this vector object.
	 */
	const Vector2 & operator=(const Vector2 & other) {
		x = other.x, y = other.y;
		return *this;
	}
	
	/*!
	 * Test if this vector is equal to another vector.
	 * @brief Equal operator.
	 * @param other A vector to be compared to.
	 * @return A boolean, \b true if the two vector are equal(all members are equals), or \b false otherwise.
	 */
	bool operator==(const Vector2 & other) const {
		return (x == other.x && y == other.y);
	}
	
	/*!
	 * Test if this vector is not equal to another vector.
	 * @brief Not equal operator.
	 * @param other A vector to be compared to.
	 * @return A boolean, \b true if the two vector are not equal(all members are not equal), or \b false otherwise.
	 */
	bool operator!=(const Vector2 & other) const {
		return !((*this) == other);
	}
	
	/*!
	 * Invert the sign of the vector.
	 * @brief Unary minus operator.
	 * @return A new vector, same as this one but with the signs of all the elements inverted.
	 */
	Vector2 operator-() const {
		return Vector2(-x, -y);
	}
	
	/*!
	 * Add a vector to this vector.
	 * @brief Addition operator.
	 * @param other a vector, to be added to this vector.
	 * @return A new vector, the result of the addition of the two vector.
	 */
	Vector2 operator+(const Vector2 & other) const {
		return Vector2(x + other.x, y + other.y);
	}
	
	/*!
	 * Substract a vector to this vector.
	 * @brief Substraction operator.
	 * @param other a vector, to be substracted to this vector.
	 * @return A new vector, the result of the substraction of the two vector.
	 */
	Vector2 operator-(const Vector2 & other) const {
		return Vector2(x - other.x, y - other.y);
	}
	
	/*!
	 * Divide this vector by a scale factor.
	 * @brief Division operator for a scalar.
	 * @param scale value to divide this vector by.
	 * @return A new vector, the result of the division.
	 */
	Vector2 operator/(T scale) const {
		return Vector2(x / scale, y / scale);
	}
	
	/*!
	 * Multiply this vector by a scalar.
	 * @brief Multiplication operator for a scalar.
	 * @param scale The vector will be multiplied by this value.
	 * @return A new vector which is the result of the operation.
	 */
	Vector2 operator*(T scale) const {
		return Vector2(x * scale, y * scale);
	}
	
	/*!
	 * Add the content of another vector to this vector.
	 * @brief Addition assignment operator for a vector.
	 * @param other The vector to add to this vector.
	 * @return A const reference to this vector.
	 */
	const Vector2 & operator+=(const Vector2 & other) {
		x += other.x, y += other.y;
		return *this;
	}
	
	/*!
	 * Substract the content of another vector to this vector.
	 * @brief Substraction assigment operator for a vector.
	 * @param other The vector to substract from this vector.
	 * @return A const reference to this vector.
	 */
	const Vector2 & operator-=(const Vector2 & other) {
		x -= other.x, y -= other.y;
		return *this;
	}
	
	/*!
	 * Divide this vector by a factor.
	 * @brief Division assigment operator for a scalar.
	 * @param scale Value to be used for the division.
	 * @return A const reference to this vector.
	 */
	const Vector2 & operator/=(T scale) {
		x /= scale, y /= scale;
		return *this;
	}
	
	/*!
	 * Multiply this vector by a factor.
	 * @brief Multiplication assigment operator for a scalar.
	 * @param scale Value to be used for the multiplication
	 * @return A const reference to this vector.
	 */
	const Vector2 & operator*=(T scale) {
		x *= scale, y *= scale;
		return *this;
	}
	
	/*!
	 * Access vector elements by their indexes.
	 * @brief Function call operator used to access vector elements.
	 * @param pIndex Index of the element to obtain.
	 * @return A reference to the element at index pIndex.
	 */
	T & operator()(const int & pIndex) {
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
	 * @return Reference to the vector.
	 */
	const Vector2 & normalize() {
		
		T length = length();
		arx_assert(length != 0);
		
		x /= length, y /= length;
		return *this;
	}
	
	/*!
	 * Create a normalized copy of this vector(Divide by its length).
	 * @brief Create a normalized copy of this vector.
	 * @return A normalized copy of the vector.
	 */
	Vector2 getNormalized() const {
		arx_assert(length() != 0);
		return ((*this) / length());
	}
	
	/*!
	 * Returns true if the vector is normalized, false otherwise.
	 */
	bool normalized() const {
		return lengthSqr() == 1;
	}
	
	/*!
	 * Get the length of this vector.
	 * @return The length of this vector.
	 */
	T length() const {
		return std::sqrt(x*x + y*y);
	}
	
	/*!
	 * Get the squared length of this vector.
	 * @return The squared length of this vector.
	 */
	T lengthSqr() const {
		return x*x + y*y;
	}
	
	/*!
	 * Get the distance between two vectors.
	 * @param other The other vector.
	 * @return The distance between the two vectors.
	 */
	T distanceFrom(const Vector2 & other) const {
		return Vector2(other - *this).length();
	}
	
	T distanceFromSqr(const Vector2 & other) const {
		return Vector2(other - *this).lengthSqr();
	}
	
	/*!
	 * Check if two vector are equals using an epsilon.
	 * @param other The other vector.
	 * @param pEps The epsilon value.
	 * @return \bTrue if the vectors values fit in the epsilon range.
	 */
	bool equalEps(const Vector2 & other, T pEps = std::numeric_limits<T>::epsilon()) const {
		return x > (other.x - pEps) && x < (other.x + pEps) && y > (other.y - pEps) && y < (other.y + pEps);
	}
	
	union {
		T elem[2]; //!< This vector as a 2 elements array.
		struct {
			T x; //!< X component of the vector.
			T y; //!< Y component of the vector.
		};
	};
	
	static const Vector2 X_AXIS; //!< The X axis.
	static const Vector2 Y_AXIS; //!< The Y axis.
	static const Vector2 ZERO; //!< A null vector.
	
};

template<class T>
T dist(const Vector2<T> & a, const Vector2<T> & b) {
	return a.distanceFrom(b);
}

template<class T>
T distSqr(const Vector2<T> & a, const Vector2<T> & b) {
	return a.distanceFromSqr(b);
}

// Constants
template<class T> const Vector2<T> Vector2<T>::X_AXIS(T(1), T(0));
template<class T> const Vector2<T> Vector2<T>::Y_AXIS(T(0), T(1));
template<class T> const Vector2<T> Vector2<T>::ZERO(T(0), T(0));

typedef Vector2<int> Vec2i;
typedef Vector2<short> Vec2s;
typedef Vector2<float> Vec2f;
typedef Vector2<double> Vec2d;

#endif // ARX_PLATFORM_MATH_VECTOR2_H

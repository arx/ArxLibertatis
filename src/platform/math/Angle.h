
#ifndef ARX_PLATFORM_MATH_ANGLE_H
#define ARX_PLATFORM_MATH_ANGLE_H

#include <limits>

#include "platform/Platform.h"

/*!
 * A 3-dimensional euler-angle.
 * @brief 3x1 Vector class.
 */
template <class T>
class Angle {
	
public:
	
	/*!
	 * Constructor.
	 */
	Angle() {}
	
	/*!
	 * Constructor accepting initial values.
	 */
	Angle(T _a, T _b, T _g) : a(_a), b(_b), g(_g) { }
	
	/*!
	 * Copy constructor.
	 * @param other An angle to be copied.
	 */
	Angle(const Angle & other) : a(other.a), b(other.b), g(other.g) { }
	
	/*!
	 * Set this angle to the content of another angle.
	 * @brief Assignement operator.
	 * @param other An euler angle to be copied.
	 * @return Reference to this object.
	 */
	Angle & operator=(const Angle & other) {
		a = other.a, b = other.b, g = other.g;
		return *this;
	}
	
	/*!
	 * Test if this angle is equal to another angle.
	 * @brief Equal operator.
	 * @param other An euler angle to be compared to.
	 * @return A boolean, \b true if the two angles are equal(all members are equals), or \b false otherwise.
	 */
	bool operator==(const Angle & other) const {
		return (a == other.a && b == other.b && g == other.g);
	}
	
	/*!
	 * Test if this angle is not equal to another angle.
	 * @brief Not equal operator.
	 * @param other An angle to be compared to.
	 * @return A boolean, \b true if the two angles are not equal(some members are not equal), or \b false otherwise.
	 */
	bool operator!=(const Angle & other) const {
		return !((*this) == other);
	}
	
	/*!
	 * Invert the sign of the angle.
	 * @brief Unary minus operator.
	 * @return A new angle, same as this one but with the signs of all the elements inverted.
	 */
	Angle operator-() const {
		return Angle(-a, -b, -g);
	}
	
	/*!
	 * Add an angle to this angle.
	 * @brief Addition operator.
	 * @param other an angle, to be added to this angle.
	 * @return A new angle, the result of the addition of the two angles.
	 */
	Angle operator+(const Angle & other) const {
		return Angle(a + other.a, b + other.b, g + other.g);
	}
	
	/*!
	 * Substract an angle to this angle.
	 * @brief Substraction operator.
	 * @param other an angle, to be substracted to this angle.
	 * @return A new angle, the result of the substraction of the two angles.
	 */
	Angle operator-(const Angle & other) const {
		return Angle(a - other.a, b - other.b, g - other.g);
	}
	
	Angle operator*(T scale) const {
		return Angle(a * scale, b * scale, g * scale);
	}
	
	const Angle & operator+=(const Angle & other) {
		a += other.a, b += other.b, g += other.g;
		return *this;
	}
	
	const Angle & operator-=(const Angle & other) {
		a -= other.a, b -= other.b, g -= other.g;
		return *this;
	}
	
	const Angle & operator/=(T scale) {
		a /= scale, b /= scale, g /= scale;
		return *this;
	}
	
	const Angle & operator *=(T scale) {
		a *= scale, b *= scale, g *= scale;
		return *this;
	}
	
	bool equalEps(const Angle & other, T pEps = std::numeric_limits<T>::epsilon()) const {
		return a > (other.a - pEps) && a < (other.a + pEps) && b > (other.b - pEps) && b < (other.b + pEps) && g > (other.g - pEps) && g < (other.g + pEps);
	}
	
	union {
		T a;
		T yaw;
	};
	union {
		T b;
		T pitch;
	};
	union {
		T g;
		T roll;
	};
	
	static const Angle ZERO; //!< A zero angle.
	
};

template<class T> const Angle<T> Angle<T>::ZERO(T(0), T(0), T(0));

typedef Angle<s32> Anglei;
typedef Angle<float> Anglef;
typedef Angle<double> Angled;

#endif // ARX_PLATFORM_MATH_ANGLE_H

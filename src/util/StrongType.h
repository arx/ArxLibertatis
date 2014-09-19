#ifndef ARX_UTIL_STRONGTYPE_H
#define ARX_UTIL_STRONGTYPE_H

#include <boost/config.hpp>
#include <boost/operators.hpp>

#define ARX_STRONG_TYPEDEF(T, D)                              \
struct D                                                        \
    : boost::totally_ordered1< D                                \
    , boost::totally_ordered2< D, T                             \
    > >                                                         \
{                                                               \
    T t;                                                        \
    explicit D(const T t_) : t(t_) {};                          \
    D(): t() {};                                                \
    D(const D & t_) : t(t_.t){}                                 \
    D & operator=(const D & rhs) { t = rhs.t; return *this;}    \
    operator const T & () const {return t; }                    \
    operator T & () { return t; }                               \
    bool operator==(const D & rhs) const { return t == rhs.t; } \
    bool operator<(const D & rhs) const { return t < rhs.t; }   \
};

#endif // ARX_UTIL_STRONGTYPE_H

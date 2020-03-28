#ifndef _HELPER_FUNS_H_
#define _HELPER_FUNS_H_

#include <functional>
#include <cstring>
#include <vector>

using namespace std;

#define HASHNS __gnu_cxx

/**
 * \struct eqstr
 * \brief function object which defines operator= for const char*
 */
struct eqstr
{
    /**
     * \fn bool operator() (const char* s1, const char* s2) const
     * \brief returns true if s1 and s2 are the same sequence of characters
     */
    bool operator()(const char* s1, const char* s2) const
    {
        return strcmp(s1, s2) == 0;
    }
}; //end struct eqstr

/**
 * \struct eqint
 * \brief function object which defines operator= for integer
 */
struct eqint
{
    /**
     * \fn bool operator() (int i1, int s2) const
     * \brief returns true if i1 and i2 are the same integer
     */
    bool operator()(int i1, int i2) const
    {
        return i1 == i2;
    }
}; //end struct eqint


template <class T>
inline void CopyEnd(const T& s1, T& s2)
{
    copy( s1.begin(), s1.end(), insert_iterator<T>(s2, s2.end()));
}

template <class T>
inline void Copy(const T& s1, T& s2)
{
    s2.clear();
    copy( s1.begin(), s1.end(), insert_iterator<T>(s2, s2.begin()));
}

template <class T>
inline void Intersect(const T& s1, const T &s2, T& r)
{
    r.clear();
    set_intersection( s1.begin(), s1.end(), s2.begin(), s2.end(), insert_iterator<T> (r, r.begin()));
}

template <class T>
inline void Union(const T& s1, const T &s2, T& r)
{
    r.clear();
    set_union( s1.begin(), s1.end(), s2.begin(), s2.end(), insert_iterator<T> (r, r.begin()));
}

double frandom( double value )
{
    return (double)rand()/RAND_MAX * value;
}

int irandom( int value )
{
    return rand() % value;
}

#endif


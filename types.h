#ifndef __types_h__
#define __types_h__

#include "traits.h"

template<typename T>
class RawArray
{
    public:
        RawArray(const T* data, size_t size) :
            myData(data), mySize(size)
        { }

        const T* data() const
        { return myData; }
        size_t size() const
        { return mySize; }

    private:
        const T* myData;
        size_t mySize;
};

template<typename T>
struct ArrayTrait<RawArray<T> >
{
    typedef RawArray<T> ArrayType;
    typedef T ElementType;

    static const bool canGetData = true;
    static const T* data(const ArrayType &array)
    { return array.data(); }

    static size_t size(const ArrayType &array)
    { return array.size(); }
};

#endif

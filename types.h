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

template<typename ArrayType>
class ArrayIterator :
    public std::iterator<std::forward_iterator_tag, ArrayType>
{
    public:
        typedef ArrayTrait<ArrayType> Trait;
        typedef typename Trait::ElementType ElementType;

        ArrayIterator(ArrayType& array, size_t index = 0) :
            myArray(array), myIndex(index)
        { }

        ArrayIterator(const ArrayIterator<ArrayType> &o) :
            myArray(o.myArray), myIndex(o.myIndex)
        { }

        ArrayIterator<ArrayType> &operator =(const ArrayIterator<ArrayType> &o)
        {
            myArray = o.myArray;
            myIndex = o.myIndex;
            return *this;
        }

        size_t operator -(const ArrayIterator<ArrayType> &o) const
        { return myIndex - o.myIndex; }

        ArrayIterator<ArrayType> &operator ++()
        { ++myIndex; return *this; }

        bool operator ==(const ArrayIterator<ArrayType> &o) const
        { return myIndex == o.myIndex; }

        bool operator !=(const ArrayIterator<ArrayType> &o) const
        { return !(*this == o); }

        const ElementType &operator *() const
        { return Trait::getElement(myArray, myIndex); }

        ElementType &operator *()
        { return Trait::getElement(myArray, myIndex); }

        const ElementType *operator ->() const
        { return &Trait::getElement(myArray, myIndex); }

        ElementType *operator ->()
        { return &Trait::getElement(myArray, myIndex); }

    private:
        ArrayType &myArray;
        size_t myIndex;
};

template<typename ArrayType>
ArrayIterator<ArrayType>
arrayBegin(ArrayType &array)
{ return ArrayIterator<ArrayType>(array); }

template<typename ArrayType>
ArrayIterator<ArrayType>
arrayEnd(ArrayType &array)
{ return ArrayIterator<ArrayType>(array, ArrayTrait<ArrayType>::size(array)); }

#endif

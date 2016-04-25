#ifndef __types_h__
#define __types_h__

#include <cassert>

#include "traits.h"

template<typename T>
class RawArray
{
    public:
        RawArray(T* data, size_t size) :
            myData(data), mySize(size)
        { }

        T* data() const
        { return myData; }
        size_t size() const
        { return mySize; }

    private:
        T* myData;
        size_t mySize;
};

template<typename T>
struct ArrayTrait<RawArray<T> >
{
    static const bool isArray = true;

    typedef RawArray<T> ArrayType;
    typedef T ElementType;

    static const bool canGetData = true;
    static const T* data(const ArrayType &array)
    { return array.data(); }
    static T* data(ArrayType &array)
    { return array.data(); }

    static size_t size(const ArrayType &array)
    { return array.size(); }
    static void resize(ArrayType &array, size_t size)
    {
        // Don't assert. This worksaround the issue where the plugin assumes a
        // Maya-specific detail attribute has tuple size == 1, but the detail
        // attribute actually has tuple size > 1.
        //assert(array.size() == size );
    }

    static const ElementType &getElement(const ArrayType &array, size_t i)
    { return array.data()[i]; }

    static ElementType &getElement(ArrayType &array, size_t i)
    { return array.data()[i]; }
};

template<typename T>
RawArray<T>
rawArray(T* data, size_t size)
{
    return RawArray<T>(data, size);
}

template<typename ArrayType>
class ArrayIterator :
    public std::iterator<std::forward_iterator_tag, ArrayType>
{
    public:
        typedef ARRAYTRAIT(ArrayType) Trait;
        typedef ELEMENTTYPE(ArrayType) ElementType;

        ArrayIterator(ArrayType& array, size_t index = 0) :
            myArray(&array), myIndex(index)
        { }

        ArrayIterator(const ArrayIterator<ArrayType> &o) :
            myArray(o.myArray), myIndex(o.myIndex)
        { }

        size_t operator -(const ArrayIterator<ArrayType> &o) const
        { return myIndex - o.myIndex; }

        ArrayIterator<ArrayType> &operator ++()
        { ++myIndex; return *this; }

        bool operator ==(const ArrayIterator<ArrayType> &o) const
        { return myIndex == o.myIndex; }

        bool operator !=(const ArrayIterator<ArrayType> &o) const
        { return !(*this == o); }

        const ElementType &operator *() const
        { return Trait::getElement(*myArray, myIndex); }

        ElementType &operator *()
        { return Trait::getElement(*myArray, myIndex); }

        const ElementType *operator ->() const
        { return &Trait::getElement(*myArray, myIndex); }

        ElementType *operator ->()
        { return &Trait::getElement(*myArray, myIndex); }

    private:
        ArrayType *myArray;
        size_t myIndex;
};

template<typename ArrayType>
ArrayIterator<ArrayType>
arrayBegin(ArrayType &array)
{ return ArrayIterator<ArrayType>(array); }

template<typename ArrayType>
ArrayIterator<ArrayType>
arrayEnd(ArrayType &array)
{ return ArrayIterator<ArrayType>(array, ARRAYTRAIT(ArrayType)::size(array)); }

template<
        typename ArrayType,
        size_t StartComponent,
        size_t NumComponents
        >
class ComponentWrapper
{
    public:
        typedef ComponentWrapper<
            ArrayType, StartComponent, NumComponents
            > Type;
        typedef ARRAYTRAIT(ArrayType) Trait;
        typedef ELEMENTTRAIT(ArrayType) ElementTrait;

    public:
        ComponentWrapper(ArrayType &array, size_t index, size_t component) :
            myArray(&array),
            myIndex(index),
            myComponent(component)
        {
            assert(myComponent == StartComponent);
        }

        Type &operator =(const Type &o)
        {
            // Assignment should always mean copying components
            assert(myArray != o.myArray);
            return this->operator=<Type>(o);
        }

        template<typename O>
        Type &operator =(const O &o)
        {
            for(size_t i = 0; i < NumComponents; i++)
            {
                (*this)[i] = o[i];
            }

            return *this;
        }

        const typename ElementTrait::ComponentType &operator [](size_t i) const
        {
            size_t index = myIndex + (StartComponent + i)
                / ElementTrait::numComponents;
            size_t component = (StartComponent + i)
                % ElementTrait::numComponents;
            return ElementTrait::getComponent(
                    Trait::getElement(*myArray, index),
                    component
                    );
        }

        typename ElementTrait::ComponentType &operator [](size_t i)
        {
            size_t index = myIndex + (StartComponent + i)
                / ElementTrait::numComponents;
            size_t component = (StartComponent + i)
                % ElementTrait::numComponents;
            return ElementTrait::getComponent(
                    Trait::getElement(*myArray, index),
                    component
                    );
        }

    private:
        ArrayType *myArray;
        size_t myIndex;
        size_t myComponent;
};

template<
        typename ArrayType,
        size_t StartComponent,
        size_t NumComponents,
        size_t Stride
        >
class ComponentIterator :
    public std::iterator<std::forward_iterator_tag, ArrayType>
{
    public:
        typedef ComponentIterator<
            ArrayType, StartComponent, NumComponents, Stride
            > Type;
        typedef ARRAYTRAIT(ArrayType) Trait;
        typedef ELEMENTTRAIT(ArrayType) ElementTrait;
        typedef ComponentWrapper<
            ArrayType,
            StartComponent % ElementTrait::numComponents,
            NumComponents
            > ComponentWrapperType;

    public:
        ComponentIterator(ArrayType &array, size_t index) :
            myArray(&array)
        {
            myIndex = (StartComponent + (Stride * index))
                / ElementTrait::numComponents;
            myComponent = (StartComponent + (Stride * index))
                % ElementTrait::numComponents;
            assert(Stride % ElementTrait::numComponents == 0);
        }

        Type &operator ++()
        {
            myIndex += (myComponent + Stride)
                / ElementTrait::numComponents;
            myComponent = (myComponent + Stride)
                % ElementTrait::numComponents;
            return *this;
        }

        bool operator ==(const Type &o) const
        { return myIndex == o.myIndex && myComponent == o.myComponent; }

        bool operator !=(const Type &o) const
        { return !(*this == o); }

        const ComponentWrapperType operator *() const
        { return ComponentWrapperType( *myArray, myIndex, myComponent); }

        ComponentWrapperType operator *()
        { return ComponentWrapperType( *myArray, myIndex, myComponent); }

        const ComponentWrapperType *operator ->() const
        { return &ComponentWrapperType( *myArray, myIndex, myComponent); }

        ComponentWrapperType *operator ->()
        { return &ComponentWrapperType( *myArray, myIndex, myComponent); }

    private:
        ArrayType *myArray;
        size_t myIndex;
        size_t myComponent;
};

template<
        size_t startComponent,
        size_t numComponents,
        size_t stride,
        typename ArrayType
        >
ComponentIterator<
        ArrayType,
        startComponent,
        numComponents,
        stride
        >
componentBegin(ArrayType &array)
{
    return ComponentIterator<
        ArrayType,
        startComponent,
        numComponents,
        stride
        >(array, 0);
}

template<
        size_t startComponent,
        size_t numComponents,
        size_t stride,
        typename ArrayType
        >
ComponentIterator<
        ArrayType,
        startComponent,
        numComponents,
        stride
        >
componentEnd(ArrayType &array)
{
    return ComponentIterator<
        ArrayType,
        startComponent,
        numComponents,
        stride
        >(
            array,
            ARRAYTRAIT(ArrayType)::size(array)
            * ELEMENTTRAIT(ArrayType)::numComponents / stride
        );
}

#endif

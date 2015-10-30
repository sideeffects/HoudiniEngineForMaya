#ifndef __traits_h__
#define __traits_h__

#include <maya/MDoubleArray.h>
#include <maya/MFloatArray.h>
#include <maya/MIntArray.h>
#include <maya/MStringArray.h>
#include <maya/MString.h>

#include <vector>

template<typename T>
struct RemoveConst
{
    typedef T type;
};

template<typename T>
struct RemoveConst<const T>
{
    typedef T type;
};

template<typename T>
struct ArrayTrait
{
    //typedef void ArrayType;
    //typedef void ElementType;

    //static const bool canGetData = false;
    //static const T* data(const ArrayType &array);
    //static T* data(ArrayType &array);

    //static size_t size(const ArrayType &array);
    //static void resize(ArrayType &array, size_t size);

    //static const ElementType &getElement(const ArrayType &array, size_t i);

    //static ElementType &getElement(ArrayType &array, size_t i);
};

template<typename T>
struct ArrayTrait<std::vector<T> >
{
    typedef std::vector<T> ArrayType;
    typedef T ElementType;

    static const bool canGetData = true;
    static const ElementType* data(const ArrayType &array)
    { return &array[0]; }
    static ElementType* data(ArrayType &array)
    { return &array[0]; }

    static size_t size(const ArrayType &array)
    { return array.size(); }
    static void resize(ArrayType &array, size_t size)
    { array.resize(size); }

    static const ElementType &getElement(const ArrayType &array, size_t i)
    { return array[i]; }

    static ElementType &getElement(ArrayType &array, size_t i)
    { return array[i]; }
};

template<>
struct ArrayTrait<MIntArray>
{
    typedef MIntArray ArrayType;
    typedef int ElementType;

    static const bool canGetData = false;

    static size_t size(const ArrayType &array)
    { return array.length(); }
    static void resize(ArrayType &array, size_t size)
    { array.setLength(size); }

    static const ElementType &getElement(const ArrayType &array, size_t i)
    {
        // const MIntArray doesn't return a int reference
        return const_cast<ArrayType&>(array)[i];
    }

    static ElementType &getElement(ArrayType &array, size_t i)
    { return array[i]; }
};

template<>
struct ArrayTrait<MFloatArray>
{
    typedef MFloatArray ArrayType;
    typedef float ElementType;

    static const bool canGetData = false;

    static size_t size(const ArrayType &array)
    { return array.length(); }
    static void resize(ArrayType &array, size_t size)
    { array.setLength(size); }

    static const ElementType &getElement(const ArrayType &array, size_t i)
    {
        // const MFloatArray doesn't return a float reference
        return const_cast<ArrayType&>(array)[i];
    }

    static ElementType &getElement(ArrayType &array, size_t i)
    { return array[i]; }
};

template<>
struct ArrayTrait<MDoubleArray>
{
    typedef MDoubleArray ArrayType;
    typedef double ElementType;

    static const bool canGetData = false;

    static size_t size(const ArrayType &array)
    { return array.length(); }
    static void resize(ArrayType &array, size_t size)
    { array.setLength(size); }

    static const ElementType &getElement(const ArrayType &array, size_t i)
    {
        // const MDoubleArray doesn't return a double reference
        return const_cast<ArrayType&>(array)[i];
    }

    static ElementType &getElement(ArrayType &array, size_t i)
    { return array[i]; }
};

template<>
struct ArrayTrait<MStringArray>
{
    typedef MStringArray ArrayType;
    typedef MString ElementType;

    static const bool canGetData = false;

    static size_t size(const ArrayType &array)
    { return array.length(); }
    static void resize(ArrayType &array, size_t size)
    { array.setLength(size); }

    static const ElementType &getElement(const ArrayType &array, size_t i)
    {
        // const MStringArray doesn't return a MString reference
        return const_cast<ArrayType&>(array)[i];
    }

    static ElementType &getElement(ArrayType &array, size_t i)
    { return array[i]; }
};

#endif

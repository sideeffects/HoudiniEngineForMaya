#ifndef __traits_h__
#define __traits_h__

#include <maya/MColorArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MStringArray.h>
#include <maya/MString.h>
#include <maya/MVectorArray.h>

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

#define REMOVECONST(T) typename RemoveConst<T>::type

template<typename T>
struct TypeTrait
{
};

#define TYPETRAIT(T) TypeTrait<typename RemoveConst<T>::type>

template<>
struct TypeTrait<float>
{
    typedef float Type;
    typedef float ComponentType;

    static const int numComponents = 1;
    static const ComponentType &getComponent(const Type &o, size_t i)
    { return o; }
    static ComponentType &getComponent(Type &o, size_t i)
    { return o; }
};

template<>
struct TypeTrait<double>
{
    typedef double Type;
    typedef double ComponentType;

    static const int numComponents = 1;
    static const ComponentType &getComponent(const Type &o, size_t i)
    { return o; }
    static ComponentType &getComponent(Type &o, size_t i)
    { return o; }
};

template<>
struct TypeTrait<MVector>
{
    typedef MVector Type;
    typedef double ComponentType;

    static const int numComponents = 3;
    static const ComponentType &getComponent(const Type &o, size_t i)
    {
        // const MVector doesn't return a double reference
        return const_cast<Type&>(o)[i];
    }
    static ComponentType &getComponent(Type &o, size_t i)
    { return o[i]; }
};

template<>
struct TypeTrait<MColor>
{
    typedef MColor Type;
    typedef float ComponentType;

    static const int numComponents = 4;
    static const ComponentType &getComponent(const Type &o, size_t i)
    {
        // const MVector doesn't return a double reference
        return const_cast<Type&>(o)[i];
    }
    static ComponentType &getComponent(Type &o, size_t i)
    { return o[i]; }
};

template<>
struct TypeTrait<MFloatPoint>
{
    typedef MFloatPoint Type;
    typedef float ComponentType;

    static const int numComponents = 4;
    static const ComponentType &getComponent(const Type &o, size_t i)
    {
        // const MFloatPoint doesn't return a float reference
        return const_cast<Type&>(o)[i];
    }
    static ComponentType &getComponent(Type &o, size_t i)
    { return o[i]; }
};

template<typename T>
struct ArrayTrait
{
    static const bool isArray = false;

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

#define ARRAYTRAIT(T) ArrayTrait<typename RemoveConst<T>::type>
#define ELEMENTTYPE(T) typename ARRAYTRAIT(T)::ElementType
#define ELEMENTTRAIT(T) TypeTrait<ELEMENTTYPE(T)>

template<typename T>
struct ArrayTrait<std::vector<T> >
{
    static const bool isArray = true;

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
    static const bool isArray = true;

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
    static const bool isArray = true;

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
    static const bool isArray = true;

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
struct ArrayTrait<MVectorArray>
{
    static const bool isArray = true;

    typedef MVectorArray ArrayType;
    typedef MVector ElementType;

    static const bool canGetData = false;

    static size_t size(const ArrayType &array)
    { return array.length(); }
    static void resize(ArrayType &array, size_t size)
    { array.setLength(size); }

    static const ElementType &getElement(const ArrayType &array, size_t i)
    { return array[i]; }

    static ElementType &getElement(ArrayType &array, size_t i)
    { return array[i]; }
};

template<>
struct ArrayTrait<MColorArray>
{
    static const bool isArray = true;

    typedef MColorArray ArrayType;
    typedef MColor ElementType;

    static const bool canGetData = false;

    static size_t size(const ArrayType &array)
    { return array.length(); }
    static void resize(ArrayType &array, size_t size)
    { array.setLength(size); }

    static const ElementType &getElement(const ArrayType &array, size_t i)
    { return array[i]; }

    static ElementType &getElement(ArrayType &array, size_t i)
    { return array[i]; }
};

template<>
struct ArrayTrait<MFloatPointArray>
{
    static const bool isArray = true;

    typedef MFloatPointArray ArrayType;
    typedef MFloatPoint ElementType;

    static const bool canGetData = false;

    static size_t size(const ArrayType &array)
    { return array.length(); }
    static void resize(ArrayType &array, size_t size)
    { array.setLength(size); }

    static const ElementType &getElement(const ArrayType &array, size_t i)
    { return array[i]; }

    static ElementType &getElement(ArrayType &array, size_t i)
    { return array[i]; }
};

template<>
struct ArrayTrait<MStringArray>
{
    static const bool isArray = true;

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

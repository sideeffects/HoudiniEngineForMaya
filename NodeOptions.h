#ifndef __NodeOptions_h__
#define __NodeOptions_h__

#include <array>

#include <maya/MPxNode.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnNumericAttribute.h>

template<int Size>
class NodeOptionDefinition
{
public:
    template<int Index, typename Type>
    class Option;

public:
    void addAttributes()
    {
        for(auto &attribute : myAttributes)
            MPxNode::addAttribute(attribute);
    }

    std::array<MObject, Size> myAttributes;
};

template<int Size>
template<int Index>
class NodeOptionDefinition<Size>::Option<Index, bool>
{
public:
    Option(NodeOptionDefinition<Size>* base,
            const char* name,
            bool defaultValue)
    {
        MObject &attribute = base->myAttributes[Index];

        MFnNumericAttribute nAttr;

        attribute = nAttr.create(
                name,
                name,
                MFnNumericData::kBoolean,
                defaultValue
                );
    }
};

template<typename T>
struct DataSourceTrait
{
    typedef T DataSource;
};

template<>
struct DataSourceTrait<MFnDagNode>
{
    typedef MFnDependencyNode DataSource;
};

template<typename Definition, typename DataSource>
class NodeOptionAccessor
{
private:
    template<typename, int, typename>
    class OptionImpl;

public:
    template<int Index, typename Type>
    class Option
    {
    public:
        // gcc 4.8 seems to need this to compile
        Option(){}
        Option(NodeOptionAccessor<Definition, DataSource> *base,
                const char* name,
                Type defaultValue
              ) :
            myBase(base) {}

        Type operator()() const
        {
            return OptionImpl<DataSource, Index, Type>()(myBase);
        }

    private:
        NodeOptionAccessor<Definition, DataSource> *myBase;
    };

public:
    NodeOptionAccessor(const Definition &definition, DataSource &dataSource) :
        myDefinition(definition),
        myDataSource(dataSource)
    {}

private:
    const Definition &myDefinition;
    DataSource &myDataSource;
};

template<typename Definition, typename DataSource>
template<int Index>
class NodeOptionAccessor<Definition, DataSource>::OptionImpl<MDataBlock, Index, bool>
{
public:
    bool operator()(NodeOptionAccessor<Definition, MDataBlock> *base) const
    {
        const MObject &attribute = base->myDefinition.myAttributes[Index];

        return base->myDataSource.inputValue(attribute).asBool();
    }
};

template<typename Definition, typename DataSource>
template<int Index>
class NodeOptionAccessor<Definition, DataSource>::OptionImpl<MFnDependencyNode, Index, bool>
{
public:
    bool operator()(NodeOptionAccessor<Definition, MFnDependencyNode> *base) const
    {
        const MObject &attribute = base->myDefinition.myAttributes[Index];

        return base->myDataSource.findPlug(attribute, true).asBool();
    }
};

#define NODE_OPTIONS_BEGIN(NAME) \
namespace NAME \
{ \
const int CounterStart = __COUNTER__ + 1; \
\
template<typename Base> \
class Interface : public Base \
{ \
private: \
    template<int Index, typename Type> \
    using Option = typename Base::template Option<Index, Type>; \
\
public: \
    using Base::Base;

#define NODE_OPTION(NAME, TYPE, DEFAULT_VALUE) \
    Option<__COUNTER__ - CounterStart, TYPE> NAME = {this, #NAME, (DEFAULT_VALUE)};

#define NODE_OPTIONS_END() \
}; \
\
typedef Interface<NodeOptionDefinition<__COUNTER__ - CounterStart>> Definition; \
\
template<typename DataSource> \
using Accessor = Interface<NodeOptionAccessor<Definition, DataSource>>; \
typedef Interface<NodeOptionAccessor<Definition, MDataBlock>> AccessorDataBlock; \
typedef Interface<NodeOptionAccessor<Definition, MFnDependencyNode>> AccessorFn; \
}

#endif

#ifndef __OptionVars_h__
#define __OptionVars_h__

namespace
{
template <typename VAL, class CHILD>
class OptionVarBase
{
public:
    OptionVarBase(const char *name, const VAL &defaultValue)
        : myDefaultValue(defaultValue)
    {
        myName = "houdiniEngine";
        myName += name;

        bool exists = false;
        static_cast<const CHILD &>(*this).getImpl(exists);

        if (!exists)
        {
            MGlobal::setOptionVarValue(myName.c_str(), myDefaultValue);
        }
    }

    VAL get() const
    {
        bool exists     = false;
        const VAL value = static_cast<const CHILD &>(*this).getImpl(exists);
        return exists ? value : myDefaultValue;
    }

    bool set(VAL value) const
    {
        return static_cast<const CHILD &>(*this).setImpl(value);
    }

protected:
    std::string myName;
    const VAL myDefaultValue;

private:
    OptionVarBase &operator=(const OptionVarBase &);
};

class IntOptionVar : public OptionVarBase<int, IntOptionVar>
{
public:
    typedef OptionVarBase<int, IntOptionVar> Base;

    IntOptionVar(const char *name, int defaultValue) : Base(name, defaultValue)
    {
    }

    int getImpl(bool &exists) const
    {
        return MGlobal::optionVarIntValue(myName.c_str(), &exists);
    }

    bool setImpl(int value) const
    {
        return MGlobal::setOptionVarValue(myName.c_str(), value);
    }
};

class StringOptionVar : public OptionVarBase<MString, StringOptionVar>
{
public:
    typedef OptionVarBase<MString, StringOptionVar> Base;

    StringOptionVar(const char *name, const char *defaultValue)
        : Base(name, defaultValue)
    {
    }

    MString getImpl(bool &exists) const
    {
        return MGlobal::optionVarStringValue(myName.c_str(), &exists);
    }

    bool setImpl(MString &value) const
    {
        return MGlobal::setOptionVarValue(myName.c_str(), value);
    }
};

struct OptionVars
{
    OptionVars()
        : hfsLocation("HfsLocation", ""),
          hapilLocation("HapilLocation", ""),
          asyncMode("AsynchronousMode", 1),
          sessionType("SessionType", 2), // named pipe
          thriftServer("ThriftServer", "localhost"),
          thriftPort("ThriftPort", 9090),
          sessionPipeCustom("SessionPipeCustom", 0),
          thriftPipe("ThriftPipe", "hapi"),
          unsetLLP("UnsetLLP", 1),
          unsetPP("UnsetPP", 0),
          viewProduct("ViewProduct", "Houdini Core"),
          timeout("Timeout", 10 * 1000)
    {
    }

    StringOptionVar hfsLocation;
    StringOptionVar hapilLocation;
    IntOptionVar asyncMode;
    IntOptionVar sessionType;
    StringOptionVar thriftServer;
    IntOptionVar thriftPort;
    IntOptionVar sessionPipeCustom;
    StringOptionVar thriftPipe;
    IntOptionVar unsetLLP;
    IntOptionVar unsetPP;
    StringOptionVar viewProduct;
    IntOptionVar timeout;

private:
    OptionVars &operator=(const OptionVars &);
};
}

#endif


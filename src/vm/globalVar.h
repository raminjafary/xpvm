#ifndef global_var_h
#define global_var_h

#include <vector>
#include "XPValue.h"

struct GlobalVar
{
    std::string name;
    XPValue value;
};

struct Global
{

    GlobalVar &get(size_t index)
    {
        return globals[index];
    }

    void set(size_t index, const XPValue &value)
    {
        if (index >= globals.size())
        {
            DIE << "Globals" << index << " doesn't exists.";
        }
        globals[index].value = value;
    }

    int getGlobalIndex(const std::string &name)
    {
        if (globals.size() > 0)
        {
            for (auto i = (int)globals.size() - 1; i >= 0; i--)
            {
                if (globals[i].name == name)
                {
                    return i;
                }
            }
        }

        return -1;
    }

    void define(const std::string &name)
    {
        auto index = getGlobalIndex(name);

        if (index != -1)
        {
            return;
        }

        globals.push_back({name, NUMBER(0)});
    }

    void addConst(const std::string &name, double value)
    {
        if (exists(name))
        {
            return;
        }

        globals.push_back({name, NUMBER(value)});
    }

    bool exists(const std::string &name)
    {
        return getGlobalIndex(name) != -1;
    }

    std::vector<GlobalVar> globals;
};

#endif
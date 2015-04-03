#include "std.hpp"


bool Values::has(const string& key) const
{
    return find(key) != cend();
}

bool Values::getBool(const string& key, bool defaultValue /*= false*/) const
{
    if (!has(key))
        return defaultValue;

    const list<string>& keyValues = at(key);

    assert(keyValues.size() == 1,
        string("Values::getBool() more than one values"));

    const string& value = keyValues.front();
    bool result = false;

    if (value == "true" || value == "1")
        result = true;
    else if (value == "false" || value == "0")
        result = false;
    else
        assert(false, string("Values::getBool() invalid value"));

    return result;
}

string Values::getString(const string& key, const string& defaultValue /*= string()*/) const
{
    if (!has(key))
        return defaultValue;

    const list<string>& keyValues = at(key);

    assert(keyValues.size() == 1,
        string("Values::getString() more than one values"));

    return keyValues.front();
}

int Values::getInt(const string& key, int defaultValue /*= 0*/) const
{
    if (!has(key))
        return defaultValue;

    const list<string>& keyValues = at(key);

    assert(keyValues.size() == 1,
        string("Values::getInt() more than one values"));

    return stoi(keyValues.front());
}

const list<string>& Values::operator[](const string& key) const
{
    return at(key);
}

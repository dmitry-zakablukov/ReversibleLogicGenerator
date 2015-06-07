// ReversibleLogicGenerator - generator of reversible logic circuits, based on permutation group theory.
// Copyright (C) 2015  <Dmitry Zakablukov>
// E-mail: dmitriy.zakablukov@gmail.com
// Web: https://github.com/dmitry-zakablukov/ReversibleLogicGenerator
// 
// This file is part of ReversibleLogicGenerator.
// 
// ReversibleLogicGenerator is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// ReversibleLogicGenerator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with ReversibleLogicGenerator.  If not, see <http://www.gnu.org/licenses/>.

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

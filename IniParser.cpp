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

//static
Values IniParser::parse(istream& stream)
{
    Values values;

    while (!stream.eof())
    {
        string line;
        getline(stream, line);

        if (isWhiteSpacesOnly(line) || isComment(line))
            continue;

        split(line, &values);
    }

    return values;
}

//static
void IniParser::split(const string& line, Values* values)
{
    string::size_type pos = line.find('=');
    if (pos == string::npos)
    {
        cerr << "Skipping line \"" << line << "\" from *.ini file" << endl;
        return;
    }

    string key = line.substr(0, pos);
    string value = line.substr(pos + 1);

    key = trim(key);
    value = removeQuotes(trim(value));

    if (key.size() == 0)
        cerr << "Found empty key in *.ini file" << endl;

    if (value.size() == 0)
        cerr << "Found empty value in *.ini file" << endl;

    list<string>& keyValues = (*values)[key];
    keyValues.push_back(value);
}

bool IniParser::isComment(const string& line)
{
    bool result = false;

    auto iter = line.cbegin();
    auto end = line.cend();

    while (iter != end && isspace(*iter))
        ++iter;

    if (iter != line.cend() && *iter == '#')
        result = true;

    return result;
}

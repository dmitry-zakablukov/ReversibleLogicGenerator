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

#include "std.h"

uint countNonZeroBits(word value)
{
    uint count = 0;
    while(value)
    {
        count += value & 1;
        value >>= 1;
    }

    return count;
}

uint findPositiveBitPosition(word value, uint startPos /* = 0 */)
{
    uint position = uintUndefined;
    value >>= startPos;

    while(value)
    {
        if(value & 1)
        {
            position = startPos;
            break;
        }

        value >>= 1;
        ++startPos;
    }

    return position;
}

uint getSignificantBitCount(word value)
{
    uint count = 0;
    word mask = 1;
    
    while (mask <= value)
    {
        mask <<= 1;
        ++count;
    }

    return count;
}

bool isWhiteSpacesOnly(const string& line)
{
    auto iter = line.cbegin();
    auto end = line.cend();

    while (iter != end && isspace(*iter))
        ++iter;

    return iter == end;
}

string trim(const string& value)
{
    uint size = value.size();

    // find first non-space character
    string::const_iterator first = value.cbegin();
    while (isspace(*first))
        ++first;

    // find last non-space character
    string::const_reverse_iterator lastReverse = value.crbegin();
    while (isspace(*lastReverse))
        ++lastReverse;

    string::const_iterator last = lastReverse.base();

    string result;
    result.reserve(size);

    while (first != last)
        result.push_back(*first++);

    return result;
}

string removeQuotes(const string& value)
{
    if (value.empty())
        return value;

    uint start = 0;
    uint count = value.size();

    if (value.front() == '"')
    {
        ++start;
        --count;
    }

    if (value.back() == '"')
        --count;

    string result = value.substr(start, count);
    return result;
}

vector<string> split(const string& value, char symbol)
{
    int count = std::count(value.cbegin(), value.cend(), symbol);
    if (!count)
        return vector<string>({ value });

    ++count;

    vector<string> parts;
    parts.reserve(count);

    auto start = value.cbegin();
    auto end = value.cend();

    int pos = 0;
    while (count--)
    {
        auto iter = std::find(start, end, symbol);
        parts.push_back(string(start, iter));

        if (iter != end)
            start = iter + 1;
    }

    return parts;
}

string getFileName(const string& path)
{
    auto windowsDelimiterPos = path.rfind('\\');
    auto linuxDelimiterPos = path.rfind('/');

    if (windowsDelimiterPos == string::npos && linuxDelimiterPos == string::npos)
        return path;

    string::size_type pos;
    if (windowsDelimiterPos == string::npos)
        pos = linuxDelimiterPos;
    else if (linuxDelimiterPos == string::npos)
        pos = windowsDelimiterPos;
    else
        pos = max(windowsDelimiterPos, linuxDelimiterPos);

    string fileName = path.substr(pos + 1);
    return fileName;
}

string appendPath(const string& left, const string& right)
{
    const char cWindowsDelimiter = '\\';
    const char cLinuxDelimiter = '/';

    const char* strWindowsCurrentFolder = ".\\";
    const char* strLinuxCurrentFolder = "./";

    bool isWindows = (left.find(cWindowsDelimiter) != string::npos ||
        right.find(cWindowsDelimiter) != string::npos);

    string path;
    if (left.size() == 0)
    {
        if (isWindows)
            path = strWindowsCurrentFolder;
        else
            path = strLinuxCurrentFolder;
    }
    else
        path = left;

    if (isWindows)
    {
        if (path.back() != cWindowsDelimiter && (right.size() == 0 || right.front() != cWindowsDelimiter))
            path += cWindowsDelimiter;
    }
    else
    {
        if (path.back() != cLinuxDelimiter && (right.size() == 0 || right.front() != cLinuxDelimiter))
            path += cLinuxDelimiter;
    }

    return path + right;
}

void debugLog(const string& context, function<void(ostream&)> logFunction)
{
    const char* strDebugContext = "debug-context";
    const char* strDebugLogFileName = "debug-log";

    if (!ProgramOptions::get().isDebugBehaviorEnabled)
        return;

    const Values& options = ProgramOptions::get().options;
    if (options.find(strDebugContext) != options.cend()
        && options.find(strDebugLogFileName) != options.cend())
    {
        auto keyValues = options[strDebugContext];
        if (find(keyValues.cbegin(), keyValues.cend(), context) != keyValues.cend())
        {
            const string& fileName = options.getString(strDebugLogFileName);
            ofstream out(fileName, ofstream::app);

            logFunction(out);
        }
    }
}

void debugBehavior(const string& context, function<void()> debugFunction)
{
    const char* strDebugContext = "debug-context";

    if (!ProgramOptions::get().isDebugBehaviorEnabled)
        return;

    const Values& options = ProgramOptions::get().options;
    if (options.find(strDebugContext) != options.cend())
    {
        auto keyValues = options[strDebugContext];
        if (find(keyValues.cbegin(), keyValues.cend(), context) != keyValues.cend())
            debugFunction();
    }
}

#include "std.hpp"

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
        return vector<string>();

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
    auto pos = path.rfind('\\');
    if (pos == string::npos)
        return path;

    string fileName = path.substr(pos + 1);
    return fileName;
}

string appendPath(const string& left, const string& right)
{
    const char cDelimiter = '\\';

    string path;
    if (left.size() == 0)
        path = ".\\";
    else
        path = left;

    if (path.back() != cDelimiter && (right.size() == 0 || right.front() != cDelimiter))
        path += cDelimiter;

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

#include "std.hpp"

//static
Values IniParser::parse(ifstream& stream)
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
    {
        cerr << "Found empty key in *.ini file" << endl;
    }

    if (value.size() == 0)
    {
        cerr << "Found empty value in *.ini file" << endl;
    }

    list<string>& keyValues = (*values)[key];
    keyValues.push_back(value);
}

bool IniParser::isWhiteSpacesOnly(const string& line)
{
    auto iter = line.cbegin();
    auto end = line.cend();

    while (iter != end && isspace(*iter))
        ++iter;

    return iter == end;
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

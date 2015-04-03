#pragma once

class IniParser
{
public:
    static Values parse(istream& stream);

private:
    static bool isComment(const string& line);
    static void split(const string& line, Values* values);
};
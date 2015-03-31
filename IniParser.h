#pragma once

class IniParser
{
public:
    static Values parse(ifstream& stream);

private:
    static bool isWhiteSpacesOnly(const string& line);
    static bool isComment(const string& line);
    static void split(const string& line, Values* values);
};
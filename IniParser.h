#pragma once

class IniParser
{
public:
    static Values parse(ifstream& stream);

private:
    static void split(const string& line, Values* values);
};
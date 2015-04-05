#include "std.hpp"

namespace ReversibleLogic
{

TruthTable TruthTableParser::parse(istream& input)
{
    string firstLine;
    getline(input, firstLine);

    int base = parseFirstLine(firstLine);
    assert(inputCount >= outputCount, string("Case N < M is not implemented"));

    return parseMainBody(input, base);
}

int TruthTableParser::parseFirstLine(const string& line)
{
    inputCount = 0;
    outputCount = 0;
    int base = 0;

    int tokenCount = sscanf_s(line.c_str(), "%ux%ux%d", &inputCount, &outputCount, &base);
    if (tokenCount != 3)
    {
        // first line must be NxMxB, where N is number of inputs, M - number of outputs
        // and B - number's base in table

        throw InvalidFormatException(
            string("Invalid first line of truth table: ") + line +
            "\nValid format is NxMxB, where N - input count, M <= N - output count, B - base");
    }

    return base;
}

TruthTable TruthTableParser::parseMainBody(istream& input, int base /*= 2*/)
{
    const char* strDelimiter = "\t=>\t";
    const uint numDelimiterLength = strlen(strDelimiter);

    TruthTable table;
    table.resize(1 << inputCount);

    // fill table with 0xFF
    memset(table.data(), 0xFF, sizeof(TruthTable::value_type)* table.size());

    uint count = 0;
    string currentLine;

    int maxInputValue = 1 << inputCount;
    int maxOutputValue = 1 << outputCount;

    try
    {
        while (input.good())
        {
            string line;
            getline(input, line);

            if (line.empty())
                continue;

            currentLine = line;

            size_t pos = 0;
            int x = stoi(line, &pos, base);

            assertFormat(pos != 0 && pos != line.size() && x >= 0);
            assertFormat(line.compare(pos, numDelimiterLength, strDelimiter) == 0);

            line = line.substr(pos + numDelimiterLength);
            int y = stoi(line, &pos, base);

            assertFormat(pos == line.size() && y >= 0);
            assertFormat(x < maxInputValue && y < maxOutputValue);
            assertFormat(table[x] == wordUndefined);

            table[x] = (word)y;
            ++count;
        }
    }
    catch (InvalidFormatException& ex)
    {
        ex.setMessage(string("Invalid truth table line: ") + currentLine);
        throw ex;
    }

    if (count != maxInputValue)
        throw InvalidFormatException(string("Truth table is incomplete"));

    return table;
}

uint TruthTableParser::getInputCount() const
{
    return inputCount;
}

uint TruthTableParser::getOutputCount() const
{
    return outputCount;
}

} //namespace ReversibleLogic

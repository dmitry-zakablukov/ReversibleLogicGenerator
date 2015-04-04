#include "std.hpp"

namespace ReversibleLogic
{

TruthTable TruthTableParser::parse(istream& input)
{
    string firstLine;
    getline(input, firstLine);

    parseFirstLine(firstLine);
    assert(inputCount >= outputCount, string("Case N < M is not implemented"));

    return parseMainBody(input);
}

void TruthTableParser::parseFirstLine(string firstLine)
{
    // first line must be NxM, where N is number of inputs and M - number of outputs
    try
    {
        assertFormat(!firstLine.empty());

        inputCount = 0;
        outputCount = 0;
        size_t pos = 0;

        inputCount = (uint)stoi(firstLine, &pos);
        assertFormat(pos != 0 && pos != firstLine.size() && firstLine.at(pos) == 'x');

        firstLine = firstLine.substr(pos + 1);
        outputCount = (uint)stoi(firstLine, &pos);

        assertFormat(pos = firstLine.size() && (int)inputCount > 0 && (int)outputCount > 0);
    }
    catch (InvalidFormatException& ex)
    {
        ex.setMessage(string("Invalid first line of truth table (must be in format NxM): ") + firstLine);
        throw ex;
    }
}

TruthTable TruthTableParser::parseMainBody(istream& input)
{
    const char* strDelimiter = "\t==>\t";
    const uint numDelimiterLength = strlen(strDelimiter);

    TruthTable table;
    table.resize(1 << inputCount);

    // fill table with 0xFF
    memset(table.data(), 0xFF, sizeof(TruthTable::value_type)* table.size());

    uint count = 0;
    string currentLine;

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
            int x = stoi(line, &pos, 2);

            assertFormat(pos != 0 && pos != line.size() && x >= 0);
            assertFormat(line.compare(pos, numDelimiterLength, strDelimiter) == 0);

            line = line.substr(pos + numDelimiterLength);
            int y = stoi(line, &pos, 2);

            assertFormat(pos == line.size() && y >= 0);
            assertFormat(table[x] == wordUndefined);

            table[x] = (word)y;
            ++count;
        }

        assertFormat(count == (uint)1 << inputCount);
    }
    catch (InvalidFormatException& ex)
    {
        ex.setMessage(string("Invalid truth table line: ") + currentLine);
        throw ex;
    }

    return table;
}

} //namespace ReversibleLogic

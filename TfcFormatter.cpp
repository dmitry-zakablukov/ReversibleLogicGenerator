#include "std.hpp"

namespace ReversibleLogic
{

char TfcFormatter::getVariableName(uint value)
{
    const char cBegin = 'a';
    const char cEnd = 'z';

    assert(value < cEnd - cBegin, string("getVariableName(): argument out of range"));
    return cBegin + (char)value;
}

void TfcFormatter::writeVariablesLine(ostream& out, const vector<char>& variables)
{
    writeHeaderLine(out, variables, ".v");
}

void TfcFormatter::writeInputLine(ostream& out, const vector<char>& variables)
{
    writeHeaderLine(out, variables, ".i");
}

void TfcFormatter::writeOutputLine(ostream& out, const vector<char>& variables)
{
    writeHeaderLine(out, variables, ".o");
}

void TfcFormatter::writeHeaderLine(ostream& out, const vector<char>& variables, const char* prefix)
{
    out << prefix << ' ';

    uint varCount = variables.size();
    for (uint index = 0; index < varCount; ++index)
    {
        out << variables[index];
        if (index != varCount - 1)
            out << ',';
    }

    out << endl;
}

void TfcFormatter::writeBegin(ostream& out)
{
    out << "BEGIN" << endl;
}

void TfcFormatter::writeEnd(ostream& out)
{
    out << "END" << endl;
}

} //namespace ReversibleLogic
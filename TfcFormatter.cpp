#include "std.hpp"

namespace ReversibleLogic
{

char TfcFormatter::getVariableName(uint value) const
{
    const char cBegin = 'a';
    const char cEnd = 'z';

    assert(value < cEnd - cBegin, string("getVariableName(): argument out of range"));
    return cBegin + (char)value;
}

void TfcFormatter::writeVariablesLine(ostream& out, const vector<char>& variables) const
{
    writeHeaderLine(out, variables, ".v");
}

void TfcFormatter::writeInputLine(ostream& out, const vector<char>& variables) const
{
    writeHeaderLine(out, variables, ".i");
}

void TfcFormatter::writeOutputLine(ostream& out, const vector<char>& variables) const
{
    writeHeaderLine(out, variables, ".o");
}

void TfcFormatter::writeHeaderLine(ostream& out, const vector<char>& variables, const char* prefix) const
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

void TfcFormatter::writeBegin(ostream& out) const
{
    out << "BEGIN" << endl;
}

void TfcFormatter::writeEnd(ostream& out) const
{
    out << "END" << endl;
}

} //namespace ReversibleLogic
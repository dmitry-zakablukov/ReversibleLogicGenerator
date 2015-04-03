#include "std.hpp"

namespace ReversibleLogic
{

// static strings
const char* TfcFormatter::strVariablesPrefix = ".v ";
const char* TfcFormatter::strInputsPrefix    = ".i ";
const char* TfcFormatter::strOutputsPrefix   = ".o ";
const char* TfcFormatter::strConstantsPrefix = ".c ";
const char* TfcFormatter::strBeginKeyword    = "BEGIN";
const char* TfcFormatter::strEndKeyword      = "END";

char TfcFormatter::getVariableName(uint value) const
{
    const char cBegin = 'a';
    const char cEnd = 'z';

    assert(value < cEnd - cBegin, string("getVariableName(): argument out of range"));
    return cBegin + (char)value;
}

void TfcFormatter::writeVariablesLine(ostream& out, const vector<char>& variables) const
{
    writeHeaderLine(out, variables, strVariablesPrefix);
}

void TfcFormatter::writeInputLine(ostream& out, const vector<char>& variables) const
{
    writeHeaderLine(out, variables, strInputsPrefix);
}

void TfcFormatter::writeOutputLine(ostream& out, const vector<char>& variables) const
{
    writeHeaderLine(out, variables, strOutputsPrefix);
}

void TfcFormatter::writeHeaderLine(ostream& out, const vector<char>& variables, const char* prefix) const
{
    out << prefix;

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
    out << strBeginKeyword << endl;
}

void TfcFormatter::writeEnd(ostream& out) const
{
    out << strEndKeyword << endl;
}

} //namespace ReversibleLogic
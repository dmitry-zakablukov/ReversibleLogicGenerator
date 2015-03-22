#pragma once

namespace ReversibleLogic
{

class TfcFormatter
{
public:
    template<typename Container>
    static void format(ostream& out, const Container& scheme);

private:
    static char getVariableName(uint value);

    static void writeVariablesLine(ostream& out, const vector<char>& variables);
    static void writeInputLine(ostream& out, const vector<char>& variables);
    static void writeOutputLine(ostream& out, const vector<char>& variables);

    static void writeHeaderLine(ostream& out, const vector<char>& variables, const char* prefix);

    static void writeBegin(ostream& out);
    static void writeEnd(ostream& out);

    template<typename Container>
    static void writeMainBody(ostream& out, const Container& scheme, const vector<char>& variables);
};

template<typename Container>
void ReversibleLogic::TfcFormatter::format(ostream& out, const Container& scheme)
{
    uint elementCount = scheme.size();

    uint n = 0;
    if (elementCount)
    {
        n = scheme.front().getInputCount();
    }

    vector<char> variables;
    variables.reserve(n);

    for (uint index = 0; index < n; ++index)
        variables.push_back(getVariableName(index));

    writeVariablesLine(out, variables);
    writeInputLine(out, variables);
    writeOutputLine(out, variables);

    writeBegin(out);
    writeMainBody(out, scheme, variables);
    writeEnd(out);
}

template<typename Container>
void ReversibleLogic::TfcFormatter::writeMainBody(ostream& out, const Container& scheme, const vector<char>& variables)
{
    for (const ReverseElement& element : scheme)
    {
        assert(element.getInversionMask() == 0,
            string("TFC format doesn't support elements with inverted control inputs"));

        word controlMask = element.getControlMask();
        uint count = countNonZeroBits(controlMask) + 1; //plus target line

        out << 't' << count << ' ';

        uint index = 0;
        uint mask = 1;

        while (mask <= controlMask)
        {
            if (controlMask & mask)
                out << variables[index] << ',';

            ++index;
            mask <<= 1;
        }

        word targetMask = element.getTargetMask();
        uint targetIndex = findPositiveBitPosition(targetMask);

        out << variables[targetIndex] << endl;
    }
}

} //namespace ReversibleLogic
#pragma once

namespace ReversibleLogic
{

class TfcFormatter
{
public:
    TfcFormatter() = default;

    template<typename Container>
    void format(ostream& out, const Container& scheme) const;

private:
    char getVariableName(uint value) const;

    void writeVariablesLine(ostream& out, const vector<char>& variables) const;
    void writeInputLine(ostream& out, const vector<char>& variables) const;
    void writeOutputLine(ostream& out, const vector<char>& variables) const;

    void writeHeaderLine(ostream& out, const vector<char>& variables, const char* prefix) const;

    void writeBegin(ostream& out) const;
    void writeEnd(ostream& out) const;

    template<typename Container>
    void writeMainBody(ostream& out, const Container& scheme, const vector<char>& variables) const;
};

template<typename Container>
void ReversibleLogic::TfcFormatter::format(ostream& out, const Container& scheme) const
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
void ReversibleLogic::TfcFormatter::writeMainBody(ostream& out, const Container& scheme,
    const vector<char>& variables) const
{
    for (const ReverseElement& element : scheme)
    {
        word controlMask = element.getControlMask();
        word inversionMask = element.getInversionMask();
        uint count = countNonZeroBits(controlMask) + 1; //plus target line

        out << 't' << count << ' ';

        uint index = 0;
        uint mask = 1;

        while (mask <= controlMask)
        {
            if (controlMask & mask)
            {
                out << variables[index];
                if (inversionMask & mask)
                    out << '\'';

                out << ',';
            }                

            ++index;
            mask <<= 1;
        }

        word targetMask = element.getTargetMask();
        uint targetIndex = findPositiveBitPosition(targetMask);

        out << variables[targetIndex] << endl;
    }
}

} //namespace ReversibleLogic
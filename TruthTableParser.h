#pragma once

namespace ReversibleLogic
{

class TruthTableParser
{
public:
    TruthTableParser() = default;
    virtual ~TruthTableParser() = default;

    TruthTable parse(istream& input);
    
    uint getInputCount() const;
    uint getOutputCount() const;

private:
    /// Returns number's base in input
    int parseFirstLine(const string& line);

    TruthTable parseMainBody(istream& input, int base = 2);

    uint inputCount = 0;
    uint outputCount = 0;
};

} //namespace ReversibleLogic
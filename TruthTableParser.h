#pragma once

namespace ReversibleLogic
{

class TruthTableParser
{
public:
    TruthTableParser() = default;
    virtual ~TruthTableParser() = default;

    TruthTable parse(istream& input);

private:
    void parseFirstLine(string firstLine);
    TruthTable parseMainBody(istream& input);

    uint inputCount = 0;
    uint outputCount = 0;
};

} //namespace ReversibleLogic
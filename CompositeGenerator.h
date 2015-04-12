#pragma once

namespace ReversibleLogic
{

class CompositeGenerator
{
public:
    CompositeGenerator() = default;
    virtual ~CompositeGenerator() = default;

    Scheme generate(const TruthTable& table, ostream& outputLog);

private:
    uint getRmGeneratorWeightThreshold(uint n);
    void logTime(ostream& out, float time);
};

} //namespace ReversibleLogic

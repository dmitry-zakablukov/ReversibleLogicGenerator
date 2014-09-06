// Generator class for reversible logic synthesis with additional input lines

#pragma once

namespace ReversibleLogic
{

class GeneratorWithMemory
{
public:
    GeneratorWithMemory() = default;
    ~GeneratorWithMemory() = default;

    Scheme generateFast(const TruthTable& table, ostream& outputLog);

private:
    void detectBitCount(const TruthTable& table, uint* n, uint* m);
};

} //ReversibleLogic
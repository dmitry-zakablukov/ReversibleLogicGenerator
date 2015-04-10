#pragma once

namespace ReversibleLogic
{

class RmGenerator
{
public:
    Scheme generate(const TruthTable& inputTable, ostream& outputLog);

private:
    void applyTransformation(TruthTable* tablePtr, word targetMask, word controlMask = 0);
};

} //namespace ReversibleLogic
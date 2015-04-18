#pragma once

namespace ReversibleLogic
{

class SchemeUtils
{
public:
    static uint calculateQuantumCost(const Scheme& scheme);

private:
    static uint getElementQuantumCost(const ReverseElement& element);
    static bool isPeresGate(const ReverseElement& left, const ReverseElement& right, uint* cost);
};

} //namespace ReversibleLogic

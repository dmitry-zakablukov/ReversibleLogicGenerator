#include "std.hpp"

namespace ReversibleLogic
{

//static
uint SchemeUtils::calculateQuantumCost(const Scheme& scheme)
{
    uint size = scheme.size();

    if (size == 0)
        return 0;

    uint cost = 0;
    for (auto& element : scheme)
        cost += getElementQuantumCost(element);

    return cost;
}

//static
uint SchemeUtils::getElementQuantumCost(const ReverseElement& element)
{
    uint n = element.getInputCount();

    word controlMask = element.getControlMask();
    uint count = countNonZeroBits(controlMask) + 1;

    uint cost = 0;
    switch (count)
    {
    case 1:
    case 2: 
        cost = 1;
        break;

    case 3:
        cost = 5;
        break;

    case 4:
        cost = 13;  
        break;

    case 5:
        if (count + 2 <= n)
            cost = 26;
        else
            cost = 29;

        break;

    case 6:
        if (2 * count - 3 <= n)
            cost = 38;
        else if (count + 1 <= n)
            cost = 52;
        else
            cost = 61;

        break;

    case 7:
        if (2 * count - 3 <= n)
            cost = 50;
        else if (count + 1 <= n)
            cost = 80;
        else
            cost = 125;

        break;

    case 8:
        if (2 * count - 3 <= n)
            cost = 62;
        else if (count + 1 <= n)
            cost = 100;
        else
            cost = 253;

        break;

    default:
        if (2 * count - 3 <= n)
            cost = 12 * count  - 34;
        else if (count + 1 <= n)
            cost = 24 * count - 88;
        else
            cost = (1 << count) - 3;

        break;
    }

    word inversionMask = element.getInversionMask();
    if (inversionMask)
    {
        if (count == 2) //CNOT
            cost = 3;
        else if (count == 3) //2-CNOT
        {
            if (controlMask == inversionMask)
                cost += 2;
            else
                cost = 6;
        }
        else if (controlMask == inversionMask)
            cost += 2;
    }

    return cost;
}

} //namespace ReversibleLogic

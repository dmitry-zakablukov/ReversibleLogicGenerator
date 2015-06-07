// ReversibleLogicGenerator - generator of reversible logic circuits, based on permutation group theory.
// Copyright (C) 2015  <Dmitry Zakablukov>
// E-mail: dmitriy.zakablukov@gmail.com
// Web: https://github.com/dmitry-zakablukov/ReversibleLogicGenerator
// 
// This file is part of ReversibleLogicGenerator.
// 
// ReversibleLogicGenerator is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// ReversibleLogicGenerator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with ReversibleLogicGenerator.  If not, see <http://www.gnu.org/licenses/>.

#include "std.hpp"

namespace ReversibleLogic
{

//static
uint SchemeUtils::calculateQuantumCost(const Scheme& scheme)
{
    uint size = scheme.size();

    if (size == 0)
        return 0;

    ReverseElement prevElement;
    bool isPreviousElementWasUsedBefore = true;

    uint cost = 0;
    for (auto& element : scheme)
    {
        if (!isPreviousElementWasUsedBefore)
        {
            uint peresCost = 0;
            if (isPeresGate(prevElement, element, &peresCost))
            {
                cost -= getElementQuantumCost(prevElement);
                cost += peresCost;

                isPreviousElementWasUsedBefore = true;
                continue;
            }
        }

        cost += getElementQuantumCost(element);

        prevElement = element;
        isPreviousElementWasUsedBefore = false;
    }

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

bool SchemeUtils::isPeresGate(const ReverseElement& left, const ReverseElement& right, uint* cost)
{
    assertd(cost, string("SchemeUtils::isPeresGate(): null ptr"));

    word leftTargetMask     = left.getTargetMask();
    word leftControlMask    = left.getControlMask();
    word leftInversionMask  = left.getInversionMask();

    word rightTargetMask    = right.getTargetMask();
    word rightControlMask   = right.getControlMask();
    word rightInversionMask = right.getInversionMask();

    uint leftCount = countNonZeroBits(leftControlMask);
    if (leftCount > 2 || leftCount == 0)
        return false;

    uint rightCount = countNonZeroBits(rightControlMask);
    if (rightCount > 2 || rightCount == 0 || leftCount == rightCount)
        return false;

    if (leftControlMask != (rightTargetMask | rightControlMask) &&
        rightControlMask != (leftTargetMask | leftControlMask))
        return false;

    // this is Peres gate
    if (leftInversionMask == 0 && rightInversionMask == 0)
        *cost = 4;
    else
    {
        if (leftCount < rightCount)
        {
            swap(leftControlMask, rightControlMask);
            swap(leftInversionMask, rightInversionMask);
        }

        if (rightInversionMask == 0)
        {
            // CNOT without inverted input
            if (leftInversionMask == leftControlMask)
                *cost = 7;
            else
                *cost = 6;
        }
        else
        {
            // CNOT with inverted input
            if (leftInversionMask == leftControlMask)
                *cost = 9; 
            else
                *cost = 8;
        }
    }

    return true;
}

} //namespace ReversibleLogic

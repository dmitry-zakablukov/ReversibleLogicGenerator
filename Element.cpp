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

ReverseElement::ReverseElement()
    : n(0)
    , independencyFlag(true)
    , targetMask(0)
    , controlMask(0)
    , inversionMask(0)
{
}

ReverseElement::ReverseElement(uint n, word targetMask,
                               word controlMask, word inversionMask)
    : n(n)
    , independencyFlag(true)
    , targetMask(targetMask)
    , controlMask(controlMask)
    , inversionMask(inversionMask)
{
}

void ReverseElement::setInputCount(uint count)
{
    n = count;
}

uint ReverseElement::getInputCount() const
{
    return n;
}

void ReverseElement::setIndependencyFlag(bool independency)
{
    independencyFlag = independency;
}

bool ReverseElement::isIndependent() const
{
    return independencyFlag;
}

void ReverseElement::setTargetMask(word theTargetMask)
{
    targetMask = theTargetMask;
}

word ReverseElement::getTargetMask() const
{
    return targetMask;
}

void ReverseElement::setControlMask(word theControlMask)
{
    controlMask = theControlMask;
}

word ReverseElement::getControlMask() const
{
    return controlMask;
}

void ReverseElement::setInversionMask(word theInversionMask)
{
    inversionMask = theInversionMask;
}

word ReverseElement::getInversionMask() const
{
    return inversionMask;
}

bool ReverseElement::operator ==(const ReverseElement& another) const
{
    assertd(isValid(), string("Reverse element is not valid"));
    assertd(another.isValid(), string("Reverse element is not valid"));

    bool equal = true;
    if(n != another.getInputCount()
            || targetMask != another.getTargetMask()
            || controlMask != another.getControlMask()
            || inversionMask != another.getInversionMask())
    {
        equal = false;
    }

    return equal;
}

bool ReverseElement::isSymmetric() const
{
    uint controlCount = countNonZeroBits(controlMask);
    return controlCount < 3;
}

bool ReverseElement::isValid() const
{
    bool valid = true;

    uint targetCount = countNonZeroBits(targetMask);
    word maxValue = 1 << n;

    if(targetCount != 1
            || (targetMask & controlMask)
            || (targetMask & inversionMask)
            || targetMask > maxValue
            || controlMask > maxValue
            || inversionMask > maxValue)
    {
        valid = false;
    }

    if(valid && !isIndependent())
    {
        uint controlCount = countNonZeroBits(controlMask);
        valid = (controlCount + 2 <= n);
    }

    return valid;
}

bool ReverseElement::isSwappable(const ReverseElement& another) const
{
    assertd(isValid(), string("Reverse element is not valid"));
    assertd(another.isValid(), string("Reverse element is not valid"));

    word anotherTargetMask = another.getTargetMask();
    word anotherControlMask = another.getControlMask();
    word anotherInversionMask = another.getInversionMask();

    bool swappable = (!(anotherControlMask & targetMask) && !(controlMask & anotherTargetMask))
        || ((inversionMask ^ anotherInversionMask) & controlMask & anotherControlMask);

    return swappable;
}

bool ReverseElement::isSwappable(const ReverseElement& another,
    bool* withOneControlLineInverting, bool withoutAnyChanges /*= false*/) const
{
    assertd(withOneControlLineInverting, string("Null ptr (withOneControlLineInverting)"));

    word anotherTargetMask  = another.getTargetMask();
    word anotherControlMask = another.getControlMask();
    word anotherInversionMask = another.getInversionMask();

    bool swappable = isSwappable(another);
    if (!swappable && !withoutAnyChanges)
    {
        // check variant, when elements can be swapped with only one control line inversion
        if ((targetMask | controlMask) == anotherControlMask)
        {
            if (inversionMask == (~targetMask & anotherInversionMask))
            {
                swappable = true;
                *withOneControlLineInverting = false;
            }
        }
        else if ((anotherTargetMask | anotherControlMask) == controlMask)
        {
            if (anotherInversionMask == (~anotherTargetMask & inversionMask))
            {
                swappable = true;
                *withOneControlLineInverting = true;
            }
        }
    }

    return swappable;
}

bool ReverseElement::isSwappable(const list<ReverseElement>& elements) const
{
    assertd(isValid(), string("Reverse element is not valid"));

    bool swappable = true;
    bool withOneControlLineInverting = false;

    for (auto& element : elements)
    {
        if (!isSwappable(element, &withOneControlLineInverting, true))
        {
            swappable = false;
            break;
        }
    }
    return swappable;
}

void ReverseElement::swap(ReverseElement* another)
{
    ReverseElement::swap(this, another);
}

void ReverseElement::swap(ReverseElement* left, ReverseElement* right)
{
    assertd(left && right, string("Null ptr (ReverseElement::swap)"));

    word leftTargetMask    = left->getTargetMask();
    word leftControlMask   = left->getControlMask();
    word leftInversionMask = left->getInversionMask();

    word rightTargetMask    = right->getTargetMask();
    word rightControlMask   = right->getControlMask();
    word rightInversionMask = right->getInversionMask();

    bool swappable = (!(rightControlMask & leftTargetMask) && !(leftControlMask & rightTargetMask))
        || ((leftInversionMask ^ rightInversionMask) & leftControlMask & rightControlMask);

    if (swappable)
    {
        ReverseElement temp = *left;
        *left = *right;
        *right = temp;
    }
    else
    {
        // check variant, when elements can be swapped with only one control line inversion
        if ((leftTargetMask | leftControlMask) == rightControlMask)
        {
            if (leftInversionMask == (~leftTargetMask & rightInversionMask))
            {
                ReverseElement temp = *left;
                
                *left = ReverseElement(left->getInputCount(), rightTargetMask,
                    rightControlMask, rightInversionMask ^ leftTargetMask);

                *right = temp;
                swappable = true;
            }
        }
        else if ((rightTargetMask | rightControlMask) == leftControlMask)
        {
            if (rightInversionMask == (~rightTargetMask & leftInversionMask))
            {
                ReverseElement temp = *right;
                *right = ReverseElement(left->getInputCount(), leftTargetMask,
                    leftControlMask, leftInversionMask ^ rightTargetMask);

                *left = temp;
                swappable = true;
            }
        }
    }

    assertd(swappable, string("Can't swap unswappable elements"));
}

word ReverseElement::getValue(word input) const
{
    word output = input;
    output ^= inversionMask;

    bool needInversion = ((output & controlMask) == controlMask);
    if(needInversion)
    {
        if(isIndependent())
            output ^= targetMask;
        else
        {
            word full = (1 << n) - 1;
            word freeMask = full ^ targetMask ^ controlMask;
            if(output == full)                      // 1111 -> 1101
                output ^= freeMask;
            else if (output == (full ^ freeMask))     // 1101 -> 1110
                output ^= freeMask ^ targetMask;
            else if (output == (full ^ targetMask))   // 1110 -> 1111
                output = full;
        }
    }

    output ^= inversionMask;
    return output;
}

deque<ReverseElement> ReverseElement::getInversionOptimizedImplementation(bool heavyRight /* = true */) const
{
    assertd(isValid(), string("Reverse element is not valid"));

    deque<ReverseElement> optImpl;

    uint controlCount   = countNonZeroBits(controlMask);
    uint inversionCount = countNonZeroBits(inversionMask);

    if(controlCount <= 2 && inversionCount > 0)
    {
        assertd(controlCount > 0, string("Control count must be positive"));
        assertd(inversionCount < 2, string("Too much inversions"));

        if(controlCount == 1)       // CNOT
        {
            if(heavyRight)
            {
                auto element = ReverseElement(n, targetMask);
                optImpl.push_back(element);

                element = ReverseElement(n, targetMask, controlMask);
                optImpl.push_back(element);
            }
            else
            {
                auto element = ReverseElement(n, targetMask, controlMask);
                optImpl.push_back(element);

                element = ReverseElement(n, targetMask);
                optImpl.push_back(element);
            }
        }
        else if(controlCount == 2)  // CCNOT
        {
            word restControlMask   = controlMask;
            word restInversionMask = inversionMask;

            uint positiveBitPosition =
                    findPositiveBitPosition(restControlMask);

            assertd(positiveBitPosition != uintUndefined,
                   string("Positive bit position not found"));

            restControlMask   ^= (uint)1 << positiveBitPosition;
            restInversionMask ^= (uint)1 << positiveBitPosition;

            if(heavyRight)
            {
                auto element = ReverseElement(n, targetMask, restControlMask);
                optImpl.push_back(element);

                element = ReverseElement(n, targetMask, controlMask);
                optImpl.push_back(element);
            }
            else
            {
                auto element = ReverseElement(n, targetMask, controlMask);
                optImpl.push_back(element);

                element = ReverseElement(n, targetMask, restControlMask);
                optImpl.push_back(element);
            }

            uint restInversionCount = countNonZeroBits(restInversionMask);
            if(restInversionCount)
            {
                ReverseElement element(n, restInversionMask);

                deque<ReverseElement> conjugations;
                conjugations.push_back(element);

                optImpl = conjugate(optImpl, conjugations);
            }
        }
    }
    else
        optImpl.push_back(*this);

    return optImpl;
}

word ReverseElement::getFreeInputMask() const
{
    word freeInputMask = (((word)1 << n) - 1 ) & ~(targetMask | controlMask);
    uint freeInputPos = findPositiveBitPosition(freeInputMask);
    assertd(freeInputPos != uintUndefined, string("Free input not found"));

    freeInputMask = (uint)1 << freeInputPos;
    return freeInputMask;
}

deque<ReverseElement> ReverseElement::getSimpleImplementation() const
{
    assertd(isValid(), string("Reverse element is not valid"));
    assertd((inversionMask & ~controlMask) == 0, string("Invalid inversion mask"));

    deque<ReverseElement> implementation;
    implementation.push_back(ReverseElement(n, targetMask, controlMask));

    // conjugate core implementation by inversions
    deque<ReverseElement> inversions;
    uint mask = 1;
    
    while(mask <= inversionMask)
    {
        if(inversionMask & mask)
            inversions.push_back(ReverseElement(n, mask));

        mask <<= 1;
    }
    
    implementation = conjugate(implementation, inversions);

    return implementation;
}

deque<ReverseElement> ReverseElement::getRecursiveImplementation() const
{
    assertd(isValid(), string("Reverse element is not valid"));

    assertd((inversionMask & ~controlMask) == 0, string("Invalid inversion mask"));
    deque<ReverseElement> implementation;

    if (inversionMask)
    {
        uint pos = findPositiveBitPosition(inversionMask);
        word mask = 1 << pos;

        word reducedMask = inversionMask ^ mask;
        ReverseElement first(n, targetMask, controlMask, reducedMask);
        ReverseElement second(n, targetMask, controlMask ^ mask, reducedMask);

        auto elements = first.getRecursiveImplementation();
        implementation.insert(implementation.end(), elements.cbegin(), elements.cend());

        elements = second.getRecursiveImplementation();
        implementation.insert(implementation.end(), elements.cbegin(), elements.cend());
    }
    else
        implementation.push_back(*this);

    return implementation;
}

deque<ReverseElement> ReverseElement::getToffoliOnlyImplementation(bool heavyRight /*= true*/) const
{
    assertd(isValid(), string("Reverse element is not valid"));

    assertd((inversionMask & ~controlMask) == 0, string("Invalid inversion mask"));
    deque<ReverseElement> implementation;

    //TODO: implement this method
    assert(false, string("Not implemented getToffolyOnlyImplementation() method"));

    return implementation;
}

ReverseElement
ReverseElement::getLeftmostElement(bool heavyRight /* = true */) const
{
    uint firstControlPos = findPositiveBitPosition(controlMask);
    assertd(firstControlPos != uintUndefined, string("First control not found"));
    word firstControlMask = 1 << firstControlPos;

    // we need one free input for implementation
    word freeInputMask = getFreeInputMask();

    if(heavyRight)
    {
        // C_{2, 1; 0}, 1 - free, 0 - target
        word newControlMask = firstControlMask ^ freeInputMask;
        auto element = ReverseElement(n, targetMask, newControlMask);
        return element;
    }
    else
    {
        word restControlMask = controlMask ^ firstControlMask;
        // C_{n-1, n-2,..., 3; 1}, 1 - free
        if(isIndependent())
        {
            auto element = ReverseElement(n, freeInputMask, restControlMask);
            return element;
        }
        else
        {
            // C_{n-1, n-2,..., 3, 1; 0}, 1 - free, 0 - target
            restControlMask ^= freeInputMask;
            auto element = ReverseElement(n, targetMask, restControlMask);
            return element;
        }
    }

    assertd(false, string("Dead code"));
}

ReverseElement
ReverseElement::getRightmostElement(bool heavyRight /* = true */) const
{
    uint firstControlPos = findPositiveBitPosition(controlMask);
    assertd(firstControlPos != uintUndefined, string("First control not found"));
    word firstControlMask = 1 << firstControlPos;

    // we need one free input for implementation
    word freeInputMask = getFreeInputMask();

    if(heavyRight)
    {
        // C_{n-1, n-2,..., 3; 1}, 1 - free
        word restControlMask = controlMask ^ firstControlMask;
        if(!isIndependent())
            // C_{n-1, n-2,..., 3, 0; 1}, 1 - free, 0 - target
            restControlMask ^= targetMask;

        auto element = ReverseElement(n, freeInputMask, restControlMask);
        return element;
    }
    else
    {
        word newControlMask = firstControlMask;

        // C_{2, 1; 0}, 1 - free, 0 - target
        if(isIndependent())
        {
            newControlMask ^= freeInputMask;
            auto element = ReverseElement(n, targetMask, newControlMask);
            return element;
        }
        else
        {
            // C_{2, 0; 1}, 1 - free, 0 - target
            newControlMask ^= targetMask;
            auto element = ReverseElement(n, freeInputMask, newControlMask);
            return element;
        }
    }

    assertd(false, string("Dead code"));
}

ReverseElement
ReverseElement::getFinalImplementation() const
{
    assertd(inversionMask == 0, string("Can't get final implementation due to inversions"));

    uint controlCount = countNonZeroBits(controlMask);
    assertd(controlCount < 3, string("Too much controls"));

    ReverseElement element(n, targetMask, controlMask);
    return element;
}

}   // namespace ReversibleLogic

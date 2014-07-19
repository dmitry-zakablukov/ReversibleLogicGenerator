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
    assert(isValid(), string("Reverse element is not valid"));
    assert(another.isValid(), string("Reverse element is not valid"));

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

bool ReverseElement::isSwappable(const ReverseElement& another,
    bool* withOneControlLineInverting) const
{
    assert(isValid(), string("Reverse element is not valid"));
    assert(another.isValid(), string("Reverse element is not valid"));
    assert(withOneControlLineInverting, string("Null ptr (withOneControlLineInverting)"));

    word anotherTargetMask  = another.getTargetMask();
    word anotherControlMask = another.getControlMask();
    word anotherInversionMask = another.getInversionMask();

    bool swappable = (!(anotherControlMask & targetMask) && !(controlMask & anotherTargetMask))
        || ((inversionMask ^ anotherInversionMask) & controlMask & anotherControlMask);

    if (!swappable)
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
    assert(isValid(), string("Reverse element is not valid"));

    bool swappable = true;
    bool withOneControlLineInverting = false;

    for (auto& element : elements)
    {
        if (!isSwappable(element, &withOneControlLineInverting)
            || withOneControlLineInverting)
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
    assert(left && right, string("Null ptr (ReverseElement::swap)"));

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

                *right = *left;
            }
        }
        else if ((rightTargetMask | rightControlMask) == leftControlMask)
        {
            if (rightInversionMask == (~rightTargetMask & leftInversionMask))
            {
                ReverseElement temp = *left;
                *left = *right;

                *right = ReverseElement(left->getInputCount(), leftTargetMask,
                    leftControlMask, leftInversionMask ^ rightTargetMask);
            }
        }
    }
}

word ReverseElement::getValue(word input) const
{
    word output = input;
    output ^= inversionMask;

    bool needInversion = ((output & controlMask) == controlMask);
    if(needInversion)
    {
        if(isIndependent())
        {
            output ^= targetMask;
        }
        else
        {
            word full = (1 << n) - 1;
            word freeMask = full ^ targetMask ^ controlMask;
            if(output == full)                      // 1111 -> 1101
            {
                output ^= freeMask;
            }
            else if(output == (full ^ freeMask))     // 1101 -> 1110
            {
                output ^= freeMask ^ targetMask;
            }
            else if(output == (full ^ targetMask))   // 1110 -> 1111
            {
                output = full;
            }
        }
    }

    output ^= inversionMask;
    return output;
}

deque<ReverseElement> ReverseElement::getInversionOptimizedImplementation(bool heavyRight /* = true */) const
{
    assert(isValid(), string("Reverse element is not valid"));

    deque<ReverseElement> optImpl;

    uint controlCount   = countNonZeroBits(controlMask);
    uint inversionCount = countNonZeroBits(inversionMask);

    if(controlCount <= 2 && inversionCount > 0)
    {
        assert(controlCount > 0, string("Control count must be positive"));
        assert(inversionCount < 2, string("Too much inversions"));

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

            assert(positiveBitPosition != uintUndefined,
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
    {
        optImpl = getImplementation(heavyRight);
    }

    return optImpl;
}

word ReverseElement::getFreeInputMask() const
{
    word freeInputMask = (((word)1 << n) - 1 ) & ~(targetMask | controlMask);
    uint freeInputPos = findPositiveBitPosition(freeInputMask);
    assert(freeInputPos != uintUndefined, string("Free input not found"));

    freeInputMask = (uint)1 << freeInputPos;
    return freeInputMask;
}

deque<ReverseElement> ReverseElement::getImplementation(bool heavyRight /* = true */) const
{
    assert(isValid(), string("Reverse element is not valid"));

    assert((inversionMask & ~controlMask ) == 0, string("Invalid inversion mask"));
    deque<ReverseElement> implementation;
    
    if(inversionMask)
    {
        uint pos = findPositiveBitPosition(inversionMask);
        word mask = 1 << pos;

        word reducedMask = inversionMask ^ mask;
        ReverseElement first(n, targetMask, controlMask, reducedMask);
        ReverseElement second(n, targetMask, controlMask ^ mask, reducedMask);

        auto elements = first.getImplementation();
        implementation.insert(implementation.end(), elements.cbegin(), elements.cend());

        elements = second.getImplementation();
        implementation.insert(implementation.end(), elements.cbegin(), elements.cend());

        //word mask = 1;
        //while(mask <= inversionMask)
        //{
        //    if(inversionMask & mask)
        //    {
        //        implementation.push_back(ReverseElement(n, mask));
        //    }

        //    mask <<= 1;
        //}

        //implementation.push_back(ReverseElement(n, targetMask, controlMask));

        //mask = 1;
        //while(mask <= inversionMask)
        //{
        //    if(inversionMask & mask)
        //    {
        //        implementation.push_back(ReverseElement(n, mask));
        //    }

        //    mask <<= 1;
        //}
    }
    else
    {
        implementation.push_back(*this);
    }

////    // core implementation
////    deque<ReverseElement> implementation;
////    uint controlCount = countNonZeroBits(controlMask);
////
////    // check control count
////    assert(controlCount + 1 <= n, string("Too much controls"));
////
////#if defined(ADDITIONAL_MEMORY_TECHNIQUE)
////    implementation.push_back(ReverseElement(n, targetMask, controlMask));
////#else //ADDITIONAL_MEMORY_TECHNIQUE
////    ////if(controlCount < 3 && isIndependent())
////    ////{
////        implementation.push_back(ReverseElement(n, targetMask, controlMask));
////    ////}
////    ////else   // hard case, use recursion
////    ////{
////    ////    // emulate this element by folowing elements
////    ////    auto leftElement  = getLeftmostElement(heavyRight);
////    ////    auto rightElement = getRightmostElement(heavyRight);
////
////    ////    // combine elements
////    ////    auto leftImpl  =  leftElement.getImplementation();
////    ////    auto rightImpl = rightElement.getImplementation();
////
////    ////    implementation.insert(implementation.end(),  leftImpl.cbegin(),  leftImpl.cend());
////    ////    implementation.insert(implementation.end(), rightImpl.cbegin(), rightImpl.cend());
////    ////    implementation.insert(implementation.end(),  leftImpl.cbegin(),  leftImpl.cend());
////    ////    implementation.insert(implementation.end(), rightImpl.cbegin(), rightImpl.cend());
////    ////}
////#endif //ADDITIONAL_MEMORY_TECHNIQUE
////
////    // conjugate core implementation by inversions
////    deque<ReverseElement> inversions;
////    uint mask = 1;
////
////    while(mask <= inversionMask)
////    {
////        if(inversionMask & mask)
////        {
////            inversions.push_back(ReverseElement(n, mask));
////        }
////
////        mask <<= 1;
////    }
////
////    implementation = conjugate(implementation, inversions);

    return implementation;
}

ReverseElement
ReverseElement::getLeftmostElement(bool heavyRight /* = true */) const
{
    uint firstControlPos = findPositiveBitPosition(controlMask);
    assert(firstControlPos != uintUndefined, string("First control not found"));
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

    assert(false, string("Dead code"));
}

ReverseElement
ReverseElement::getRightmostElement(bool heavyRight /* = true */) const
{
    uint firstControlPos = findPositiveBitPosition(controlMask);
    assert(firstControlPos != uintUndefined, string("First control not found"));
    word firstControlMask = 1 << firstControlPos;

    // we need one free input for implementation
    word freeInputMask = getFreeInputMask();

    if(heavyRight)
    {
        // C_{n-1, n-2,..., 3; 1}, 1 - free
        word restControlMask = controlMask ^ firstControlMask;
        if(!isIndependent())
        {
            // C_{n-1, n-2,..., 3, 0; 1}, 1 - free, 0 - target
            restControlMask ^= targetMask;
        }

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

    assert(false, string("Dead code"));
}

ReverseElement
ReverseElement::getFinalImplementation() const
{
    assert(inversionMask == 0, string("Can't get final implementation due to inversions"));

    uint controlCount = countNonZeroBits(controlMask);
    assert(controlCount < 3, string("Too much controls"));

    ReverseElement element(n, targetMask, controlMask);
    return element;
}

}   // namespace ReversibleLogic

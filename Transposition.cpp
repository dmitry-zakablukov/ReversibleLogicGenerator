#include "std.hpp"

namespace ReversibleLogic
{

//////////////////////////////////////////////
// Transposition class
//////////////////////////////////////////////
Transposition::Transposition()
    : x(0)
    , y(0)
    , empty(true)
{

}

Transposition::Transposition(word left, word right, bool needSort /* = false */)
    : empty(false)
{
    if(left == right)
    {
        ostringstream stream;
        stream << "Transposition constructor failed with x = ";
        stream << left << " and y = " << right;

        assertd(false, stream.str());
    }

    x = left;
    y = right;

    if(needSort)
    {
        sort();
    }
}

Transposition::Transposition(const Transposition& another)
    : x(another.x)
    , y(another.y)
    , empty(another.empty)
{
}

bool Transposition::isEmpty() const
{
    return empty;
}

void Transposition::sort()
{
    uint xWeight = countNonZeroBits(x);
    uint yWeight = countNonZeroBits(y);

    if(xWeight > yWeight)
    {
        swap();
    }
}

void Transposition::swap()
{
    std::swap(x, y);
}

void Transposition::setX(word value)
{
    x = value;
    empty = false;
}

word Transposition::getX() const
{
    return x;
}

void Transposition::setY(word value)
{
    y = value;
    empty = false;
}

word Transposition::getY() const
{
    return y;
}

word Transposition::getDiff() const
{
    return x ^ y;
}

uint Transposition::getDist() const
{
    return countNonZeroBits(x ^ y);
}

bool Transposition::has(word value) const
{
    bool result = (x == value) || (y == value);
    return result;
}

bool Transposition::operator ==(const Transposition& another) const
{
    word z = another.getX();
    word w = another.getY();

    bool isEqual = ( (x == z && y == w)
            || (x == w && y == z) );

    return isEqual;
}

bool Transposition::operator !=(const Transposition& another) const
{
    return !operator ==(another);
}

Transposition::operator string() const
{
    ostringstream stream;
    stream << "(" << x << ", " << y << ")";
    return stream.str();
}

word Transposition::getOutput(word input) const
{
    word output = input;
    if(output == x)
    {
        output = y;
    }
    else if(output == y)
    {
        output = x;
    }

    return output;
}

//////////////////////////////////////////////
// TransposPair class
//////////////////////////////////////////////
TransposPair::TransposPair()
    : n(uintUndefined)
    , first()
    , second()
    , independencyFlag(false)
{
}

TransposPair::TransposPair(const Transposition &theFirst, const Transposition &theSecond)
    : n(uintUndefined)
    , first(theFirst)
    , second(theSecond)
    , independencyFlag(true)
{

    if(first == second)
    {
        ostringstream stream;
        stream << "TransposPair constructor failed with first and second equal to ";
        stream << (string)first;
        assertd(false, stream.str());
    }

    first.sort();
    second.sort();

    word x = first.getX();
    word y = first.getY();
    word w = second.getY();

    if(second.has(x))
    {
        independencyFlag = false;
        if(x == w)
        {
            second.swap();
        }
    }
    else if(second.has(y))
    {
        independencyFlag = false;
        first.swap();
        if(y == w)
        {
            second.swap();
        }
    }

    if(isIndependent())
    {
        uint xWeight = countNonZeroBits(first.getX());
        uint zWeight = countNonZeroBits(second.getX());
        if(xWeight > zWeight)
        {
            swap(first, second);
        }
    }

    x = first.getX();
    word z = second.getX();
    w = second.getY();

    if(isIndependent())
    {
        uint dxz = countNonZeroBits(x ^ z);
        uint dxw = countNonZeroBits(x ^ w);

        if(dxz > dxw)
        {
            second.swap();
        }
    }
}

void TransposPair::setN(uint value)
{
    n = value;
}

bool TransposPair::isIndependent() const
{
    return independencyFlag;
}

TransposPair::operator string() const
{
    ostringstream stream;
    stream << "[" << (string)first << ", " << (string)second << "]";
    return stream.str();
}

uint TransposPair::getEstimateImplComplexity() const
{
    word x =  first.getX();
    word y =  first.getY();
    word z = second.getX();
    word w = second.getY();

    // TODO: check this method
    uint complexity = first.getDist() - 1;
    uint dist = countNonZeroBits(x ^ z);
    if(dist)
    {
        complexity += dist - 1;
    }

    dist = countNonZeroBits(x ^ w);
    if(dist)
    {
        if(dist > 1)
        {
            complexity += dist - 2;
        }
        else
        {
            ++complexity;
        }
    }

    // TODO: check this method
//    uint complexity = 0;

//    uint dist = countNonZeroBits(x ^ z);
//    if(dist)
//    {
//        complexity += dist - 1;
//    }

//    dist = countNonZeroBits(x ^ y ^ z ^ w);
//    if(dist)
//    {
//        complexity += dist - 1;
//    }

    complexity *= 2;
    complexity += getPrecomputedComplexity();

    return complexity;
}

deque<ReverseElement> TransposPair::getImplementation()
{
    assertd(n != uintUndefined, string("Number of inputs is not specified"));

    deque<ReverseElement> conjugationElements;

    uint yControlBitPosition = findYControlBit();
    deque<ReverseElement> result = transformY(yControlBitPosition);

    conjugationElements.insert(conjugationElements.end(),
                               result.cbegin(), result.cend());

    uint zControlBitPosition = findZControlBit(yControlBitPosition);
    result = transformZ(zControlBitPosition);

    conjugationElements.insert(conjugationElements.end(),
                               result.cbegin(), result.cend());

    result = transformW(yControlBitPosition, zControlBitPosition);
    conjugationElements.insert(conjugationElements.end(),
                               result.cbegin(), result.cend());

    deque<ReverseElement> coreImplementation =
            getCoreImplementation(yControlBitPosition, zControlBitPosition);

    deque<ReverseElement> implementation =
            conjugate(coreImplementation, conjugationElements);

    return implementation;
}

uint TransposPair::getPrecomputedComplexity() const
{
    assertd(n > 3, string("Can't get estimated complexity for n < 4"));

    uint complexity = 3 * (1 << (n - 4)) - 2;
    if(!isIndependent())
    {
        complexity *= 2;    // 2 times more complex
        complexity += 2;    // 2 CCNOT
    }

    return complexity;
}

uint TransposPair::findYControlBit()
{
#if defined(ADDITIONAL_MEMORY_TECHNIQUE)
    return 0;
#endif //ADDITIONAL_MEMORY_TECHNIQUE

    word x = first.getX();
    word y = first.getY();

    word z = second.getX();
    word w = second.getY();

    word dxy = x ^ y;
    word dxz = x ^ z;

    uint yControlBitPosition = uintUndefined;

    uint index = 0;
    word mask = 1;

    word maxNumber = x | y | z | w;
    uint minSum = uintUndefined;

    auto count = countNonZeroBits;

    //////debug
    ////mask = (word)1 << (n-1);
    ////yControlBitPosition = (n-1);
    ////while(mask)
    ////{
    ////    if(dxy & mask)
    ////    {
    ////        break;
    ////    }

    ////    mask >>= 1;
    ////    --yControlBitPosition;
    ////}

    //debug
    while(/*false*/true)
    {
        uint s = 0;
        if(dxy & mask)
        {
            // I.1)
            if((y & mask) == 0)
            {
                // 2 NOT for inversion of i-th bit
                s += 2;
            }

            // I.2)
            word delta = dxy ^ mask;

            // I.3)
            if(isIndependent())
            {
                word temp = dxz;
                if((y & mask) == (z & mask))
                {
                    temp ^= delta;
                }

                uint tempWeight = count(temp);
                assertd(tempWeight != 0, string("Error in algorithm"));

                s += tempWeight - 1;
            }

            // I.4)
            word temp = (x ^ mask) ^ w;
            if((y & mask) == (w & mask))
            {
                temp = y ^ w;
            }

            uint tempWeight = count(temp);
            assertd(tempWeight != 0, string("Error in algorithm"));

            s += tempWeight - 1;

            if(minSum == uintUndefined || s < minSum)
            {
                minSum = s;
                yControlBitPosition = index;
            }
        }

        // check next iteration
        ++index;
        mask <<= 1;

        if(mask > maxNumber)
        {
            break;
        }
    }

    assertd(yControlBitPosition != uintUndefined,
           string("Control bit in Y not found"));

    return yControlBitPosition;
}

deque<ReverseElement> TransposPair::transformY(uint yControlBitPosition)
{
    word x = first.getX();
    word y = first.getY();

    word z = second.getX();
    word w = second.getY();

    deque<ReverseElement> conjugationElements;
    word controlMask = 1 << yControlBitPosition;

    word delta = x ^ y ^ controlMask;
    if(delta)
    {
        // 1) change y
        first.setY(x ^ controlMask);

        // 2) change z
        if((z & controlMask) == (y & controlMask))
        {
            second.setX(z ^ delta);
        }

        // 3) change w
        if((w & controlMask) == (y & controlMask))
        {
            second.setY(w ^ delta);
        }

        // 4) check if inversion of i-th bit required
        bool needInversion = ((y & controlMask) == 0);

        // 5) reduce delta
        uint index = 0;
        while(delta)
        {
            if(delta & 1)
            {
                auto element = ReverseElement(n, 1 << index, controlMask);
                conjugationElements.push_back(element);
            }

            delta >>= 1;
            ++index;
        }

        // 6) conjugate by inversion
        if(needInversion)
        {
            auto element = ReverseElement(n, controlMask);
            conjugationElements.push_front(element);
            conjugationElements.push_back(element);
        }
    }

    return conjugationElements;
}

uint TransposPair::findZControlBit(uint yControlBitPosition)
{
#if defined(ADDITIONAL_MEMORY_TECHNIQUE)
    return 1;
#endif //ADDITIONAL_MEMORY_TECHNIQUE

    word x = first.getX();
    word y = first.getY();

    word z = second.getX();
    word w = second.getY();

    if(!isIndependent())
    {
        z = w;
    }

    word dxz = x ^ z;

    uint zControlBitPosition = uintUndefined;

    uint index = 0;
    word mask = 1;

    word maxNumber = x | y | z | w;
    uint minSum = uintUndefined;

    auto count = countNonZeroBits;

    //////debug
    ////mask = (word)1 << (n-1);
    ////zControlBitPosition = (n-1);
    ////while(mask)
    ////{
    ////    if((dxz & mask) && zControlBitPosition != yControlBitPosition )
    ////    {
    ////        break;
    ////    }

    ////    mask >>= 1;
    ////    --zControlBitPosition;
    ////}

    //debug
    while(/*false*/true)
    {
        uint s = 0;

        if((dxz & mask) != 0 && index != yControlBitPosition)
        {
            // II.1)
            if((z & mask) == 0)
            {
                // 2 NOT for inversion of j-th bit
                s += 2;
            }

            if(isIndependent())
            {
                // II.2)
                word delta = dxz ^ mask;

                // II.3)
                word temp = (y ^ mask) ^ w;
                if((z & mask) == (w & mask))
                {
                    temp ^= delta;
                }

                uint tempWeight = count(temp);
                if(tempWeight > 0)
                {
                    s += tempWeight - 1;
                }
            }

            if(minSum == uintUndefined || s < minSum)
            {
                minSum = s;
                zControlBitPosition = index;
            }
        }

        // check next iteration
        ++index;
        mask <<= 1;

        if(mask > maxNumber)
        {
            break;
        }
    }

    assertd(zControlBitPosition != uintUndefined,
           string("Control bit in Z not found"));

    return zControlBitPosition;
}

deque<ReverseElement> TransposPair::transformZ(uint zControlBitPosition)
{
    deque<ReverseElement> conjugationElements;
    if(isIndependent())
    {
        word x = first.getX();
        word y = first.getY();

        word z = second.getX();
        word w = second.getY();

        word controlMask = 1 << zControlBitPosition;
        word delta = x ^ z ^ controlMask;

        if(delta)
        {
            // 1) change z
            second.setX(x ^ controlMask);

            // 2) change w
            if((w & controlMask) == (z & controlMask))
            {
                second.setY(w ^ delta);
            }

            // 3) check if inversion of j-th bit required
            bool needInversion = ((z & controlMask) == 0);

            // 4) reduce delta
            uint index = 0;
            while(delta)
            {
                if(delta & 1)
                {
                    auto element = ReverseElement(n, 1 << index, controlMask);
                    conjugationElements.push_back(element);
                }

                delta >>= 1;
                ++index;
            }

            // 5) conjugate by inversion
            if(needInversion)
            {
                auto element = ReverseElement(n, controlMask);
                conjugationElements.push_front(element);
                conjugationElements.push_back(element);
            }
        }
    }

    return conjugationElements;
}

deque<ReverseElement> TransposPair::transformW(uint yControlBitPosition, uint zControlBitPosition)
{
    word x = first.getX();
    word y = first.getY();

    word z = second.getX();
    word w = second.getY();

    word dxw = x ^ w;

    deque<ReverseElement> conjugationElements;
    word yControlMask = 1 << yControlBitPosition;
    word zControlMask = 1 << zControlBitPosition;

    // change w
    word mask = yControlMask | zControlMask;
    second.setY(x ^ yControlMask ^ zControlMask);

    if(isIndependent())
    {
        // first check, if something wrong
        if((dxw & mask) != mask)
        {
            // there exist k-th bit in dxw equal to 1
            bool reverseKthBit = false;
            uint index = 0;
            uint kBitPosition = uintUndefined;
            word mask = 1;

            // find k-th bit to fix this problem
            while(mask <= dxw)
            {
                if((dxw & mask)
                        && index != yControlBitPosition
                        && (!isIndependent() || index != zControlBitPosition))
                {

                    bool needReverse = ((w & mask) == 0);
                    if(kBitPosition == uintUndefined
                            || (!needReverse && reverseKthBit))
                    {
                        kBitPosition = index;
                        reverseKthBit = needReverse;
                    }
                }

                mask <<= 1;
                ++index;
            }

            assertd(kBitPosition != uintUndefined,
                   string("Control bit k in W not found"));

            // fix problems
            list<ReverseElement> elements;
            word kControlMask = 1 << kBitPosition;

            if((dxw & yControlMask) == 0)
            {
                w ^= yControlMask;
                auto element = ReverseElement(n, yControlMask, kControlMask);
                elements.push_back(element);
            }

            if((dxw & zControlMask) == 0)
            {
                w ^= zControlMask;
                auto element = ReverseElement(n, zControlMask, kControlMask);
                elements.push_back(element);
            }

            if(reverseKthBit)
            {
                auto element = ReverseElement(n, kControlMask);
                elements.push_front(element);
                elements.push_back(element);
            }

            conjugationElements.insert(conjugationElements.end(),
                                       elements.cbegin(), elements.cend());
        }

        // generate rest reversible elements to implement w = (x ^ (1 << i) ^ (1 << j))
        word controlMask = yControlMask | zControlMask;
        word inversionMask = 0;

        if((w & yControlMask) == 0)
        {
            inversionMask |= yControlMask;
        }

        if((w & zControlMask) == 0)
        {
            inversionMask |= zControlMask;
        }

        word delta = x ^ w ^ controlMask;
        uint index = 0;

        while(delta)
        {
            if(delta & 1)
            {
                auto element = ReverseElement(n, 1 << index, controlMask, inversionMask);
                conjugationElements.push_back(element);
            }

            delta >>= 1;
            ++index;
        }
    }
    else
    {
        // not isIndependent
        word controlMask = yControlMask | zControlMask;
        word inversionMask = 0;

        if((w & zControlMask) == 0)
        {
            inversionMask |= zControlMask;
        }

        word delta = x ^ w ^ controlMask;
        uint index = 0;

        while(delta)
        {
            if(delta & 1)
            {
                auto element = ReverseElement(n, 1 << index, zControlMask, inversionMask);
                conjugationElements.push_back(element);
            }

            delta >>= 1;
            ++index;
        }
    }

    return conjugationElements;
}

deque<ReverseElement> TransposPair::getCoreImplementation(uint yControlBitPosition, uint zControlBitPosition)
{
    // find inputs, that should be inversed
    word controlMask   = 0;
    word inversionMask = 0;

    word x = first.getX();
    word mask = 1;

    for(uint index = 0; index < n; ++index)
    {
        if(index != yControlBitPosition
                && index != zControlBitPosition)
        {
            word currentMask = 1 << index;
            controlMask |= currentMask;

            if((x & mask) == 0)
            {
                inversionMask |= currentMask;
            }
        }

        mask <<= 1;
    }

    word yControlMask = 1 << yControlBitPosition;
    word zControlMask = 1 << zControlBitPosition;

    if(!isIndependent())
    {
        if(x & yControlMask)
        {
            inversionMask |= yControlMask;
        }

        if((x & zControlMask) == 0)
        {
            inversionMask |= zControlMask;
        }
    }

    auto coreElement = ReverseElement(n, yControlMask, controlMask, inversionMask);
    coreElement.setIndependencyFlag(isIndependent());

    deque<ReverseElement> elements;
    elements.push_back(coreElement);
    return elements;
}

}   // namespace ReversibleLogic

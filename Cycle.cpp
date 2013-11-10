#include "std.hpp"

namespace ReversibleLogic
{

Cycle::Cycle()
    : elements()
    , finalized(false)
    , distnancesSum(uintUndefined)
    , newDistancesSum(uintUndefined)
    , disjointTransp()
    , disjointIndex(0)
{
}

Cycle::Cycle(vector<word>&& source)
    : elements( move(source) )
    , finalized(false)
    , distnancesSum(uintUndefined)
    , newDistancesSum(uintUndefined)
    , disjointTransp()
    , disjointIndex(0)
{
    uint elementCount = length();
    if(elements[0] == elements[elementCount - 1])
    {
        elements.resize(elementCount - 1);
        finalize();
    }
}

void Cycle::finalize()
{
    finalized = true;
}

bool Cycle::isFinal() const
{
    return finalized;
}

void Cycle::append(word element)
{
    assert(!finalized, string("Failed to append element to finalized cycle"));

    auto pos = find(elements.cbegin(), elements.cend(), element);
    if(pos == elements.cend())
    {
        elements.push_back(element);
    }
    else if(pos == elements.cbegin())
    {
        finalize();
    }
    else
    {
        ostringstream stream;
        stream << "Failed to append " << element << " to cycle";
        assert(false, stream.str());
    }
}

uint Cycle::length() const
{
    uint elementCount = elements.size();
    return elementCount;
}

bool Cycle::isEmpty() const
{
    uint elementCount = elements.size();
    return (elementCount == 0);
}

const word& Cycle::operator[]( uint index ) const
{
    return elements[index];
}

Transposition Cycle::getTranspositionByPosition(uint index) const
{
    Transposition transp;
    
    uint elementCount = length();
    if(index < elementCount)
    {
        word x = elements[index];
        word y = 0;

        if(index == elementCount - 1)
        {
            y = elements[0];
        }
        else
        {
            y = elements[index + 1];
        }

        transp.setX(x);
        transp.setY(y);
    }

    return transp;
}

bool Cycle::operator ==(const Cycle& another) const
{
    bool result = (elements == another.elements);
    return result;
}

bool Cycle::has(const Transposition& target) const
{
    bool result = false;

    word x = target.getX();
    uint xPos = uintUndefined;

    word y = target.getY();
    uint yPos = uintUndefined;

    uint elementCount = length();
    for(uint index = 0; index < elementCount; ++index)
    {
        if(elements[index] == x)
        {
            xPos = index;
            if(yPos != uintUndefined)
            {
                break;
            }
        }

        if(elements[index] == y)
        {
            yPos = index;
            if(xPos != uintUndefined)
            {
                break;
            }
        }

        //Transposition transp = getTranspositionByPosition(index);
        //if(transp == target)
        //{
        //    result = true;
        //    break;
        //}
    }

    if(xPos != uintUndefined && yPos != uintUndefined)
    {
        if(yPos < xPos)
        {
            swap(xPos, yPos);
        }

        uint delta = yPos - xPos;
        if(delta == 1 || delta == elementCount - 1)
        {
            result = true;
        }
    }

    return result;
}

void Cycle::remove(const Transposition& target)
{
    assert(disjointTransp == target, string("Removing non-existent transposition from cycle"));

    uint elementCount = length();
    if(elementCount == 2)
    {
        elements.resize(0);
    }
    else
    {
        uint newElementCount = elementCount - 1;
        word* ptr = elements.data();

        if(disjointIndex < newElementCount)
        {
            // (..., x, y, w, ...) -> (..., x, w, ...)
            memcpy(ptr + disjointIndex + 1,
                   ptr + disjointIndex + 2,
                   (elementCount - disjointIndex - 2) * sizeof(word));
        }
        else
        {
            // (y, w, ..., x) -> (w, ..., x)
            memcpy(ptr, ptr + 1, newElementCount * sizeof(word));
        }

        elements.resize(newElementCount);
        distnancesSum = newDistancesSum;
    }
}

Cycle::operator string() const
{
    ostringstream result;
    result << "(";

    uint elementCount = length();
    for(uint index = 0; index < elementCount; ++index)
    {
        result << elements[index] << ", ";
    }

    result << ")";
    return result.str();
}

Transposition Cycle::getNextDisjointTransposition()
{
    uint elementCount = length();
    if(elementCount == 2) //simple transposition
    {
        disjointTransp.setX(elements[0]);
        disjointTransp.setY(elements[1]);
    }
    else //there are more than 2 elements
    {
        findBestDisjointIndex();
    }

    return disjointTransp;
}

void Cycle::findBestDisjointIndex()
{
    uint elementCount = length();
    assert(elementCount > 2, string("Disjoint transposition can't be found for 2 elements"));

    if(distnancesSum == uintUndefined)
    {
        //this is the first call
        calculateDistancesSum();
    }

    // find disjoint index for which transposition will get the minimum of distances sum
    disjointIndex = 0;
    uint minSum = distnancesSum;

    for(uint index = 0; index < elementCount - 2; ++index)
    {
        //(x, y, w, ...)
        uint sum = distnancesSum;
        sum += countNonZeroBits(elements[index] ^ elements[index + 2]);     //distance between x and w
        sum -= countNonZeroBits(elements[index + 1] ^ elements[index + 2]); //distance between y and w

        if(sum < minSum)
        {
            disjointIndex = index;
            newDistancesSum = sum;
        }
    }

    // case of (w, ..., x, y)
    uint sum = distnancesSum;
    sum += countNonZeroBits(elements[elementCount - 2] ^ elements[0]);  //distance between x and w
    sum -= countNonZeroBits(elements[elementCount - 1] ^ elements[0]);  //distance between y and w

    if(sum < minSum)
    {
        disjointIndex = elementCount - 2;
        newDistancesSum = sum;
    }

    // case of (y, w, ..., x)
    sum = distnancesSum;
    sum += countNonZeroBits(elements[elementCount - 1] ^ elements[1]);  //distance between x and w
    sum -= countNonZeroBits(elements[0] ^ elements[1]);                 //distance between y and w

    if(sum < minSum)
    {
        disjointIndex = elementCount - 1;
        newDistancesSum = sum;
    }

    // now make disjoint transposition
    disjointTransp.setX(elements[disjointIndex]);
    if(disjointIndex < elementCount - 1)
    {
        disjointTransp.setY(elements[disjointIndex + 1]);
    }
    else //disjointIndex == elementCount - 1
    {
        disjointTransp.setY(elements[0]);
    }
}

void Cycle::calculateDistancesSum()
{
    distnancesSum = 0;

    // we assume, that element count > 2
    uint elementCount = length();
    for(uint index = 0; index < elementCount - 1; ++index)
    {
        distnancesSum += countNonZeroBits(elements[index] ^ elements[index + 1]);
    }

    distnancesSum += countNonZeroBits(elements[0] ^ elements[elementCount - 1]);
    newDistancesSum = distnancesSum;
}

}   // namespace ReversibleLogic


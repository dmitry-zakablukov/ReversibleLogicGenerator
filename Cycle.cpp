#include "std.hpp"

namespace ReversibleLogic
{

Cycle::Cycle()
    : elements()
    , finalized(false)
{
}

Cycle::Cycle(vector<word>&& source)
    : elements( move(source) )
    , finalized(false)
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

void Cycle::remove(const Transposition& target, Transposition* first /* = 0 */,
    Transposition* second /* = 0 */)
{
    uint elementCount = length();
    for(uint index = 0; index < elementCount; ++index)
    {
        Transposition transp = getTranspositionByPosition(index);
        if(transp == target)
        {
            if(elementCount == 2)
            {
                elements.resize(0);
            }
            else
            {
                uint newElementCount = elementCount - 1;
                word* ptr = elements.data();

                if(index < newElementCount)
                {
                    memcpy(ptr + index + 1, ptr + index + 2,
                        (elementCount - index - 2) * sizeof(word));
                }
                else
                {
                    memcpy(ptr, ptr + 1, newElementCount * sizeof(word));
                }

                elements.resize(newElementCount);
            }

            break;
        }
    }

    word x = target.getX();
    word y = target.getY();

    Transposition newTransp;
    Transposition oldTransp;

    elementCount = length();
    for(uint index = 0; index < elementCount; ++index)
    {
        Transposition transp = getTranspositionByPosition(index);
        word newX = transp.getX();

        if(newX == x)
        {
            newTransp = transp;
            oldTransp = Transposition(y, transp.getY());
            break;
        }
        else if(newX == y)
        {
            newTransp = transp;
            oldTransp = Transposition(x, transp.getY());
            break;
        }
        else
        {
            uint elementCount = length();
            if(elementCount == 2)
            {
                word newY = transp.getY();
                if(newY == x)
                {
                    newTransp = transp;
                    oldTransp = Transposition(y, transp.getX());
                    break;
                }
                else if(newY == y)
                {
                    newTransp = transp;
                    oldTransp = Transposition(x, transp.getX());
                    break;
                }
            }
        }
    }

    if(first)
    {
        *first = newTransp;
    }

    if(second)
    {
        *second = oldTransp;
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

}   // namespace ReversibleLogic


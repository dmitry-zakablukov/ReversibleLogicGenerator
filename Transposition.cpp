#include "std.hpp"

namespace ReversibleLogic
{

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
        sort();
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
        swap();
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

    bool isEqual = ((x == z && y == w) || (x == w && y == z));
    return isEqual;
}

bool Transposition::operator !=(const Transposition& another) const
{
    return !operator ==(another);
}

ostream& operator <<(ostream& out, const Transposition& t)
{
    out << "(" << t.x << ", " << t.y << ")";
    return out;
}

word Transposition::getOutput(word input) const
{
    word output = input;
    if(output == x)
        output = y;
    else if (output == y)
        output = x;

    return output;
}

}   // namespace ReversibleLogic

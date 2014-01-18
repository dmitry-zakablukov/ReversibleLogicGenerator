#pragma once

namespace ReversibleLogic
{

class Transposition
{
public:
    Transposition();
    explicit Transposition(word left, word right, bool needSort = false);
    Transposition(const Transposition& another);

    bool isEmpty() const;

    // sort x and y by they weights
    void sort();

    // swap x and y
    void swap();

    void setX(word value);
    word getX() const;

    void setY(word value);
    word getY() const;

    // Returns difference between x and y
    word getDiff() const;

    // Returns distance between x and y
    uint getDist() const;

    // Returns true if transposition has specified value
    bool has(word value) const;

    // Returns new value for input if it is in transposition
    // i.e. input == x, then output = y and vice versa
    word getOutput(word input) const;

    bool operator ==(const Transposition& another) const;
    bool operator !=(const Transposition& another) const;

    operator string() const;

private:
    word x;
    word y;

    bool empty;
};

class TransposPair
{
public:
    TransposPair();
    explicit TransposPair(const Transposition& theFirst,
                          const Transposition& theSecond);

    void setN(uint value);

    bool isIndependent() const;

    operator string() const;

    uint getEstimateImplComplexity() const;

    deque<ReverseElement> getImplementation();

private:
    uint getPrecomputedComplexity() const;

    uint findYControlBit();
    deque<ReverseElement> transformY(uint yControlBitPosition);

    uint findZControlBit(uint yControlBitPosition);
    deque<ReverseElement> transformZ(uint zControlBitPosition);

    deque<ReverseElement> transformW(uint yControlBitPosition,
                                     uint zControlBitPosition);

    deque<ReverseElement>
    getCoreImplementation(uint yControlBitPosition,
                          uint zControlBitPosition);

    uint n;
    Transposition first;
    Transposition second;

    bool independencyFlag;
};

}   // namespace ReversibleLogic

#pragma once

namespace ReversibleLogic
{

class Transposition
{
public:
    Transposition();
    explicit Transposition(word left, word right, bool needSort = false);
    Transposition(const Transposition& another);

    virtual ~Transposition() = default;

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

    friend ostream& operator <<(ostream& out, const Transposition& t);

private:
    word x;
    word y;

    bool empty;
};

}   // namespace ReversibleLogic

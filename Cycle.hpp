#pragma once

namespace ReversibleLogic
{

class Cycle
{
public:
    Cycle();
    Cycle(vector<word>&& source);

    //Cycle& operator=(const Cycle& another) = default;

    // Finalize cycle: none element could be added to cycle
    void finalize();

    // Return true if cycle is finalized
    bool isFinal() const;

    // Add new element to cycle (before finalization)
    void append(word element);

    uint length() const;
    bool isEmpty() const;

    const word& operator[](uint index) const;
    Transposition getTranspositionByPosition(uint index) const;

    bool operator ==(const Cycle& another) const;
    bool has(const Transposition& target) const;

    void remove(const Transposition& target,
        Transposition* first = 0, Transposition* second = 0);

    operator string() const;

private:
    vector<word> elements;
    bool finalized;
};

}   // namespace ReversibleLogic

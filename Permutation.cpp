#include "std.hpp"

namespace ReversibleLogic
{

Permutation::Permutation()
    : cycles()
{
}

Permutation::Permutation(vector<shared_ptr<Cycle>> theCycles)
    : cycles(theCycles)
{
}

void Permutation::append(shared_ptr<Cycle> cycle)
{
    cycles.push_back(cycle);
}

//void Permutation::remove(shared_ptr<Cycle> cycle)
//{
//    cycles.remove(cycle);
//}
//
uint Permutation::length() const
{
    return cycles.size();
}

shared_ptr<Cycle> Permutation::getCycle(uint index) const
{
    auto iter = cycles.begin();
    advance(iter, index);

    return *iter;
}

uint Permutation::getElementCount() const
{
    uint count = 0;
    for (auto cycle : cycles)
        count += cycle->length();

    return count;
}

uint Permutation::getTranspositionsCount() const
{
    uint count = 0;
    for (auto cycle : cycles)
        count += cycle->length() - 1;

    return count;
}

bool Permutation::isEmpty() const
{
    //uint cycleCount = cycles.size();
    //return (cycleCount == 0);

    bool empty = true;
    for (auto cycle : cycles)
    {
        if(cycle->length())
        {
            empty = false;
            break;
        }
    }

    return empty;
}

bool Permutation::isEven() const
{
    uint length = 0;
    for (auto cycle : cycles)
    {
        length += cycle->length() - 1;
    }

    return ((length & 1) == 0);
}

void Permutation::completeToEven()
{
    list<word> elements;
    word maxValue = 0;

    shared_ptr<Cycle> incompleteCycle;
    bool incompleteFound = false;
    for (auto cycle : cycles)
    {
        uint elementCount = cycle->length();
        for(uint index = 0; index < elementCount; ++index)
        {
            const word& element = (*cycle)[index];
            elements.push_back(element);
            maxValue |= element;
        }

        if(!cycle->isFinal() && !incompleteFound)
        {
            incompleteCycle = cycle;
            incompleteFound = true;
        }
    }

    word first;
    bool firstFound = false;

    word second;
    bool secondFound = false;

    for(word index = 0; index < maxValue + 3; ++index)
    {
        if(!found(elements, index))
        {
            if(!firstFound)
            {
                first = index;
                firstFound = true;
            }
            else if(!secondFound)
            {
                second = index;
                break;
            }
        }
    }

    if(!incompleteFound)
    {
        vector<word> cycleElements(2);
        cycleElements[0] = first;
        cycleElements[1] = second;

        append( shared_ptr<Cycle>(new Cycle(move(cycleElements))) );
    }
    else
    {
        incompleteCycle->append(first);
    }
}

void Permutation::completeToEven(word truthTableSize)
{
    assertd(countNonZeroBits(truthTableSize) == 1,
        string("Table size is not power of 2"));

    Cycle& firstCycle = **begin();
    word diff = firstCycle[0] ^ firstCycle[1];

    shared_ptr<Cycle> lastCycle(new Cycle());
    lastCycle->append(truthTableSize);
    lastCycle->append(truthTableSize ^ diff);

    append(lastCycle);
}

ostream& operator <<(ostream& out, const Permutation& permutation)
{
    out << "[ ";

    for (auto cycle : permutation)
    {
        out << *cycle << ", ";
    }

    out << " ]";
    return out;
}

vector<shared_ptr<Cycle>>::const_iterator Permutation::begin() const
{
    return cycles.cbegin();
}

vector<shared_ptr<Cycle>>::const_iterator Permutation::end() const
{
    return cycles.cend();
}

Permutation Permutation::multiplyByTranspositions(
    shared_ptr<list<Transposition>> transpositions, bool isLeftMultiplication) const
{
    return multiplyByTranspositions(*transpositions, isLeftMultiplication);
}

ReversibleLogic::Permutation Permutation::multiplyByTranspositions(const list<Transposition>& transpositions,
    bool isLeftMultiplication) const
{
    vector<shared_ptr<Cycle>> newCycles;

    // remember all elements in transpositions and in this permutation
    unordered_set<word> storage;
    for (const auto& transp : transpositions)
    {
        word x = transp.getX();
        word y = transp.getY();

        if (storage.find(x) == storage.cend())
            storage.insert(x);

        if (storage.find(y) == storage.cend())
            storage.insert(y);
    }

    for (auto cycle : *this)
    {
        uint elementCount = cycle->length();
        for (uint index = 0; index < elementCount; ++index)
        {
            word element = (*cycle)[index];
            if (storage.find(element) == storage.cend())
                storage.insert(element);
        }
    }

    // now multiply
    unordered_set<word> visitedElements;
    unordered_set<word>::const_iterator end = visitedElements.cend();
    shared_ptr<Cycle> nextCycle(new Cycle());

    for (word x : storage)
    {
        if (visitedElements.find(x) == end)
        {
            while (!nextCycle->isFinal())
            {
                word y = x;
                if (isLeftMultiplication)
                {
                    // multiply in reverse order for common pair case
                    // for other cases this reverse order won't affect on result

                    //for (auto& transp : *transpositions)
                    //    y = transp.getOutput(y);
                    for (auto iter = transpositions.crbegin(); iter != transpositions.crend(); ++iter)
                        y = iter->getOutput(y);

                    for (auto cycle : *this)
                        y = cycle->getOutput(y);
                }
                else
                {
                    for (auto cycle : *this)
                        y = cycle->getOutput(y);

                    // multiply in reverse order for common pair case
                    // for other cases this reverse order won't affect on result

                    //for (auto& transp : *transpositions)
                    //    y = transp.getOutput(y);
                    for (auto iter = transpositions.crbegin(); iter != transpositions.crend(); ++iter)
                        y = iter->getOutput(y);
                }

                if (nextCycle->isEmpty())
                    nextCycle->append(x);

                nextCycle->append(y);

                visitedElements.insert(x);
                x = y;
            }

            // skip fixed point
            uint cycleLength = nextCycle->length();
            if (cycleLength > 1)
            {
                newCycles.push_back(nextCycle);
            }

            nextCycle = shared_ptr<Cycle>(new Cycle());
        }
    }

    assertd(!nextCycle->length(),
        string("Permutation::multiplyByTranspositions() failed because of last cycle"));

    Permutation result(newCycles);
    return result;
}

uint Permutation::getDistancesSum() const
{
    uint sum = 0;
    for (auto cycle : *this)
    {
        sum += cycle->getDistancesSum();
    }

    return sum;
}

ReversibleLogic::Permutation Permutation::clone() const
{
    vector<shared_ptr<Cycle>> cyclesCopy;
    cyclesCopy.reserve(cycles.size());

    for (auto cycle : cycles)
    {
        shared_ptr<Cycle> cycleCopy(new Cycle());
        *cycleCopy = *cycle;

        cyclesCopy.push_back(cycleCopy);
    }

    Permutation result(cyclesCopy);
    return result;
}

}   // namespace ReversibleLogic

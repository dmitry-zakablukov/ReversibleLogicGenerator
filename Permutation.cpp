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

bool Permutation::isEmpty() const
{
    //uint cycleCount = cycles.size();
    //return (cycleCount == 0);

    bool empty = true;
    forcin(iter, cycles)
    {
        auto cycle = *iter;
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
    forin(iter, cycles)
    {
        const shared_ptr<Cycle>& cycle = *iter;
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
    forin(cycleIter, cycles)
    {
        shared_ptr<Cycle>& cycle = *cycleIter;

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

Permutation::operator string()
{
    ostringstream result;
    result << "[ ";

    forin(iter, *this)
    {
        const shared_ptr<Cycle>& cycle = *iter;
        result << (string)*cycle << ", ";
    }

    result << " ]";
    return result.str();
}

vector<shared_ptr<Cycle>>::const_iterator Permutation::begin() const
{
    return cycles.cbegin();
}

vector<shared_ptr<Cycle>>::const_iterator Permutation::end() const
{
    return cycles.cend();
}

}   // namespace ReversibleLogic

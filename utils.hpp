#pragma once

typedef vector<word> TruthTable;

class AssertionError: public exception
{
public:
    AssertionError();
    AssertionError(string&& message);

    virtual const char* what() const;

private:
    string message;
};

// debug assert
#if defined(DEBUG) || defined(_DEBUG)
    #define assertd(condition, message)  if(!(condition)) throw AssertionError( move((message)) );
#else
    #define assertd(condition, message)
#endif  // DEBUG

// assert
#define assert(condition, message)  if(!(condition)) throw AssertionError( move((message)) );

#define forin(name, collection) for(auto (name) = (collection).begin(); (name) != (collection).end(); ++(name))
#define forcin(name, collection) for(auto (name) = (collection).cbegin(); (name) != (collection).cend(); ++(name))

// reversed
#define forrin(name, collection) for(auto (name) = (collection).rbegin(); (name) != (collection).rend(); ++(name))
#define forrcin(name, collection) for(auto (name) = (collection).crbegin(); (name) != (collection).crend(); ++(name))

uint countNonZeroBits(word value);
uint findPositiveBitPosition(word value, uint startPos = 0);
uint getSignificantBitCount(word value);

template< typename T >
deque<T> conjugate(deque<T> target, deque<T> conjugations, bool withReverse = false)
{
    deque<T> implementation;

    if(withReverse)
    {
        implementation.insert(implementation.end(), conjugations.crbegin(),
                              conjugations.crend());

        implementation.insert(implementation.end(), target.cbegin(),
                              target.cend());

        implementation.insert(implementation.end(), conjugations.cbegin(),
                              conjugations.cend());
    }
    else
    {
        implementation.insert(implementation.end(), conjugations.cbegin(),
                              conjugations.cend());

        implementation.insert(implementation.end(), target.cbegin(),
                              target.cend());

        implementation.insert(implementation.end(), conjugations.crbegin(),
                              conjugations.crend());
    }

    return move(implementation);
}

template<typename T>
bool found(const list<T>& container, T element)
{
    auto iter = find(container.cbegin(), container.cend(), element);
    bool result = (iter != container.cend());

    return result;
}

template<typename T>
bool found(const deque<T>& container, T element)
{
    auto iter = find(container.cbegin(), container.cend(), element);
    bool result = (iter != container.cend());

    return result;
}

template<typename T>
inline void bufferize(vector<T>& container, uint bufferSize = 1024)
{
    uint size = container.capacity();
    if(!(size & ~(bufferSize - 1)))
    {
        container.reserve(size + bufferSize);
    }
}

template<typename T>
inline void toVector(const list<T>& container, vector<T>* output)
{
    output->reserve(container.size());
    for (auto& element : container)
    {
        output->push_back(element);
    }
}

string trim(const string& value);
string removeQuotes(const string& value);

void debugLog(const string& context, function<void(ostream&)> logFunction);
void debugBehavior(const string& context, function<void()> debugFunction);

#pragma once

class AssertionError: public exception
{
public:
    AssertionError();
    AssertionError(string&& message);

    virtual const char* what() const;

private:
    string message;
};

#if defined(DEBUG) || defined(_DEBUG)
    #define assert(condition, message)  if(!(condition)) throw AssertionError( move((message)) );
#else
    #define assert(condition, message)
#endif  // DEBUG

#define forin(name, collection) for(auto (name) = (collection).begin(); (name) != (collection).end(); ++(name))
#define forcin(name, collection) for(auto (name) = (collection).cbegin(); (name) != (collection).cend(); ++(name))

uint countNonZeroBits(word value);
uint findPositiveBitPosition(word value, uint startPos = 0);

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
inline void bufferize(vector<T>& container)
{
    const uint numBufferSize = 1024;

    uint size = container.capacity();
    if(!(size & ~(numBufferSize - 1)))
    {
        container.reserve(size + numBufferSize);
    }
}

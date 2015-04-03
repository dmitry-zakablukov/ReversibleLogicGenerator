#include "std.hpp"

void Timer::start()
{
    startTime = clock();
}

void Timer::stop()
{
    endTime = clock();
}

float Timer::getElapsedMs() const
{
    float elapsedTime = 0;
    if( endTime > startTime )
        elapsedTime = 1000.0f * (endTime - startTime) / CLOCKS_PER_SEC;

    return elapsedTime;
}

AutoTimer::AutoTimer( float* elapsedTime )
    : timer()
    , elapsedTime( elapsedTime )
{
    assertd( elapsedTime != 0, string("Elapsed time pointer is null") );
    timer.start();
}

AutoTimer::~AutoTimer()
{
    timer.stop();
    if( elapsedTime )
        *elapsedTime = timer.getElapsedMs();
}

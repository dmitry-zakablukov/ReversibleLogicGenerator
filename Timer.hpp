#pragma once

class Timer
{
public:
    Timer() = default;
    virtual ~Timer() = default;

    void start();
    void stop();

    float getElapsedMs() const;

private:
    clock_t startTime = 0;
    clock_t endTime = 0;
};

class AutoTimer
{
public:
    explicit AutoTimer( float* elapsedTime );
    virtual ~AutoTimer();

private:
    Timer  timer;
    float* elapsedTime;
};
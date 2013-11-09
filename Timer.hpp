#pragma once

class Timer
{
public:
    Timer();
    ~Timer();

    void start();
    void stop();

    float getElapsedMs() const;

private:
    clock_t startTime;
    clock_t endTime;
};

class AutoTimer
{
public:
    explicit AutoTimer( float* elapsedTime );
    ~AutoTimer();

private:
    Timer  timer;
    float* elapsedTime;
};
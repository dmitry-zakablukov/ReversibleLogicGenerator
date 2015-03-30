#pragma once

class ProgramOptions
{
public:
    ProgramOptions& get();

    void init(const Values& values);

private:
    ProgramOptions();

    static ProgramOptions* instance;
};
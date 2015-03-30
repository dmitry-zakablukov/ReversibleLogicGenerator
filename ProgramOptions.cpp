#include "std.hpp"

//static
ProgramOptions* ProgramOptions::instance = 0;

ProgramOptions& ProgramOptions::get()
{
    if (!instance)
        instance = new ProgramOptions(); //bugbug: no delete

    return *instance;
}

void ProgramOptions::init(const Values& values)
{

}

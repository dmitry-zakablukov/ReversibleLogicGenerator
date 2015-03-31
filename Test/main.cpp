#include "std.hpp"

int main(int argc, const char* argv[])
{
    Values values;
    if (argc == 2)
    {
        ifstream ini(argv[1]);
        values = IniParser::parse(ini);
    }

    ProgramOptions::init(values);

    generalSynthesis();
    //discreteLogSynthesis(argc, argv);
    //testOptimization(argc, argv);

    //system("pause");

    ProgramOptions::uninit();
    return 0;
}

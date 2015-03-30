#include "std.hpp"

int main(int argc, const char* argv[])
{
    if (argc == 2)
    {
        ifstream ini(argv[1]);
        Values values = IniParser::parse(ini);
    }

    generalSynthesis(argc, argv);
    //discreteLogSynthesis(argc, argv);
    //testOptimization(argc, argv);

    //system("pause");

    return 0;
}

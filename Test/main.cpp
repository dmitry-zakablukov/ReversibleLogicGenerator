#include "std.hpp"

void printUsage(const string& fullProgramPath)
{
    string shortName = fullProgramPath;

    auto pos = fullProgramPath.rfind('\\');
    if (pos != string::npos)
        shortName = shortName.substr(pos + 1);

    cout << 
        "Usage: " << shortName << " [config_file_name]\n"
        "    config_file_name - configuration file containing options (see config.ini);\n"
        "                       if not specified, default options would be used"
        << endl;
}

int main(int argc, const char* argv[])
{
    if (argc == 2)
    {
        string second = argv[1];
        if (second == "/?" || second == "-?" || second == "-h" || second == "--help")
        {
            printUsage(argv[0]);
            return 0;
        }
    }

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

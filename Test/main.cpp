#include "std.hpp"

void printUsage(const string& fullProgramPath)
{
    cout << 
        "Usage: " << getFileName(fullProgramPath) << " [config_file_name]\n"
        "    config_file_name - configuration file containing options (see config.ini);\n"
        "                       if not specified, default options would be used"
        << endl;
}

int main(int argc, const char* argv[])
{
    const char* strWorkModeKey = "work-mode";
    const char* strGeneralSynthesisMode = "general-synthesis";
    const char* strDiscreteLogSynthesisMode = "discrete-log-synthesis";
    const char* strPostProcessingMode = "post-processing";

    if (argc == 2)
    {
        string second = argv[1];
        if (second == "/?" || second == "-?" || second == "-h" || second == "--help")
        {
            printUsage(argv[0]);
            return 0;
        }
    }

    int returnCode = 0;
    try
    {
        Values values;
        if (argc == 2)
        {
            ifstream ini(argv[1]);
            values = IniParser::parse(ini);
        }

        ProgramOptions::init(values);
        const Values& options = ProgramOptions::get().options;

        string workMode = options.getString(strWorkModeKey);
        if (workMode == strGeneralSynthesisMode)
            generalSynthesis();
        else if (workMode == strDiscreteLogSynthesisMode)
            discreteLogSynthesis();
        else if (workMode == strPostProcessingMode)
            testOptimization(argc, argv);
        else
        {
            if (workMode.empty())
                cerr << "Error: work mode is not specified, valid values are:\n";
            else
                cerr << "Error: unknown work mode \"" << workMode << "\", valid values are:\n";

            cerr <<
                "    " << strGeneralSynthesisMode << '\n' <<
                "    " << strDiscreteLogSynthesisMode << '\n' <<
                "    " << strPostProcessingMode << endl;
        }

        ProgramOptions::uninit();
    }
    catch (exception& ex)
    {
        cerr << "Exception: " << ex.what() << endl;
        returnCode = -1;
    }
    catch (...)
    {
        cerr << "Unknown exception was caught" << endl;
        returnCode = -1;
    }

    system("pause");

    return returnCode;
}

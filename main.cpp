// ReversibleLogicGenerator - generator of reversible logic circuits, based on permutation group theory.
// Copyright (C) 2015  <Dmitry Zakablukov>
// E-mail: dmitriy.zakablukov@gmail.com
// Web: https://github.com/dmitry-zakablukov/ReversibleLogicGenerator
// 
// This file is part of ReversibleLogicGenerator.
// 
// ReversibleLogicGenerator is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// ReversibleLogicGenerator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with ReversibleLogicGenerator.  If not, see <http://www.gnu.org/licenses/>.

#include "std.h"

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
            testOptimization();
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

    return returnCode;
}

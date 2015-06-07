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

using namespace ReversibleLogic;

bool validateOptimizedScheme(const Scheme& before, const Scheme& after)
{
    uint n = 0;
    if (before.size())
        n = before.front().getInputCount();
    else if (after.size())
        n = after.front().getInputCount();

    word total = 1 << n;
    for (word x = 0; x < total; ++x)
    {
        word y = x;
        for (auto& element : before)
            y = element.getValue(y);

        word z = x;
        for (auto& element : after)
            z = element.getValue(z);

        if (y != z)
            return false;
    }

    return true;
}

void testOptimization()
{
    using namespace ReversibleLogic;

    const char* strTfcInput = "tfc-input";

    const ProgramOptions& options = ProgramOptions::get();

    // open results output
    ofstream outputFile(options.resultsFile);
    assert(outputFile.is_open(),
        string("Failed to open output file \"") + options.resultsFile + "\" for writing");

    // check schemes folder existence, create if not exist
    string schemesFolder = options.schemesFolder;
    if (_access(schemesFolder.c_str(), 0))
        _mkdir(schemesFolder.c_str());

    // process all tfc input files
    if (options.options.has(strTfcInput))
    {
        auto tfcInputFiles = options.options[strTfcInput];
        for (auto& tfcInputFileName : tfcInputFiles)
        {
            try
            {
                ifstream inputFile(tfcInputFileName);
                assert(inputFile.is_open(),
                    string("Failed to open input file \"") + tfcInputFileName + "\" for reading");

                outputFile << "Original scheme file: " << tfcInputFileName << endl;

                TfcFormatter formatter;
                Scheme scheme = formatter.parse(inputFile);

                uint elementCount = scheme.size();
                outputFile << "Complexity before optimization: " << scheme.size() << endl;

                PostProcessor optimizer;
                Scheme optimizedScheme = optimizer.optimize(scheme);
                assert(validateOptimizedScheme(scheme, optimizedScheme),
                    string("Optimized scheme is not valid"));

                outputFile << "Complexity after optimization: " << optimizedScheme.size() << endl;

                string tfcOutputFileName = appendPath(schemesFolder,
                    getFileName(tfcInputFileName) + "-opt.tfc");

                ofstream tfcOutput(tfcOutputFileName);
                assert(tfcOutput.is_open(),
                    string("Failed to open tfc file \"") + tfcOutputFileName + "\" for writing");

                formatter.format(tfcOutput, optimizedScheme);
                outputFile << "Optimized scheme file: " << tfcOutputFileName << endl;

                outputFile << "\n===============================================================\n";
                outputFile.flush();
            }
            catch (exception& ex)
            {
                outputFile << ex.what() << endl;
                outputFile << "\n===============================================================" << endl;
            }
        }
    }

    outputFile.close();
}

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

Scheme removeNegativeLines(const Scheme& scheme)
{
    Scheme result;
    for (auto& element : scheme)
    {
        if (element.getInversionMask() != 0)
        {
            deque<ReverseElement> replacement = element.getSimpleImplementation();
            for (auto& e : replacement)
                result.push_back(e);
        }
        else
            result.push_back(element);
    }

    return result;
}

void dumpSchemeInTfcFormat(const Scheme& scheme, const TfcFormatter& formatter,
    const string& fileName)
{
    TfcFormatter formatterCopy = formatter;

    ofstream tfcOutput(fileName);
    assert(tfcOutput.is_open(),
        string("Failed to open tfc file \"") + fileName + "\" for writing");

    formatterCopy.format(tfcOutput, scheme);
    tfcOutput.close();
}

void dumpSchemeInRealFormat(const Scheme& scheme, const string& fileName)
{
    ofstream realOutput(fileName);
    assert(realOutput.is_open(),
        string("Failed to open real file \"") + fileName + "\" for writing");

    uint n = 0;
    uint index = 0;

    if (scheme.size())
        n = scheme.front().getInputCount();

    // header
    realOutput << ".version 1.0" << endl;
    realOutput << ".numvars " << n << endl;

    realOutput << ".variables";
    index = 0;
    while (index++ < n)
        realOutput << " a" << index;
    realOutput << endl;

    realOutput << ".inputs";
    index = 0;
    while (index++ < n)
        realOutput << " a" << index;
    realOutput << endl;

    realOutput << ".outputs";
    index = 0;
    while (index++ < n)
        realOutput << " b" << index;
    realOutput << endl;
    
    realOutput << ".constants ";
    index = 0;
    while (index++ < n)
        realOutput << '-';
    realOutput << endl;

    realOutput << ".garbage ";
    index = 0;
    while (index++ < n)
        realOutput << '-';
    realOutput << endl;

    realOutput << ".begin" << endl;

    if (scheme.size())
    {
        realOutput << endl;

        for (auto& element : scheme)
        {
            word targetMask = element.getTargetMask();
            word controlMask = element.getControlMask();
            word inversionMask = element.getInversionMask();

            uint count = countNonZeroBits(controlMask) + 1;

            ostringstream line;
            line << 't' << count << ' ';

            word mask = 1;
            index = 1;
            while (mask <= controlMask)
            {
                if (controlMask & mask)
                {
                    if (inversionMask & mask)
                        line << '-';

                    line << 'a' << index << ' ';
                }

                mask <<= 1;
                ++index;
            }

            mask = 1;
            index = 1;
            while (mask != targetMask)
            {
                mask <<= 1;
                ++index;
            }

            line << 'a' << index;

            realOutput << line.str() << endl;
        }
    }

    realOutput << "\n.end\n" << endl;
    realOutput.close();
}

void processTfcFiles(const string& schemesFolder)
{
    const char* strTfcInput = "tfc-input";

    const ProgramOptions& options = ProgramOptions::get();
    if (options.options.has(strTfcInput))
    {
        auto tfcInputFiles = options.options[strTfcInput];
        for (auto& tfcInputFileName : tfcInputFiles)
        {
            try
            {
                TfcFormatter formatter;
                ifstream inputFile(tfcInputFileName);
                assert(inputFile.is_open(),
                    string("Failed to open input file \"") + tfcInputFileName + "\" for reading");

                Scheme originalScheme = formatter.parse(inputFile);
                Scheme schemeWithoutNegativeLines = removeNegativeLines(originalScheme);

                // TFC, with negative
                {
                    string fileName = appendPath(schemesFolder,
                        getFileName(tfcInputFileName) + "-with.tfc");
                    dumpSchemeInTfcFormat(originalScheme, formatter, fileName);
                }

                // TFC, without negative
                {
                    string fileName = appendPath(schemesFolder,
                        getFileName(tfcInputFileName) + "-without.tfc");
                    dumpSchemeInTfcFormat(schemeWithoutNegativeLines, formatter, fileName);
                }

                // Real, with negative
                {
                    string fileName = appendPath(schemesFolder,
                        getFileName(tfcInputFileName) + "-with.real");
                    dumpSchemeInRealFormat(originalScheme, fileName);
                }

                // Real, without negative
                {
                    string fileName = appendPath(schemesFolder,
                        getFileName(tfcInputFileName) + "-without.real");
                    dumpSchemeInRealFormat(schemeWithoutNegativeLines, fileName);
                }
            }
            catch (exception& ex)
            {
                cerr << ex.what() << endl;
            }
        }
    }
}

void removeNegativeLines()
{
    const ProgramOptions& options = ProgramOptions::get();

    // check schemes folder existence, create if not exist
    string schemesFolder = options.schemesFolder;
    if (_access(schemesFolder.c_str(), 0))
        _mkdir(schemesFolder.c_str());

    processTfcFiles(schemesFolder);
}

#include "std.hpp"

TruthTable getHwb(uint n)
{
    word count = 1 << n;
    word mask = count - 1;
    
    TruthTable table;
    table.resize(count);

    for (word x = 0; x < count; ++x)
    {
        uint weight = countNonZeroBits(x);

        word y = ((x << weight) & mask) |
            ((x >> (n - weight)) & mask);

        table[x] = y;
    }

    return table;
}

void synthesizeScheme(const TruthTable& table, ostream& resultsOutput, const string& tfcOutputFileName,
    ReversibleLogic::TfcFormatter& formatter = ReversibleLogic::TfcFormatter())
{
    using namespace ReversibleLogic;

    try
    {
        Generator generator;
        auto scheme = generator.generate(table, resultsOutput);

        ofstream tfcOutput(tfcOutputFileName);
        resultsOutput << "Scheme file: " << tfcOutputFileName << endl;

        formatter.format(tfcOutput, scheme);
        tfcOutput.close();
    }
    catch (exception& ex)
    {
        resultsOutput << ex.what() << endl;
    }
    catch (...)
    {
        resultsOutput << "Unknown exception" << endl;
    }

    resultsOutput << "\n===============================================================" << endl;
}

void generalSynthesis()
{
    using namespace ReversibleLogic;

    const char* strTfcInput = "tfc-input";
    const char* strTruthTableInput = "truth-table-input";

    const ProgramOptions& options = ProgramOptions::get();

    // open results output
    string resultsFileName = options.resultsFile;
    assert(!resultsFileName.empty(), string("Results file name is empty"));

    ofstream resultsOutput(resultsFileName);

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
                TfcFormatter formatter;
                ifstream inputFile(tfcInputFileName);

                Scheme scheme = formatter.parse(inputFile);
                TruthTable table = makePermutationFromScheme(scheme, formatter.getVariablesCount());

                string tfcOutputFileName = appendPath(schemesFolder,
                    getFileName(tfcInputFileName) + "-out.tfc");

                synthesizeScheme(table, resultsOutput, tfcOutputFileName, formatter);
            }
            catch (exception& ex)
            {
                resultsOutput << ex.what() << endl;
                resultsOutput << "\n===============================================================" << endl;
            }
        }
    }

    resultsOutput.close();
}

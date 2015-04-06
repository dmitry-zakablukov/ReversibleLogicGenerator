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
        assert(tfcOutput.is_open(),
            string("Failed to open tfc file \"") + tfcOutputFileName + "\" for writing");

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
    ofstream resultsOutput(options.resultsFile);
    assert(resultsOutput.is_open(),
        string("Failed to open results file \"") + options.resultsFile + "\" for writing");

    // check schemes folder existence, create if not exist
    string schemesFolder = options.schemesFolder;
    if (_access(schemesFolder.c_str(), 0))
        _mkdir(schemesFolder.c_str());

    // process all truth table input files
    if (options.options.has(strTruthTableInput))
    {
        auto truthTableInputFiles = options.options[strTruthTableInput];
        for (auto& truthTableInputFileName : truthTableInputFiles)
        {
            try
            {
                ifstream inputFile(truthTableInputFileName);
                assert(inputFile.is_open(),
                    string("Failed to open input file \"") + truthTableInputFileName + "\" for reading");

                TruthTableParser parser;
                TruthTable table = parser.parse(inputFile);

                table = TruthTableUtils::optimizeHammingDistance(table,
                    parser.getInputCount(), parser.getOutputCount());

                string tfcOutputFileName = appendPath(schemesFolder,
                    getFileName(truthTableInputFileName) + "-out.tfc");

                // todo: pass new output variable order
                TfcFormatter formatter;
                synthesizeScheme(table, resultsOutput, tfcOutputFileName, formatter);
            }
            catch (exception& ex)
            {
                resultsOutput << ex.what() << endl;
                resultsOutput << "\n===============================================================" << endl;
            }
        }
    }

    // process all tfc input files
    if (false && options.options.has(strTfcInput))
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

                //Scheme scheme = formatter.parse(inputFile);
                //TruthTable table = makePermutationFromScheme(scheme, formatter.getVariablesCount());
                
                TruthTable table = getHwb(9);

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

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
        CompositeGenerator generator;
        Scheme scheme = generator.generate(table, resultsOutput);
        
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

void processTruthTables(ostream& resultsOutput, const string& schemesFolder)
{
    using namespace ReversibleLogic;

    const char* strTruthTableInput = "truth-table-input";

    const ProgramOptions& options = ProgramOptions::get();
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

                resultsOutput << "Truth table: " << truthTableInputFileName << endl;

                TruthTableParser parser;
                TruthTable table = parser.parse(inputFile);

                uint inputCount  = parser.getInputCount();
                uint outputCount = parser.getOutputCount();

                unordered_map<uint, uint> outputVariablesOrder;
                if (inputCount == outputCount)
                {
                    for (uint index = 0; index < inputCount; ++index)
                        outputVariablesOrder[index] = index;
                }

                if (inputCount != outputCount || !options.isTuningEnabled ||
                    !options.options.getBool("do-not-alter-output-variables-order", false))
                {
                    table = TruthTableUtils::optimizeHammingDistance(table,
                        inputCount, outputCount, &outputVariablesOrder);
                }

                string tfcOutputFileName = appendPath(schemesFolder,
                    getFileName(truthTableInputFileName) + "-out.tfc");

                TfcFormatter formatter(inputCount, outputCount, outputVariablesOrder);
                synthesizeScheme(table, resultsOutput, tfcOutputFileName, formatter);
            }
            catch (exception& ex)
            {
                resultsOutput << ex.what() << endl;
                resultsOutput << "\n===============================================================" << endl;
            }
        }
    }
}

void processTfcFiles(ostream& resultsOutput, const string& schemesFolder)
{
    using namespace ReversibleLogic;

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

                resultsOutput << "TFC file: " << tfcInputFileName << endl;

                Scheme scheme = formatter.parse(inputFile);

                resultsOutput << "Original quantum cost: ";
                resultsOutput << SchemeUtils::calculateQuantumCost(scheme) << endl;

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
}

void generalSynthesis()
{
    using namespace ReversibleLogic;

    const ProgramOptions& options = ProgramOptions::get();

    // open results output
    ofstream resultsOutput(options.resultsFile);
    assert(resultsOutput.is_open(),
        string("Failed to open results file \"") + options.resultsFile + "\" for writing");

    // check schemes folder existence, create if not exist
    string schemesFolder = options.schemesFolder;
    if (_access(schemesFolder.c_str(), 0))
        _mkdir(schemesFolder.c_str());

    processTruthTables(resultsOutput, schemesFolder);
    processTfcFiles(resultsOutput, schemesFolder);

    resultsOutput.close();
}

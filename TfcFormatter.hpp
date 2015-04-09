#pragma once

namespace ReversibleLogic
{

class TfcFormatter
{
public:
    TfcFormatter() = default;
    TfcFormatter(uint n, uint m, unordered_map<uint, uint> outputVariablesOrder);
    virtual ~TfcFormatter() = default;

    Scheme parse(istream& stream);
    uint getVariablesCount() const;

    template<typename Container>
    void format(ostream& out, const Container& scheme);

private:
    string getVariableName(uint value) const;

    /// Parsing methods
    enum MarkerType
    {
        mtUnknown = 0,
        mtComment,
        mtVariables,
        mtInputs,
        mtOutputs,
        mtConstants,
        mtBegin,
        mtEnd,
        mtToffoliElement,
    };

    MarkerType parseMarkerType(const string& line) const;

    struct Markers
    {
        Markers() = default;

        bool variablesParsed = false;
        bool inputsParsed = false;
        bool outputsParsed = false;
        bool constantsParsed = false;
        bool beginParsed = false;
        bool endParsed = false;

        bool singleElement = false;
    };

    void checkMarker(Markers* markers, MarkerType type) const;
    void parseVariables(const string& line);
    void parseElement(const string& line, Scheme* scheme) const;

    /// Formatting methods
    void fillIndexToVariableMap(uint n);

    void writeVariablesLine(ostream& out) const;
    void writeInputLine(ostream& out) const;
    void writeOutputLine(ostream& out) const;
    void writeConstantsLine(ostream& out) const;

    void writeHeaderLine(ostream& out, const char* prefix) const;

    void writeBegin(ostream& out) const;
    void writeEnd(ostream& out) const;

    template<typename Container>
    void writeMainBody(ostream& out, const Container& scheme) const;

    static const char* strVariablesPrefix;
    static const char* strInputsPrefix;
    static const char* strOutputsPrefix;
    static const char* strConstantsPrefix;
    static const char* strBeginKeyword;
    static const char* strEndKeyword;

    unordered_map<string, int> variableToIndexMap;
    unordered_map<int, string> indexToVariableMap;

    bool hasSpecificInputOutputs = false;

    uint inputCount;
    uint outputCount;
    unordered_map<uint, uint> outputVariablesOrder;

    string inputsLine;
    string outputsLine;
    string constantsLine;
};

template<typename Container>
void ReversibleLogic::TfcFormatter::format(ostream& out, const Container& scheme)
{
    uint elementCount = scheme.size();

    uint n = 0;
    if (elementCount)
    {
        n = scheme.front().getInputCount();
    }

    fillIndexToVariableMap(n);

    writeVariablesLine(out);
    writeInputLine(out);
    writeOutputLine(out);
    writeConstantsLine(out);

    writeBegin(out);
    writeMainBody(out, scheme);
    writeEnd(out);
}

template<typename Container>
void ReversibleLogic::TfcFormatter::writeMainBody(ostream& out, const Container& scheme) const
{
    for (const ReverseElement& element : scheme)
    {
        word controlMask = element.getControlMask();
        word inversionMask = element.getInversionMask();
        uint count = countNonZeroBits(controlMask) + 1; //plus target line

        out << 't' << count << ' ';

        uint index = 0;
        uint mask = 1;

        while (mask <= controlMask)
        {
            if (controlMask & mask)
            {
                out << indexToVariableMap.at(index);
                if (inversionMask & mask)
                    out << '\'';

                out << ',';
            }                

            ++index;
            mask <<= 1;
        }

        word targetMask = element.getTargetMask();
        uint targetIndex = findPositiveBitPosition(targetMask);

        out << indexToVariableMap.at(targetIndex) << endl;
    }
}

} //namespace ReversibleLogic
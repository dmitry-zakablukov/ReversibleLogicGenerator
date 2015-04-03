#include "std.hpp"

namespace ReversibleLogic
{

// static strings
const char* TfcFormatter::strVariablesPrefix = ".v ";
const char* TfcFormatter::strInputsPrefix    = ".i ";
const char* TfcFormatter::strOutputsPrefix   = ".o ";
const char* TfcFormatter::strConstantsPrefix = ".c ";
const char* TfcFormatter::strBeginKeyword    = "BEGIN";
const char* TfcFormatter::strEndKeyword      = "END";

string TfcFormatter::getVariableName(uint value) const
{
    const char cBegin = 'a';
    const char cEnd = 'z';

    assert(value < cEnd - cBegin, string("getVariableName(): argument out of range"));
    char name = cBegin + (char)value;

    string result;
    result.push_back(name);

    return result;
}

Scheme TfcFormatter::parse(istream& stream)
{
    Scheme scheme;
    Markers markers;

    bool parsed = false;
    string currentLine;

    try
    {
        while (!stream.eof() && !parsed)
        {
            string line;
            getline(stream, line);

            if (isWhiteSpacesOnly(line))
                continue;

            currentLine = line;

            MarkerType type = parseMarkerType(line);
            checkMarker(&markers, type);

            switch (type)
            {
            case MarkerType::mtVariables:
                parseVariables(line);
                break;

            case MarkerType::mtInputs:
                inputsLine = line;
                break;

            case MarkerType::mtOutputs:
                outputsLine = line;
                break;

            case MarkerType::mtConstants:
                constantsLine = line;
                break;

            case MarkerType::mtBegin:
                break;

            case MarkerType::mtEnd:
                parsed = true;
                break;

            case MarkerType::mtToffoliElement:
                parseElement(line, &scheme);
                break;
            }
        }
    }
    catch (InvalidFormatException& ex)
    {
        ostringstream errorStream;
        errorStream << "Error: invalid line in TFC file:\n" << currentLine;

        ex.setMessage(errorStream.str());
        throw ex;
    }

    return scheme;
}

TfcFormatter::MarkerType TfcFormatter::parseMarkerType(const string& line) const
{
    MarkerType type = MarkerType::mtUnknown;
    if (line.front() == '#')
        type = MarkerType::mtComment;
    else if (line.compare(0, strlen(strVariablesPrefix), strVariablesPrefix) == 0)
        type = MarkerType::mtVariables;
    else if (line.compare(0, strlen(strInputsPrefix), strInputsPrefix) == 0)
        type = MarkerType::mtInputs;
    else if (line.compare(0, strlen(strOutputsPrefix), strOutputsPrefix) == 0)
        type = MarkerType::mtOutputs;
    else if (line.compare(0, strlen(strConstantsPrefix), strConstantsPrefix) == 0)
        type = MarkerType::mtConstants;
    else if (line.compare(strBeginKeyword) == 0)
        type = MarkerType::mtBegin;
    else if (line.compare(strEndKeyword) == 0)
        type = MarkerType::mtEnd;
    else if (line.size() > 3 && line.front() == 't') // "t* x"
        type = MarkerType::mtToffoliElement;

    return type;
}

void TfcFormatter::checkMarker(Markers* markers, MarkerType type) const
{
    assertd(markers, string("TfcFormatter::checkMarker(): null ptr"));

    bool result = true;
    switch (type)
    {
    case MarkerType::mtVariables:
        result = !markers->variablesParsed;
        markers->variablesParsed = true;
        break;

    case MarkerType::mtInputs:
        result = !markers->inputsParsed;
        markers->inputsParsed = true;
        break;

    case MarkerType::mtOutputs:
        result = !markers->outputsParsed;
        markers->outputsParsed = true;
        break;

    case MarkerType::mtConstants:
        result = !markers->constantsParsed;
        markers->constantsParsed = true;
        break;

    case MarkerType::mtBegin:
        if (!markers->variablesParsed || markers->singleElement)
            result = false;

        markers->beginParsed = true;
        break;

    case MarkerType::mtEnd:
        result = markers->beginParsed;
        markers->endParsed = true;
        break;

    case MarkerType::mtToffoliElement:
        if (!markers->beginParsed)
        {
            if (!markers->singleElement)
                markers->singleElement = true;
            else
                result = false;
        }
        break;

    default:
        result = false;
    }

    if (!result)
        throw InvalidFormatException();
}

void TfcFormatter::parseVariables(const string& line)
{
    string variables = line.substr(strlen(strVariablesPrefix));
    vector<string> parts = split(variables, ',');

    uint count = parts.size();
    if (count == 0)
        throw InvalidFormatException();

    for (uint index = 0; index < count; ++index)
    {
        const string& key = parts[index];
        if (variableToIndexMap.find(key) != variableToIndexMap.cend())
            throw InvalidFormatException();

        variableToIndexMap[key] = index;
    }
}

void TfcFormatter::parseElement(const string& line, Scheme* scheme) const
{
    assertd(scheme, string("TfcFormatter::checkMarker(): null ptr"));

    // first symbol is 't' (other types are not supported for now)
    assertd(line.size() > 3 && line.front() == 't',
        string("TfcFormatter::parseElement(): wrong toffolyelement line"));

    string temp = line.substr(1); //skip 't'
    uint size = temp.size();

    size_t spacePos = 0;
    int count = stoi(temp, &spacePos);

    while (spacePos < size && isspace(temp.at(spacePos)))
        ++spacePos;
    
    if (spacePos == size)
        throw InvalidFormatException();

    string controls = temp.substr(spacePos);
    vector<string> parts = split(controls, ',');

    if (parts.size() != count)
        throw InvalidFormatException();

    // parse control inputs
    word controlMask = 0;
    word inversionMask = 0;

    for (int index = 0; index < count - 1; ++index)
    {
        string control = parts[index];
        if (control.size() == 0)
            throw InvalidFormatException();

        bool withInversion = (control.back() == '\'');
        if (withInversion)
            control.pop_back();

        if (variableToIndexMap.find(control) == variableToIndexMap.cend())
            throw InvalidFormatException();

        int controlIndex = variableToIndexMap.at(control);
        word mask = (word)1 << controlIndex;

        controlMask |= mask;
        if (withInversion)
            inversionMask |= mask;
    }

    // parse target line
    string target = parts.back();
    if (target.size() == 0 || target.back() == '\'' ||
        variableToIndexMap.find(target) == variableToIndexMap.cend())
        throw InvalidFormatException();

    int targetIndex = variableToIndexMap.at(target);
    word targetMask = (word)1 << targetIndex;

    ReverseElement element(variableToIndexMap.size(), targetMask, controlMask, inversionMask);
    scheme->push_back(element);
}

uint TfcFormatter::getVariablesCount() const
{
    return variableToIndexMap.size();
}

void TfcFormatter::fillIndexToVariableMap(uint n)
{
    if (variableToIndexMap.size() == 0)
    {
        for (uint index = 0; index < n; ++index)
        {
            string name = getVariableName(index);
            variableToIndexMap[name] = index;
        }
    }

    for (const auto& iter : variableToIndexMap)
        indexToVariableMap[iter.second] = iter.first;
}

void TfcFormatter::writeVariablesLine(ostream& out) const
{
    writeHeaderLine(out, strVariablesPrefix);
}

void TfcFormatter::writeInputLine(ostream& out) const
{
    if (inputsLine.empty())
        writeHeaderLine(out, strInputsPrefix);
    else
        out << inputsLine << endl;
}

void TfcFormatter::writeOutputLine(ostream& out) const
{
    if (outputsLine.empty())
        writeHeaderLine(out, strOutputsPrefix);
    else
        out << outputsLine << endl;
}

void TfcFormatter::writeConstantsLine(ostream& out) const
{
    if (!constantsLine.empty())
        out << constantsLine << endl;
}

void TfcFormatter::writeHeaderLine(ostream& out, const char* prefix) const
{
    out << prefix;

    uint varCount = indexToVariableMap.size();
    for (uint index = 0; index < varCount; ++index)
    {
        out << indexToVariableMap.at(index);
        if (index != varCount - 1)
            out << ',';
    }

    out << endl;
}

void TfcFormatter::writeBegin(ostream& out) const
{
    out << strBeginKeyword << endl;
}

void TfcFormatter::writeEnd(ostream& out) const
{
    out << strEndKeyword << endl;
}

} //namespace ReversibleLogic
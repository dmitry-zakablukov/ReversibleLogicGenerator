#pragma once

namespace ReversibleLogic
{

template<typename Container>
string SchemePrinter::schemeToString(const Container& scheme,
    bool horizontal /*= true*/)
{
    string result;
    if(horizontal)
    {
        result = horizontalPrint(scheme);
    }
    else
    {
        result = verticalPrint(scheme);
    }

    return result;
}

template<typename Container>
string SchemePrinter::horizontalPrint(const Container& scheme)
{
    ostringstream result;
    uint elementCount = scheme.size();
    //result << "Element count: " << elementCount << "\n\n";

    uint n = 0;
    if(elementCount)
    {
        n = scheme.front().getInputCount();
    }

    uint stringCount = 2 * n - 1;
    vector<ostringstream> strings(stringCount);

    // beginning
    for(uint index = 0; index < stringCount; ++index)
    {
        if(index & 1)
        {
            strings[index] << "    ";
        }
        else
        {
            strings[index] << "x" << index / 2 << " -";
        }
    }

    // middle
    forcin(element, scheme)
    {
        word targetMask    = element->getTargetMask();
        word controlMask   = element->getControlMask();
        word inversionMask = element->getInversionMask();
        word elementMask   = targetMask | controlMask;

        uint positiveBitCount = countNonZeroBits(elementMask);

        uint minIndex = uintUndefined;
        uint maxIndex = uintUndefined;

        uint pos = 0;
        while(elementMask)
        {
            if(elementMask & 1)
            {
                if(minIndex == uintUndefined)
                {
                    minIndex = pos;
                }

                maxIndex = pos;
            }

            ++pos;
            elementMask >>= 1;
        }

        minIndex *= 2;
        maxIndex *= 2;
        elementMask = targetMask | controlMask;

        for(uint index = 0; index < stringCount; ++index)
        {
            if(index & 1)
            {
                if(positiveBitCount > 1
                    && index > minIndex && index < maxIndex)
                {
                    strings[index] << "  |  ";
                }
                else
                {
                    strings[index] << "     ";
                }
            }
            else
            {
                word mask = 1 << (index / 2);
                if(elementMask & mask)
                {
                    if(positiveBitCount == 1)
                    {
                        strings[index] << "-(x)-";
                    }
                    else
                    {
                        if(mask == targetMask)
                        {
                            strings[index] << "-(+)-";
                        }
                        else
                        {
                            if(inversionMask & mask)
                            {
                                strings[index] << "-(0)-";
                            }
                            else
                            {
                                strings[index] << "-( )-";
                            }
                        }
                    }
                }
                else
                {
                    if(index > minIndex && index < maxIndex)
                    {
                        strings[index] << "--|--";
                    }
                    else
                    {
                        strings[index] << "-----";
                    }
                }
            }
        }
    }

    // ending
    for(uint index = 0; index < stringCount; ++index)
    {
        if(index & 1)
        {
            strings[index] << "    ";
        }
        else
        {
            strings[index] << " y" << index / 2;
        }
    }

    // print scheme
    for(uint index = 0; index < stringCount; ++index)
    {
        result << strings[index].str() << "\n";
    }

    return result.str();
}

template<typename Container>
string SchemePrinter::verticalPrint(const Container& scheme)
{
    ostringstream result;
    uint elementCount = scheme.size();
    //result << "Element count: " << elementCount << "\n\n";

    uint n = 0;
    if(elementCount)
    {
        n = scheme.front().getInputCount();
    }

    // beginning
    ostringstream string;
    for(uint x = n - 1; x != uintUndefined; --x)
    {
        ostringstream postfix;
        postfix << " ";
        if(x < 10)
        {
            postfix << " ";
        }

        string << " x" << x << postfix.str();
    }

    result << string.str() << "\n";

    // middle
    ostringstream connectionString;
    for(uint x = n - 1; x != uintUndefined; --x)
    {
        connectionString << "  |  ";
    }
    
    connectionString << "\n";

    forcin(element, scheme)
    {
        word targetMask    = element->getTargetMask();
        word controlMask   = element->getControlMask();
        word inversionMask = element->getInversionMask();
        word elementMask   = targetMask | controlMask;

        uint positiveBitCount = countNonZeroBits(elementMask);

        uint minBound = uintUndefined;
        uint maxBound = uintUndefined;

        uint pos = 0;
        while(elementMask)
        {
            if(elementMask & 1)
            {
                if(minBound == uintUndefined)
                {
                    minBound = pos;
                }

                maxBound = pos;
            }

            ++pos;
            elementMask >>= 1;
        }
        elementMask = targetMask | controlMask;

        string.str("");
        for(uint x = n - 1; x != uintUndefined; --x)
        {
            uint index = (uint)x;
            if(index < maxBound && index >= minBound)
            {
                string << "-";
            }
            else
            {
                string << " ";
            }

            word mask = 1 << index;
            if(mask == targetMask)
            {
                if(positiveBitCount == 1)
                {
                    string << "(x)";
                }
                else
                {
                    string << "(+)";
                }
            }
            else if(elementMask & mask)
            {
                if(inversionMask & mask)
                {
                    string << "(0)";
                }
                else
                {
                    string << "( )";
                }
            }
            else
            {
                if(index < maxBound && index > minBound)
                {
                    string << "-|-";
                }
                else
                {
                    string << " | ";
                }
            }

            if(index <= maxBound && index > minBound)
            {
                string << "-";
            }
            else
            {
                string << " ";
            }
        }

        result << connectionString.str();
        result << string.str() << "\n";
    }

    // ending
    result << connectionString.str();

    string.str("");
    for(uint x = n - 1; x != uintUndefined; --x)
    {
        ostringstream postfix;
        postfix << " ";
        if(x < 10)
        {
            postfix << " ";
        }

        string << " y" << x << postfix.str();
    }

    result << string.str() << "\n";
    return result.str();
}

}   // namespace ReversibleLogic

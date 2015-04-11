#include "std.hpp"

namespace ReversibleLogic
{

Scheme RmGenerator::generate(const TruthTable& inputTable, ostream& outputLog)
{
    TruthTable table = inputTable;
    RmSpectra spectra = RmSpectraUtils::calculateRmSpectra(table);

    uint size = table.size();
    uint n = (uint)(log(size) / log(2));

    Scheme scheme;
    processFirstSpectraRow(&scheme, &table, &spectra, n);

    // process rest rows
    for (uint index = 1; index < size; ++index)
    {
        word row = spectra[index];
        if (RmSpectraUtils::isSpectraRowIdent(row, index))
            continue;

        bool isVariableRow = RmSpectraUtils::isVariableRow(index);
        if (isVariableRow)
            processVariableSpectraRow(&scheme, &table, &spectra, n, index);
        else
            processNonVariableSpectraRow(&scheme, &table, &spectra, n, index);
    }

    return scheme;
}

void RmGenerator::processFirstSpectraRow(Scheme* scheme, TruthTable* table,
    RmSpectra* spectra, uint n)
{
    word row = spectra->front();
    if (!row)
        return;

    word mask = 1;
    while (mask <= row)
    {
        if (row & mask)
        {
            word targetMask = mask;

            scheme->push_front(ReverseElement(n, targetMask));

            applyTransformation(table, targetMask);
            applyTransformation(spectra, targetMask);
        }

        mask <<= 1;
    }
}

void RmGenerator::processVariableSpectraRow(Scheme* scheme, TruthTable* table, RmSpectra* spectra,
    uint n, uint index)
{
    word row = (*spectra)[index];
    if ((row & index) == 0)
    {
        word mask = (word)1 << (n - 1);
        while ((row & mask) == 0 && mask)
            mask >>= 1;

        assertd(mask && mask != index,
            string("RmGenerator::processVariableSpectraRow(): failed to process variable row"));

        scheme->push_front(ReverseElement(n, index, mask));

        applyTransformation(table, index, mask);
        applyTransformation(spectra, index, mask);
    }

    word mask = 1;
    while (mask <= row)
    {
        if (mask != index && (row & mask))
        {
            scheme->push_front(ReverseElement(n, mask, index));

            applyTransformation(table, mask, index);
            applyTransformation(spectra, mask, index);
        }

        mask <<= 1;
    }
}

void RmGenerator::processNonVariableSpectraRow(Scheme* scheme, TruthTable* tablePtr,
    RmSpectra* spectraPtr, uint n, uint index)
{
    TruthTable& table   = *tablePtr;
    RmSpectra&  spectra = *spectraPtr;

    word row = spectra[index];

    word controlMask = (word)1 << (n - 1);
    while (controlMask)
    {
        if ((row & controlMask) != 0 &&
            (index & controlMask) == 0)
            break;

        controlMask >>= 1;
    }

    assertd(controlMask,
        string("RmGenerator::processNonVariableSpectraRow(): failed to process non-variable row"));

    deque<ReverseElement> elements;

    // check, if there are some non-zero bits except control bit
    bool hasNonZeroBitsExceptControlOne = ((row & ~controlMask) != 0);
    if (hasNonZeroBitsExceptControlOne)
    {
        word mask = 1;
        while (mask <= row)
        {
            if (mask != controlMask && (row & mask))
            {
                ReverseElement element(n, mask, controlMask);
                elements.push_front(element);

                scheme->push_front(element);
                applyTransformation(tablePtr, mask, controlMask);
            }

            mask <<= 1;
        }
    }

    // core element
    scheme->push_front(ReverseElement(n, controlMask, index));
    applyTransformation(tablePtr, controlMask, index);

    if (hasNonZeroBitsExceptControlOne)
    {
        // check if we need to apply elements to make previous rows canonical
        bool isEarlierRowsHasBeenChanged = false;
        for (uint i = 0; i < index && !isEarlierRowsHasBeenChanged; ++i)
        {
            if (spectra[i] & controlMask)
                isEarlierRowsHasBeenChanged = true;
        }

        if (isEarlierRowsHasBeenChanged)
        {
            for (auto& element : elements)
            {
                scheme->push_front(element);
                applyTransformation(tablePtr, element.getTargetMask(), element.getControlMask());
            }
        }
    }

    // because of core element, it's easier to calculate RM spectra again rather than modify existing one
    spectra = RmSpectraUtils::calculateRmSpectra(table);
}

} //namespace ReversibleLogic
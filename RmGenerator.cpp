#include "std.hpp"

namespace ReversibleLogic
{

Scheme RmGenerator::generate(const TruthTable& inputTable, ostream& outputLog)
{
    uint size = inputTable.size();
    uint n = (uint)(log(size) / log(2));

    directParams.table = inputTable;
    directParams.spectra = RmSpectraUtils::calculateSpectra(directParams.table);

    inverseParams.table = invertTable(directParams.table);
    inverseParams.spectra = RmSpectraUtils::calculateSpectra(inverseParams.table);

    Scheme scheme;
    auto iter = scheme.end();

    for (uint index = 0; index < size; ++index)
    {
        word row = directParams.spectra[index];
        if (RmSpectraUtils::isSpectraRowIdent(row, index))
            continue;

        calculatePartialResult(&directParams, n, index);
        calculatePartialResult(&inverseParams, n, index);

        if (isInverseParamsBetter())
        {
            iter = updateScheme(&scheme, iter,
                inverseParams.elements.crbegin(), inverseParams.elements.crend());

            advance(iter, inverseParams.elements.size());

            directParams.table = invertTable(inverseParams.table);
            directParams.spectra = RmSpectraUtils::calculateSpectra(directParams.table);
        }
        else
        {
            iter = updateScheme(&scheme, iter,
                directParams.elements.cbegin(), directParams.elements.cend());

            inverseParams.table = invertTable(directParams.table);
            inverseParams.spectra = RmSpectraUtils::calculateSpectra(inverseParams.table);
        }
    }

    return scheme;
}

TruthTable RmGenerator::invertTable(const TruthTable& directTable) const
{
    uint size = directTable.size();

    TruthTable inverseTable;
    inverseTable.resize(size);

    for (uint index = 0; index < size; ++index)
        inverseTable[directTable[index]] = index;

    return inverseTable;
}

void RmGenerator::calculatePartialResult(SynthesisParams* params, uint n, uint index)
{
    assertd(params, string("RmGenerator::calculatePartialResult(): null ptr"));
    params->elements.resize(0);

    if (!index) //first row
        processFirstSpectraRow(params, n);
    else
    {
        bool isVariableRow = RmSpectraUtils::isVariableRow(index);
        if (isVariableRow)
            processVariableSpectraRow(params, n, index);
        else
            processNonVariableSpectraRow(params, n, index);
    }

    params->spectraCost = RmSpectraUtils::calculateCost(params->spectra);
}

void RmGenerator::processFirstSpectraRow(SynthesisParams* params, uint n)
{
    word row = params->spectra.front();
    if (!row)
        return;

    word mask = 1;
    while (mask <= row)
    {
        if (row & mask)
        {
            word targetMask = mask;

            params->elements.push_front(ReverseElement(n, targetMask));

            applyTransformation(&(params->table), targetMask);
            applyTransformation(&(params->spectra), targetMask);
        }

        mask <<= 1;
    }
}

void RmGenerator::processVariableSpectraRow(SynthesisParams* params, uint n, uint index)
{
    word row = params->spectra[index];
    if ((row & index) == 0)
    {
        word mask = (word)1 << (n - 1);
        while ((row & mask) == 0 && mask)
            mask >>= 1;

        assertd(mask && mask != index,
            string("RmGenerator::processVariableSpectraRow(): failed to process variable row"));

        params->elements.push_front(ReverseElement(n, index, mask));

        applyTransformation(&(params->table), index, mask);
        applyTransformation(&(params->spectra), index, mask);
    }

    word mask = 1;
    while (mask <= row)
    {
        if (mask != index && (row & mask))
        {
            params->elements.push_front(ReverseElement(n, mask, index));

            applyTransformation(&(params->table), mask, index);
            applyTransformation(&(params->spectra), mask, index);
        }

        mask <<= 1;
    }
}

void RmGenerator::processNonVariableSpectraRow(SynthesisParams* params, uint n, uint index)
{
    word row = params->spectra[index];

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

                params->elements.push_front(element);
                applyTransformation(&(params->table), mask, controlMask);
            }

            mask <<= 1;
        }
    }

    // core element
    params->elements.push_front(ReverseElement(n, controlMask, index));
    applyTransformation(&(params->table), controlMask, index);

    if (hasNonZeroBitsExceptControlOne)
    {
        // check if we need to apply elements to make previous rows canonical
        bool isEarlierRowsHasBeenChanged = false;
        for (uint i = 0; i < index && !isEarlierRowsHasBeenChanged; ++i)
        {
            if (params->spectra[i] & controlMask)
                isEarlierRowsHasBeenChanged = true;
        }

        if (isEarlierRowsHasBeenChanged)
        {
            for (auto& element : elements)
            {
                params->elements.push_front(element);
                applyTransformation(&(params->table), element.getTargetMask(), element.getControlMask());
            }
        }
    }

    // because of core element, it's easier to calculate RM spectra again rather than modify existing one
    params->spectra = RmSpectraUtils::calculateSpectra(params->table);
}

template<typename TableType>
void RmGenerator::applyTransformation(TableType* tablePtr, word targetMask, word controlMask /*= 0*/)
{
    assertd(tablePtr, string("RmGenerator::applyTransformation(): null ptr"));

    assertd(countNonZeroBits(targetMask) == 1 && (controlMask & targetMask) == 0,
        string("RmGenerator::applyTransformation(): invalid arguments"));

    TruthTable& table = *tablePtr;
    uint size = table.size();

    for (uint index = 0; index < size; ++index)
    {
        word value = table[index];
        if ((value & controlMask) == controlMask)
            table[index] = value ^ targetMask;
    }
}

bool RmGenerator::isInverseParamsBetter() const
{
    bool isBetter = false;

    if (inverseParams.spectraCost < directParams.spectraCost)
        isBetter = true;
    else if (inverseParams.spectraCost == directParams.spectraCost)
    {
        // compare number of control inputs
        uint inverseControlCount = calculateTotalControlInputCount(inverseParams.elements);
        uint directControlCount = calculateTotalControlInputCount(directParams.elements);

        isBetter = (inverseControlCount < directControlCount);
    }

    return isBetter;
}

uint RmGenerator::calculateTotalControlInputCount(const deque<ReverseElement>& elements) const
{
    uint totalCount = 0;
    for (auto& element : elements)
        totalCount += countNonZeroBits(element.getControlMask());

    return totalCount;
}

template<typename IteratorType>
Scheme::iterator ReversibleLogic::RmGenerator::updateScheme(Scheme* scheme,
    Scheme::iterator iter, IteratorType from, IteratorType to)
{
    Scheme::iterator localIter = iter;
    while (from != to)
        localIter = scheme->insert(localIter, *from++);

    return localIter;
}

} //namespace ReversibleLogic
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

#include "std.hpp"

namespace ReversibleLogic
{

RmGenerator::RmGenerator(uint threshold /*= uintUndefined*/)
    : weightThreshold(threshold)
    , pushPolicy()
    , directParams()
    , inverseParams()
{
}

void RmGenerator::generate(const TruthTable& inputTable, SynthesisResult* result)
{
    assertd(result, string("RmGenerator::generate(): null ptr"));

    uint size = inputTable.size();
    uint n = (uint)(log(size) / log(2));

    initResult(result, size);
    initPushPolicy();
    initSynthesisParams(inputTable);

    Scheme& scheme = result->scheme;
    auto iter = scheme.end();

    for (uint index = 0; index < size; ++index)
    {
        word row = directParams.spectra[index];
        if (RmSpectraUtils::isSpectraRowIdent(row, index))
            continue;

        if (countNonZeroBits(index) >= weightThreshold)
        {
            processAlienSpectraRow(n, index, scheme, iter, result);
            continue;
        }

        calculatePartialResult(&directParams, n, index);
        calculatePartialResult(&inverseParams, n, index);

        iter = implementPartialResult(&scheme, iter);
    }
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

void RmGenerator::initResult(SynthesisResult* result, uint size)
{
    assertd(result, string("RmGenerator::initResult(): null ptr"));

    result->leftMultTable.resize(size);
    result->rightMultTable.resize(size);

    for (uint index = 0; index < size; ++index)
        result->leftMultTable[index] = result->rightMultTable[index] = index;
}

void RmGenerator::initPushPolicy()
{
    const ProgramOptions& options = ProgramOptions::get();
    if (options.isTuningEnabled)
    {
        pushPolicy.forceLeft  = options.options.getBool("push-policy-force-left");
        pushPolicy.forceRight = options.options.getBool("push-policy-force-right");

        pushPolicy.autoHammingDistance =
            options.options.getBool("push-policy-auto-mode-min-hamming-distance");

        pushPolicy.autoRmCostReduction =
            options.options.getBool("push-policy-auto-mode-max-rm-cost-reduction");

        pushPolicy.defaultPolicy = !(pushPolicy.forceLeft || pushPolicy.forceRight ||
            pushPolicy.autoHammingDistance || pushPolicy.autoRmCostReduction);

        uint count = pushPolicy.defaultPolicy + pushPolicy.forceLeft + pushPolicy.forceRight +
            pushPolicy.autoHammingDistance + pushPolicy.autoRmCostReduction;

        assert(count == 1,
            string("RmGenerator::initPushPolicy(): mutual exclusive push policies are enabled"));
    }
}

const RmGenerator::PushPolicy& RmGenerator::getPushPolicy() const
{
    return pushPolicy;
}

void RmGenerator::initSynthesisParams(const TruthTable& inputTable)
{
    directParams.table = inputTable;
    directParams.spectra = RmSpectraUtils::calculateSpectra(directParams.table);

    inverseParams.table = invertTable(directParams.table);
    inverseParams.spectra = RmSpectraUtils::calculateSpectra(inverseParams.table);
}

void RmGenerator::processAlienSpectraRow(uint n, uint index, const Scheme& scheme,
    Scheme::const_iterator iter, SynthesisResult* result)
{
    assertd(result, string("RmGenerator::processAlienSpectraRow(): null ptr"));

    word x = index;
    word y = directParams.table[index];
    word z = wordUndefined;

    // find z, for which table[z] == x
    uint size = directParams.table.size();
    for (uint i = index + 1; i < size; ++i)
    {
        word temp = directParams.table[i];
        if (temp == x)
        {
            z = i;
            break;
        }
    }

    assertd(z != wordUndefined,
        string("RmGenerator::processAlienSpectraRow(): direct table is not bijective"));

    applyPushPolicy(x, y, z, scheme, iter, result);

    // modify tables
    directParams.table[index] = index;
    directParams.table[z] = y;
    directParams.spectra = RmSpectraUtils::calculateSpectra(directParams.table);

    inverseParams.table[index] = index;
    inverseParams.table[y] = z;
    inverseParams.spectra = RmSpectraUtils::calculateSpectra(inverseParams.table);
}

void RmGenerator::applyPushPolicy(word x, word y, word z, const Scheme& scheme,
    Scheme::const_iterator iter, SynthesisResult* result)
{
    assertd(result, string("RmGenerator::applyPushPolicy(): null ptr"));

    // for pushing to left, we conjugate transposition (x, z)
    std::reverse_iterator<Scheme::const_iterator> from(iter);
    std::reverse_iterator<Scheme::const_iterator> to(scheme.cbegin());

    word xLeft = conjugateValue(x, from, to);
    word zLeft = conjugateValue(z, from, to);

    Transposition leftTransp(xLeft, zLeft);

    // for pushing to right, we conjugate transposition (x, y)
    word xRight = conjugateValue(x, iter, scheme.cend());
    word yRight = conjugateValue(y, iter, scheme.cend());

    Transposition rightTransp(xRight, yRight);

    // apply policy
    if (pushPolicy.forceLeft)
        pushTranpsositionToLeft(leftTransp, result);
    else if (pushPolicy.forceRight)
        pushTranpsositionToRight(rightTransp, result);
    else if (pushPolicy.autoHammingDistance)
    {
        uint leftDist = leftTransp.getDist();
        uint rightDist = rightTransp.getDist();

        if (leftDist < rightDist)
            pushTranpsositionToLeft(leftTransp, result);
        else
            pushTranpsositionToRight(rightTransp, result);
    }
    else if (pushPolicy.autoRmCostReduction)
    {
        uint leftCostBefore = RmSpectraUtils::calculateCost(
            RmSpectraUtils::calculateSpectra(result->leftMultTable));

        uint rightCostBefore = RmSpectraUtils::calculateCost(
            RmSpectraUtils::calculateSpectra(result->rightMultTable));

        pushTranpsositionToLeft(leftTransp, result);
        pushTranpsositionToRight(rightTransp, result);

        uint leftCostAfter = RmSpectraUtils::calculateCost(
            RmSpectraUtils::calculateSpectra(result->leftMultTable));

        uint rightCostAfter = RmSpectraUtils::calculateCost(
            RmSpectraUtils::calculateSpectra(result->rightMultTable));

        if ((int)(leftCostAfter - leftCostBefore) < (int)(rightCostAfter - rightCostBefore))
            // push left, restore right mul table
            pushTranpsositionToRight(rightTransp, result);
        else
            // push right, restore left mul table
            pushTranpsositionToLeft(leftTransp, result);
    }
    else
    {
        // default policy
        assertd(pushPolicy.defaultPolicy,
            string("RmGenerator::applyPushPolicy(): default policy is not enabled but should be"));

        pushTranpsositionToLeft(leftTransp, result);
        pushTranpsositionToRight(rightTransp, result);
    }
}

template<typename Iterator>
word ReversibleLogic::RmGenerator::conjugateValue(word x, Iterator from, Iterator to) const
{
    word y = x;
    while (from != to)
    {
        const ReverseElement& element = *from++;
        y = element.getValue(y);
    }

    return y;
}

void RmGenerator::pushTranpsositionToLeft(const Transposition& transp, SynthesisResult* result)
{
    assertd(result, string("RmGenerator::pushTranpsositionToRight(): null ptr"));

    TruthTable& table = result->leftMultTable;

    word x = transp.getX();
    word y = transp.getY();

    uint xIndex = uintUndefined;
    uint yIndex = uintUndefined;

    uint size = table.size();
    for (uint index = 0; index < size && (xIndex == uintUndefined || yIndex == uintUndefined); ++index)
    {
        word value = table[index];

        if (value == x)
            xIndex = index;
        else if (value == y)
            yIndex = index;
    }

    swap(table[xIndex], table[yIndex]);
}

void RmGenerator::pushTranpsositionToRight(const Transposition& transp, SynthesisResult* result)
{
    assertd(result, string("RmGenerator::pushTranpsositionToRight(): null ptr"));

    TruthTable& table = result->rightMultTable;

    word x = transp.getX();
    word y = transp.getY();

    swap(table[x], table[y]);
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

    word mask = 1;
    while (mask <= row)
    {
        if (row & mask)
        {
            word targetMask = mask;

            params->elements.push_back(ReverseElement(n, targetMask));
            applyTransformation(&(params->table), targetMask);
        }

        mask <<= 1;
    }

    params->spectra[0] = 0;
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

        params->elements.push_back(ReverseElement(n, index, mask));

        applyTransformation(&(params->table), index, mask);
        applyTransformation(&(params->spectra), index, mask);
    }

    word mask = 1;
    while (mask <= row)
    {
        if (mask != index && (row & mask))
        {
            params->elements.push_back(ReverseElement(n, mask, index));

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
        if ((row & controlMask) != 0 && (index & controlMask) == 0)
            break;

        controlMask >>= 1;
    }

    assert(controlMask,
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
                elements.push_back(element);

                params->elements.push_back(element);
                applyTransformation(&(params->table), mask, controlMask);
            }

            mask <<= 1;
        }
    }

    // core element
    params->elements.push_back(ReverseElement(n, controlMask, index));
    applyTransformation(&(params->table), controlMask, index);

    if (hasNonZeroBitsExceptControlOne)
    {
        // check if we need to apply elements to make previous rows canonical
        bool isEarlierRowsHasBeenChanged = controlMask < index;
        if (isEarlierRowsHasBeenChanged)
        {
            for (auto& element : elements)
            {
                params->elements.push_back(element);
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

    TableType& table = *tablePtr;
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
        // compare number of elements
        // we have no need to compare number of control inputs, because we are working at this moment
        // with the same row of direct and inverse spectra => core element is equal for them
        // and only thing that can be differ - number of CNOT gates

        isBetter = (inverseParams.elements.size() < directParams.elements.size());
    }

    return isBetter;
}

deque<ReversibleLogic::ReverseElement>::iterator RmGenerator::implementPartialResult(Scheme* scheme,
    deque<ReversibleLogic::ReverseElement>::iterator iter)
{
    assertd(scheme, string("RmGenerator::implementPartialResult(): null ptr"));

    if (isInverseParamsBetter())
    {
        iter = updateScheme(scheme, iter, inverseParams.elements.crbegin(), inverseParams.elements.crend());
        advance(iter, inverseParams.elements.size());

        directParams.table = invertTable(inverseParams.table);
        directParams.spectra = RmSpectraUtils::calculateSpectra(directParams.table);
    }
    else
    {
        iter = updateScheme(scheme, iter, directParams.elements.cbegin(), directParams.elements.cend());

        inverseParams.table = invertTable(directParams.table);
        inverseParams.spectra = RmSpectraUtils::calculateSpectra(inverseParams.table);
    }

    return iter;
}

template<typename Iterator>
Scheme::iterator ReversibleLogic::RmGenerator::updateScheme(Scheme* scheme,
    Scheme::iterator iter, Iterator from, Iterator to)
{
    Scheme::iterator localIter = iter;
    while (from != to)
        localIter = scheme->insert(localIter, *from++);

    return localIter;
}

} //namespace ReversibleLogic

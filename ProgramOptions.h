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

#pragma once

class ProgramOptions
{
public:
    virtual ~ProgramOptions() = default;

    static const ProgramOptions& get();

    static void init(const Values& values);
    static void uninit();

    void load(const Values& values);

    string inputFile = "";
    string resultsFile = "results.txt";

    string schemesFolder = "schemes/";
    string schemeOutputFile = "scheme.txt";

    int rmGeneratorWeightThreshold = -1;
    int transpositionsPackSize = 2;

    bool isDebugBehaviorEnabled = false;

    bool doPostOptimization = true;
    uint maxElementsDistanceForOptimization = 20;
    uint maxSubSchemeSizeForOptimization = 100;

    bool isTuningEnabled = false;

    Values options;

private:
    ProgramOptions() = default;

    static shared_ptr<ProgramOptions> instance;
};
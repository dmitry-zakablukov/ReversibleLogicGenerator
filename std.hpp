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

// system headers
#include <string.h>
#include <stdarg.h>
#include <exception>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <list>
#include <deque>
#include <vector>
#include <tuple>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <memory>
#include <ctime>
#include <fstream>

#include <direct.h> //mkdir
#include <io.h> //access

using namespace std;

//// GNU Multiple Precision Arithmetic Library
//#include "libs/gmp/include/gmpxx.h"
//typedef mpz_class word;

#if defined(_WIN64)
    typedef unsigned __int64 word;
#else   // WIN64
    typedef unsigned __int32 word;
#endif  // WIN64

typedef size_t uint;

#define uintUndefined (size_t)(-1)
#define wordUndefined (word)(-1)

// headers
#include "Timer.hpp"
#include "Range.hpp"

#include "Exceptions.h"
#include "utils.hpp"
#include "Element.hpp"
#include "SchemeUtils.h"
#include "Transposition.hpp"
#include "Cycle.hpp"
#include "Permutation.hpp"
#include "PermutationUtils.hpp"
#include "PostProcessor.hpp"
#include "TfcFormatter.hpp"
#include "TruthTableParser.h"
#include "TruthTableUtils.h"
#include "BooleanEdgeSearcher.hpp"
#include "PartialResultParams.hpp"
#include "PartialGtGenerator.hpp"
#include "GtGenerator.hpp"
#include "GtGeneratorWithMemory.hpp"
#include "Values.h"
#include "IniParser.h"
#include "ProgramOptions.h"
#include "RmSpectraUtils.h"
#include "RmGenerator.h"
#include "CompositeGenerator.h"

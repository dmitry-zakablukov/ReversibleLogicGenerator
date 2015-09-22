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

#if defined(__GNUC__)
#   include <inttypes.h>
#   include <sys/stat.h>
#   include <sys/types.h>
#   define _access(x,y) access((x), (y))
#   define _mkdir(x) mkdir((x), 0777)
#else //__GNUC__
#   include <direct.h> //mkdir
#   include <io.h> //access
#endif //__GNUC__

using namespace std;

//// GNU Multiple Precision Arithmetic Library
//#include "libs/gmp/include/gmpxx.h"
//typedef mpz_class word;

#if defined(__GNUC__)
#   if defined(__LP64__)
        typedef uint64_t word;
#   else //__LP64__
        typedef uint32_t word;
#   endif //__LP64__
#else //__GNUC
#   if defined(_WIN64)
        typedef unsigned __int64 word;
#   else   // WIN64
        typedef unsigned __int32 word;
#   endif  // WIN64
    typedef size_t uint;
#endif //__GNUC__

#define uintUndefined (uint)(-1)
#define wordUndefined (word)(-1)

// headers
#include "Timer.h"
#include "Range.h"

#include "Exceptions.h"
#include "utils.h"
#include "Element.h"
#include "SchemeUtils.h"
#include "Transposition.h"
#include "Cycle.h"
#include "Permutation.h"
#include "PermutationUtils.h"
#include "PostProcessor.h"
#include "TfcFormatter.h"
#include "TruthTableParser.h"
#include "TruthTableUtils.h"
#include "BooleanEdgeSearcher.h"
#include "PartialResultParams.h"
#include "PartialGtGenerator.h"
#include "GtGenerator.h"
#include "GtGeneratorWithMemory.h"
#include "Values.h"
#include "IniParser.h"
#include "ProgramOptions.h"
#include "RmSpectraUtils.h"
#include "RmGenerator.h"
#include "CompositeGenerator.h"

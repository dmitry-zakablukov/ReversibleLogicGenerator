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
#include <map>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <memory>
#include <ctime>

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

// Optimization techniques
//#define ADDITIONAL_MEMORY_TECHNIQUE

// headers
#include "Timer.hpp"

#include "utils.hpp"
#include "Element.hpp"
#include "Transposition.hpp"
#include "Cycle.hpp"
#include "Permutation.hpp"
#include "PermutationUtils.hpp"
#include "PostProcessor.hpp"
#include "SchemePrinter.hpp"
#include "BooleanEdgeSearcher.hpp"
#include "PartialResultParams.h"
#include "PartialGenerator.hpp"
#include "Generator.hpp"

#ifndef __TYPES_H__
#define __TYPES_H__

#include <string>
using namespace std;

// C style
// typedef int T;

// C++11 style
using TI = int;
using TD = double;
using TS = string;

// XT must be 32bit integer in Windows and 64bit in Linux
#if defined(_WIN32) || defined(_WIN64)
    using XT = int;
#else
    using XT = long;
#endif

using Ref = long;

using KeyRef   = long;
using Counter  = long;
using Order    = int;
using Capacity = int;
using Count    = int;

using Nombre = string;
using Tipo   = string;
using Letra = string;
using Edad = int;
using Total = int;

#endif // __TYPES_H__
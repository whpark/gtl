// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#define NOMINMAX
#include <windows.h>
// C RunTime Header Files

#include <Windowsx.h>
#include <assert.h>
#include <unknwn.h>

#include <wrl.h>
#include <wil/com.h>

#include "webview2.h"

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

//#include "gtest/gtest.h"
//#include "benchmark/benchmark.h"

#include "gtl/gtl.h"
#include "gtl/ui.h"


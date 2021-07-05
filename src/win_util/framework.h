#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT _WIN32_WINNT_WIN10

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include <sdkddkver.h>

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components

#include <afxlistctrl.h>
#include <afxdialogex.h>
#include <ATLImage.h>

#include <Iphlpapi.h>

#include <PSApi.h>
#include <WinSvc.h>

#undef min
#undef max
#include <algorithm>
#include <cmath>
#include <streambuf>
#include <string>
#include <string_view>

#include "gtl/_config.h"
#include "gtl/_macro.h"

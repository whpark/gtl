//
// pch.h
// Header for standard system include files.
//

#pragma once

#define NOMINMAX
#pragma warning(push)
#pragma warning(disable: 5104 5105)


#include "gtl/gtl.h"

#include <windows.h>

#include "gtest/gtest.h"
#include "benchmark/benchmark.h"

#include "opencv2/opencv.hpp"

#include "boost/json.hpp"

#include "nlohmann/json.hpp"

#pragma warning(pop)

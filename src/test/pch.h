//
// pch.h
// Header for standard system include files.
//

#pragma once

#pragma warning(push)
#pragma warning(disable: 5104 5105)

//#define NOMINMAX
//#include <windows.h>

#include <catch2/catch_test_macros.hpp>

#define TEST(suite, name) TEST_CASE(#suite "." #name, "[" #suite "]")
#define EXPECT_TRUE(expr) CHECK(expr)
#define EXPECT_FALSE(expr) CHECK_FALSE(expr)
#define EXPECT_EQ(lhs, rhs) CHECK((lhs) == (rhs))
#define EXPECT_NE(lhs, rhs) CHECK((lhs) != (rhs))

#include "opencv2/opencv.hpp"

#include "boost/json.hpp"
#include "boost/variant.hpp"
#include "boost/ptr_container/ptr_deque.hpp"
#include "boost/ptr_container/serialize_ptr_deque.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/optional.hpp"
#include "boost/serialization/map.hpp"
#include "boost/serialization/variant.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/serialization/export.hpp"

#pragma warning(pop)

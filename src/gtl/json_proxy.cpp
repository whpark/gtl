#include "pch.h"
#include "gtl/config_gtl.h"
#include "gtl/json_proxy.h"

//////////////////////////////////////////////////////////////////////
//
// json_proxy.cpp
//
// PWH
// 2021.01.17.
//
//////////////////////////////////////////////////////////////////////

#if (GTL__BOOST_JSON__AS_STANDALONE) || (GTL__BOOST_JSON__AS_NESTED_LIB)

	#include "boost/json/src.hpp"   // impl

	namespace boost {
		// When exceptions are disabled
		// in standalone, you must provide
		// this function.
		void throw_exception(std::exception const&) {
		}
	}

#endif

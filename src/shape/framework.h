#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <deque>
#include <optional>

#define NOMINMAX
#pragma warning(push)
#pragma warning(disable: 5104 5105)
#include <windows.h>
#pragma warning(pop)

#include "gtl/_config.h"
#include "gtl/concepts.h"
#include "gtl/unit.h"
#include "gtl/coord.h"
#include "gtl/dynamic.h"
#include "gtl/json_proxy.h"

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


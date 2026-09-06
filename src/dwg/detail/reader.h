#pragma once

#include "gtl/dwg/sections.h"
#include <span>
#include "gtl/dwg/container.h"

namespace gtl::dwg::detail {
	// Throws xParseError on malformed input. Only a completely parsed document is published.
	eVERSION DetectVersion(std::span<std::uint8_t const> bytes);
	sDocument ReadLegacy(std::span<std::uint8_t const> bytes);
	sDocument ReadContainerObjects(sContainer const& container);
}

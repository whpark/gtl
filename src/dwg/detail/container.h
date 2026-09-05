#pragma once
#include "gtl/dwg/container.h"
#include <span>

namespace gtl::dwg::detail {
	std::vector<std::uint8_t> Decompress2004(std::span<std::uint8_t const> input, size_t size);
	std::vector<std::uint8_t> Decompress2007(std::span<std::uint8_t const> input, size_t size);
	sContainer DecodeContainer(std::span<std::uint8_t const> input, sContainerOptions const &options = {});
} // namespace gtl::dwg::detail

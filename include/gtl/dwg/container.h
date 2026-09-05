#pragma once

#include "_lib_gtl_dwg.h"
#include "aliases.h"
#include <filesystem>
#include <map>
#include <optional>
#include <vector>

namespace gtl::dwg {
	struct sReadReport;
	struct sContainerOptions {
		size_t maxFileBytes{512u * 1024 * 1024};
		size_t maxDecodedBytes{512u * 1024 * 1024};
		size_t maxPages{1000000};
	};
	struct sContainerSection {
		std::uint64_t id{};
		std::vector<std::uint8_t> data;
		size_t pageCount{};
	};
	struct sContainer {
		eVERSION version{eVERSION::unknown};
		std::uint16_t codepage{};
		std::map<std::string, sContainerSection> sections;
	};
	// Reads decompressed logical sections, not drawing objects. AC1018 through AC1032.
	GTL__DWG_API std::optional<sContainer> ReadDWGContainer(std::filesystem::path const& path,
		sReadReport* report = nullptr, sContainerOptions const& options = {});
} // namespace gtl::dwg

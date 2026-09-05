#pragma once

#include <filesystem>
#include <optional>
#include "_lib_gtl_dwg.h"
#include "gtl/shape/shape.h"

namespace gtl::dwg {
	class xDWG;
	struct sReadReport;
	struct sShapeOptions {
		size_t maxBlockDepth{64}; // Requests above the hard ceiling of 64 are clamped.
		size_t maxEntities{100000};
		size_t maxVisits{1000000}; // Includes INSERTs and empty/unsupported entities.
	};

	// Converts model-space geometry. Reports omitted objects and unsupported properties.
	// Throws std::logic_error when called on a drawing that has not been read successfully.
	GTL__DWG_API gtl::shape::xDrawing ToShape(xDWG const& dwg, sReadReport* report = nullptr, sShapeOptions const& options = {});
	GTL__DWG_API std::optional<gtl::shape::xDrawing> ReadDWGShape(std::filesystem::path const& path, sReadReport* report = nullptr, sShapeOptions const& options = {});
}

#pragma once

#include <filesystem>
#include <optional>
#include "_lib_gtl_dwg.h"
#include "gtl/shape/shape.h"

namespace gtl::dwg {
	class xDWG;
	struct sReadReport;
	enum class eSPACE { model, paper, all };
	struct sShapeOptions {
		size_t maxBlockDepth{64}; // Requests above the hard ceiling of 64 are clamped.
		size_t maxEntities{100000};
		size_t maxVisits{1000000}; // Includes INSERTs and empty/unsupported entities.
		eSPACE space{eSPACE::model};
		std::uint64_t paperBlock{}; // Zero selects all paper-space blocks; otherwise select a block handle.
		double curveTolerance{0.01}; // Drawing units, sampled chord deviation for rational curves.
		size_t maxCurveSegments{65536};
		bool hatchBoundaryOnly{false};
		double solidHatchSpacing{1.}; // Scanline approximation in drawing units; boundaries remain available.
		size_t maxHatchSegments{100000};
	};

	// Converts the selected drawing space (model by default). Reports omissions and approximations.
	// Throws std::logic_error when called on a drawing that has not been read successfully.
	GTL__DWG_API gtl::shape::xDrawing ToShape(xDWG const& dwg, sReadReport* report = nullptr, sShapeOptions const& options = {});
	GTL__DWG_API std::optional<gtl::shape::xDrawing> ReadDWGShape(std::filesystem::path const& path, sReadReport* report = nullptr, sShapeOptions const& options = {});
}

#pragma once

#include <cstdint>
#include <string>

namespace gtl::dwg {

	using handle_t = std::uint64_t;
	using string_t = std::string; // File codepage before R2007; UTF-8 from R2007. ToShape produces wide strings.
	struct point_t {
		double x{}, y{}, z{};
		bool operator==(point_t const&) const = default;
	};

	enum class eVERSION { unknown, r14, r2000, r2004, r2007, r2010, r2013, r2018 };
	enum class eREAD_ERROR { none, io, unsupported_version, invalid_data, resource_limit };
	struct sDiagnostic {
		handle_t handle{};
		std::uint16_t type{};
		std::string message;
	};

}

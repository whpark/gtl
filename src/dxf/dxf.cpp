#include "gtl/dxf/dxf.h"

namespace gtl::dxf {

	// Keeps gtl.dxf as a concrete library target while the heavily templated parser
	// remains available from ordinary C++ headers.
	int LibraryAnchor() noexcept {
		return 0;
	}

} // namespace gtl::dxf


#pragma once

#include <filesystem>
#include "_lib_gtl_dwg.h"
#include "sections.h"
#include "container.h"

namespace gtl::dwg {

	struct sReadReport {
		eVERSION version{eVERSION::unknown};
		eREAD_ERROR error{eREAD_ERROR::none};
		std::string message;
		std::vector<sDiagnostic> diagnostics;
		size_t convertedEntities{};
	};

	class GTL__DWG_CLASS xDWG {
		sDocument m_document;
		sReadReport m_report;
		bool m_loaded{};
	public:
		using this_t = xDWG;
		// A successful read validates the container and decodes supported objects.
		// Check GetReport().diagnostics for payloads outside the current implementation.
		// A failed read clears all previous drawing data.
		bool ReadDWG(std::filesystem::path const& path);
		bool IsLoaded() const { return m_loaded; }
		auto const& GetDocument() const { return m_document; }
		auto const& GetReport() const { return m_report; }
	};

}

#include "shape.h"

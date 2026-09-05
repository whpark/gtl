#pragma once

#include "entities.h"
#include <map>

namespace gtl::dwg {

	struct sSection { std::uint8_t number{}; std::uint32_t offset{}, size{}; };
	struct sObject { handle_t handle{}; std::uint32_t offset{}, size{}; std::uint16_t type{}; };
	struct sClass {
		std::uint16_t number{}, proxyFlags{}, itemClass{};
		string_t application, cppName, dxfName;
		bool zombie{};
	};
	struct sLayer {
		handle_t handle{};
		string_t name;
		std::int16_t color{7};
		std::uint16_t flags{};
		std::uint8_t lineWeight{31};
		bool frozen{}, off{}, locked{}, plot{true};
	};
	struct sDocument {
		eVERSION version{eVERSION::unknown};
		std::uint16_t codepage{}; // DWG codepage identifier, not Windows codepage number.
		std::vector<sSection> sections;
		std::vector<sObject> objects;
		std::map<std::uint16_t, sClass> classes;
		std::map<handle_t, sLayer> layers;
		struct sBlock {
			handle_t handle{}, begin{}, end{}, first{}, last{};
			string_t name, xrefPath, description;
			point_t base;
			bool xref{}, overlay{}, unloaded{};
			std::uint8_t space{}; // 0: block definition, 1: paper, 2: model
			std::vector<handle_t> entities;
		};
		std::map<handle_t, sBlock> blocks;
		std::vector<entities::sEntity> entities;
		std::vector<sDiagnostic> diagnostics;
	};

}

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
		handle_t lineType{};
		string_t name;
		std::int16_t color{7};
		std::optional<std::uint32_t> rgb;
		std::uint16_t flags{};
		std::uint8_t lineWeight{31};
		bool frozen{}, off{}, locked{}, plot{true};
	};
	struct sTextStyle {
		handle_t handle{}; string_t name, font, bigFont;
		bool vertical{}, shapeFile{};
		double fixedHeight{}, widthFactor{}, oblique{}, lastHeight{};
		std::uint8_t generation{};
	};
	struct sLineType {
		struct sDash { double length{}, x{}, y{}, scale{}, rotation{}; std::int16_t shapeCode{}, flags{}; handle_t style{}; };
		handle_t handle{}; string_t name, description;
		double length{}; std::uint8_t alignment{};
		std::vector<sDash> dashes; std::vector<std::uint8_t> strings;
	};
	struct sDocument {
		using header_value_t=std::variant<std::int64_t,std::uint64_t,double,point_t,string_t>;
		std::map<string_t,header_value_t> headerVariables;
		std::optional<std::uint16_t> insertionUnits, measurement;
		eVERSION version{eVERSION::unknown};
		std::uint16_t codepage{}; // DWG codepage identifier, not Windows codepage number.
		bool unicodeStrings{}; // R2007+ TU strings are normalized to UTF-8.
		std::vector<sSection> sections;
		std::vector<sObject> objects;
		std::map<std::uint16_t, sClass> classes;
		std::map<handle_t, sLayer> layers;
		std::map<handle_t, sTextStyle> textStyles;
		std::map<handle_t, sLineType> lineTypes;
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

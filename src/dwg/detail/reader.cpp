#include "reader.h"
#include "bit_stream.h"

#include <algorithm>
#include <set>
#include <string_view>

namespace gtl::dwg::detail {

	eVERSION DetectVersion(std::span<std::uint8_t const> bytes) {
		if (bytes.size() < 6) return eVERSION::unknown;
		std::string_view signature{reinterpret_cast<char const*>(bytes.data()), 6};
		constexpr std::array signatures{"AC1014", "AC1015", "AC1018", "AC1021", "AC1024", "AC1027", "AC1032"};
		for (size_t i{}; i < signatures.size(); ++i)
			if (signature == signatures[i]) return static_cast<eVERSION>(i + 1);
		return eVERSION::unknown;
	}

	namespace {
		using bytes_t = std::span<std::uint8_t const>;
		bytes_t Slice(bytes_t bytes, size_t offset, size_t size) {
			if (offset > bytes.size() || size > bytes.size() - offset) throw xParseError("range outside file");
			return bytes.subspan(offset, size);
		}
		std::uint16_t ShortAt(bytes_t bytes, size_t offset, bool bigEndian = false) {
			auto b = Slice(bytes, offset, 2);
			return bigEndian ? static_cast<std::uint16_t>((b[0] << 8) | b[1]) : static_cast<std::uint16_t>(b[0] | (b[1] << 8));
		}
		point_t Point(xBitStream& stream) { return {stream.BD(), stream.BD(), stream.BD()}; }
		void ExtendedData(xBitStream& stream) {
			for (;;) {
				auto length = stream.BS();
				if (!length) return;
				if (length < 0) throw xParseError("negative extended data length");
				stream.Handle();
				stream.SkipBytes(length);
			}
		}
		std::uint32_t Count(xBitStream& stream) {
			auto value = stream.BL();
			if (value < 0 || static_cast<size_t>(value) > stream.Remaining()) throw xParseError("invalid item count");
			return static_cast<std::uint32_t>(value);
		}

		void ReadClasses(bytes_t bytes, sSection const& section, sDocument& document) {
			if (!section.size) return;
			auto data = Slice(bytes, section.offset, section.size);
			constexpr std::array<std::uint8_t, 16> sentinel{0x8d,0xa1,0xc4,0xb8,0xc4,0xa9,0xf8,0xc5,0xc0,0xdc,0xf4,0x5f,0xe7,0xcf,0xb6,0x8a};
			xBitStream head{data};
			for (auto value : sentinel) if (head.RC() != value) throw xParseError("invalid classes sentinel");
			auto size = head.RL();
			if (data.size() < 38 || size != data.size() - 38) throw xParseError("invalid classes section size");
			if (CRC16(Slice(data, 16, size + 4)) != ShortAt(data, 20 + size)) throw xParseError("classes CRC mismatch");
			for (size_t i{}; i < sentinel.size(); ++i)
				if (data[22 + size + i] != static_cast<std::uint8_t>(~sentinel[i])) throw xParseError("invalid classes end sentinel");
			xBitStream stream{Slice(data, 20, size)};
			while (stream.Remaining() >= 8) {
				sClass item;
				item.number = static_cast<std::uint16_t>(stream.BS());
				item.proxyFlags = static_cast<std::uint16_t>(stream.BS());
				item.application = stream.Text();
				item.cppName = stream.Text();
				item.dxfName = stream.Text();
				item.zombie = stream.B();
				item.itemClass = static_cast<std::uint16_t>(stream.BS());
				if (item.number < 500 || !document.classes.emplace(item.number, std::move(item)).second)
					throw xParseError("invalid or duplicate class number");
			}
		}

		void ReadMap(bytes_t file, sSection const& section, sDocument& document) {
			auto bytes = Slice(file, section.offset, section.size);
			size_t position{};
			std::set<handle_t> seen;
			std::set<std::uint32_t> offsets;
			while (position < bytes.size()) {
				auto size = ShortAt(bytes, position, true);
				// Up to 2032 payload bytes, plus the two-byte size field.
				if (size < 2 || size > 2034) throw xParseError("invalid object map page size");
				auto page = Slice(bytes, position, size);
				if (CRC16(page) != ShortAt(bytes, position + size, true)) throw xParseError("object map CRC mismatch");
				position += size + 2;
				if (size == 2) {
					if (position != bytes.size()) throw xParseError("data after object map terminator");
					return;
				}
				xBitStream stream{page.subspan(2)};
				handle_t handle{};
				std::int64_t offset{};
				while (stream.Remaining()) {
					auto delta = stream.UMC();
					if (!delta || delta > UINT64_MAX - handle) throw xParseError("invalid object handle delta");
					handle += delta;
					auto offsetDelta = stream.MC();
					if (offsetDelta < -offset || offsetDelta > static_cast<std::int64_t>(file.size()) - offset)
						throw xParseError("object offset outside file");
					offset += offsetDelta;
					if (offset >= static_cast<std::int64_t>(file.size()) || offset > UINT32_MAX) throw xParseError("invalid object offset");
					if (!seen.insert(handle).second || !offsets.insert(static_cast<std::uint32_t>(offset)).second)
						throw xParseError("duplicate object map entry");
					document.objects.push_back({handle, static_cast<std::uint32_t>(offset)});
				}
			}
			throw xParseError("missing object map terminator");
		}

		struct sCommon {
			size_t handles{};
			std::uint32_t reactors{};
			bool noLinks{}, byLayer{};
			unsigned lineType{}, plotStyle{};
		};

		sCommon EntityHeader(xBitStream& stream, entities::sEntity& entity, bool r2000) {
			sCommon common;
			if (r2000) common.handles = stream.RL();
			if (stream.Handle() != entity.handle) throw xParseError("object handle does not match map");
			ExtendedData(stream);
			if (stream.B()) stream.SkipBytes(stream.RL());
			if (!r2000) common.handles = stream.RL();
			stream.Limit(common.handles);
			entity.mode = static_cast<std::uint8_t>(stream.Bits(2));
			if (entity.mode == 3) throw xParseError("reserved entity space mode");
			common.reactors = Count(stream);
			if (!r2000) common.byLayer = stream.B();
			common.noLinks = stream.B();
			entity.color = stream.BS();
			entity.lineTypeScale = stream.BD();
			if (r2000) { common.lineType = static_cast<unsigned>(stream.Bits(2)); common.plotStyle = static_cast<unsigned>(stream.Bits(2)); }
			entity.invisible = (stream.BS() & 1) != 0;
			if (r2000) entity.lineWeight = stream.RC();
			return common;
		}

		void EntityHandles(xBitStream& stream, entities::sEntity& entity, sCommon const& common, bool r2000) {
			stream.Seek(common.handles);
			auto handle = [&] { return stream.Handle(entity.handle); };
			if (!entity.mode) entity.owner = handle();
			if (common.reactors > stream.Remaining() / 8) throw xParseError("reactor count exceeds handle stream");
			for (std::uint32_t i{}; i < common.reactors; ++i) handle();
			handle(); // extension dictionary
			if (!r2000) { entity.layer = handle(); if (!common.byLayer) handle(); }
			if (!common.noLinks) { entity.previous = handle(); entity.next = handle(); }
			else {
				entity.previous = entity.handle - 1;
				if (entity.handle == UINT64_MAX) throw xParseError("default next handle overflow");
				entity.next = entity.handle + 1;
			}
			if (r2000) {
				entity.layer = handle();
				if (common.lineType == 3) handle();
				if (common.plotStyle == 3) handle();
			}
		}

		void GeometryHandles(xBitStream& stream, entities::sEntity& entity) {
			auto handle = [&] { return stream.Handle(entity.handle); };
			if (auto* poly = std::get_if<entities::sPolyline>(&entity.geometry); poly && (entity.type == 0x0f || entity.type == 0x10)) {
				poly->firstVertex = handle(); poly->lastVertex = handle(); poly->sequenceEnd = handle();
			}
			if (auto* insert = std::get_if<entities::sInsert>(&entity.geometry)) {
				insert->block = handle();
				if (insert->hasAttributes) {
					insert->firstAttribute = handle(); insert->lastAttribute = handle(); insert->sequenceEnd = handle();
				}
			}
			if (stream.Remaining() > 7) throw xParseError("unexpected entity handle data");
		}

		void ThicknessExtrusion(xBitStream& stream, entities::sEntity& entity, bool r2000) {
			entity.thickness = (r2000 && stream.B()) ? 0. : stream.BD();
			entity.extrusion = (r2000 && stream.B()) ? point_t{0., 0., 1.} : Point(stream);
			if (entity.extrusion == point_t{}) throw xParseError("zero extrusion vector");
		}

		void Geometry(xBitStream& stream, entities::sEntity& entity, bool r2000, std::uint16_t type) {
			switch (type) {
			case 4: stream.Text(); break; // BLOCK marker; properties reside in BLOCK_HEADER.
			case 5: case 6: break; // ENDBLK, SEQEND
			case 7: case 8: {
				entities::sInsert insert;
				insert.position = Point(stream);
				if (!r2000) insert.scale = Point(stream);
				else {
					auto flags = stream.Bits(2);
					if (flags == 1) { insert.scale.y = stream.DD(1.); insert.scale.z = stream.DD(1.); }
					else if (flags == 2) { insert.scale.x = stream.RD(); insert.scale.y = insert.scale.z = insert.scale.x; }
					else if (flags == 0) { insert.scale.x = stream.RD(); insert.scale.y = stream.DD(insert.scale.x); insert.scale.z = stream.DD(insert.scale.x); }
				}
				insert.rotation = stream.BD();
				entity.extrusion = Point(stream);
				if (entity.extrusion == point_t{}) throw xParseError("zero INSERT extrusion");
				insert.hasAttributes = stream.B();
				if (type == 8) {
					auto columns = stream.BS(), rows = stream.BS();
					if (columns <= 0 || rows <= 0) throw xParseError("invalid MINSERT array dimensions");
					insert.columns = static_cast<std::uint16_t>(columns); insert.rows = static_cast<std::uint16_t>(rows);
					insert.columnSpacing = stream.BD(); insert.rowSpacing = stream.BD();
				}
				entity.geometry = insert;
				break;
			}
			case 0x0a: case 0x0b: {
				entities::sVertex vertex;
				vertex.flags = stream.RC();
				vertex.position = Point(stream);
				if (type == 0x0a) {
					vertex.startWidth = stream.BD();
					if (vertex.startWidth < 0.) vertex.startWidth = vertex.endWidth = -vertex.startWidth;
					else vertex.endWidth = stream.BD();
					vertex.bulge = stream.BD(); vertex.tangent = stream.BD();
				}
				entity.geometry = vertex;
				break;
			}
			case 0x0f: case 0x10: {
				entities::sPolyline poly;
				poly.is3d = type == 0x10;
				if (poly.is3d) {
					poly.curveType = stream.RC();
					poly.flags = stream.RC();
				}
				else {
					poly.flags = static_cast<std::uint16_t>(stream.BS());
					poly.curveType = static_cast<std::uint16_t>(stream.BS());
					poly.defaultStartWidth = stream.BD(); poly.defaultEndWidth = stream.BD();
					entity.thickness = (r2000 && stream.B()) ? 0. : stream.BD();
					poly.elevation = stream.BD();
					entity.extrusion = (r2000 && stream.B()) ? point_t{0., 0., 1.} : Point(stream);
				}
				poly.closed = (poly.flags & 1) != 0;
				entity.geometry = std::move(poly);
				break;
			}
			case 0x13: {
				entities::sLine line;
				if (!r2000) { line.start = Point(stream); line.end = Point(stream); }
				else {
					bool zeroZ = stream.B();
					line.start.x = stream.RD(); line.end.x = stream.DD(line.start.x);
					line.start.y = stream.RD(); line.end.y = stream.DD(line.start.y);
					if (!zeroZ) { line.start.z = stream.RD(); line.end.z = stream.DD(line.start.z); }
				}
				ThicknessExtrusion(stream, entity, r2000);
				entity.geometry = line;
				break;
			}
			case 0x11: case 0x12: {
				auto center = Point(stream);
				auto radius = stream.BD();
				if (radius < 0.) throw xParseError("negative radius");
				ThicknessExtrusion(stream, entity, r2000);
				if (type == 0x12) entity.geometry = entities::sCircle{center, radius};
				else entity.geometry = entities::sArc{center, radius, stream.BD(), stream.BD()};
				break;
			}
			case 0x1b: {
				entity.geometry = entities::sPoint{Point(stream)};
				ThicknessExtrusion(stream, entity, r2000);
				stream.BD(); // x axis angle
				break;
			}
			case 0x4d: {
				entities::sPolyline line;
				auto flags = stream.BS();
				line.closed = (flags & 512) != 0;
				if (flags & 4) line.constantWidth = stream.BD();
				double elevation = (flags & 8) ? stream.BD() : 0.;
				if (flags & 2) entity.thickness = stream.BD();
				if (flags & 1) entity.extrusion = Point(stream);
				auto points = Count(stream);
				auto bulges = (flags & 16) ? Count(stream) : 0;
				auto widths = (flags & 32) ? Count(stream) : 0;
				if (bulges > points || widths > points || points > stream.Remaining() / (r2000 ? 4 : 128))
					throw xParseError("invalid polyline counts");
				line.points.reserve(points);
				for (std::uint32_t i{}; i < points; ++i) {
					point_t p{0., 0., elevation};
					if (!i || !r2000) { p.x = stream.RD(); p.y = stream.RD(); }
					else { p.x = stream.DD(line.points.back().x); p.y = stream.DD(line.points.back().y); }
					line.points.push_back(p);
				}
				for (std::uint32_t i{}; i < bulges; ++i) line.bulges.push_back(stream.BD());
				for (std::uint32_t i{}; i < widths; ++i) {
					auto start = stream.BD(), end = stream.BD();
					line.widths.emplace_back(start, end);
				}
				entity.geometry = std::move(line);
				break;
			}
			default: throw xParseError("unsupported geometry decoder");
			}
			if (stream.Remaining()) throw xParseError("unexpected entity geometry data");
		}

		sDocument::sBlock Block(xBitStream& stream, xBitStream& handles, sObject const& object, bool r2000) {
			size_t end{};
			if (r2000) end = stream.RL();
			if (stream.Handle() != object.handle) throw xParseError("block handle does not match map");
			ExtendedData(stream);
			if (!r2000) end = stream.RL();
			stream.Limit(end);
			auto reactors = Count(stream);
			sDocument::sBlock block;
			block.handle = object.handle;
			block.name = stream.Text();
			auto name = block.name;
			for (auto& c : name) if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
			block.space = name == "*MODEL_SPACE" ? 2 : name.starts_with("*PAPER_SPACE") ? 1 : 0;
			stream.B(); stream.BS(); stream.B(); // symbol table flags, xref index, dependency
			stream.B(); stream.B(); // anonymous, contains ATTDEFs
			block.xref = stream.B(); block.overlay = stream.B();
			if (r2000) block.unloaded = stream.B();
			block.base = Point(stream);
			block.xrefPath = stream.Text();
			size_t inserts{};
			if (r2000) {
				while (stream.RC()) ++inserts;
				block.description = stream.Text();
				auto preview = Count(stream);
				stream.SkipBytes(preview);
			}
			if (stream.Remaining()) throw xParseError("unexpected block data");
			handles.Seek(end);
			auto handle = [&] { return handles.Handle(object.handle); };
			handle(); // control
			if (reactors > handles.Remaining() / 8) throw xParseError("invalid block reactor count");
			for (std::uint32_t i{}; i < reactors; ++i) handle();
			handle(); handle(); // extension dictionary, external reference
			block.begin = handle();
			if (!block.xref && !block.overlay) { block.first = handle(); block.last = handle(); }
			block.end = handle();
			if (r2000) {
				if (inserts > handles.Remaining() / 8) throw xParseError("invalid block insert count");
				for (size_t i{}; i < inserts; ++i) handle();
				handle(); // layout
			}
			if (handles.Remaining() > 7) throw xParseError("unexpected block handle data");
			return block;
		}

		void Resolve(sDocument& document) {
			std::map<handle_t, entities::sEntity*> entities;
			for (auto& entity : document.entities) entities.emplace(entity.handle, &entity);
			auto find = [&](handle_t handle) -> entities::sEntity& {
				auto found = entities.find(handle);
				if (found == entities.end()) throw xParseError("missing referenced entity " + std::to_string(handle));
				return *found->second;
			};
			auto chain = [&](handle_t first, handle_t last, auto&& append) {
				if (!first && !last) return;
				if (!first || !last) throw xParseError("incomplete entity chain endpoints");
				std::set<handle_t> visited;
				for (auto current = first;;) {
					if (!visited.insert(current).second) throw xParseError("cyclic entity chain");
					auto& entity = find(current);
					append(entity);
					if (current == last) break;
					current = entity.next;
				}
			};
			for (auto& entity : document.entities) {
				try {
				if (!document.layers.contains(entity.layer)) throw xParseError("entity references a missing layer");
				if (auto* poly = std::get_if<entities::sPolyline>(&entity.geometry); poly && (entity.type == 0x0f || entity.type == 0x10)) {
					auto& seq = find(poly->sequenceEnd);
					if (seq.type != 6 || seq.owner != entity.handle) throw xParseError("invalid POLYLINE sequence end");
					chain(poly->firstVertex, poly->lastVertex, [&](entities::sEntity& child) {
						auto vertex = std::get_if<entities::sVertex>(&child.geometry);
						if (!vertex || child.owner != entity.handle || child.type != (poly->is3d ? 0x0b : 0x0a))
							throw xParseError("invalid POLYLINE vertex ownership or type");
						auto point = vertex->position;
						if (!poly->is3d) point.z = poly->elevation;
						poly->points.push_back(point);
						poly->bulges.push_back(vertex->bulge);
						poly->widths.emplace_back(vertex->startWidth, vertex->endWidth);
					});
				}
				if (auto* insert = std::get_if<entities::sInsert>(&entity.geometry)) {
					if (!document.blocks.contains(insert->block)) throw xParseError("INSERT references a missing block");
					if (insert->hasAttributes) {
						auto& seq = find(insert->sequenceEnd);
						if (seq.type != 6 || seq.owner != entity.handle) throw xParseError("invalid INSERT sequence end");
						chain(insert->firstAttribute, insert->lastAttribute, [&](entities::sEntity& child) {
							if (child.type != 2 || child.owner != entity.handle) throw xParseError("invalid INSERT attribute ownership");
						});
					}
				}
				} catch (xParseError const& error) {
					throw xParseError("entity " + std::to_string(entity.handle) + ": " + error.what());
				}
			}
			for (auto& [handle, block] : document.blocks) {
				try {
				if (block.xref || block.overlay) continue;
				auto owns = [&](entities::sEntity const& child) {
					return (child.mode == 0 && child.owner == handle) || (child.mode != 0 && child.mode == block.space);
				};
				auto const& begin = find(block.begin);
				auto const& end = find(block.end);
				if (begin.type != 4 || end.type != 5 || !owns(begin) || !owns(end))
					throw xParseError("invalid BLOCK/ENDBLK ownership or type");
				chain(block.first, block.last, [&](entities::sEntity& child) {
					bool own = owns(child);
					if (own) block.entities.push_back(child.handle);
					else {
						auto const& parent = find(child.owner);
						if (!owns(parent))
							throw xParseError("entity belongs to a different block");
					}
				});
				} catch (xParseError const& error) {
					throw xParseError("block " + std::to_string(handle) + " (" + block.name + "): " + error.what());
				}
			}
		}

		sLayer Layer(xBitStream& stream, xBitStream& handles, sObject const& object, bool r2000) {
			size_t end{};
			if (r2000) end = stream.RL();
			if (stream.Handle() != object.handle) throw xParseError("layer handle does not match map");
			ExtendedData(stream);
			if (!r2000) end = stream.RL();
			stream.Limit(end);
			auto reactors = Count(stream);
			sLayer layer;
			layer.handle = object.handle;
			layer.name = stream.Text();
			if (stream.B()) layer.flags |= 64;
			stream.BS(); // xref index + 1
			if (stream.B()) layer.flags |= 16;
			if (r2000) {
				auto flags = static_cast<std::uint16_t>(stream.BS());
				layer.frozen = (flags & 1) != 0;
				layer.off = (flags & 2) != 0;
				if (flags & 4) layer.flags |= 2;
				layer.locked = (flags & 8) != 0;
				layer.plot = (flags & 16) != 0;
				layer.lineWeight = static_cast<std::uint8_t>((flags >> 5) & 31);
			}
			else {
				layer.frozen = stream.B();
				layer.off = stream.B();
				if (stream.B()) layer.flags |= 2;
				layer.locked = stream.B();
			}
			if (layer.frozen) layer.flags |= 1;
			if (layer.locked) layer.flags |= 4;
			layer.color = stream.BS();
			if (stream.Remaining()) throw xParseError("unexpected layer data");
			handles.Seek(end);
			handles.Handle(object.handle);
			if (reactors > handles.Remaining() / 8) throw xParseError("invalid layer reactor count");
			for (std::uint32_t i{}; i < reactors; ++i) handles.Handle(object.handle);
			handles.Handle(object.handle); // dictionary
			handles.Handle(object.handle); // xref
			if (r2000) handles.Handle(object.handle); // plotstyle
			handles.Handle(object.handle); // linetype
			// Some legacy writers append a null handle after the linetype.
			if (handles.Remaining() >= 8 && handles.Handle(object.handle)) throw xParseError("unexpected layer handle");
			if (handles.Remaining() > 7) throw xParseError("unexpected layer handle data");
			return layer;
		}
	}

	sDocument ReadLegacy(std::span<std::uint8_t const> bytes) {
		sDocument document;
		document.version = DetectVersion(bytes);
		bool const r2000 = document.version == eVERSION::r2000;
		if (!r2000 && document.version != eVERSION::r14) throw xParseError("unsupported legacy version");
		xBitStream header{bytes};
		header.Seek(0x13 * 8);
		document.codepage = header.RS();
		auto count = header.RL();
		if (count < 3 || count > 6) throw xParseError("invalid section count");
		std::set<unsigned> numbers;
		for (std::uint32_t i{}; i < count; ++i) {
			sSection section{header.RC(), header.RL(), header.RL()};
			Slice(bytes, section.offset, section.size);
			if (!numbers.insert(section.number).second) throw xParseError("duplicate section number");
			document.sections.push_back(section);
		}
		constexpr std::array<std::uint16_t, 4> masks{0xa598, 0x8101, 0x3cc4, 0x8461};
		auto headerEnd = header.Position() / 8;
		if ((CRC16(bytes.first(headerEnd), 0) ^ masks[count - 3]) != header.RS()) throw xParseError("file header CRC mismatch");
		constexpr std::array<std::uint8_t, 16> sentinel{0x95,0xa0,0x4e,0x28,0x99,0x82,0x1a,0xe5,0x5e,0x41,0xe0,0x5f,0x9d,0x3a,0x4d,0x00};
		for (auto byte : sentinel) if (header.RC() != byte) throw xParseError("invalid file header sentinel");
		auto map = std::ranges::find(document.sections, 2, &sSection::number);
		if (map == document.sections.end()) throw xParseError("missing object map");
		auto classes = std::ranges::find(document.sections, 1, &sSection::number);
		if (classes == document.sections.end()) throw xParseError("missing classes section");
		ReadClasses(bytes, *classes, document);
		ReadMap(bytes, *map, document);
		for (auto& object : document.objects) {
			try {
				xBitStream sizeStream{bytes.subspan(object.offset)};
				object.size = sizeStream.MS();
				auto prefix = sizeStream.Position() / 8;
				auto payload = Slice(bytes, object.offset + prefix, object.size);
				if (CRC16(Slice(bytes, object.offset, prefix + object.size)) != ShortAt(bytes, object.offset + prefix + object.size))
					throw xParseError("object CRC mismatch");
				xBitStream stream{payload}, handles{payload};
				object.type = static_cast<std::uint16_t>(stream.BS());
				auto type = object.type;
				if (type >= 500 && type != 0x1f2 && type != 0x1f3) {
					auto found = document.classes.find(type);
					if (found == document.classes.end()) throw xParseError("undefined object class");
					if (found->second.dxfName == "LWPOLYLINE" && found->second.itemClass == 0x1f2) type = 0x4d;
				}
				if (object.type == 0x33) {
					auto layer = Layer(stream, handles, object, r2000);
					document.layers.emplace(object.handle, std::move(layer));
				}
				else if (object.type == 0x31) {
					auto block = Block(stream, handles, object, r2000);
					document.blocks.emplace(object.handle, std::move(block));
				}
				else if ((type >= 1 && type <= 0x2f && type != 0x2a) || type == 0x4a || type == 0x4d || type == 0x4e || type == 0x1f2 ||
					(document.classes.contains(type) && document.classes.at(type).itemClass == 0x1f2)) {
					entities::sEntity entity;
					entity.handle = object.handle;
					entity.type = object.type;
					auto common = EntityHeader(stream, entity, r2000);
					bool supported = (type >= 4 && type <= 8) || type == 0x0a || type == 0x0b || type == 0x0f || type == 0x10 ||
						type == 0x11 || type == 0x12 || type == 0x13 || type == 0x1b || type == 0x4d;
					if (supported) Geometry(stream, entity, r2000, type);
					EntityHandles(handles, entity, common, r2000);
					if (supported) GeometryHandles(handles, entity);
					else document.diagnostics.push_back({object.handle, object.type, "entity geometry not decoded"});
					document.entities.push_back(std::move(entity));
				}
				else document.diagnostics.push_back({object.handle, object.type, "object payload not decoded"});
			}
			catch (xParseError const& error) {
				throw xParseError("object " + std::to_string(object.handle) + " at byte " + std::to_string(object.offset) + ": " + error.what());
			}
		}
		Resolve(document);
		return document;
	}

}

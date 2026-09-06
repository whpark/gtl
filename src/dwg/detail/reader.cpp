#include "reader.h"
#include "bit_stream.h"

#include <algorithm>
#include <set>
#include <string_view>

namespace gtl::dwg::detail {

	eVERSION DetectVersion(std::span<std::uint8_t const> bytes) {
		if (bytes.size() < 6)
			return eVERSION::unknown;
		std::string_view signature{reinterpret_cast<char const*>(bytes.data()), 6};
		constexpr std::array signatures{"AC1014", "AC1015", "AC1018", "AC1021", "AC1024", "AC1027", "AC1032"};
		for (size_t i{}; i < signatures.size(); ++i)
			if (signature == signatures[i])
				return static_cast<eVERSION>(i + 1);
		return eVERSION::unknown;
	}

	namespace {
		using bytes_t = std::span<std::uint8_t const>;
		bytes_t Slice(bytes_t bytes, size_t offset, size_t size) {
			if (offset > bytes.size() || size > bytes.size() - offset)
				throw xParseError("range outside file");
			return bytes.subspan(offset, size);
		}
		std::uint16_t ShortAt(bytes_t bytes, size_t offset, bool bigEndian = false) {
			auto b = Slice(bytes, offset, 2);
			return bigEndian ? static_cast<std::uint16_t>((b[0] << 8) | b[1])
			                 : static_cast<std::uint16_t>(b[0] | (b[1] << 8));
		}
		point_t Point(xBitStream& stream) {
			return {stream.BD(), stream.BD(), stream.BD()};
		}
		void ExtendedData(xBitStream& stream) {
			for (;;) {
				auto length = stream.BS();
				if (!length)
					return;
				if (length < 0)
					throw xParseError("negative extended data length");
				stream.Handle();
				stream.SkipBytes(length);
			}
		}
		std::uint32_t Count(xBitStream& stream) {
			auto value = stream.BL();
			if (value < 0 || static_cast<size_t>(value) > stream.Remaining())
				throw xParseError("invalid item count");
			return static_cast<std::uint32_t>(value);
		}
		std::uint32_t OwnedCount(xBitStream& stream) {
			auto count = stream.BL();
			if (count < 0)
				throw xParseError("negative owned object count");
			return static_cast<std::uint32_t>(count); // Bounded against the separate handle stream before allocation.
		}
		struct sRevision {
			eVERSION version{eVERSION::unknown};
			size_t handleStart{};
			xBitStream* strings{};
		};
		std::string Text(xBitStream& stream, sRevision const& revision) {
			return revision.version >= eVERSION::r2007 ? revision.strings->UnicodeText() : stream.Text();
		}
		void Split(xBitStream& stream, size_t end, sRevision const& revision) {
			if (revision.version < eVERSION::r2007) {
				stream.Limit(end);
				return;
			}
			if (!end)
				throw xParseError("missing string stream presence flag");
			auto& strings = *revision.strings;
			strings.Seek(end - 1);
			if (!strings.B()) {
				strings.Seek(end - 1);
				strings.Limit(end - 1);
				stream.Limit(end - 1);
				return;
			}
			if (end < 17)
				throw xParseError("truncated string stream size");
			size_t trailer = end - 17;
			strings.Seek(trailer);
			size_t size = strings.RS();
			if (size & 0x8000) {
				if (trailer < 16)
					throw xParseError("truncated extended string size");
				trailer -= 16;
				strings.Seek(trailer);
				size = (size & 0x7fff) | (static_cast<size_t>(strings.RS()) << 15);
			}
			if (size > trailer)
				throw xParseError("string stream exceeds object boundary");
			strings.Seek(trailer - size);
			strings.Limit(trailer);
			stream.Limit(trailer - size);
		}

		void ReadClasses(bytes_t bytes, sSection const& section, sDocument& document) {
			if (!section.size)
				return;
			auto data = Slice(bytes, section.offset, section.size);
			constexpr std::array<std::uint8_t, 16> sentinel{0x8d, 0xa1, 0xc4, 0xb8, 0xc4, 0xa9, 0xf8, 0xc5,
			                                                0xc0, 0xdc, 0xf4, 0x5f, 0xe7, 0xcf, 0xb6, 0x8a};
			xBitStream head{data};
			for (auto value : sentinel)
				if (head.RC() != value)
					throw xParseError("invalid classes sentinel");
			auto size = head.RL();
			bool modern = document.version >= eVERSION::r2004;
			size_t dataStart = 20;
			if (document.version >= eVERSION::r2010 && data.size() >= 50 && size == data.size() - 50) {
				if (head.RL())
					throw xParseError("oversized classes section");
				dataStart = 24;
			}
			if (data.size() < 38 ||
			    (size != data.size() - (dataStart + 18) &&
			     !(modern && data.size() >= dataStart + 26 && size == data.size() - (dataStart + 26))))
				throw xParseError("invalid classes section size");
			if (CRC16(Slice(data, 16, size + dataStart - 16)) != ShortAt(data, dataStart + size))
				throw xParseError("classes CRC mismatch");
			for (size_t i{}; i < sentinel.size(); ++i)
				if (data[dataStart + 2 + size + i] != static_cast<std::uint8_t>(~sentinel[i]))
					throw xParseError("invalid classes end sentinel");
			xBitStream stream{Slice(data, dataStart, size)}, strings{Slice(data, dataStart, size)};
			sRevision revision{document.version, 0, &strings};
			if (document.version >= eVERSION::r2007) {
				auto end = stream.RL();
				Split(stream, end, revision);
			}
			std::uint16_t maximum{};
			if (modern) {
				maximum = static_cast<std::uint16_t>(stream.BS());
				stream.RC();
				stream.RC();
				stream.B();
			}
			while (stream.Remaining() >= 8) {
				sClass item;
				item.number = static_cast<std::uint16_t>(stream.BS());
				item.proxyFlags = static_cast<std::uint16_t>(stream.BS());
				item.application = Text(stream, revision);
				item.cppName = Text(stream, revision);
				item.dxfName = Text(stream, revision);
				item.zombie = stream.B();
				item.itemClass = static_cast<std::uint16_t>(stream.BS());
				if (modern) {
					stream.BL();
					stream.BL();
					stream.BL();
					stream.BL();
					stream.BL();
				}
				if (item.number < 500 || !document.classes.emplace(item.number, std::move(item)).second)
					throw xParseError("invalid or duplicate class number " + std::to_string(item.number) + " at bit " +
					                  std::to_string(stream.Position()));
			}
			if (modern && !document.classes.empty() && document.classes.rbegin()->first != maximum)
				throw xParseError("class maximum disagrees with class records");
			if (document.version >= eVERSION::r2007 && strings.Remaining())
				throw xParseError("unconsumed class strings");
		}

		void ReadMap(bytes_t file, sSection const& section, sDocument& document, size_t addressSpace = 0) {
			if (!addressSpace)
				addressSpace = file.size();
			auto bytes = Slice(file, section.offset, section.size);
			size_t position{};
			bool terminated{};
			std::set<handle_t> seen;
			std::set<std::uint32_t> offsets;
			while (position < bytes.size()) {
				auto size = ShortAt(bytes, position, true);
				// Up to 2032 payload bytes, plus the two-byte size field.
				if (size < 2 || size > (document.version >= eVERSION::r2004 ? 2048 : 2034))
					throw xParseError("invalid object map page size " + std::to_string(size) + " at " +
					                  std::to_string(position));
				auto page = Slice(bytes, position, size);
				if (CRC16(page) != ShortAt(bytes, position + size, true))
					throw xParseError("object map CRC mismatch");
				position += size + 2;
				if (size == 2) {
					if (position != bytes.size() && document.version < eVERSION::r2004)
						throw xParseError("data after object map terminator");
					terminated = true;
					continue;
				}
				if (terminated)
					throw xParseError("object map data after terminator");
				xBitStream stream{page.subspan(2)};
				handle_t handle{};
				std::int64_t offset{};
				while (stream.Remaining()) {
					auto delta = stream.UMC();
					if (!delta || delta > UINT64_MAX - handle)
						throw xParseError("invalid object handle delta");
					handle += delta;
					auto offsetDelta = stream.MC();
					if (offsetDelta < -offset || offsetDelta > static_cast<std::int64_t>(addressSpace) - offset)
						throw xParseError("object offset outside file");
					offset += offsetDelta;
					if (offset >= static_cast<std::int64_t>(addressSpace) || offset > UINT32_MAX)
						throw xParseError("invalid object offset");
					if (!seen.insert(handle).second || !offsets.insert(static_cast<std::uint32_t>(offset)).second)
						throw xParseError("duplicate object map entry");
					document.objects.push_back({handle, static_cast<std::uint32_t>(offset)});
				}
			}
			if (!terminated)
				throw xParseError("missing object map terminator");
		}

		struct sCommon {
			size_t handles{};
			std::uint32_t reactors{};
			bool noLinks{}, byLayer{};
			bool dictionaryMissing{}, colorBook{};
			unsigned lineType{}, plotStyle{};
			unsigned material{};
			std::array<bool, 3> visualStyles{};
		};

		sCommon EntityHeader(xBitStream& stream, entities::sEntity& entity, bool r2000, bool modern = false,
		                     sRevision const& revision = {}, bool embedded = false) {
			sCommon common;
			if (!embedded) {
				if (revision.version >= eVERSION::r2010)
					common.handles = revision.handleStart;
				else if (r2000)
					common.handles = stream.RL();
				if (stream.Handle() != entity.handle)
					throw xParseError("object handle does not match map");
				ExtendedData(stream);
				if (stream.B()) {
					auto length = revision.version >= eVERSION::r2010
					                  ? stream.Raw(static_cast<unsigned>(stream.Bits(3)))
					                  : stream.RL();
					if (length > stream.Remaining() / 8)
						throw xParseError("graphic data exceeds object");
					stream.SkipBytes(static_cast<size_t>(length));
				}
				if (!r2000)
					common.handles = stream.RL();
				Split(stream, common.handles, revision);
			}
			entity.mode = static_cast<std::uint8_t>(stream.Bits(2));
			if (entity.mode == 3)
				throw xParseError("reserved entity space mode");
			common.reactors = OwnedCount(stream);
			if (modern)
				common.dictionaryMissing = stream.B();
			if (revision.version >= eVERSION::r2013)
				stream.B();
			if (!r2000)
				common.byLayer = stream.B();
			common.noLinks = modern || stream.B();
			entity.color = stream.BS();
			if (modern) {
				auto color = static_cast<std::uint16_t>(entity.color);
				common.colorBook = (color & 0x4000) != 0;
				if ((color & 0x8000) && !common.colorBook)
					entity.rgb = static_cast<std::uint32_t>(stream.BL()) & 0xffffff;
				if (color & 0x2000)
					entity.transparency = static_cast<std::uint32_t>(stream.BL());
				entity.color = static_cast<std::int16_t>(color & 0x1ff);
			}
			entity.lineTypeScale = stream.BD();
			if (r2000) {
				common.lineType = static_cast<unsigned>(stream.Bits(2));
				common.plotStyle = static_cast<unsigned>(stream.Bits(2));
			}
			if (revision.version >= eVERSION::r2007) {
				common.material = static_cast<unsigned>(stream.Bits(2));
				stream.RC();
			}
			if (revision.version >= eVERSION::r2010)
				for (auto& present : common.visualStyles)
					present = stream.B();
			entity.invisible = (stream.BS() & 1) != 0;
			if (r2000)
				entity.lineWeight = stream.RC();
			return common;
		}

		void EntityHandles(xBitStream& stream, entities::sEntity& entity, sCommon const& common, bool r2000,
		                   bool modern = false, sRevision const& revision = {}) {
			stream.Seek(common.handles);
			auto handle = [&] { return stream.Handle(entity.handle); };
			if (!entity.mode)
				entity.owner = handle();
			if (common.reactors > stream.Remaining() / 8)
				throw xParseError("reactor count exceeds handle stream");
			for (std::uint32_t i{}; i < common.reactors; ++i)
				handle();
			if (!common.dictionaryMissing)
				handle(); // extension dictionary
			if (!r2000) {
				entity.layer = handle();
				if (!common.byLayer)
					entity.lineType = handle();
				entity.lineTypeMode = common.byLayer ? 0 : 3;
			}
			if (!modern && !common.noLinks) {
				entity.previous = handle();
				entity.next = handle();
			} else {
				entity.previous = entity.handle - 1;
				if (entity.handle == UINT64_MAX)
					throw xParseError("default next handle overflow");
				entity.next = entity.handle + 1;
			}
			if (r2000) {
				if (common.colorBook)
					entity.colorBook = handle();
				entity.layer = handle();
				entity.lineTypeMode = static_cast<std::uint8_t>(common.lineType);
				if (common.lineType == 3)
					entity.lineType = handle();
				if (common.material == 3)
					handle();
				if (common.plotStyle == 3)
					handle();
				for (auto present : common.visualStyles)
					if (present)
						handle();
			}
		}

		void GeometryHandles(xBitStream& stream, entities::sEntity& entity, bool modern = false,
		                     sRevision const& revision = {}) {
			auto handle = [&] { return stream.Handle(entity.handle); };
			if (auto* text = std::get_if<entities::sText>(&entity.geometry)) {
				text->style = handle();
				if (text->embeddedHandles > stream.Remaining() / 8)
					throw xParseError("embedded attribute handles exceed stream");
				for (std::uint32_t i = 0; i < text->embeddedHandles; ++i)
					handle();
			}
			if (auto* dim = std::get_if<entities::sDimension>(&entity.geometry)) {
				dim->style = handle();
				dim->block = handle();
			}
			if (auto* hatch = std::get_if<entities::sHatch>(&entity.geometry))
				for (auto& path : hatch->paths) {
					if (path.boundaryCount > stream.Remaining() / 8)
						throw xParseError("hatch boundary handles exceed stream");
					for (std::uint32_t i = 0; i < path.boundaryCount; ++i)
						path.boundaries.push_back(handle());
				}
			if (auto* text = std::get_if<entities::sMText>(&entity.geometry)) {
				text->style = handle();
				for (std::uint32_t i = 0; i < text->extraHandles; ++i)
					handle();
			}
			if (auto* poly = std::get_if<entities::sPolyline>(&entity.geometry);
			    poly && (entity.type == 0x0f || entity.type == 0x10)) {
				if (modern) {
					if (poly->ownedCount > stream.Remaining() / 8)
						throw xParseError("invalid vertex count");
					for (std::uint32_t i = 0; i < poly->ownedCount; ++i)
						poly->vertices.push_back(handle());
				} else {
					poly->firstVertex = handle();
					poly->lastVertex = handle();
				}
				poly->sequenceEnd = handle();
			}
			if (auto* insert = std::get_if<entities::sInsert>(&entity.geometry)) {
				insert->block = handle();
				if (insert->hasAttributes) {
					if (modern) {
						if (insert->ownedCount > stream.Remaining() / 8)
							throw xParseError("invalid attribute count");
						for (std::uint32_t i = 0; i < insert->ownedCount; ++i)
							insert->attributes.push_back(handle());
					} else {
						insert->firstAttribute = handle();
						insert->lastAttribute = handle();
					}
					insert->sequenceEnd = handle();
				}
			}
			if (stream.Remaining() > 7)
				throw xParseError("unexpected entity handle data");
		}

		void ThicknessExtrusion(xBitStream& stream, entities::sEntity& entity, bool r2000) {
			entity.thickness = (r2000 && stream.B()) ? 0. : stream.BD();
			entity.extrusion = (r2000 && stream.B()) ? point_t{0., 0., 1.} : Point(stream);
			if (entity.extrusion == point_t{})
				throw xParseError("zero extrusion vector");
		}

		entities::sMText MText(xBitStream& stream, sRevision const& revision) {
			entities::sMText text;
			text.insertion = Point(stream);
			text.normal = Point(stream);
			text.xAxis = Point(stream);
			text.width = stream.BD();
			if (revision.version >= eVERSION::r2007)
				text.height = stream.BD();
			text.textHeight = stream.BD();
			text.attachment = stream.BS();
			text.direction = stream.BS();
			text.extentHeight = stream.BD();
			text.extentWidth = stream.BD();
			text.text = Text(stream, revision);
			if (revision.version >= eVERSION::r2000) {
				text.spacingStyle = stream.BS();
				text.lineSpacing = stream.BD();
				stream.B();
			}
			if (revision.version >= eVERSION::r2004) {
				text.backgroundFlags = static_cast<std::uint32_t>(stream.BL());
				if ((text.backgroundFlags & 1) ||
				    (revision.version >= eVERSION::r2018 && (text.backgroundFlags & 16))) {
					text.backgroundScale = stream.BD();
					stream.BS();
					text.backgroundColor = static_cast<std::uint32_t>(stream.BL());
					auto flags = stream.RC();
					if (flags & 1)
						Text(stream, revision);
					if (flags & 2)
						Text(stream, revision);
					text.backgroundTransparency = static_cast<std::uint32_t>(stream.BL());
				}
			}
			if (revision.version >= eVERSION::r2018 && stream.B()) {
				stream.BS();
				stream.B();
				++text.extraHandles;
				stream.BL();
				Point(stream);
				Point(stream);
				for (int i = 0; i < 4; ++i)
					stream.BD();
				text.columnType = stream.BS();
				if (text.columnType) {
					auto count = Count(stream);
					text.columnWidth = stream.BD();
					text.gutter = stream.BD();
					text.autoHeight = stream.B();
					text.reversed = stream.B();
					if (!text.autoHeight && text.columnType == 2)
						for (std::uint32_t i = 0; i < count; ++i)
							text.columnHeights.push_back(stream.BD());
				}
			}
			return text;
		}

		void Geometry(xBitStream& stream, entities::sEntity& entity, bool r2000, std::uint16_t type,
		              bool modern = false, sRevision const& revision = {}) {
			switch (type) {
			case 0x1f:
			case 0x20: {
				entities::sSolid solid;
				entity.thickness = r2000 && stream.B() ? 0. : stream.BD();
				double elevation = stream.BD();
				for (auto& corner : solid.corners)
					corner = {stream.RD(), stream.RD(), elevation};
				entity.extrusion = r2000 && stream.B() ? point_t{0, 0, 1} : Point(stream);
				entity.geometry = solid;
				break;
			}
			case 0x14:
			case 0x15:
			case 0x16:
			case 0x17:
			case 0x18:
			case 0x19:
			case 0x1a: {
				entities::sDimension dim;
				if (revision.version >= eVERSION::r2010)
					dim.version = stream.RC();
				entity.extrusion = Point(stream);
				dim.textMidpoint = {stream.RD(), stream.RD(), stream.BD()};
				dim.flags = stream.RC();
				dim.text = Text(stream, revision);
				dim.textRotation = stream.BD();
				dim.horizontalDirection = stream.BD();
				dim.scale = Point(stream);
				dim.rotation = stream.BD();
				if (r2000) {
					dim.attachment = stream.BS();
					dim.spacingStyle = stream.BS();
					dim.lineSpacing = stream.BD();
					dim.measurement = stream.BD();
				}
				if (revision.version >= eVERSION::r2007) {
					dim.userTextPosition = stream.B();
					dim.flipFirst = stream.B();
					dim.flipSecond = stream.B();
				}
				dim.insertion = {stream.RD(), stream.RD(), dim.textMidpoint.z};
				if (type == 0x18)
					dim.definitionPoints.push_back({stream.RD(), stream.RD(), dim.textMidpoint.z});
				int points = (type == 0x17 || type == 0x18) ? 4 : (type == 0x19 || type == 0x1a) ? 2 : 3;
				for (int i = 0; i < points; ++i)
					dim.definitionPoints.push_back(Point(stream));
				if (type == 0x14)
					dim.ordinateFlags = stream.RC();
				if (type == 0x15 || type == 0x16)
					dim.extensionRotation = stream.BD();
				if (type == 0x15)
					dim.dimensionRotation = stream.BD();
				if (type == 0x19 || type == 0x1a)
					dim.leaderLength = stream.BD();
				entity.geometry = std::move(dim);
				break;
			}
			case 0x4e: {
				entities::sHatch hatch;
				if (modern) {
					hatch.gradient = stream.BL();
					hatch.reserved = stream.BL();
					hatch.gradientAngle = stream.BD();
					hatch.gradientShift = stream.BD();
					hatch.singleColor = stream.BL();
					hatch.gradientTint = stream.BD();
					auto count = Count(stream);
					for (std::uint32_t i = 0; i < count; ++i) {
						double at = stream.BD();
						stream.BS();
						auto color = static_cast<std::uint32_t>(stream.BL());
						stream.RC();
						hatch.gradientColors.emplace_back(at, color);
					}
					hatch.gradientName = Text(stream, revision);
				}
				hatch.elevation = stream.BD();
				entity.extrusion = Point(stream);
				hatch.name = Text(stream, revision);
				hatch.solid = stream.B();
				hatch.associative = stream.B();
				auto count = Count(stream);
				bool pixel = false;
				auto point2 = [&]() { return point_t{stream.RD(), stream.RD(), hatch.elevation}; };
				for (std::uint32_t i = 0; i < count; ++i) {
					entities::sHatch::sPath path;
					path.flags = static_cast<std::uint32_t>(stream.BL());
					pixel |= (path.flags & 4) != 0;
					if (path.flags & 2) {
						bool bulges = stream.B();
						path.polyline.closed = stream.B();
						auto n = Count(stream);
						if (n > stream.Remaining() / 128)
							throw xParseError("hatch vertices exceed data");
						for (std::uint32_t j = 0; j < n; ++j) {
							path.polyline.points.push_back(point2());
							if (bulges)
								path.polyline.bulges.push_back(stream.BD());
						}
					} else {
						auto n = Count(stream);
						for (std::uint32_t j = 0; j < n; ++j) {
							entities::sHatch::sEdge edge;
							auto kind = stream.RC();
							if (kind == 1)
								edge.geometry = entities::sLine{point2(), point2()};
							else if (kind == 2) {
								entities::sArc arc;
								arc.center = point2();
								arc.radius = stream.BD();
								arc.startAngle = stream.BD();
								arc.endAngle = stream.BD();
								edge.ccw = stream.B();
								edge.geometry = arc;
							} else if (kind == 3) {
								entities::sEllipse ellipse;
								ellipse.center = point2();
								ellipse.majorAxis = {stream.RD(), stream.RD(), 0};
								ellipse.ratio = stream.BD();
								ellipse.startAngle = stream.BD();
								ellipse.endAngle = stream.BD();
								edge.ccw = stream.B();
								edge.geometry = ellipse;
							} else if (kind == 4) {
								entities::sSpline spline;
								spline.scenario = 1;
								spline.degree = static_cast<std::uint32_t>(stream.BL());
								spline.rational = stream.B();
								spline.periodic = stream.B();
								auto knots = Count(stream), points = Count(stream);
								for (std::uint32_t k = 0; k < knots; ++k)
									spline.knots.push_back(stream.BD());
								for (std::uint32_t k = 0; k < points; ++k) {
									spline.controlPoints.push_back(point2());
									if (spline.rational)
										spline.weights.push_back(stream.BD());
								}
								if (revision.version >= eVERSION::r2010) {
									auto fits = Count(stream);
									for (std::uint32_t k = 0; k < fits; ++k)
										spline.fitPoints.push_back(point2());
									if (fits) {
										spline.startTangent = {stream.RD(), stream.RD(), 0};
										spline.endTangent = {stream.RD(), stream.RD(), 0};
									}
								}
								edge.geometry = std::move(spline);
							} else
								throw xParseError("invalid hatch edge type");
							path.edges.push_back(std::move(edge));
						}
					}
					path.boundaryCount = OwnedCount(stream);
					hatch.paths.push_back(std::move(path));
				}
				hatch.style = stream.BS();
				hatch.patternType = stream.BS();
				if (!hatch.solid) {
					hatch.angle = stream.BD();
					hatch.scale = stream.BD();
					hatch.doublePattern = stream.B();
					auto n = stream.BS();
					if (n < 0)
						throw xParseError("negative hatch pattern count");
					for (int i = 0; i < n; ++i) {
						entities::sHatch::sPatternLine line;
						line.angle = stream.BD();
						line.base = {stream.BD(), stream.BD(), 0};
						line.offset = {stream.BD(), stream.BD(), 0};
						auto dashes = stream.BS();
						if (dashes < 0)
							throw xParseError("negative hatch dash count");
						for (int j = 0; j < dashes; ++j)
							line.dashes.push_back(stream.BD());
						hatch.patternLines.push_back(std::move(line));
					}
				}
				if (pixel)
					hatch.pixelSize = stream.BD();
				auto seeds = Count(stream);
				if (seeds > stream.Remaining() / 128)
					throw xParseError("hatch seeds exceed data");
				for (std::uint32_t i = 0; i < seeds; ++i)
					hatch.seeds.push_back(point2());
				entity.geometry = std::move(hatch);
				break;
			}
			case 1:
			case 2:
			case 3: {
				entities::sText text;
				unsigned flags = r2000 ? stream.RC() : 0;
				double elevation = r2000 ? ((flags & 1) ? 0. : stream.RD()) : stream.BD();
				text.insertion = {stream.RD(), stream.RD(), elevation};
				text.alignment = text.insertion;
				if (!(flags & 2))
					text.alignment = {r2000 ? stream.DD(text.insertion.x) : stream.RD(),
					                  r2000 ? stream.DD(text.insertion.y) : stream.RD(), elevation};
				entity.extrusion = r2000 && stream.B() ? point_t{0, 0, 1} : Point(stream);
				entity.thickness = r2000 && stream.B() ? 0. : stream.BD();
				if (!(flags & 4))
					text.oblique = r2000 ? stream.RD() : stream.BD();
				if (!(flags & 8))
					text.rotation = r2000 ? stream.RD() : stream.BD();
				text.height = r2000 ? stream.RD() : stream.BD();
				if (!(flags & 16))
					text.widthFactor = r2000 ? stream.RD() : stream.BD();
				text.text = Text(stream, revision);
				if (!(flags & 32))
					text.generation = stream.BS();
				if (!(flags & 64))
					text.horizontal = stream.BS();
				if (!(flags & 128))
					text.vertical = stream.BS();
				if (type != 1) {
					if (revision.version >= eVERSION::r2010)
						text.version = stream.RC();
					if (revision.version >= eVERSION::r2018)
						text.attributeType = stream.RC();
					if (text.attributeType != 1) {
						entities::sEntity embedded;
						auto common = EntityHeader(stream, embedded, true, true, revision, true);
						text.embeddedHandles = (!embedded.mode) + common.reactors + (!common.dictionaryMissing) + 1 +
						                       common.colorBook + (common.lineType == 3) + (common.material == 3) +
						                       (common.plotStyle == 3);
						for (auto flag : common.visualStyles)
							text.embeddedHandles += flag;
						text.multiline = MText(stream, revision);
						text.embeddedHandles += 1 + text.multiline->extraHandles;
						auto bytes = stream.BS();
						if (bytes < 0)
							throw xParseError("negative annotation data size");
						if (bytes) {
							stream.SkipBytes(bytes);
							++text.embeddedHandles;
							stream.BS();
						}
					}
					text.tag = Text(stream, revision);
					text.fieldLength = stream.BS();
					text.flags = stream.RC();
					if (revision.version >= eVERSION::r2007)
						text.lockPosition = stream.B();
					if (type == 3) {
						if (revision.version >= eVERSION::r2010)
							stream.RC();
						text.prompt = Text(stream, revision);
					}
					entity.invisible = entity.invisible || (text.flags & 1);
				}
				if (text.height < 0 || text.widthFactor <= 0)
					throw xParseError("invalid text dimensions");
				entity.geometry = std::move(text);
				break;
			}
			case 0x2c:
				entity.geometry = MText(stream, revision);
				break;
			case 4:
				Text(stream, revision);
				break; // BLOCK marker; properties reside in BLOCK_HEADER.
			case 5:
			case 6:
				break; // ENDBLK, SEQEND
			case 7:
			case 8: {
				entities::sInsert insert;
				insert.position = Point(stream);
				if (!r2000)
					insert.scale = Point(stream);
				else {
					auto flags = stream.Bits(2);
					if (flags == 1) {
						insert.scale.y = stream.DD(1.);
						insert.scale.z = stream.DD(1.);
					} else if (flags == 2) {
						insert.scale.x = stream.RD();
						insert.scale.y = insert.scale.z = insert.scale.x;
					} else if (flags == 0) {
						insert.scale.x = stream.RD();
						insert.scale.y = stream.DD(insert.scale.x);
						insert.scale.z = stream.DD(insert.scale.x);
					}
				}
				insert.rotation = stream.BD();
				entity.extrusion = Point(stream);
				if (entity.extrusion == point_t{})
					throw xParseError("zero INSERT extrusion");
				insert.hasAttributes = stream.B();
				if (modern && insert.hasAttributes)
					insert.ownedCount = OwnedCount(stream);
				if (type == 8) {
					auto columns = stream.BS(), rows = stream.BS();
					if (columns <= 0 || rows <= 0)
						throw xParseError("invalid MINSERT array dimensions");
					insert.columns = static_cast<std::uint16_t>(columns);
					insert.rows = static_cast<std::uint16_t>(rows);
					insert.columnSpacing = stream.BD();
					insert.rowSpacing = stream.BD();
				}
				entity.geometry = insert;
				break;
			}
			case 0x0a:
			case 0x0b: {
				entities::sVertex vertex;
				vertex.flags = stream.RC();
				vertex.position = Point(stream);
				if (type == 0x0a) {
					vertex.startWidth = stream.BD();
					if (vertex.startWidth < 0.)
						vertex.startWidth = vertex.endWidth = -vertex.startWidth;
					else
						vertex.endWidth = stream.BD();
					vertex.bulge = stream.BD();
					if (revision.version >= eVERSION::r2010)
						stream.BL();
					vertex.tangent = stream.BD();
				}
				entity.geometry = vertex;
				break;
			}
			case 0x0f:
			case 0x10: {
				entities::sPolyline poly;
				poly.is3d = type == 0x10;
				if (poly.is3d) {
					poly.curveType = stream.RC();
					poly.flags = stream.RC();
				} else {
					poly.flags = static_cast<std::uint16_t>(stream.BS());
					poly.curveType = static_cast<std::uint16_t>(stream.BS());
					poly.defaultStartWidth = stream.BD();
					poly.defaultEndWidth = stream.BD();
					entity.thickness = (r2000 && stream.B()) ? 0. : stream.BD();
					poly.elevation = stream.BD();
					entity.extrusion = (r2000 && stream.B()) ? point_t{0., 0., 1.} : Point(stream);
				}
				poly.closed = (poly.flags & 1) != 0;
				if (modern)
					poly.ownedCount = OwnedCount(stream);
				entity.geometry = std::move(poly);
				break;
			}
			case 0x13: {
				entities::sLine line;
				if (!r2000) {
					line.start = Point(stream);
					line.end = Point(stream);
				} else {
					bool zeroZ = stream.B();
					line.start.x = stream.RD();
					line.end.x = stream.DD(line.start.x);
					line.start.y = stream.RD();
					line.end.y = stream.DD(line.start.y);
					if (!zeroZ) {
						line.start.z = stream.RD();
						line.end.z = stream.DD(line.start.z);
					}
				}
				ThicknessExtrusion(stream, entity, r2000);
				entity.geometry = line;
				break;
			}
			case 0x11:
			case 0x12: {
				auto center = Point(stream);
				auto radius = stream.BD();
				if (radius < 0.)
					throw xParseError("negative radius");
				ThicknessExtrusion(stream, entity, r2000);
				if (type == 0x12)
					entity.geometry = entities::sCircle{center, radius};
				else
					entity.geometry = entities::sArc{center, radius, stream.BD(), stream.BD()};
				break;
			}
			case 0x24: {
				entities::sSpline spline;
				spline.scenario = static_cast<std::uint32_t>(stream.BL());
				if (revision.version >= eVERSION::r2013) {
					spline.flags = static_cast<std::uint32_t>(stream.BL());
					spline.knotParameter = static_cast<std::uint32_t>(stream.BL());
					spline.scenario = spline.knotParameter == 15 || !(spline.flags & 1) ? 1 : 2;
					spline.closed = (spline.flags & 4) != 0;
				}
				auto degree = stream.BL();
				if (degree < 1)
					throw xParseError("invalid spline degree");
				spline.degree = static_cast<std::uint32_t>(degree);
				std::uint32_t knots{}, controls{}, fits{};
				bool weighted{};
				if (spline.scenario == 1) {
					spline.rational = stream.B();
					spline.closed = stream.B();
					spline.periodic = stream.B();
					spline.knotTolerance = stream.BD();
					spline.controlTolerance = stream.BD();
					knots = Count(stream);
					controls = Count(stream);
					weighted = stream.B();
				} else if (spline.scenario == 2) {
					spline.fitTolerance = stream.BD();
					spline.startTangent = Point(stream);
					spline.endTangent = Point(stream);
					fits = Count(stream);
				} else
					throw xParseError("invalid spline scenario");
				if (spline.knotTolerance < 0 || spline.controlTolerance < 0 || spline.fitTolerance < 0)
					throw xParseError("negative spline tolerance");
				// Minimum encodings: BD >= 2 bits, 3BD >= 6 bits, raw weight = 64 bits.
				if (std::uint64_t{knots} * 2 + std::uint64_t{controls} * (weighted ? 70 : 6) + std::uint64_t{fits} * 6 >
				    stream.Remaining())
					throw xParseError("spline counts exceed data stream");
				for (std::uint32_t i = 0; i < knots; ++i)
					spline.knots.push_back(stream.BD());
				for (std::uint32_t i = 0; i < controls; ++i) {
					spline.controlPoints.push_back(Point(stream));
					if (weighted)
						spline.weights.push_back(stream.RD());
				}
				for (std::uint32_t i = 0; i < fits; ++i)
					spline.fitPoints.push_back(Point(stream));
				if (spline.scenario == 1 &&
				    (controls <= spline.degree || std::uint64_t{controls} + spline.degree + 1 != knots ||
				     !std::ranges::is_sorted(spline.knots) || !(spline.knots[spline.degree] < spline.knots[controls])))
					throw xParseError("invalid spline control points or knot vector");
				entity.geometry = std::move(spline);
				break;
			}
			case 0x23: {
				entities::sEllipse ellipse;
				ellipse.center = Point(stream);
				ellipse.majorAxis = Point(stream);
				entity.extrusion = Point(stream);
				ellipse.ratio = stream.BD();
				ellipse.startAngle = stream.BD();
				ellipse.endAngle = stream.BD();
				if (!(ellipse.ratio > 0.) || ellipse.ratio > 1.)
					throw xParseError("invalid ellipse axis ratio");
				entity.geometry = ellipse;
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
				if (flags & 4)
					line.constantWidth = stream.BD();
				double elevation = (flags & 8) ? stream.BD() : 0.;
				if (flags & 2)
					entity.thickness = stream.BD();
				if (flags & 1)
					entity.extrusion = Point(stream);
				auto points = Count(stream);
				auto bulges = (flags & 16) ? Count(stream) : 0;
				auto vertexIds = revision.version >= eVERSION::r2010 && (flags & 1024) ? Count(stream) : 0;
				auto widths = (flags & 32) ? Count(stream) : 0;
				if (bulges > points || vertexIds > points || widths > points ||
				    points > stream.Remaining() / (r2000 ? 4 : 128))
					throw xParseError("invalid polyline counts");
				line.points.reserve(points);
				for (std::uint32_t i{}; i < points; ++i) {
					point_t p{0., 0., elevation};
					if (!i || !r2000) {
						p.x = stream.RD();
						p.y = stream.RD();
					} else {
						p.x = stream.DD(line.points.back().x);
						p.y = stream.DD(line.points.back().y);
					}
					line.points.push_back(p);
				}
				for (std::uint32_t i{}; i < bulges; ++i)
					line.bulges.push_back(stream.BD());
				for (std::uint32_t i = 0; i < vertexIds; ++i)
					stream.BL();
				for (std::uint32_t i{}; i < widths; ++i) {
					auto start = stream.BD(), end = stream.BD();
					line.widths.emplace_back(start, end);
				}
				entity.geometry = std::move(line);
				break;
			}
			default:
				throw xParseError("unsupported geometry decoder");
			}
			if (stream.Remaining())
				throw xParseError("unexpected entity geometry data");
		}

		sDocument::sBlock Block(xBitStream& stream, xBitStream& handles, sObject const& object, bool r2000,
		                        bool modern = false, sRevision const& revision = {}) {
			size_t end{};
			if (revision.version >= eVERSION::r2010)
				end = revision.handleStart;
			else if (r2000)
				end = stream.RL();
			if (stream.Handle() != object.handle)
				throw xParseError("block handle does not match map");
			ExtendedData(stream);
			if (!r2000)
				end = stream.RL();
			Split(stream, end, revision);
			auto reactors = OwnedCount(stream);
			bool dictionaryMissing = modern && stream.B();
			if (revision.version >= eVERSION::r2013)
				stream.B();
			sDocument::sBlock block;
			block.handle = object.handle;
			block.name = Text(stream, revision);
			auto name = block.name;
			for (auto& c : name)
				if (c >= 'a' && c <= 'z')
					c -= 'a' - 'A';
			block.space = name == "*MODEL_SPACE" ? 2 : name.starts_with("*PAPER_SPACE") ? 1 : 0;
			if (revision.version >= eVERSION::r2007)
				stream.BS();
			else {
				stream.B();
				stream.BS();
				stream.B();
			} // legacy symbol table flags, xref index, dependency
			stream.B();
			stream.B(); // anonymous, contains ATTDEFs
			block.xref = stream.B();
			block.overlay = stream.B();
			if (r2000)
				block.unloaded = stream.B();
			auto owned = modern && !block.xref && !block.overlay ? OwnedCount(stream) : 0;
			block.base = Point(stream);
			block.xrefPath = Text(stream, revision);
			size_t inserts{};
			if (r2000) {
				while (stream.RC())
					++inserts;
				block.description = Text(stream, revision);
				auto preview = Count(stream);
				stream.SkipBytes(preview);
			}
			if (revision.version >= eVERSION::r2007) {
				stream.BS();
				stream.B();
				stream.RC();
			}
			if (stream.Remaining())
				throw xParseError("unexpected block data");
			handles.Seek(end);
			auto handle = [&] { return handles.Handle(object.handle); };
			handle(); // control
			if (reactors > handles.Remaining() / 8)
				throw xParseError("invalid block reactor count");
			for (std::uint32_t i{}; i < reactors; ++i)
				handle();
			if (!dictionaryMissing)
				handle();
			handle(); // external reference
			block.begin = handle();
			if (modern) {
				if (owned > handles.Remaining() / 8)
					throw xParseError("invalid block ownership count");
				for (std::uint32_t i = 0; i < owned; ++i)
					block.entities.push_back(handle());
			} else if (!block.xref && !block.overlay) {
				block.first = handle();
				block.last = handle();
			}
			block.end = handle();
			if (r2000) {
				if (inserts > handles.Remaining() / 8)
					throw xParseError("invalid block insert count");
				for (size_t i{}; i < inserts; ++i)
					handle();
				handle(); // layout
			}
			if (handles.Remaining() > 7)
				throw xParseError("unexpected block handle data");
			return block;
		}

		void Resolve(sDocument& document) {
			std::map<handle_t, entities::sEntity*> entities;
			for (auto& entity : document.entities)
				entities.emplace(entity.handle, &entity);
			auto find = [&](handle_t handle) -> entities::sEntity& {
				auto found = entities.find(handle);
				if (found == entities.end())
					throw xParseError("missing referenced entity " + std::to_string(handle));
				return *found->second;
			};
			auto chain = [&](handle_t first, handle_t last, auto&& append) {
				if (!first && !last)
					return;
				if (!first || !last)
					throw xParseError("incomplete entity chain endpoints");
				std::set<handle_t> visited;
				for (auto current = first;;) {
					if (!visited.insert(current).second)
						throw xParseError("cyclic entity chain");
					auto& entity = find(current);
					append(entity);
					if (current == last)
						break;
					current = entity.next;
				}
			};
			auto owned = [&](handle_t first, handle_t last, std::vector<handle_t> const& list, auto&& append) {
				if (document.version < eVERSION::r2004) {
					chain(first, last, append);
					return;
				}
				std::set<handle_t> seen;
				for (auto handle : list) {
					if (!seen.insert(handle).second)
						throw xParseError("duplicate owned entity");
					append(find(handle));
				}
			};
			for (auto& entity : document.entities) {
				try {
					if (!document.layers.contains(entity.layer))
						throw xParseError("entity references a missing layer");
					if (auto* poly = std::get_if<entities::sPolyline>(&entity.geometry);
					    poly && (entity.type == 0x0f || entity.type == 0x10)) {
						auto& seq = find(poly->sequenceEnd);
						if (seq.type != 6 || seq.owner != entity.handle)
							throw xParseError("invalid POLYLINE sequence end");
						owned(poly->firstVertex, poly->lastVertex, poly->vertices, [&](entities::sEntity& child) {
							auto vertex = std::get_if<entities::sVertex>(&child.geometry);
							if (!vertex || child.owner != entity.handle || child.type != (poly->is3d ? 0x0b : 0x0a))
								throw xParseError("invalid POLYLINE vertex ownership or type");
							auto point = vertex->position;
							if (!poly->is3d)
								point.z = poly->elevation;
							poly->points.push_back(point);
							poly->bulges.push_back(vertex->bulge);
							poly->widths.emplace_back(vertex->startWidth, vertex->endWidth);
						});
					}
					if (auto* insert = std::get_if<entities::sInsert>(&entity.geometry)) {
						if (!document.blocks.contains(insert->block))
							throw xParseError("INSERT references a missing block");
						if (insert->hasAttributes) {
							auto& seq = find(insert->sequenceEnd);
							if (seq.type != 6 || seq.owner != entity.handle)
								throw xParseError("invalid INSERT sequence end");
							owned(insert->firstAttribute, insert->lastAttribute, insert->attributes,
							      [&](entities::sEntity& child) {
								      if (child.type != 2 || child.owner != entity.handle)
									      throw xParseError("invalid INSERT attribute ownership");
								      if (document.version < eVERSION::r2004)
									      insert->attributes.push_back(child.handle);
							      });
						}
					}
				} catch (xParseError const& error) {
					throw xParseError("entity " + std::to_string(entity.handle) + ": " + error.what());
				}
			}
			for (auto& [handle, block] : document.blocks) {
				try {
					if (block.xref || block.overlay)
						continue;
					auto owns = [&](entities::sEntity const& child) {
						return (child.mode == 0 && child.owner == handle) ||
						       (child.mode != 0 && child.mode == block.space);
					};
					auto const& begin = find(block.begin);
					auto const& end = find(block.end);
					if (begin.type != 4 || end.type != 5 || !owns(begin) || !owns(end))
						throw xParseError("invalid BLOCK/ENDBLK ownership or type");
					owned(block.first, block.last, block.entities, [&](entities::sEntity& child) {
						bool own = owns(child);
						if (own) {
							if (document.version < eVERSION::r2004)
								block.entities.push_back(child.handle);
						} else {
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

		sLayer Layer(xBitStream& stream, xBitStream& handles, sObject const& object, bool r2000, bool modern = false,
		             sRevision const& revision = {}) {
			size_t end{};
			if (revision.version >= eVERSION::r2010)
				end = revision.handleStart;
			else if (r2000)
				end = stream.RL();
			if (stream.Handle() != object.handle)
				throw xParseError("layer handle does not match map");
			ExtendedData(stream);
			if (!r2000)
				end = stream.RL();
			Split(stream, end, revision);
			auto reactors = OwnedCount(stream);
			bool dictionaryMissing = modern && stream.B();
			if (revision.version >= eVERSION::r2013)
				stream.B();
			sLayer layer;
			layer.handle = object.handle;
			layer.name = Text(stream, revision);
			if (revision.version >= eVERSION::r2007)
				layer.flags |= static_cast<std::uint16_t>(stream.BS()) & (64 | 16);
			else {
				if (stream.B())
					layer.flags |= 64;
				stream.BS();
				if (stream.B())
					layer.flags |= 16;
			}
			if (r2000) {
				auto flags = static_cast<std::uint16_t>(stream.BS());
				layer.frozen = (flags & 1) != 0;
				layer.off = (flags & 2) != 0;
				if (flags & 4)
					layer.flags |= 2;
				layer.locked = (flags & 8) != 0;
				layer.plot = (flags & 16) != 0;
				layer.lineWeight = static_cast<std::uint8_t>((flags >> 5) & 31);
			} else {
				layer.frozen = stream.B();
				layer.off = stream.B();
				if (stream.B())
					layer.flags |= 2;
				layer.locked = stream.B();
			}
			if (layer.frozen)
				layer.flags |= 1;
			if (layer.locked)
				layer.flags |= 4;
			layer.color = stream.BS();
			if (modern) {
				auto color = static_cast<std::uint32_t>(stream.BL());
				auto flags = stream.RC();
				if (flags & 1)
					Text(stream, revision);
				if (flags & 2)
					Text(stream, revision);
				auto method = color >> 24;
				if (method == 0xc2)
					layer.rgb = color & 0xffffff;
				else if (method == 0xc3)
					layer.color = static_cast<std::int16_t>(color & 0xffff);
				else if (method == 0xc0)
					layer.color = 256;
				else if (method == 0xc1)
					layer.color = 0;
			}
			if (stream.Remaining())
				throw xParseError("unexpected layer data");
			handles.Seek(end);
			handles.Handle(object.handle);
			if (reactors > handles.Remaining() / 8)
				throw xParseError("invalid layer reactor count");
			for (std::uint32_t i{}; i < reactors; ++i)
				handles.Handle(object.handle);
			if (!dictionaryMissing)
				handles.Handle(object.handle); // dictionary
			handles.Handle(object.handle);     // xref
			if (r2000)
				handles.Handle(object.handle); // plotstyle
			if (revision.version >= eVERSION::r2007)
				handles.Handle(object.handle);              // material
			layer.lineType = handles.Handle(object.handle); // linetype
			// Some legacy writers append a null handle after the linetype.
			if (handles.Remaining() >= 8 && handles.Handle(object.handle))
				throw xParseError("unexpected layer handle");
			if (handles.Remaining() > 7)
				throw xParseError("unexpected layer handle data");
			return layer;
		}
	} // namespace

	namespace {
		void Symbol(xBitStream& stream, xBitStream& handles, sObject const& object, sDocument& document,
		            sRevision const& revision) {
			bool r2000 = revision.version >= eVERSION::r2000, modern = revision.version >= eVERSION::r2004;
			size_t end = revision.version >= eVERSION::r2010 ? revision.handleStart : r2000 ? stream.RL() : 0;
			if (stream.Handle() != object.handle)
				throw xParseError("symbol handle mismatch");
			ExtendedData(stream);
			if (!r2000)
				end = stream.RL();
			Split(stream, end, revision);
			auto reactors = OwnedCount(stream);
			bool missing = modern && stream.B();
			if (revision.version >= eVERSION::r2013)
				stream.B();
			auto name = Text(stream, revision);
			if (revision.version >= eVERSION::r2007)
				stream.BS();
			else {
				stream.B();
				stream.BS();
				stream.B();
			}
			sLineType line;
			sTextStyle style;
			if (object.type == 0x35) {
				style.handle = object.handle;
				style.name = name;
				style.vertical = stream.B();
				style.shapeFile = stream.B();
				style.fixedHeight = stream.BD();
				style.widthFactor = stream.BD();
				style.oblique = stream.BD();
				style.generation = stream.RC();
				style.lastHeight = stream.BD();
				style.font = Text(stream, revision);
				style.bigFont = Text(stream, revision);
			} else {
				line.handle = object.handle;
				line.name = name;
				line.description = Text(stream, revision);
				line.length = stream.BD();
				line.alignment = stream.RC();
				auto count = stream.RC();
				bool strings = false;
				for (unsigned i = 0; i < count; ++i) {
					sLineType::sDash dash;
					dash.length = stream.BD();
					dash.shapeCode = stream.BS();
					dash.x = stream.RD();
					dash.y = stream.RD();
					dash.scale = stream.BD();
					dash.rotation = stream.BD();
					dash.flags = stream.BS();
					strings |= (dash.flags & 2) != 0;
					line.dashes.push_back(dash);
				}
				if (revision.version <= eVERSION::r2004 || strings) {
					unsigned size = revision.version <= eVERSION::r2004 ? 256 : 512;
					for (unsigned i = 0; i < size; ++i)
						line.strings.push_back(stream.RC());
				}
			}
			if (stream.Remaining() || (document.unicodeStrings && revision.strings->Remaining()))
				throw xParseError("unexpected symbol data");
			handles.Seek(end);
			handles.Handle(object.handle);
			if (reactors > handles.Remaining() / 8)
				throw xParseError("symbol reactors exceed handles");
			for (std::uint32_t i = 0; i < reactors; ++i)
				handles.Handle(object.handle);
			if (!missing)
				handles.Handle(object.handle);
			handles.Handle(object.handle);
			for (auto& dash : line.dashes)
				dash.style = handles.Handle(object.handle);
			if (handles.Remaining() > 7)
				throw xParseError("unexpected symbol handles");
			if (object.type == 0x35)
				document.textStyles.emplace(object.handle, std::move(style));
			else
				document.lineTypes.emplace(object.handle, std::move(line));
		}

		void ReadHeader(bytes_t bytes, sDocument& document) {
			if (bytes.empty())
				return; // Small parser fixtures may omit header variables.
			constexpr std::array<std::uint8_t, 16> sentinel{0xcf, 0x7b, 0x1f, 0x23, 0xfd, 0xde, 0x38, 0xa9,
			                                                0x5f, 0x7c, 0x68, 0xb8, 0x4e, 0x6d, 0x33, 0x5f};
			xBitStream prefix{bytes};
			for (auto byte : sentinel)
				if (prefix.RC() != byte)
					throw xParseError("invalid header variables sentinel");
			auto size = prefix.RL();
			size_t start = 20;
			if (document.version >= eVERSION::r2010 && bytes.size() >= 42 &&
			    (size == bytes.size() - 42 || (bytes.size() >= 50 && size == bytes.size() - 50))) {
				if (prefix.RL())
					throw xParseError("oversized header variables");
				start = 24;
			}
			auto body = Slice(bytes, start, size);
			if (CRC16(Slice(bytes, 16, start - 16 + size)) != ShortAt(bytes, start + size))
				throw xParseError("header variables CRC mismatch");
			auto suffix = Slice(bytes, start + size + 2, 16);
			for (size_t i = 0; i < 16; ++i)
				if (suffix[i] != static_cast<std::uint8_t>(~sentinel[i]))
					throw xParseError("invalid header variables end sentinel");
			xBitStream stream{body}, strings{body}, handles{body};
			sRevision revision{document.version, 0, &strings};
			if (document.unicodeStrings) {
				auto end = stream.RL();
				handles.Seek(end);
				Split(stream, end, revision);
			}
			struct Field {
				std::string_view kind, name;
				int first, last;
			};
			static constexpr Field fields[]{
#include "header_fields.inc"
			};
			for (auto const& field : fields) {
				auto v = static_cast<int>(document.version);
				if (v < field.first || v > field.last)
					continue;
				try {
					if (field.name == "CPSNID" && std::get<std::int64_t>(document.headerVariables.at("CEPSNTYPE")) != 3)
						continue;
					sDocument::header_value_t value;
					if (field.kind == "B")
						value = std::int64_t{stream.B()};
					else if (field.kind == "RC")
						value = std::int64_t{stream.RC()};
					else if (field.kind == "BS")
						value = std::int64_t{stream.BS()};
					else if (field.kind == "BL")
						value = std::int64_t{stream.BL()};
					else if (field.kind == "BLL")
						value = stream.Raw(static_cast<unsigned>(stream.Bits(3)));
					else if (field.kind == "BD")
						value = stream.BD();
					else if (field.kind == "3BD")
						value = Point(stream);
					else if (field.kind == "2RD")
						value = point_t{stream.RD(), stream.RD(), 0};
					else if (field.kind == "TV" || field.kind == "T")
						value = Text(stream, revision);
					else if (field.kind == "H")
						value = (document.unicodeStrings && field.name != "HANDSEED" ? handles : stream).Handle();
					else if (field.kind == "CMC") {
						auto index = stream.BS();
						value = std::int64_t{index};
						if (document.version >= eVERSION::r2004) {
							value = std::uint64_t{static_cast<std::uint32_t>(stream.BL())};
							auto flags = stream.RC();
							if (flags & 1)
								Text(stream, revision);
							if (flags & 2)
								Text(stream, revision);
						}
					} else
						throw xParseError("unknown header field encoding");
					if (!field.name.empty())
						document.headerVariables[std::string(field.name)] = std::move(value);
				} catch (xParseError const& error) {
					throw xParseError("header variable " + std::string(field.name) + ": " + error.what());
				}
			}
			if (auto found = document.headerVariables.find("INSUNITS"); found != document.headerVariables.end())
				document.insertionUnits = static_cast<std::uint16_t>(std::get<std::int64_t>(found->second));
			if (document.unicodeStrings) {
				// Writers append an empty reserved string and four undocumented
				// bitshorts after the named variables/handle references.
				if (strings.Remaining() == 2 && strings.BS() != 0)
					throw xParseError("invalid reserved header string");
				if (strings.Remaining())
					throw xParseError("unconsumed header strings");
				for (int i = 0; i < 4 && handles.Remaining() > 7; ++i)
					document.headerVariables["TRAILER_" + std::to_string(i)] = std::int64_t{handles.BS()};
				if (handles.Remaining() > 15)
					throw xParseError("unexpected header trailer size " + std::to_string(handles.Remaining()));
			}
		}

		void ReadObjects(bytes_t bytes, sDocument& document) {
			bool r2000 = document.version >= eVERSION::r2000;
			bool modern = document.version >= eVERSION::r2004;
			for (auto& object : document.objects) {
				try {
					xBitStream sizeStream{bytes.subspan(object.offset)};
					object.size = sizeStream.MS();
					std::uint64_t handleBits{};
					if (document.version >= eVERSION::r2010) {
						handleBits = sizeStream.UMC();
						if (handleBits > std::uint64_t{object.size} * 8)
							throw xParseError("handle stream exceeds object");
					}
					auto prefix = sizeStream.Position() / 8;
					auto payload = Slice(bytes, object.offset + prefix, object.size);
					if (CRC16(Slice(bytes, object.offset, prefix + object.size)) !=
					    ShortAt(bytes, object.offset + prefix + object.size))
						throw xParseError("object CRC mismatch");
					xBitStream stream{payload}, handles{payload}, strings{payload};
					sRevision revision{document.version,
					                   static_cast<size_t>(object.size) * 8 - static_cast<size_t>(handleBits),
					                   &strings};
					if (document.version >= eVERSION::r2010) {
						auto code = stream.Bits(2);
						object.type = code == 0   ? stream.RC()
						              : code == 1 ? static_cast<std::uint16_t>(stream.RC() + 0x1f0)
						                          : stream.RS();
					} else
						object.type = static_cast<std::uint16_t>(stream.BS());
					auto type = object.type;
					if (type >= 500 && type != 0x1f2 && type != 0x1f3) {
						auto found = document.classes.find(type);
						if (found == document.classes.end())
							throw xParseError("undefined object class");
						if (found->second.dxfName == "LWPOLYLINE" && found->second.itemClass == 0x1f2)
							type = 0x4d;
						if (found->second.dxfName == "HATCH" && found->second.itemClass == 0x1f2)
							type = 0x4e;
					}
					if (object.type == 0x35 || object.type == 0x39) {
						Symbol(stream, handles, object, document, revision);
					} else if (object.type == 0x33) {
						auto layer = Layer(stream, handles, object, r2000, modern, revision);
						if (document.unicodeStrings && strings.Remaining())
							throw xParseError("unconsumed layer strings");
						document.layers.emplace(object.handle, std::move(layer));
					} else if (object.type == 0x31) {
						auto block = Block(stream, handles, object, r2000, modern, revision);
						if (document.unicodeStrings && strings.Remaining())
							throw xParseError("unconsumed block strings");
						document.blocks.emplace(object.handle, std::move(block));
					} else if ((type >= 1 && type <= 0x2f && type != 0x2a) || type == 0x4a || type == 0x4d ||
					           type == 0x4e || type == 0x1f2 ||
					           (document.classes.contains(type) && document.classes.at(type).itemClass == 0x1f2)) {
						entities::sEntity entity;
						entity.handle = object.handle;
						entity.type = object.type;
						auto common = EntityHeader(stream, entity, r2000, modern, revision);
						bool supported = (type >= 1 && type <= 8) || type == 0x0a || type == 0x0b || type == 0x0f ||
						                 type == 0x10 || type == 0x11 || type == 0x12 || type == 0x13 || type == 0x1b ||
						                 (type >= 0x14 && type <= 0x1a) || type == 0x1f || type == 0x20 ||
						                 type == 0x23 || type == 0x24 || type == 0x2c || type == 0x4d || type == 0x4e;
						if (supported)
							Geometry(stream, entity, r2000, type, modern, revision);
						EntityHandles(handles, entity, common, r2000, modern, revision);
						if (supported) {
							GeometryHandles(handles, entity, modern, revision);
							if (document.unicodeStrings && strings.Remaining())
								throw xParseError("unconsumed entity strings");
						} else
							document.diagnostics.push_back({object.handle, object.type, "entity geometry not decoded"});
						document.entities.push_back(std::move(entity));
					} else
						document.diagnostics.push_back({object.handle, object.type, "object payload not decoded"});
				} catch (xParseError const& error) {
					throw xParseError("object " + std::to_string(object.handle) + " type " +
					                  std::to_string(object.type) + " at byte " + std::to_string(object.offset) + ": " +
					                  error.what());
				}
			}
		}
	} // namespace

	sDocument ReadLegacy(std::span<std::uint8_t const> bytes) {
		sDocument document;
		document.version = DetectVersion(bytes);
		bool const r2000 = document.version == eVERSION::r2000;
		if (!r2000 && document.version != eVERSION::r14)
			throw xParseError("unsupported legacy version");
		xBitStream header{bytes};
		header.Seek(0x13 * 8);
		document.codepage = header.RS();
		auto count = header.RL();
		if (count < 3 || count > 6)
			throw xParseError("invalid section count");
		std::set<unsigned> numbers;
		for (std::uint32_t i{}; i < count; ++i) {
			sSection section{header.RC(), header.RL(), header.RL()};
			Slice(bytes, section.offset, section.size);
			if (!numbers.insert(section.number).second)
				throw xParseError("duplicate section number");
			document.sections.push_back(section);
		}
		constexpr std::array<std::uint16_t, 4> masks{0xa598, 0x8101, 0x3cc4, 0x8461};
		auto headerEnd = header.Position() / 8;
		if ((CRC16(bytes.first(headerEnd), 0) ^ masks[count - 3]) != header.RS())
			throw xParseError("file header CRC mismatch");
		constexpr std::array<std::uint8_t, 16> sentinel{0x95, 0xa0, 0x4e, 0x28, 0x99, 0x82, 0x1a, 0xe5,
		                                                0x5e, 0x41, 0xe0, 0x5f, 0x9d, 0x3a, 0x4d, 0x00};
		for (auto byte : sentinel)
			if (header.RC() != byte)
				throw xParseError("invalid file header sentinel");
		auto map = std::ranges::find(document.sections, 2, &sSection::number);
		if (map == document.sections.end())
			throw xParseError("missing object map");
		auto classes = std::ranges::find(document.sections, 1, &sSection::number);
		if (classes == document.sections.end())
			throw xParseError("missing classes section");
		ReadClasses(bytes, *classes, document);
		for (auto const& section : document.sections) {
			if (section.number == 0 && section.size)
				ReadHeader(Slice(bytes, section.offset, section.size), document);
			if (section.number == 4 && section.size >= 4) {
				xBitStream measurement{Slice(bytes, section.offset, section.size)};
				document.measurement = static_cast<std::uint16_t>(measurement.RL());
			}
		}
		ReadMap(bytes, *map, document);
		ReadObjects(bytes, document);
		Resolve(document);
		return document;
	}

	sDocument ReadContainerObjects(sContainer const& container) {
		if (container.version < eVERSION::r2004)
			throw xParseError("object decoder requires AC1018 or later");
		auto section = [&](char const* name) -> bytes_t {
			auto found = container.sections.find(name);
			if (found == container.sections.end())
				throw xParseError(std::string{"missing section "} + name);
			return found->second.data;
		};
		sDocument document;
		document.version = container.version;
		document.codepage = container.codepage;
		document.unicodeStrings = container.version >= eVERSION::r2007;
		if (auto found = container.sections.find("AcDb:Header"); found != container.sections.end())
			ReadHeader(found->second.data, document);
		if (auto found = container.sections.find("AcDb:Template"); found != container.sections.end()) {
			xBitStream value{found->second.data};
			auto length = value.RS();
			// The specification counts bytes; Unicode writers also use UTF-16 units.
			size_t bytes=length;
			if(document.unicodeStrings && value.Remaining()/8==size_t{length}*2+2)bytes=size_t{length}*2;
			if(value.Remaining()/8!=bytes+2)throw xParseError("invalid template description length");
			value.SkipBytes(bytes);
			document.measurement = value.RS();
		}
		auto objects = section("AcDb:AcDbObjects"), handles = section("AcDb:Handles"),
		     classes = section("AcDb:Classes");
		if (objects.size() < 4)
			throw xParseError("truncated objects section");
		if (objects.size() > UINT32_MAX || handles.size() > UINT32_MAX || classes.size() > UINT32_MAX)
			throw xParseError("section exceeds object address space");
		ReadClasses(classes, {1, 0, static_cast<std::uint32_t>(classes.size())}, document);
		ReadMap(handles, {2, 0, static_cast<std::uint32_t>(handles.size())}, document, objects.size());
		ReadObjects(objects, document);
		Resolve(document);
		return document;
	}

} // namespace gtl::dwg::detail

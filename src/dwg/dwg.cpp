#include "pch.h"
#include "gtl/dwg/dwg.h"
#include "detail/reader.h"
#include "detail/bit_stream.h"
#include "detail/transform.h"
#include "detail/curves.h"
#include "detail/hatch.h"
#include "detail/container.h"
#include "gtl/shape/color_table.h"
#include "gtl/string.h"
#include "gtl/unit.h"
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <set>
#include <functional>

namespace gtl::dwg {
	std::optional<sContainer> ReadDWGContainer(std::filesystem::path const& path, sReadReport* report,
	                                           sContainerOptions const& options) {
		sReadReport status;
		auto fail = [&](eREAD_ERROR error, std::string message) -> std::optional<sContainer> {
			status.error = error;
			status.message = std::move(message);
			if (report)
				*report = std::move(status);
			return std::nullopt;
		};
		try {
			std::ifstream file(path, std::ios::binary | std::ios::ate);
			if (!file)
				return fail(eREAD_ERROR::io, "cannot open DWG file");
			auto size = file.tellg();
			if (size < 0)
				return fail(eREAD_ERROR::io, "cannot determine DWG file size");
			if (static_cast<std::uint64_t>(size) > options.maxFileBytes)
				return fail(eREAD_ERROR::resource_limit, "DWG file byte limit exceeded");
			file.seekg(0);
			std::array<std::uint8_t, 6> signature{};
			if (!file.read(reinterpret_cast<char*>(signature.data()), signature.size()))
				return fail(eREAD_ERROR::invalid_data, "truncated DWG signature");
			status.version = detail::DetectVersion(signature);
			if (status.version < eVERSION::r2004)
				return fail(eREAD_ERROR::unsupported_version,
				            "compressed container API requires AC1018 through AC1032");
			file.seekg(0);
			std::vector<std::uint8_t> bytes(static_cast<size_t>(size));
			if (!file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())))
				return fail(eREAD_ERROR::io, "incomplete DWG file read");
			auto container = detail::DecodeContainer(bytes, options);
			if (report)
				*report = std::move(status);
			return container;
		} catch (detail::xParseError const& error) {
			return fail(eREAD_ERROR::invalid_data, error.what());
		} catch (std::length_error const& error) {
			return fail(eREAD_ERROR::resource_limit, error.what());
		} catch (std::bad_alloc const&) {
			return fail(eREAD_ERROR::resource_limit, "DWG allocation failed");
		} catch (std::filesystem::filesystem_error const& error) {
			return fail(eREAD_ERROR::io, error.what());
		}
	}

	bool xDWG::ReadDWG(std::filesystem::path const& path) {
		m_loaded = false;
		m_document = {};
		m_report = {};
		auto fail = [&](eREAD_ERROR error, std::string message) {
			m_report.error = error;
			m_report.message = std::move(message);
			return false;
		};
		try {
			std::ifstream file(path, std::ios::binary | std::ios::ate);
			if (!file)
				return fail(eREAD_ERROR::io, "cannot open DWG file");
			auto size = file.tellg();
			if (size < 0)
				return fail(eREAD_ERROR::io, "cannot determine DWG file size");
			// Bound allocation before reading an untrusted container into memory.
			constexpr std::streamoff maxFileBytes = 512 * 1024 * 1024;
			if (size > maxFileBytes)
				return fail(eREAD_ERROR::resource_limit, "DWG file exceeds 512 MiB read limit");
			if (size < 6)
				return fail(eREAD_ERROR::invalid_data, "truncated DWG signature");
			file.seekg(0);
			std::array<std::uint8_t, 6> signature;
			if (!file.read(reinterpret_cast<char*>(signature.data()), signature.size()))
				return fail(eREAD_ERROR::io, "cannot read DWG signature");
			m_report.version = detail::DetectVersion(signature);
			if (m_report.version == eVERSION::unknown)
				return fail(eREAD_ERROR::unsupported_version, "unrecognized DWG signature");
			file.seekg(0);
			std::vector<std::uint8_t> bytes(static_cast<size_t>(size));
			if (!file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())))
				return fail(eREAD_ERROR::io, "incomplete DWG file read");
			auto document = m_report.version >= eVERSION::r2004
			                    ? detail::ReadContainerObjects(detail::DecodeContainer(bytes))
			                    : detail::ReadLegacy(bytes);
			m_report.diagnostics = document.diagnostics;
			m_document = std::move(document);
			m_loaded = true;
			return true;
		} catch (detail::xParseError const& error) {
			return fail(eREAD_ERROR::invalid_data, error.what());
		} catch (std::length_error const& error) {
			return fail(eREAD_ERROR::resource_limit, error.what());
		} catch (std::bad_alloc const&) {
			return fail(eREAD_ERROR::resource_limit, "DWG allocation failed");
		} catch (std::filesystem::filesystem_error const& error) {
			return fail(eREAD_ERROR::io, error.what());
		}
	}

	std::optional<gtl::shape::xDrawing> ReadDWGShape(std::filesystem::path const& path, sReadReport* report,
	                                                 sShapeOptions const& options) {
		xDWG dwg;
		if (!dwg.ReadDWG(path)) {
			if (report)
				*report = dwg.GetReport();
			return std::nullopt;
		}
		return ToShape(dwg, report, options);
	}

} // namespace gtl::dwg

namespace gtl::dwg {

	namespace {
		int Codepage(std::uint16_t code) {
			// DWG codepage identifiers are different from Windows codepage numbers.
			switch (code) {
			case 0:
				return 65001;
			case 1:
				return 20127;
			case 22:
			case 38:
				return 932;
			case 24:
			case 41:
				return 950;
			case 25:
			case 40:
				return 949;
			case 26:
			case 42:
				return 1361;
			case 27:
				return 866;
			case 28:
				return 1250;
			case 29:
				return 1251;
			case 30:
				return 1252;
			case 31:
			case 39:
				return 936;
			case 32:
				return 1253;
			case 33:
				return 1254;
			case 34:
				return 1255;
			case 35:
				return 1256;
			case 36:
				return 1257;
			case 37:
				return 874;
			case 44:
				return 1258;
			default:
				return -1;
			}
		}
		gtl::shape::point_t Point(point_t p) {
			return {p.x, p.y, p.z};
		}
		gtl::shape::color_t Color(int index) {
			index = std::abs(index);
			return gtl::shape::colorTable_s[(index > 0 && index < 256) ? index : 7];
		}
		gtl::shape::color_t TrueColor(std::uint32_t rgb) {
			return gtl::ColorRGBA(static_cast<std::uint8_t>(rgb >> 16), static_cast<std::uint8_t>(rgb >> 8),
			                      static_cast<std::uint8_t>(rgb));
		}

		std::unique_ptr<gtl::shape::xShape> Curve(point_t center, double radius, double start, double sweep,
		                                          bool circle, detail::sTransform const& transform,
		                                          bool forceEllipse = false) {
			using namespace detail;
			if (!Planar(transform))
				throw std::runtime_error("tilted or degenerate curve omitted; gtl.shape curves are XY-only");
			auto u = transform.axes[0], v = transform.axes[1];
			double scale = std::max(Length(u), Length(v));
			u = Scale(u, 1. / scale);
			v = Scale(v, 1. / scale);
			double det = u.x * v.y - u.y * v.x;
			if (det == 0.)
				throw std::runtime_error("singular circular curve transform");
			double direction = det < 0. ? -1. : 1.;
			auto point = Point(transform.Point(center));
			if (!forceEllipse && Similarity(transform)) {
				auto value = radius * scale;
				if (!std::isfinite(value))
					throw std::runtime_error("non-finite transformed radius");
				if (circle) {
					auto item = std::make_unique<gtl::shape::xCircle>();
					item->m_ptCenter = point;
					item->m_radius = value;
					item->m_angle_length = gtl::deg_t{direction * 360.};
					return item;
				}
				auto item = std::make_unique<gtl::shape::xArc>();
				auto atStart = Add(Scale(u, std::cos(start)), Scale(v, std::sin(start)));
				item->m_ptCenter = point;
				item->m_radius = value;
				item->m_angle_start = gtl::deg_t{gtl::rad_t{std::atan2(atStart.y, atStart.x)}};
				item->m_angle_length = gtl::deg_t{gtl::rad_t{direction * sweep}};
				return item;
			}
			// Eigenvectors of A*A^T give the ellipse axes. Normalize A first to avoid squaring large scales.
			double xx = u.x * u.x + v.x * v.x, yy = u.y * u.y + v.y * v.y, xy = u.x * u.y + v.x * v.y;
			double angle = .5 * std::atan2(2. * xy, xx - yy);
			double major = std::sqrt(.5 * (xx + yy + std::hypot(xx - yy, 2. * xy)));
			double minor = std::abs(det) / major;
			auto item = std::make_unique<gtl::shape::xEllipse>();
			item->m_ptCenter = point;
			item->m_radius = radius * scale * major;
			item->m_radiusH = radius * scale * minor;
			if (!std::isfinite(item->m_radius) || !std::isfinite(item->m_radiusH))
				throw std::runtime_error("non-finite transformed ellipse");
			auto atStart = Add(Scale(u, std::cos(start)), Scale(v, std::sin(start)));
			double c = std::cos(angle), s = std::sin(angle);
			double parameter =
			    std::atan2((-s * atStart.x + c * atStart.y) / minor, (c * atStart.x + s * atStart.y) / major);
			item->m_angle_first_axis = gtl::deg_t{gtl::rad_t{angle}};
			item->m_angle_start = gtl::deg_t{gtl::rad_t{parameter}};
			item->m_angle_length = gtl::deg_t{gtl::rad_t{direction * sweep}};
			return item;
		}
	} // namespace

	gtl::shape::xDrawing ToShape(xDWG const& dwg, sReadReport* report, sShapeOptions const& options) {
		if (!dwg.IsLoaded())
			throw std::logic_error("ToShape requires a successfully read DWG");
		auto resultReport = dwg.GetReport();
		auto const& document = dwg.GetDocument();
		gtl::shape::xDrawing drawing;
		std::map<handle_t, gtl::shape::xLayer*> layers;
		int codepage = document.unicodeStrings ? 65001 : Codepage(document.codepage);
		auto wide = [&](string_t const& text) {
			if (codepage < 0)
				throw std::runtime_error("unsupported DWG codepage");
			return gtl::ToString<wchar_t, char>(std::string_view{text},
			                                    {.from = static_cast<gtl::eCODEPAGE>(codepage)});
		};
		for (auto const& [handle, source] : document.layers) {
			std::wstring name;
			try {
				if (codepage < 0)
					throw std::runtime_error("unsupported codepage");
				name = gtl::ToString<wchar_t, char>(std::string_view(source.name),
				                                    {.from = static_cast<gtl::eCODEPAGE>(codepage)});
			} catch (std::exception const&) {
				name = L"DWG_LAYER_" + std::to_wstring(handle);
				resultReport.diagnostics.push_back(
				    {handle, 0x33, "layer name could not be decoded; using handle-based name"});
			}
			auto layer = std::make_unique<gtl::shape::xLayer>(name);
			layer->m_color = source.rgb ? TrueColor(*source.rgb) : Color(source.color);
			layer->m_flags = source.flags;
			layer->m_bVisible = !source.off && !source.frozen && source.color >= 0;
			layer->m_bUse = source.plot;
			layer->m_lineWeight = source.lineWeight;
			layers.emplace(handle, layer.get());
			drawing.m_layers.push_back(std::move(layer));
		}
		std::map<handle_t, entities::sEntity const*> entities;
		for (auto const& entity : document.entities)
			entities.emplace(entity.handle, &entity);
		struct sContext {
			detail::sTransform transform;
			handle_t layer{};
			gtl::shape::color_t color = Color(7);
			int lineWeight{31};
			bool visible{true};
			handle_t lineType{};
		};
		std::set<handle_t> activeBlocks;
		size_t visits{};
		bool exhausted{};
		std::function<void(entities::sEntity const&, sContext const&, size_t)> emit;
		emit = [&](entities::sEntity const& source, sContext const& parent, size_t depth) {
			if (exhausted)
				return;
			auto warn = [&](char const* message) {
				resultReport.diagnostics.push_back({source.handle, source.type, message});
			};
			if (visits++ >= options.maxVisits) {
				warn("conversion visit limit reached");
				exhausted = true;
				return;
			}
			if (std::holds_alternative<std::monostate>(source.geometry) ||
			    std::holds_alternative<entities::sVertex>(source.geometry))
				return;
			handle_t layerHandle = source.layer;
			if (depth && document.layers.at(source.layer).name == "0")
				layerHandle = parent.layer;
			auto& layer = *layers.at(layerHandle);
			sContext current{parent.transform, layerHandle};
			current.color = source.rgb            ? TrueColor(*source.rgb)
			                : source.color == 256 ? layer.m_color
			                : source.color == 0   ? parent.color
			                                      : Color(source.color);
			if (source.colorBook)
				warn("color book reference is not resolved; indexed color used");
			if (source.transparency)
				warn("entity transparency omitted from Shape conversion");
			current.lineWeight = source.lineWeight == 29   ? layer.m_lineWeight
			                     : source.lineWeight == 30 ? parent.lineWeight
			                                               : source.lineWeight;
			current.visible = parent.visible && layer.m_bVisible && !source.invisible && source.color >= 0;
			current.lineType = source.lineTypeMode == 0   ? document.layers.at(layerHandle).lineType
			                   : source.lineTypeMode == 1 ? parent.lineType
			                   : source.lineTypeMode == 3 ? source.lineType
			                                              : 0;
			try {
				if (auto text = std::get_if<entities::sText>(&source.geometry); text && text->multiline) {
					if (source.type == 3 && !(text->flags & 2))
						return;
					auto copy = source;
					auto multiline = *text->multiline;
					if (!multiline.style)
						multiline.style = text->style;
					copy.geometry = std::move(multiline);
					emit(copy, parent, depth);
					return;
				}
				if (auto dimension = std::get_if<entities::sDimension>(&source.geometry)) {
					if (!document.blocks.contains(dimension->block)) {
						warn("dimension has no available anonymous block; definition retained");
						return;
					}
					auto copy = source;
					entities::sInsert insert;
					insert.block = dimension->block;
					insert.position = dimension->insertion;
					insert.scale = dimension->scale;
					insert.rotation = dimension->rotation;
					copy.geometry = insert;
					emit(copy, parent, depth);
					return;
				}
				if (auto solid = std::get_if<entities::sSolid>(&source.geometry)) {
					entities::sHatch hatch;
					hatch.solid = true;
					hatch.elevation = solid->corners[0].z;
					entities::sHatch::sPath path;
					path.flags = 3;
					path.polyline.closed = true;
					for (unsigned i : {0u, 1u, 3u, 2u})
						path.polyline.points.push_back(solid->corners[i]);
					hatch.paths.push_back(std::move(path));
					auto copy = source;
					copy.geometry = std::move(hatch);
					emit(copy, parent, depth);
					return;
				}
				if (auto hatch = std::get_if<entities::sHatch>(&source.geometry)) {
					auto copy = source;
					copy.lineTypeMode = 2; // Fill strokes have their own pattern, not the entity linetype.
					auto ocs = detail::OCS(source.extrusion);
					if (!options.hatchBoundaryOnly) {
						auto loops = detail::HatchLoops(*hatch, options.curveTolerance, options.maxCurveSegments);
						if (hatch->solid || hatch->gradient)
							warn("solid/gradient hatch approximated by scanlines using solidHatchSpacing");
						double gradientLow = std::numeric_limits<double>::infinity(), gradientHigh = -gradientLow,
						       gc = std::cos(hatch->gradientAngle), gs = std::sin(hatch->gradientAngle);
						if (hatch->gradient)
							for (auto const& loop : loops)
								for (auto p : loop) {
									double t = p.x * gc + p.y * gs;
									gradientLow = std::min(gradientLow, t);
									gradientHigh = std::max(gradientHigh, t);
								}
						detail::HatchPattern(
						    *hatch, loops, options.solidHatchSpacing, options.maxHatchSegments,
						    [&](point_t a, point_t b) {
							    if (exhausted)
								    throw std::runtime_error("hatch stopped at conversion limit");
							    if (hatch->gradient && !hatch->gradientColors.empty()) {
								    double t =
								        gradientHigh > gradientLow
									        ? std::clamp(((a.x + b.x) * .5 * gc + (a.y + b.y) * .5 * gs - gradientLow) /
									                         (gradientHigh - gradientLow),
									                     0., 1.)
									        : 0.;
								    auto first = hatch->gradientColors.front().second,
								         last = hatch->gradientColors.back().second;
								    std::uint32_t rgb{};
								    for (unsigned shift : {0u, 8u, 16u})
									    rgb |= static_cast<std::uint32_t>(std::lerp(double((first >> shift) & 255),
										                                            double((last >> shift) & 255), t))
									           << shift;
								    copy.rgb = rgb;
							    }
							    copy.geometry = entities::sLine{ocs.Point(a), ocs.Point(b)};
							    emit(copy, parent, depth);
						    });
						return;
					}
					for (auto const& path : hatch->paths) {
						if (path.flags & 2) {
							copy.geometry = path.polyline;
							emit(copy, parent, depth);
						} else
							for (auto const& edge : path.edges) {
								std::visit(
								    [&](auto geometry) {
									    using T = std::decay_t<decltype(geometry)>;
									    if constexpr (std::is_same_v<T, entities::sLine>) {
										    geometry.start = ocs.Point(geometry.start);
										    geometry.end = ocs.Point(geometry.end);
									    } else if constexpr (std::is_same_v<T, entities::sArc>) {
										    if (!edge.ccw)
											    std::swap(geometry.startAngle, geometry.endAngle);
									    } else if constexpr (std::is_same_v<T, entities::sEllipse>) {
										    geometry.center = ocs.Point(geometry.center);
										    geometry.majorAxis = ocs.Vector(geometry.majorAxis);
										    if (!edge.ccw)
											    std::swap(geometry.startAngle, geometry.endAngle);
									    } else if constexpr (std::is_same_v<T, entities::sSpline>) {
										    for (auto& p : geometry.controlPoints)
											    p = ocs.Point(p);
										    for (auto& p : geometry.fitPoints)
											    p = ocs.Point(p);
									    }
									    copy.geometry = std::move(geometry);
									    emit(copy, parent, depth);
								    },
								    edge.geometry);
								if (exhausted)
									break;
							}
						if (exhausted)
							break;
					}
					return;
				}
				if (auto insert = std::get_if<entities::sInsert>(&source.geometry)) {
					if (depth >= std::min<size_t>(options.maxBlockDepth, 64)) {
						warn("block nesting limit reached");
						return;
					}
					if (activeBlocks.contains(insert->block)) {
						warn("cyclic INSERT reference omitted");
						return;
					}
					auto const& block = document.blocks.at(insert->block);
					if (block.xref || block.overlay || block.unloaded) {
						warn("external or unloaded block omitted");
						return;
					}
					activeBlocks.insert(insert->block);
					struct sUnmark {
						std::set<handle_t>& set;
						handle_t handle;
						~sUnmark() { set.erase(handle); }
					} unmark{activeBlocks, insert->block};
					for (unsigned row{}; row < insert->rows && !exhausted; ++row) {
						for (unsigned column{}; column < insert->columns && !exhausted; ++column) {
							// Empty arrays must be bounded too: they might never emit a leaf entity.
							if (visits++ >= options.maxVisits) {
								warn("conversion visit limit reached");
								exhausted = true;
								break;
							}
							current.transform = detail::Compose(
							    parent.transform,
							    detail::Insertion(insert->position, insert->scale, insert->rotation, source.extrusion,
								                  block.base, column * insert->columnSpacing,
								                  row * insert->rowSpacing));
							for (auto handle : block.entities) {
								emit(*entities.at(handle), current, depth + 1);
								if (exhausted)
									break;
							}
							auto attributes = current;
							detail::sTransform offset;
							offset.offset = detail::Insertion({}, {1, 1, 1}, insert->rotation, source.extrusion, {},
							                                  column * insert->columnSpacing, row * insert->rowSpacing)
							                    .offset;
							attributes.transform = detail::Compose(parent.transform, offset);
							for (auto handle : insert->attributes)
								emit(*entities.at(handle), attributes, depth + 1);
						}
					}
					return;
				}
				if (resultReport.convertedEntities >= options.maxEntities) {
					warn("converted entity limit reached");
					exhausted = true;
					return;
				}
				auto transform = parent.transform;
				bool world = std::holds_alternative<entities::sLine>(source.geometry) ||
				             std::holds_alternative<entities::sPoint>(source.geometry) ||
				             std::holds_alternative<entities::sEllipse>(source.geometry) ||
				             std::holds_alternative<entities::sSpline>(source.geometry) ||
				             std::holds_alternative<entities::sMText>(source.geometry);
				if (auto poly = std::get_if<entities::sPolyline>(&source.geometry); poly && poly->is3d)
					world = true;
				if (!world)
					transform = detail::Compose(transform, detail::OCS(source.extrusion));
				std::unique_ptr<gtl::shape::xShape> target = std::visit(
				    [&](auto const& geometry) -> std::unique_ptr<gtl::shape::xShape> {
					    using T = std::decay_t<decltype(geometry)>;
					    if constexpr (std::is_same_v<T, entities::sLine>) {
						    auto item = std::make_unique<gtl::shape::xLine>();
						    item->m_pt0 = Point(transform.Point(geometry.start));
						    item->m_pt1 = Point(transform.Point(geometry.end));
						    return item;
					    } else if constexpr (std::is_same_v<T, entities::sCircle>) {
						    return Curve(geometry.center, geometry.radius, 0., 2 * std::numbers::pi, true, transform);
					    } else if constexpr (std::is_same_v<T, entities::sArc>) {
						    double sweep = std::fmod(geometry.endAngle - geometry.startAngle, 2 * std::numbers::pi);
						    if (sweep < 0)
							    sweep += 2 * std::numbers::pi;
						    return Curve(geometry.center, geometry.radius, geometry.startAngle, sweep, false,
							             transform);
					    } else if constexpr (std::is_same_v<T, entities::sMText>) {
						    double axisLength = detail::Length(geometry.xAxis);
						    if (!(axisLength > 0) || !std::isfinite(axisLength))
							    throw std::runtime_error("invalid MTEXT axis");
						    auto axis = detail::Scale(geometry.xAxis, 1. / axisLength);
						    auto x = transform.Vector(axis);
						    auto normal = detail::OCS(geometry.normal).axes[2];
						    auto y = transform.Vector(detail::Cross(normal, axis));
						    if (std::abs(x.z) + std::abs(y.z) > 1e-10 * (detail::Length(x) + detail::Length(y)))
							    throw std::runtime_error("tilted MTEXT cannot be represented by gtl.shape");
						    double sx = detail::Length(x), det = x.x * y.y - x.y * y.x;
						    if (!(sx > 0) || det == 0)
							    throw std::runtime_error("singular MTEXT transform");
						    double sy = std::abs(det) / sx;
						    auto item = std::make_unique<gtl::shape::xMText>();
						    item->m_pt0 = Point(transform.Point(geometry.insertion));
						    item->m_pt1 = item->m_pt0;
						    item->m_text = wide(geometry.text);
						    item->m_height = geometry.textHeight * sy;
						    item->m_widthScale = sx / sy;
						    item->m_textgen = det < 0 ? 4 : 0;
						    item->m_oblique = gtl::deg_t{gtl::rad_t{std::atan(detail::Dot(x, y) / sx / sy)}};
						    item->m_angle = gtl::deg_t{gtl::rad_t{std::atan2(x.y, x.x)}};
						    item->m_interlin = geometry.lineSpacing;
						    item->SetAttachPoint(static_cast<gtl::shape::xMText::eATTACH>(geometry.attachment));
						    if (auto style = document.textStyles.find(geometry.style);
							    style != document.textStyles.end())
							    item->m_textStyle = wide(style->second.name);
						    warn("MTEXT formatting, wrapping and background require a compatible text renderer");
						    return item;
					    } else if constexpr (std::is_same_v<T, entities::sText>) {
						    if (source.type == 3 && !(geometry.flags & 2))
							    return {}; // Nonconstant ATTDEF is replaced by INSERT attributes.
						    if (!detail::Planar(transform))
							    throw std::runtime_error("tilted text cannot be represented by gtl.shape");
						    auto x = transform.Vector({std::cos(geometry.rotation), std::sin(geometry.rotation), 0});
						    auto y = transform.Vector({-std::sin(geometry.rotation), std::cos(geometry.rotation), 0});
						    double sx = detail::Length(x), det = x.x * y.y - x.y * y.x;
						    if (!(sx > 0) || det == 0)
							    throw std::runtime_error("singular text transform");
						    double sy = std::abs(det) / sx;
						    auto item = std::make_unique<gtl::shape::xText>();
						    item->m_pt0 = Point(transform.Point(geometry.insertion));
						    item->m_pt1 = Point(transform.Point(geometry.alignment));
						    item->m_text = gtl::ToString<wchar_t, char>(
						        geometry.text, {.from = static_cast<gtl::eCODEPAGE>(codepage)});
						    item->m_height = geometry.height * sy;
						    item->m_widthScale = geometry.widthFactor * sx / sy;
						    item->m_angle = gtl::deg_t{gtl::rad_t{std::atan2(x.y, x.x)}};
						    item->m_oblique = gtl::deg_t{
						        gtl::rad_t{std::atan((detail::Dot(x, y) / sx + sx * std::tan(geometry.oblique)) / sy)}};
						    item->m_textgen = geometry.generation ^ (det < 0 ? 4 : 0);
						    item->m_alignHorz = static_cast<gtl::shape::xText::eALIGN_HORZ>(geometry.horizontal);
						    item->m_alignVert = static_cast<gtl::shape::xText::eALIGN_VERT>(geometry.vertical);
						    if (auto style = document.textStyles.find(geometry.style);
							    style != document.textStyles.end())
							    item->m_textStyle = wide(style->second.name);
						    else if (geometry.style) {
							    item->m_textStyle = L"DWG_STYLE_" + std::to_wstring(geometry.style);
							    warn("text style font not resolved");
						    }
						    return item;
					    } else if constexpr (std::is_same_v<T, entities::sSpline>) {
						    if (geometry.scenario == 2) {
							    auto spline = detail::Interpolate(geometry);
							    auto item = std::make_unique<gtl::shape::xSpline>();
							    item->m_degree = static_cast<int>(spline.degree);
							    item->m_knots = spline.knots;
							    for (auto point : spline.controlPoints)
								    item->m_ptsControl.push_back(Point(transform.Point(point)));
							    warn("fit-point spline reconstructed by parameterized interpolation; original fit "
								     "solver is not retained in DWG");
							    return item;
						    }
						    if (!geometry.weights.empty() && (!std::ranges::all_of(geometry.weights, [&](double w) {
							        return w == geometry.weights.front();
						        }) || geometry.weights.front() <= 0.)) {
							    auto item = std::make_unique<gtl::shape::xPolyline>();
							    for (auto p : detail::Tessellate(geometry, transform, options.curveTolerance,
								                                 options.maxCurveSegments))
								    item->m_pts.emplace_back(p.x, p.y, p.z, 0.);
							    item->m_bLoop = geometry.closed;
							    warn("rational spline sampled to polyline using curveTolerance");
							    return item;
						    }
						    auto item = std::make_unique<gtl::shape::xSpline>();
						    item->m_degree = static_cast<int>(geometry.degree);
						    item->m_flags = (geometry.closed ? 1 : 0) | (geometry.periodic ? 2 : 0);
						    item->m_knots = geometry.knots;
						    item->m_toleranceKnot = geometry.knotTolerance;
						    item->m_toleranceControlPoint = geometry.controlTolerance;
						    for (auto p : geometry.controlPoints)
							    item->m_ptsControl.push_back(Point(transform.Point(p)));
						    return item;
					    } else if constexpr (std::is_same_v<T, entities::sEllipse>) {
						    using namespace detail;
						    auto major = geometry.majorAxis;
						    double length = Length(major), normalLength = Length(source.extrusion);
						    if (!(length > 0.) || !std::isfinite(length) || !(normalLength > 0.) ||
							    !std::isfinite(normalLength))
							    throw std::runtime_error("invalid ellipse axis or normal");
						    auto normal = Scale(source.extrusion, 1. / normalLength);
						    if (std::abs(Dot(Scale(major, 1. / length), normal)) > 1e-10)
							    throw std::runtime_error("ellipse axis is not perpendicular to normal");
						    auto minor = Scale(Cross(normal, major), geometry.ratio);
						    sTransform axes{{major, minor, normal}, geometry.center};
						    double sweep = geometry.endAngle - geometry.startAngle;
						    if (std::abs(sweep) >= 2 * std::numbers::pi)
							    sweep = 2 * std::numbers::pi;
						    else if (sweep < 0.)
							    sweep += 2 * std::numbers::pi;
						    return Curve({}, 1., geometry.startAngle, sweep, false, Compose(transform, axes), true);
					    } else if constexpr (std::is_same_v<T, entities::sPoint>) {
						    auto item = std::make_unique<gtl::shape::xDot>();
						    item->m_pt = Point(transform.Point(geometry.position));
						    return item;
					    } else if constexpr (std::is_same_v<T, entities::sPolyline>) {
						    if (geometry.curveType || (geometry.flags & 6))
							    throw std::runtime_error("fitted POLYLINE omitted; fit evaluation is not implemented");
						    bool curved =
						        std::ranges::any_of(geometry.bulges, [](double value) { return value != 0.; });
						    if (curved && (!detail::Planar(transform) || !detail::Similarity(transform)))
							    throw std::runtime_error(
							        "bulged polyline under tilted or non-uniform transform omitted");
						    double sign = (transform.axes[0].x * transform.axes[1].y -
							               transform.axes[0].y * transform.axes[1].x) < 0.
							                  ? -1.
							                  : 1.;
						    auto item = std::make_unique<gtl::shape::xPolyline>();
						    item->m_bLoop = geometry.closed;
						    for (size_t i{}; i < geometry.points.size(); ++i) {
							    auto point = transform.Point(geometry.points[i]);
							    item->m_pts.emplace_back(point.x, point.y, point.z,
								                         i < geometry.bulges.size() ? sign * geometry.bulges[i] : 0.);
						    }
						    if (geometry.constantWidth != 0. || geometry.defaultStartWidth != 0. ||
							    geometry.defaultEndWidth != 0. || std::ranges::any_of(geometry.widths, [](auto widths) {
							        return widths.first != 0. || widths.second != 0.;
						        }))
							    warn("polyline widths omitted from centerline conversion");
						    return item;
					    } else
						    return {};
				    },
				    source.geometry);
				if (!target)
					return;
				target->m_strLayer = layer.m_name;
				target->m_bVisible = current.visible;
				target->m_lineWeight = current.lineWeight;
				target->m_color = current.color;
				if (auto lineType = document.lineTypes.find(current.lineType); lineType != document.lineTypes.end()) {
					target->m_strLineType = wide(lineType->second.name);
					if (!lineType->second.dashes.empty() && !std::holds_alternative<entities::sText>(source.geometry) &&
					    !std::holds_alternative<entities::sMText>(source.geometry)) {
						if (std::ranges::any_of(lineType->second.dashes,
						                        [](auto const& dash) { return (dash.flags & 6) != 0; }))
							warn("complex linetype glyphs retained in document; dash strokes rendered");
						double scale = source.lineTypeScale * detail::Length(parent.transform.axes[0]);
						if (auto field = document.headerVariables.find("LTSCALE");
						    field != document.headerVariables.end())
							scale *= std::get<double>(field->second);
						std::vector<double> pattern;
						double period{};
						for (auto const& dash : lineType->second.dashes) {
							pattern.push_back(dash.length * scale);
							period += std::abs(pattern.back());
						}
						if (!(period > 0) || !std::isfinite(period))
							throw std::runtime_error("invalid resolved linetype scale or pattern");
						struct Stroke : gtl::shape::ICanvas {
							std::vector<std::vector<gtl::shape::point_t>> paths;
							size_t count{}, limit{};
							void MoveTo_Target(gtl::shape::point_t const& p) override {
								if (++count > limit)
									throw std::runtime_error("linetype curve segment limit reached");
								paths.push_back({p});
							}
							void LineTo_Target(gtl::shape::point_t const& p) override {
								if (++count > limit)
									throw std::runtime_error("linetype curve segment limit reached");
								if (paths.empty())
									paths.push_back({p});
								else
									paths.back().push_back(p);
							}
							void PreDraw(gtl::shape::xShape const&) override {}
							void Curve(gtl::shape::point_t center, double a, double b, double axis, double start,
							           double sweep) {
								double samples =
								    std::ceil(std::abs(sweep) * std::sqrt(std::max(std::abs(a), std::abs(b)) /
									                                      (8 * m_target_interpolation_inverval)));
								if (!std::isfinite(samples) || samples > limit)
									throw std::runtime_error("linetype curve segment limit reached");
								auto n = std::max<size_t>(1, static_cast<size_t>(samples));
								for (size_t i = 0; i <= n; ++i) {
									double t = start + sweep * i / n, x = a * std::cos(t), y = b * std::sin(t);
									gtl::shape::point_t p{center.x + std::cos(axis) * x - std::sin(axis) * y,
									                      center.y + std::sin(axis) * x + std::cos(axis) * y, center.z};
									if (i)
										LineTo_Target(p);
									else
										MoveTo_Target(p);
								}
							}
							void Arc(gtl::shape::point_t const& center, double radius, gtl::deg_t start,
							         gtl::deg_t sweep) override {
								Curve(center, radius, radius, 0, (double)(gtl::rad_t)start, (double)(gtl::rad_t)sweep);
							}
							void Ellipse(gtl::shape::point_t const& center, double a, double b, gtl::deg_t axis,
							             gtl::deg_t start, gtl::deg_t sweep) override {
								Curve(center, a, b, (double)(gtl::rad_t)axis, (double)(gtl::rad_t)start,
								      (double)(gtl::rad_t)sweep);
							}
						} stroke;
						if (!(options.curveTolerance > 0) || !std::isfinite(options.curveTolerance))
							throw std::runtime_error("invalid linetype curve tolerance");
						stroke.limit = options.maxCurveSegments;
						stroke.m_target_interpolation_inverval = options.curveTolerance;
						target->Draw(stroke);
						for (auto const& path : stroke.paths) {
							size_t index{};
							double remaining = std::abs(pattern[0]);
							for (size_t i = 1; i < path.size() && !exhausted; ++i) {
								auto a = path[i - 1], b = path[i];
								double length = a.Distance(b), at{};
								while (at < length) {
									if (visits++ >= options.maxVisits ||
									    resultReport.convertedEntities >= options.maxEntities) {
										warn("linetype conversion limit reached");
										exhausted = true;
										break;
									}
									double step = std::min(remaining, length - at);
									if (pattern[index] >= 0) {
										auto line = std::make_unique<gtl::shape::xLine>();
										line->m_pt0 = a + (b - a) * (at / length);
										line->m_pt1 = a + (b - a) * ((at + step) / length);
										line->m_strLayer = target->m_strLayer;
										line->m_strLineType = L"CONTINUOUS";
										line->m_color = target->m_color;
										line->m_bVisible = target->m_bVisible;
										line->m_lineWeight = target->m_lineWeight;
										layer.m_shapes.push_back(std::move(line));
										++resultReport.convertedEntities;
									}
									at += step;
									remaining -= step;
									if (remaining <= 0) {
										index = (index + 1) % pattern.size();
										remaining = std::abs(pattern[index]);
									}
								}
							}
						}
						return;
					}
				}
				if (source.thickness != 0.)
					warn("3D thickness omitted from curve conversion");
				layer.m_shapes.push_back(std::move(target));
				++resultReport.convertedEntities;
			} catch (std::runtime_error const& error) {
				warn(error.what());
			}
		};
		for (auto const& source : document.entities) {
			auto space = source.mode;
			if (space == 0 && document.blocks.contains(source.owner))
				space = document.blocks.at(source.owner).space;
			bool selected =
			    (space == 2 && options.space != eSPACE::paper) || (space == 1 && options.space != eSPACE::model);
			if (space == 1 && options.paperBlock) {
				auto block = document.blocks.find(options.paperBlock);
				selected = selected && block != document.blocks.end() &&
				           std::ranges::find(block->second.entities, source.handle) != block->second.entities.end();
			}
			if (selected)
				emit(source, sContext{}, 0);
		}
		drawing.m_rectBoundary.SetRectEmptyForMinMax();
		if (!drawing.UpdateBoundary(drawing.m_rectBoundary))
			drawing.m_rectBoundary = {};
		if (report)
			*report = std::move(resultReport);
		return drawing;
	}

} // namespace gtl::dwg

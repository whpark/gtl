#include "pch.h"
#include "gtl/dwg/dwg.h"
#include "detail/reader.h"
#include "detail/bit_stream.h"
#include "detail/transform.h"
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
	std::optional<sContainer> ReadDWGContainer(std::filesystem::path const& path,sReadReport* report,sContainerOptions const& options) {
		sReadReport status;
		auto fail=[&](eREAD_ERROR error,std::string message)->std::optional<sContainer>{
			status.error=error;status.message=std::move(message);if(report)*report=std::move(status);return std::nullopt;
		};
		try {
			std::ifstream file(path,std::ios::binary|std::ios::ate);
			if(!file)return fail(eREAD_ERROR::io,"cannot open DWG file");
			auto size=file.tellg();if(size<0)return fail(eREAD_ERROR::io,"cannot determine DWG file size");
			if(static_cast<std::uint64_t>(size)>options.maxFileBytes)return fail(eREAD_ERROR::resource_limit,"DWG file byte limit exceeded");
			file.seekg(0);std::array<std::uint8_t,6> signature{};
			if(!file.read(reinterpret_cast<char*>(signature.data()),signature.size()))return fail(eREAD_ERROR::invalid_data,"truncated DWG signature");
			status.version=detail::DetectVersion(signature);
			if(status.version<eVERSION::r2004)return fail(eREAD_ERROR::unsupported_version,"compressed container API requires AC1018 through AC1032");
			file.seekg(0);std::vector<std::uint8_t> bytes(static_cast<size_t>(size));
			if(!file.read(reinterpret_cast<char*>(bytes.data()),static_cast<std::streamsize>(bytes.size())))return fail(eREAD_ERROR::io,"incomplete DWG file read");
			auto container=detail::DecodeContainer(bytes,options);
			if(report)*report=std::move(status);return container;
		}catch(detail::xParseError const& error){return fail(eREAD_ERROR::invalid_data,error.what());}
		catch(std::length_error const& error){return fail(eREAD_ERROR::resource_limit,error.what());}
		catch(std::bad_alloc const&){return fail(eREAD_ERROR::resource_limit,"DWG allocation failed");}
		catch(std::filesystem::filesystem_error const& error){return fail(eREAD_ERROR::io,error.what());}
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
			if (!file) return fail(eREAD_ERROR::io, "cannot open DWG file");
			auto size = file.tellg();
			if (size < 0) return fail(eREAD_ERROR::io, "cannot determine DWG file size");
			// Bound allocation before reading an untrusted container into memory.
			constexpr std::streamoff maxFileBytes = 512 * 1024 * 1024;
			if (size > maxFileBytes) return fail(eREAD_ERROR::resource_limit, "DWG file exceeds 512 MiB read limit");
			if (size < 6) return fail(eREAD_ERROR::invalid_data, "truncated DWG signature");
			file.seekg(0);
			std::array<std::uint8_t, 6> signature;
			if (!file.read(reinterpret_cast<char*>(signature.data()), signature.size())) return fail(eREAD_ERROR::io, "cannot read DWG signature");
			m_report.version = detail::DetectVersion(signature);
			if (m_report.version == eVERSION::unknown) return fail(eREAD_ERROR::unsupported_version, "unrecognized DWG signature");
			if (m_report.version != eVERSION::r14 && m_report.version != eVERSION::r2000)
				return fail(eREAD_ERROR::unsupported_version, "AC1018+ object decoding is not implemented; use ReadDWGContainer for decompressed sections");
			file.seekg(0);
			std::vector<std::uint8_t> bytes(static_cast<size_t>(size));
			if (!file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())))
				return fail(eREAD_ERROR::io, "incomplete DWG file read");
			auto document = detail::ReadLegacy(bytes);
			m_report.diagnostics = document.diagnostics;
			m_document = std::move(document);
			m_loaded = true;
			return true;
		}
		catch (detail::xParseError const& error) { return fail(eREAD_ERROR::invalid_data, error.what()); }
		catch (std::bad_alloc const&) { return fail(eREAD_ERROR::resource_limit, "DWG allocation failed"); }
		catch (std::filesystem::filesystem_error const& error) { return fail(eREAD_ERROR::io, error.what()); }
	}

	std::optional<gtl::shape::xDrawing> ReadDWGShape(std::filesystem::path const& path, sReadReport* report, sShapeOptions const& options) {
		xDWG dwg;
		if (!dwg.ReadDWG(path)) {
			if (report) *report = dwg.GetReport();
			return std::nullopt;
		}
		return ToShape(dwg, report, options);
	}

}

namespace gtl::dwg {

	namespace {
		int Codepage(std::uint16_t code) {
			// DWG codepage identifiers are different from Windows codepage numbers.
			switch (code) {
			case 0: return 65001;
			case 1: return 20127;
			case 22: case 38: return 932;
			case 24: case 41: return 950;
			case 25: case 40: return 949;
			case 26: case 42: return 1361;
			case 27: return 866;
			case 28: return 1250;
			case 29: return 1251;
			case 30: return 1252;
			case 31: case 39: return 936;
			case 32: return 1253;
			case 33: return 1254;
			case 34: return 1255;
			case 35: return 1256;
			case 36: return 1257;
			case 37: return 874;
			case 44: return 1258;
			default: return -1;
			}
		}
		gtl::shape::point_t Point(point_t p) { return {p.x, p.y, p.z}; }
		gtl::shape::color_t Color(int index) {
			index = std::abs(index);
			return gtl::shape::colorTable_s[(index > 0 && index < 256) ? index : 7];
		}

		std::unique_ptr<gtl::shape::xShape> Curve(point_t center, double radius, double start, double sweep,
			bool circle, detail::sTransform const& transform) {
			using namespace detail;
			if (!Planar(transform)) throw std::runtime_error("tilted circular curve omitted; gtl.shape curves are XY-only");
			auto u = transform.axes[0], v = transform.axes[1];
			double scale = std::max(Length(u), Length(v));
			u = Scale(u, 1./scale); v = Scale(v, 1./scale);
			double det = u.x*v.y - u.y*v.x;
			if (det == 0.) throw std::runtime_error("singular circular curve transform");
			double direction = det < 0. ? -1. : 1.;
			auto point = Point(transform.Point(center));
			if (Similarity(transform)) {
				auto value = radius * scale;
				if (!std::isfinite(value)) throw std::runtime_error("non-finite transformed radius");
				if (circle) {
					auto item = std::make_unique<gtl::shape::xCircle>();
					item->m_ptCenter = point; item->m_radius = value;
					item->m_angle_length = gtl::deg_t{direction*360.};
					return item;
				}
				auto item = std::make_unique<gtl::shape::xArc>();
				auto atStart = Add(Scale(u,std::cos(start)), Scale(v,std::sin(start)));
				item->m_ptCenter = point; item->m_radius = value;
				item->m_angle_start = gtl::deg_t{gtl::rad_t{std::atan2(atStart.y, atStart.x)}};
				item->m_angle_length = gtl::deg_t{gtl::rad_t{direction*sweep}};
				return item;
			}
			// Eigenvectors of A*A^T give the ellipse axes. Normalize A first to avoid squaring large scales.
			double xx = u.x*u.x + v.x*v.x, yy = u.y*u.y + v.y*v.y, xy = u.x*u.y + v.x*v.y;
			double angle = .5*std::atan2(2.*xy, xx-yy);
			double major = std::sqrt(.5*(xx+yy+std::hypot(xx-yy,2.*xy)));
			double minor = std::abs(det)/major;
			auto item = std::make_unique<gtl::shape::xEllipse>();
			item->m_ptCenter = point;
			item->m_radius = radius*scale*major; item->m_radiusH = radius*scale*minor;
			if (!std::isfinite(item->m_radius) || !std::isfinite(item->m_radiusH)) throw std::runtime_error("non-finite transformed ellipse");
			auto atStart = Add(Scale(u,std::cos(start)), Scale(v,std::sin(start)));
			double c = std::cos(angle), s = std::sin(angle);
			double parameter = std::atan2((-s*atStart.x+c*atStart.y)/minor, (c*atStart.x+s*atStart.y)/major);
			item->m_angle_first_axis = gtl::deg_t{gtl::rad_t{angle}};
			item->m_angle_start = gtl::deg_t{gtl::rad_t{parameter}};
			item->m_angle_length = gtl::deg_t{gtl::rad_t{direction*sweep}};
			return item;
		}
	}

	gtl::shape::xDrawing ToShape(xDWG const& dwg, sReadReport* report, sShapeOptions const& options) {
		if (!dwg.IsLoaded()) throw std::logic_error("ToShape requires a successfully read DWG");
		auto resultReport = dwg.GetReport();
		auto const& document = dwg.GetDocument();
		gtl::shape::xDrawing drawing;
		std::map<handle_t, gtl::shape::xLayer*> layers;
		int codepage = Codepage(document.codepage);
		for (auto const& [handle, source] : document.layers) {
			std::wstring name;
			try {
				if (codepage < 0) throw std::runtime_error("unsupported codepage");
				name = gtl::ToString<wchar_t, char>(std::string_view(source.name), {.from = static_cast<gtl::eCODEPAGE>(codepage)});
			}
			catch (std::exception const&) {
				name = L"DWG_LAYER_" + std::to_wstring(handle);
				resultReport.diagnostics.push_back({handle, 0x33, "layer name could not be decoded; using handle-based name"});
			}
			auto layer = std::make_unique<gtl::shape::xLayer>(name);
			layer->m_color = Color(source.color);
			layer->m_flags = source.flags;
			layer->m_bVisible = !source.off && !source.frozen && source.color >= 0;
			layer->m_bUse = source.plot;
			layer->m_lineWeight = source.lineWeight;
			layers.emplace(handle, layer.get());
			drawing.m_layers.push_back(std::move(layer));
		}
		std::map<handle_t, entities::sEntity const*> entities;
		for (auto const& entity : document.entities) entities.emplace(entity.handle, &entity);
		struct sContext {
			detail::sTransform transform;
			handle_t layer{};
			gtl::shape::color_t color = Color(7);
			int lineWeight{31};
			bool visible{true};
		};
		std::set<handle_t> activeBlocks;
		size_t visits{};
		bool exhausted{};
		std::function<void(entities::sEntity const&, sContext const&, size_t)> emit;
		emit = [&](entities::sEntity const& source, sContext const& parent, size_t depth) {
			if (exhausted) return;
			auto warn = [&](char const* message) { resultReport.diagnostics.push_back({source.handle, source.type, message}); };
			if (visits++ >= options.maxVisits) { warn("conversion visit limit reached"); exhausted = true; return; }
			if (std::holds_alternative<std::monostate>(source.geometry) || std::holds_alternative<entities::sVertex>(source.geometry)) return;
			handle_t layerHandle = source.layer;
			if (depth && document.layers.at(source.layer).name == "0") layerHandle = parent.layer;
			auto& layer = *layers.at(layerHandle);
			sContext current{parent.transform, layerHandle};
			current.color = source.color == 256 ? layer.m_color : source.color == 0 ? parent.color : Color(source.color);
			current.lineWeight = source.lineWeight == 29 ? layer.m_lineWeight : source.lineWeight == 30 ? parent.lineWeight : source.lineWeight;
			current.visible = parent.visible && layer.m_bVisible && !source.invisible && source.color >= 0;
			try {
			if (auto insert = std::get_if<entities::sInsert>(&source.geometry)) {
				if (depth >= std::min<size_t>(options.maxBlockDepth, 64)) { warn("block nesting limit reached"); return; }
				if (activeBlocks.contains(insert->block)) { warn("cyclic INSERT reference omitted"); return; }
				auto const& block = document.blocks.at(insert->block);
				if (block.xref || block.overlay || block.unloaded) { warn("external or unloaded block omitted"); return; }
				if (insert->hasAttributes) warn("INSERT attributes are not converted yet");
				activeBlocks.insert(insert->block);
				struct sUnmark { std::set<handle_t>& set; handle_t handle; ~sUnmark() { set.erase(handle); } } unmark{activeBlocks,insert->block};
				for (unsigned row{}; row < insert->rows && !exhausted; ++row) {
					for (unsigned column{}; column < insert->columns && !exhausted; ++column) {
						// Empty arrays must be bounded too: they might never emit a leaf entity.
						if (visits++ >= options.maxVisits) { warn("conversion visit limit reached"); exhausted = true; break; }
						current.transform = detail::Compose(parent.transform, detail::Insertion(insert->position, insert->scale,
							insert->rotation, source.extrusion, block.base, column*insert->columnSpacing, row*insert->rowSpacing));
						for (auto handle : block.entities) {
							emit(*entities.at(handle), current, depth+1);
							if (exhausted) break;
						}
					}
				}
				return;
			}
			if (resultReport.convertedEntities >= options.maxEntities) { warn("converted entity limit reached"); exhausted = true; return; }
			auto transform = parent.transform;
			bool world = std::holds_alternative<entities::sLine>(source.geometry) || std::holds_alternative<entities::sPoint>(source.geometry);
			if (auto poly = std::get_if<entities::sPolyline>(&source.geometry); poly && poly->is3d) world = true;
			if (!world) transform = detail::Compose(transform, detail::OCS(source.extrusion));
			std::unique_ptr<gtl::shape::xShape> target = std::visit([&](auto const& geometry) -> std::unique_ptr<gtl::shape::xShape> {
				using T = std::decay_t<decltype(geometry)>;
				if constexpr (std::is_same_v<T, entities::sLine>) {
					auto item = std::make_unique<gtl::shape::xLine>();
					item->m_pt0 = Point(transform.Point(geometry.start)); item->m_pt1 = Point(transform.Point(geometry.end));
					return item;
				}
				else if constexpr (std::is_same_v<T, entities::sCircle>) {
					return Curve(geometry.center, geometry.radius, 0., 2*std::numbers::pi, true, transform);
				}
				else if constexpr (std::is_same_v<T, entities::sArc>) {
					double sweep = std::fmod(geometry.endAngle - geometry.startAngle, 2 * std::numbers::pi);
					if (sweep < 0) sweep += 2 * std::numbers::pi;
					return Curve(geometry.center, geometry.radius, geometry.startAngle, sweep, false, transform);
				}
				else if constexpr (std::is_same_v<T, entities::sPoint>) {
					auto item = std::make_unique<gtl::shape::xDot>();
					item->m_pt = Point(transform.Point(geometry.position));
					return item;
				}
				else if constexpr (std::is_same_v<T, entities::sPolyline>) {
					if (geometry.curveType || (geometry.flags & 6)) throw std::runtime_error("fitted POLYLINE omitted; fit evaluation is not implemented");
					bool curved = std::ranges::any_of(geometry.bulges, [](double value) { return value != 0.; });
					if (curved && (!detail::Planar(transform) || !detail::Similarity(transform)))
						throw std::runtime_error("bulged polyline under tilted or non-uniform transform omitted");
					double sign = (transform.axes[0].x*transform.axes[1].y-transform.axes[0].y*transform.axes[1].x) < 0. ? -1. : 1.;
					auto item = std::make_unique<gtl::shape::xPolyline>();
					item->m_bLoop = geometry.closed;
					for (size_t i{}; i < geometry.points.size(); ++i) {
						auto point = transform.Point(geometry.points[i]);
						item->m_pts.emplace_back(point.x, point.y, point.z, i < geometry.bulges.size() ? sign*geometry.bulges[i] : 0.);
					}
					if (geometry.constantWidth != 0. || geometry.defaultStartWidth != 0. || geometry.defaultEndWidth != 0. ||
						std::ranges::any_of(geometry.widths, [](auto widths) { return widths.first != 0. || widths.second != 0.; }))
						warn("polyline widths omitted from centerline conversion");
					return item;
				}
				else return {};
			}, source.geometry);
			if (!target) return;
			target->m_strLayer = layer.m_name;
			target->m_bVisible = current.visible;
			target->m_lineWeight = current.lineWeight;
			target->m_color = current.color;
			if (source.thickness != 0.) warn("3D thickness omitted from curve conversion");
			layer.m_shapes.push_back(std::move(target));
			++resultReport.convertedEntities;
			} catch (std::runtime_error const& error) { warn(error.what()); }
		};
		for (auto const& source : document.entities) {
			bool model = source.mode == 2;
			if (source.mode == 0 && document.blocks.contains(source.owner)) model = document.blocks.at(source.owner).space == 2;
			if (model) emit(source, sContext{}, 0);
			else if (source.mode == 1 && !std::holds_alternative<std::monostate>(source.geometry))
				resultReport.diagnostics.push_back({source.handle, source.type, "paper-space entity omitted"});
		}
		drawing.m_rectBoundary.SetRectEmptyForMinMax();
		if (!drawing.UpdateBoundary(drawing.m_rectBoundary)) drawing.m_rectBoundary = {};
		if (report) *report = std::move(resultReport);
		return drawing;
	}

}

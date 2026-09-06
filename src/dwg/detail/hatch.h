#pragma once
#include "curves.h"
#include <numbers>

namespace gtl::dwg::detail {
	inline std::vector<std::vector<point_t>> HatchLoops(entities::sHatch const& hatch, double tolerance, size_t limit) {
		if (!(tolerance > 0) || !std::isfinite(tolerance))
			throw std::runtime_error("invalid hatch boundary tolerance");
		std::vector<std::vector<point_t>> loops;
		size_t points{};
		for (auto const& path : hatch.paths) {
			std::vector<point_t> loop;
			auto add = [&](point_t p) {
				if (++points > limit)
					throw std::runtime_error("hatch boundary segment limit reached");
				loop.push_back(Checked(p));
			};
			auto arc = [&](point_t center, point_t u, point_t v, double start, double sweep) {
				double radius = std::max(Length(u), Length(v));
				double segments = std::ceil(std::abs(sweep) * std::sqrt(radius / (8 * tolerance)));
				if (!std::isfinite(segments) || segments > limit - points)
					throw std::runtime_error("hatch curve segment limit reached");
				auto n = std::max<size_t>(1, static_cast<size_t>(segments));
				for (size_t i = 0; i <= n; ++i) {
					double t = start + sweep * i / n;
					add(Add(center, Add(Scale(u, std::cos(t)), Scale(v, std::sin(t)))));
				}
			};
			auto sweep = [](double start, double end, bool ccw) {
				double value = end - start;
				if (std::abs(value) >= 2 * std::numbers::pi)
					return ccw ? 2 * std::numbers::pi : -2 * std::numbers::pi;
				value = std::fmod(value, 2 * std::numbers::pi);
				if (ccw && value < 0)
					value += 2 * std::numbers::pi;
				if (!ccw && value > 0)
					value -= 2 * std::numbers::pi;
				return value;
			};
			if (path.flags & 2) {
				auto const& poly = path.polyline;
				for (size_t i = 0; i < poly.points.size(); ++i) {
					auto a = poly.points[i], b = poly.points[(i + 1) % poly.points.size()];
					double bulge = i < poly.bulges.size() ? poly.bulges[i] : 0;
					if (!bulge || (!poly.closed && i + 1 == poly.points.size())) {
						add(a);
						continue;
					}
					auto d = Add(b, Scale(a, -1));
					double length = Length(d);
					if (!length) {
						add(a);
						continue;
					}
					auto center = Add(Scale(Add(a, b), .5), Scale({-d.y, d.x, 0}, (1 - bulge * bulge) / (4 * bulge)));
					double radius = length * (1 + bulge * bulge) / (4 * std::abs(bulge));
					arc(center, {radius, 0, 0}, {0, radius, 0}, std::atan2(a.y - center.y, a.x - center.x),
					    4 * std::atan(bulge));
				}
			} else
				for (auto const& edge : path.edges)
					std::visit(
					    [&](auto const& geometry) {
						    using T = std::decay_t<decltype(geometry)>;
						    if constexpr (std::is_same_v<T, entities::sLine>) {
							    add(geometry.start);
							    add(geometry.end);
						    } else if constexpr (std::is_same_v<T, entities::sArc>)
							    arc(geometry.center, {geometry.radius, 0, 0}, {0, geometry.radius, 0},
								    geometry.startAngle, sweep(geometry.startAngle, geometry.endAngle, edge.ccw));
						    else if constexpr (std::is_same_v<T, entities::sEllipse>)
							    arc(geometry.center, geometry.majorAxis,
								    Scale({-geometry.majorAxis.y, geometry.majorAxis.x, 0}, geometry.ratio),
								    geometry.startAngle, sweep(geometry.startAngle, geometry.endAngle, edge.ccw));
						    else
							    for (auto p : Tessellate(geometry, {}, tolerance, limit - points))
								    add(p);
					    },
					    edge.geometry);
			if (loop.size() >= 3) {
				if (loop.front() != loop.back())
					add(loop.front());
				loops.push_back(std::move(loop));
			}
		}
		return loops;
	}

	// Clip each explicit pattern line against all contours. Style 0 uses odd-even
	// islands, 1 keeps the outermost band, and 2 ignores internal islands.
	template <class Emit>
	void HatchPattern(entities::sHatch const& hatch, std::vector<std::vector<point_t>> const& loops,
	                  double solidSpacing, size_t limit, Emit&& emit) {
		if (loops.empty())
			return;
		auto patterns = hatch.patternLines;
		if (hatch.solid || hatch.gradient) {
			if (!(solidSpacing > 0) || !std::isfinite(solidSpacing))
				throw std::runtime_error("invalid solid hatch spacing");
			double low=std::numeric_limits<double>::infinity(),high=-low;
			for(auto const& loop:loops)for(auto point:loop){low=std::min(low,point.y);high=std::max(high,point.y);}
			if(!(high>low))return;
			solidSpacing=std::min(solidSpacing,high-low);
			// Center scanlines inside the bounds so sub-spacing arrowheads are not lost.
			patterns = {entities::sHatch::sPatternLine{0, {0,low+solidSpacing*.5,0}, {0, solidSpacing, 0}, {}}};
		}
		size_t visits{};
		for (auto const& pattern : patterns) {
			double c = std::cos(pattern.angle), s = std::sin(pattern.angle),
			       spacing = -s * pattern.offset.x + c * pattern.offset.y;
			if (!std::isfinite(spacing) || std::abs(spacing) < 1e-14)
				throw std::runtime_error("zero hatch pattern spacing");
			std::vector<std::vector<point_t>> rotated;
			double low = std::numeric_limits<double>::infinity(), high = -low;
			for (auto const& loop : loops) {
				auto& row = rotated.emplace_back();
				for (auto p : loop) {
					p = Add(p, Scale(pattern.base, -1));
					point_t q{c * p.x + s * p.y, -s * p.x + c * p.y, p.z};
					row.push_back(q);
					low = std::min(low, q.y);
					high = std::max(high, q.y);
				}
			}
			double begin = std::ceil(std::min(low / spacing, high / spacing)),
			       end = std::floor(std::max(low / spacing, high / spacing));
			if (!std::isfinite(begin) || !std::isfinite(end) || end - begin > limit || std::abs(begin) > 9e15 ||
			    std::abs(end) > 9e15)
				throw std::runtime_error("hatch pattern line limit reached");
			for (double index = begin; index <= end; index += 1.) {
				if (++visits > limit)
					throw std::runtime_error("hatch pattern work limit reached");
				double y = index * spacing;
				std::vector<std::pair<double, int>> events;
				for (auto const& loop : rotated) {
					std::vector<double> crossings;
					for (size_t i = 1; i < loop.size(); ++i) {
						auto a = loop[i - 1], b = loop[i];
						if ((a.y <= y && y < b.y) || (b.y <= y && y < a.y))
							crossings.push_back(a.x + (y - a.y) * (b.x - a.x) / (b.y - a.y));
					}
					std::ranges::sort(crossings);
					for (size_t i = 1; i < crossings.size(); i += 2) {
						events.emplace_back(crossings[i - 1], 1);
						events.emplace_back(crossings[i], -1);
					}
				}
				std::ranges::sort(events);
				int inside{};
				double from{};
				auto line = [&](double a, double b) {
					if (++visits > limit)
						throw std::runtime_error("hatch segment limit reached");
					auto point = [&](double x) {
						return point_t{pattern.base.x + c * x - s * y, pattern.base.y + s * x + c * y, hatch.elevation};
					};
					emit(point(a), point(b));
				};
				for (auto [x, delta] : events) {
					bool fill = hatch.style == 2 ? inside > 0 : hatch.style == 1 ? inside == 1 : (inside & 1) != 0;
					if (fill && x > from) {
						if (pattern.dashes.empty())
							line(from, x);
						else {
							double period{};
							for (auto dash : pattern.dashes)
								period += std::abs(dash);
							if (!(period > 0) || !std::isfinite(period))
								throw std::runtime_error("invalid hatch dash period");
							double phase = index * (c * pattern.offset.x + s * pattern.offset.y),
							       at = phase + std::floor((from - phase) / period) * period;
							while (at < x) {
								double previous = at;
								for (auto dash : pattern.dashes) {
									if (++visits > limit)
										throw std::runtime_error("hatch dash work limit reached");
									double next = at + std::abs(dash);
									if (dash >= 0 && next >= from && at <= x)
										line(std::max(from, at), std::min(x, next));
									at = next;
								}
								if (!(at > previous))
									throw std::runtime_error("hatch dash period below coordinate precision");
							}
						}
					}
					inside += delta;
					from = x;
				}
			}
		}
	}
} // namespace gtl::dwg::detail

// Included in shape.cpp to keep GTL Boost export registrations in one translation unit.
namespace gtl::shape {
namespace {
constexpr size_t maxSegments = 16384;
constexpr double pi = std::numbers::pi;
bool Finite(point_t p) { return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z); }
double Dot(point_t a, point_t b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
point_t Cross(point_t a, point_t b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
double Length(point_t p) { return std::sqrt(Dot(p, p)); }
point_t Unit(point_t p, point_t fallback = {}) {
	auto n = Length(p);
	return std::isfinite(n) && n > 1e-12 ? p / n : fallback;
}
double Number(sCadGroup const &g, double fallback = 0.) {
	return boost::apply_visitor(
		[&](auto const &v) -> double {
			if constexpr (std::is_arithmetic_v<std::decay_t<decltype(v)>>)
				return std::isfinite((double)v) ? (double)v : fallback;
			else
				return fallback;
		},
		g.value);
}
struct Tags {
	std::span<sCadGroup const> tags;
	double N(int code, double fallback = 0.) const {
		for (auto const &g : tags)
			if (g.code == code)
				return Number(g, fallback);
		return fallback;
	}
	bool Has(int code) const {
		return std::ranges::any_of(tags, [&](auto const &g) { return g.code == code; });
	}
	string_t S(int code) const {
		for (auto const &g : tags)
			if (g.code == code)
				if (auto s = boost::get<string_t>(&g.value))
					return *s;
		return {};
	}
	point_t P(int code, point_t fallback = {}) const {
		return {N(code, fallback.x), N(code + 10, fallback.y), N(code + 20, fallback.z)};
	}
	Tags Sub(string_t const &name) const {
		for (size_t i = 0; i < tags.size(); ++i)
			if (tags[i].code == 100)
				if (auto s = boost::get<string_t>(&tags[i].value); s && *s == name)
					return {tags.subspan(i + 1)};
		return *this;
	}
	std::vector<Tags> Split(int code) const {
		std::vector<Tags> out;
		for (size_t i = 0; i < tags.size() && out.size() < maxSegments; ++i)
			if (tags[i].code == code) {
				size_t end = i + 1;
				while (end < tags.size() && tags[end].code != code)
					++end;
				out.push_back({tags.subspan(i, end - i)});
				i = end - 1;
			}
		return out;
	}
};
struct Builder {
	sCadGeometry geometry;
	point_t normal{0., 0., 1.};
	void Line(point_t a, point_t b) {
		if (geometry.lines.size() < maxSegments && Finite(a) && Finite(b) && a != b)
			geometry.lines.push_back({a, b});
	}
	void Path(std::vector<point_t> const &p) {
		for (size_t i = 1; i < p.size(); ++i)
			Line(p[i - 1], p[i]);
	}
	void Arrow(point_t tip, point_t toward, double size) {
		auto dir = Unit(toward - tip);
		auto side = Unit(Cross(normal, dir));
		if (size <= 0 || dir == point_t{} || side == point_t{})
			return;
		auto a = tip + dir * size + side * (size * .3), b = tip + dir * size - side * (size * .3);
		Line(tip, a);
		Line(a, b);
		Line(b, tip);
	}
	void Text(point_t p, string_t text, double height = 2.5, point_t x = {1., 0., 0.}, point_t y = {0., 1., 0.}) {
		if (text.empty() || text == L" " || !Finite(p) || !std::isfinite(height) || height <= 0 ||
			geometry.texts.size() >= 4096)
			return;
		x = Unit(x, {1., 0., 0.});
		y = Unit(y, {0., 1., 0.});
		for (auto separator : {L"\\P", L"\\X", L"^J"})
			for (size_t at = 0; (at = text.find(separator, at)) != string_t::npos;)
				text.replace(at, 2, L"\n");
		size_t start = 0, row = 0;
		while (start < text.size() && geometry.texts.size() < 4096) {
			auto end = text.find('\n', start);
			auto line = text.substr(start, end == string_t::npos ? end : end - start);
			if (!line.empty())
				geometry.texts.push_back({p - y * (height * 1.5 * row), x, y, std::move(line), height});
			if (end == string_t::npos)
				break;
			start = end + 1;
			++row;
		}
	}
	void Box(point_t p, point_t x, point_t y, double w, double h) {
		Line(p, p + x * w);
		Line(p + x * w, p + x * w + y * h);
		Line(p + x * w + y * h, p + y * h);
		Line(p + y * h, p);
	}
};
point_t OCS(point_t p, point_t n) {
	auto x =
		Unit(Cross(std::abs(n.x) < 1. / 64 && std::abs(n.y) < 1. / 64 ? point_t{0., 1., 0.} : point_t{0., 0., 1.}, n));
	return x * p.x + Cross(n, x) * p.y + n * p.z;
}
string_t Measurement(Tags g, double value, string_t prefix = {}) {
	auto measured = prefix + std::format(L"{:.4f}", value);
	while (measured.size() > 1 && measured.back() == '0')
		measured.pop_back();
	if (measured.back() == '.')
		measured.pop_back();
	auto text = g.S(1);
	if (text.empty())
		return measured;
	for (size_t p = 0; (p = text.find(L"<>", p)) != string_t::npos; p += measured.size())
		text.replace(p, 2, measured);
	return text;
}
void Dimension(Builder &b, Tags g, eSHAPE type) {
	auto n = b.normal, ex = OCS({1., 0., 0.}, n), ey = Cross(n, ex);
	double h = 2.5, arrow = 2.5;
	auto a = g.P(13), z = g.P(14), d = g.P(10), text = g.Has(11) ? OCS(g.P(11), n) : d;
	auto label = [&](double value, string_t prefix = {}) { b.Text(text, Measurement(g, value, prefix), h, ex, ey); };
	if (type == eSHAPE::dimension) {
		constexpr std::array types{eSHAPE::dimlinear, eSHAPE::dimaligned,	eSHAPE::dimangular, eSHAPE::dimdiametric,
								   eSHAPE::dimradial, eSHAPE::dimangular3p, eSHAPE::dimordinate};
		auto k = (int)g.N(70) & 7;
		if (k >= 7)
			return;
		type = types[k];
	}
	if (type == eSHAPE::dimlinear || type == eSHAPE::dimaligned) {
		if (!g.Has(13) || !g.Has(14) || !g.Has(10))
			return;
		auto dir = type == eSHAPE::dimaligned ? Unit(z - a)
											  : ex * std::cos(g.N(50) * pi / 180) + ey * std::sin(g.N(50) * pi / 180);
		auto offset = Unit(Cross(n, dir));
		auto p = a + offset * Dot(d - a, offset), q = z + offset * Dot(d - z, offset);
		b.Line(a, p);
		b.Line(z, q);
		b.Line(p, q);
		b.Arrow(p, q, arrow);
		b.Arrow(q, p, arrow);
		if (!g.Has(11))
			text = (p + q) / 2. + offset * h * .5;
		label(std::abs(Dot(z - a, dir)));
		return;
	}
	if (type == eSHAPE::dimradial || type == eSHAPE::dimdiametric) {
		if (!g.Has(10) || !g.Has(15))
			return;
		auto end = g.P(15);
		b.Line(d, end);
		b.Arrow(end, d, arrow);
		if (type == eSHAPE::dimdiametric)
			b.Arrow(d, end, arrow);
		if (!g.Has(11))
			text = (d + end) / 2.;
		label(Length(end - d), type == eSHAPE::dimradial ? L"R" : L"⌀");
		return;
	}
	if (type == eSHAPE::dimordinate) {
		if (!g.Has(13) || !g.Has(14))
			return;
		b.Line(a, z);
		b.Arrow(a, z, arrow);
		if (!g.Has(11))
			text = z;
		label(std::abs(Dot(a - d, ((int)g.N(70) & 64) ? ex : ey)));
		return;
	}
	if (type == eSHAPE::dimangular || type == eSHAPE::dimangular3p) {
		if (!g.Has(13) || !g.Has(14) || !g.Has(15))
			return;
		point_t center = g.P(15), u = a - center, v = z - center, arcPoint = d;
		if (type == eSHAPE::dimangular) {
			if (!g.Has(16))
				return;
			auto u0 = z - a, v0 = d - center;
			auto denom = Dot(Cross(u0, v0), n);
			if (std::abs(denom) < 1e-12)
				return;
			center = a + u0 * (Dot(Cross(center - a, v0), n) / denom);
			u = z - center;
			v = d - center;
			arcPoint = OCS(g.P(16), n);
		}
		u = Unit(u);
		v = Unit(v);
		auto r = Length(arcPoint - center);
		if (r <= 1e-12 || u == point_t{} || v == point_t{})
			return;
		auto sweep = std::atan2(Dot(Cross(u, v), n), Dot(u, v));
		if (sweep < 0)
			sweep += 2 * pi;
		auto side = Cross(n, u);
		auto at = [&](double t) { return center + (u * std::cos(t) + side * std::sin(t)) * r; };
		// Select the arc containing the stored dimension-arc point.
		auto angle = std::atan2(Dot(arcPoint - center, side), Dot(arcPoint - center, u));
		if (angle < 0)
			angle += 2 * pi;
		if (angle > sweep)
			sweep -= 2 * pi;
		b.Line(a, at(0));
		b.Line(z, at(sweep));
		for (int i = 1; i <= 96; ++i)
			b.Line(at(sweep * (i - 1) / 96), at(sweep * i / 96));
		b.Arrow(at(0), at(sweep / 96), arrow);
		b.Arrow(at(sweep), at(sweep * 95 / 96), arrow);
		if (!g.Has(11))
			text = at(sweep / 2);
		label(std::abs(sweep) * 180 / pi);
		return;
	}
}
void Leader(Builder &b, Tags g) {
	std::vector<point_t> p;
	for (auto vertex : g.Split(10))
		p.push_back(vertex.P(10));
	b.Path(p); // Spline-path leaders currently use their vertex polygon.
	if (p.size() < 2)
		return;
	auto h = g.N(40, 2.5);
	if (h <= 0)
		h = 2.5;
	if (g.N(71, 1) != 0)
		b.Arrow(p[0], p[1], h);
	if (g.N(75) != 0)
		b.Line(p.back(), p.back() + Unit(g.P(211, {1., 0., 0.})) * (g.N(41, h) * (g.N(74) == 0 ? -1 : 1)));
}
void MLeader(Builder &b, Tags g) {
	// Delimited context/leader/leader-line records reuse the same group codes.
	std::vector<point_t> points;
	bool inLine = false;
	int depth = 0;
	double arrow = 2.5;
	Tags context = g;
	for (size_t i = 0; i < g.tags.size(); ++i) {
		auto const &tag = g.tags[i];
		auto s = boost::get<string_t>(&tag.value);
		if (s && *s == L"CONTEXT_DATA{") {
			context = {g.tags.subspan(i + 1)};
			arrow = context.N(140, 2.5);
		}
		if (s && *s == L"LEADER{") {
			size_t end = i + 1;
			for (; end < g.tags.size(); ++end)
				if (auto marker = boost::get<string_t>(&g.tags[end].value);
					marker && (*marker == L"LEADER_LINE{" || *marker == L"}"))
					break;
			Tags node{g.tags.subspan(i + 1, end - i - 1)};
			if (node.N(290) != 0 && node.N(291) != 0)
				b.Line(node.P(10), node.P(10) + Unit(node.P(11)) * node.N(40));
		}
		if (s && *s == L"LEADER_LINE{") {
			inLine = true;
			depth = 1;
			points.clear();
			continue;
		}
		if (inLine && s && *s == L"}") {
			if (--depth == 0) {
				b.Path(points);
				if (points.size() > 1)
					b.Arrow(points[0], points[1], arrow);
				inLine = false;
			}
			continue;
		}
		if (inLine && s && s->ends_with('{'))
			++depth;
		if (inLine && depth == 1 && tag.code == 10) {
			size_t end = i + 1;
			while (end < g.tags.size() && g.tags[end].code != 10 &&
				   !(boost::get<string_t>(&g.tags[end].value) != nullptr))
				++end;
			if (points.size() < maxSegments)
				points.push_back(Tags{g.tags.subspan(i, end - i)}.P(10));
		}
	}
	// Context text ends before the first nested leader (whose codes overlap).
	size_t end = 0;
	for (; end < context.tags.size(); ++end)
		if (auto s = boost::get<string_t>(&context.tags[end].value); s && *s == L"LEADER{")
			break;
	context.tags = context.tags.first(end);
	auto x = Unit(context.P(13, {1., 0., 0.}));
	b.Text(context.P(12), context.S(304), context.N(41, 2.5), x, Cross(b.normal, x));
}
void Helix(Builder &b, Tags g) {
	g = g.Sub(L"AcDbHelix");
	if (!g.Has(10) || !g.Has(11) || !g.Has(12))
		return;
	auto base = g.P(10), start = g.P(11), axis = Unit(g.P(12));
	auto radial = start - base - axis * Dot(start - base, axis);
	auto x = Unit(radial), y = Cross(axis, x);
	auto radius = g.N(40), turns = g.N(41), pitch = g.N(42);
	if (axis == point_t{} || x == point_t{} || radius <= 0 || turns <= 0 || turns > 10000)
		return;
	int count = (int)std::clamp(std::ceil(turns * 96), 16., (double)maxSegments);
	auto at = [&](double t) {
		auto angle = (g.N(290, 1) != 0 ? 1 : -1) * 2 * pi * turns * t;
		return base + axis * (Dot(start - base, axis) + pitch * turns * t) +
			   (x * std::cos(angle) + y * std::sin(angle)) * radius;
	};
	for (int i = 1; i <= count; ++i)
		b.Line(at((i - 1.) / count), at((double)i / count));
}
void MLine(Builder &b, Tags g) {
	auto vertices = g.Split(11);
	if (vertices.size() < 2)
		return;
	bool closed = ((int)g.N(71) & 2) != 0;
	for (size_t i = 0; i < (closed ? vertices.size() : vertices.size() - 1); ++i) {
		auto v = vertices[i], next = vertices[(i + 1) % vertices.size()];
		auto elements = v.Split(74), ends = next.Split(74);
		auto p = v.P(11), q = next.P(11), dir = Unit(v.P(12), Unit(q - p));
		for (size_t j = 0; j < std::min(elements.size(), ends.size()); ++j) {
			std::vector<double> params;
			for (auto const &t : elements[j].tags)
				if (t.code == 41)
					params.push_back(Number(t));
			if (params.empty())
				continue;
			auto a = p + v.P(13) * params[0], z = q + next.P(13) * ends[j].N(41);
			double length = Dot(z - a, dir), cursor = params.size() > 1 ? params[1] : 0.;
			bool visible = true;
			for (size_t k = 2; k < params.size(); ++k) {
				auto end = std::clamp(cursor + params[k], cursor, std::max(cursor, length));
				if (visible)
					b.Line(a + dir * cursor, a + dir * end);
				cursor = end;
				visible = !visible;
			}
			if (visible && cursor < length)
				b.Line(a + dir * cursor, z);
		}
	}
}
void Tolerance(Builder &b, Tags g) {
	auto text = g.S(1);
	if (text.empty() || !g.Has(10))
		return;
	auto p = g.P(10), x = Unit(g.P(11, {1., 0., 0.})), y = Unit(Cross(b.normal, x));
	double h = 2.5;
	// %%v separates feature-control-frame compartments; ^J separates rows.
	size_t start = 0;
	double offset = 0, row = 0;
	while (start < text.size() && b.geometry.texts.size() < 4096) {
		auto cell = std::min(text.find(L"%%v", start), text.find(L"%%V", start)), nl = text.find(L"^J", start),
			 end = std::min(cell, nl);
		auto value = text.substr(start, end == string_t::npos ? end : end - start);
		double units = 0;
		std::vector<std::pair<string_t, char>> runs;
		for (size_t pos = 0; pos < value.size();) {
			auto token = value.find(L"{\\Fgdt;", pos);
			if (token == string_t::npos) {
				auto plain = value.substr(pos);
				units += plain.size() * .65;
				runs.push_back({plain, 0});
				break;
			}
			if (token > pos) {
				auto plain = value.substr(pos, token - pos);
				units += plain.size() * .65;
				runs.push_back({plain, 0});
			}
			if (token + 8 < value.size() && value[token + 8] == '}' &&
				string_t{L"jnmls"}.find(value[token + 7]) != string_t::npos) {
				runs.push_back({{}, value[token + 7]});
				units += 1.;
				pos = token + 9;
			} else {
				auto plain = value.substr(token);
				units += plain.size() * .65;
				runs.push_back({plain, 0});
				break;
			}
		}
		auto width = std::max(1., units + 1) * h;
		auto origin = p + x * offset - y * row;
		b.Box(origin, x, y, width, h * 1.5);
		double advance = .5;
		for (auto const &[plain, symbol] : runs) {
			auto at = origin + x * (advance * h) + y * (h * .25);
			if (!symbol) {
				b.Text(at, plain, h, x, y);
				advance += plain.size() * .65;
				continue;
			}
			auto center = at + x * (h * .5) + y * (h * .5);
			auto circle = [&](double t) { return center + (x * std::cos(t) + y * std::sin(t)) * (h * .45); };
			for (int k = 1; k <= 32; ++k)
				b.Line(circle((k - 1) * 2 * pi / 32), circle(k * 2 * pi / 32));
			if (symbol == 'j') {
				b.Line(center - x * (h * .6), center + x * (h * .6));
				b.Line(center - y * (h * .6), center + y * (h * .6));
			} else if (symbol == 'n')
				b.Line(center - (x + y) * (h * .45), center + (x + y) * (h * .45));
			else
				b.Text(at + x * (h * .3) + y * (h * .2), string_t(1, (char)std::toupper((unsigned char)symbol)), h * .6,
					   x, y);
			advance += 1.;
		}
		if (end == string_t::npos)
			break;
		if (end == nl) {
			row += h * 1.5;
			offset = 0;
			start = end + 2;
		} else {
			offset += width;
			start = end + 3;
		}
	}
}
void Table(Builder &b, Tags all) {
	auto g = all.Sub(L"AcDbTable");
	auto nr = g.N(91), nc = g.N(92);
	if (nr < 1 || nc < 1 || nr > 1024 || nc > 1024 || nr * nc > 4096)
		return;
	size_t rows = (size_t)nr, cols = (size_t)nc;
	std::vector<double> ys{0}, xs{0};
	for (auto const &t : g.tags) {
		if (t.code == 141 && ys.size() <= rows) {
			auto h = Number(t);
			if (h <= 0)
				return;
			ys.push_back(ys.back() + h);
		}
		if (t.code == 142 && xs.size() <= cols) {
			auto w = Number(t);
			if (w <= 0)
				return;
			xs.push_back(xs.back() + w);
		}
	}
	if (ys.size() != rows + 1 || xs.size() != cols + 1)
		return;
	auto p = all.P(10), x = Unit(g.P(11, {1., 0., 0.})), y = Unit(Cross(b.normal, x));
	for (auto pos : xs)
		b.Line(p + x * pos, p + x * pos - y * ys.back());
	for (auto pos : ys)
		b.Line(p - y * pos, p + x * xs.back() - y * pos);
	auto cells = g.Split(171);
	for (size_t i = 0; i < std::min(cells.size(), rows * cols); ++i) {
		auto cell = cells[i];
		if (cell.N(171) != 1 || cell.Has(344))
			continue;
		string_t text;
		for (auto const &t : cell.tags)
			if (t.code == 1 || t.code == 3)
				if (auto s = boost::get<string_t>(&t.value))
					text += *s;
		size_t r = i / cols, c = i % cols;
		auto h = cell.N(140, std::min(2.5, (ys[r + 1] - ys[r]) * .5));
		b.Text(p + x * (xs[c] + h * .25) - y * (ys[r] + h * 1.25), text, h, x, y);
	}
}
} // namespace

sCadGeometry xCadEntity::GetRenderGeometry() const {
	if (m_bExternalGraphics)
		return {};
	Builder b;
	Tags g{m_groups};
	b.normal = Unit(g.P(210, {0., 0., 1.}), {0., 0., 1.});
	switch (GetShapeType()) {
	case eSHAPE::dimension:
	case eSHAPE::dimlinear:
	case eSHAPE::dimaligned:
	case eSHAPE::dimradial:
	case eSHAPE::dimdiametric:
	case eSHAPE::dimangular:
	case eSHAPE::dimangular3p:
	case eSHAPE::dimordinate:
		Dimension(b, g, GetShapeType());
		break;
	case eSHAPE::leader:
		Leader(b, g.Sub(L"AcDbLeader"));
		break;
	case eSHAPE::mleader:
		MLeader(b, g);
		break;
	case eSHAPE::tolerance:
		Tolerance(b, g.Sub(L"AcDbFcf"));
		break;
	case eSHAPE::mline:
		MLine(b, g.Sub(L"AcDbMline"));
		break;
	case eSHAPE::helix:
		Helix(b, g);
		break;
	case eSHAPE::table:
		Table(b, g);
		break;
	default:
		break;
	}
	for (auto &line : b.geometry.lines) {
		line.pt0 = ToWorld(line.pt0);
		line.pt1 = ToWorld(line.pt1);
	}
	std::erase_if(b.geometry.lines, [](auto const &l) { return !Finite(l.pt0) || !Finite(l.pt1); });
	for (auto &text : b.geometry.texts) {
		text.position = ToWorld(text.position);
		text.xAxis = ToWorld(text.xAxis) - m_origin;
		text.yAxis = ToWorld(text.yAxis) - m_origin;
	}
	std::erase_if(b.geometry.texts,
				  [](auto const &t) { return !Finite(t.position) || !Finite(t.xAxis) || !Finite(t.yAxis); });
	return std::move(b.geometry);
}
bool xCadEntity::UpdateBoundary(rect_t &bounds) const {
	auto geometry = GetRenderGeometry();
	bool changed = false;
	for (auto const &line : geometry.lines) {
		changed |= bounds.UpdateBoundary(line.pt0);
		changed |= bounds.UpdateBoundary(line.pt1);
	}
	for (auto const &t : geometry.texts) {
		auto width = t.height * .65 * t.text.size();
		for (auto p : {t.position, t.position + t.xAxis * width, t.position + t.yAxis * t.height,
					   t.position + t.xAxis * width + t.yAxis * t.height})
			changed |= bounds.UpdateBoundary(p);
	}
	return changed;
}
void xCadEntity::Draw(ICanvas &canvas) const {
	if (!m_bVisible)
		return;
	auto geometry = GetRenderGeometry();
	if (geometry.Empty()) {
		if (!m_bExternalGraphics) {
			xShape::Draw(canvas);
			canvas.DrawCadEntity(*this);
		}
		return;
	}
	xShape::Draw(canvas);
	for (auto const &line : geometry.lines)
		canvas.Line(line.pt0, line.pt1);
	for (auto const &t : geometry.texts) {
		xText text;
		static_cast<xShape &>(text) = static_cast<xShape const &>(*this);
		text.m_pt0 = t.position;
		text.m_pt1 = t.position;
		text.m_text = t.text;
		text.m_height = t.height * Length(t.yAxis);
		text.m_widthScale = Length(t.yAxis) > 1e-12 ? Length(t.xAxis) / Length(t.yAxis) : 1.;
		text.m_angle = deg_t{std::atan2(t.xAxis.y, t.xAxis.x) * 180 / pi};
		text.Draw(canvas);
	}
}
bool xCadEntity::DrawROI(ICanvas &canvas, rect_t const &roi) const {
	if (!m_bVisible)
		return false;
	auto geometry = GetRenderGeometry();
	if (geometry.Empty()) {
		if (m_bExternalGraphics)
			return false;
		Draw(canvas);
		return true;
	}
	auto bounds = GetBoundary();
	if (bounds.pt1().x < roi.pt0().x || bounds.pt0().x > roi.pt1().x || bounds.pt1().y < roi.pt0().y ||
		bounds.pt0().y > roi.pt1().y)
		return false;
	Draw(canvas);
	return true;
}
} // namespace gtl::shape

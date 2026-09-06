#pragma once

#include "gtl/dwg/entities.h"
#include "transform.h"
#include <functional>
#include <limits>
#include <numeric>

namespace gtl::dwg::detail {
	inline point_t SplinePoint(entities::sSpline const& spline, double t) {
		auto n = spline.controlPoints.size(), p = static_cast<size_t>(spline.degree);
		if (!p || p >= n || p > 64 || spline.knots.size() != n + p + 1 || !std::ranges::is_sorted(spline.knots) ||
		    !(spline.knots[p] < spline.knots[n]) || (!spline.weights.empty() && spline.weights.size() != n))
			throw std::runtime_error("invalid spline evaluation data");
		size_t span =
		    t >= spline.knots[n]
		        ? n - 1
		        : static_cast<size_t>(std::upper_bound(spline.knots.begin() + p, spline.knots.begin() + n + 1, t) -
		                              spline.knots.begin() - 1);
		std::vector<std::array<double, 4>> values(p + 1);
		for (size_t i = 0; i <= p; ++i) {
			auto at = span - p + i;
			auto point = spline.controlPoints[at];
			double weight = spline.weights.empty() ? 1. : spline.weights[at];
			values[i] = {point.x * weight, point.y * weight, point.z * weight, weight};
		}
		for (size_t r = 1; r <= p; ++r)
			for (size_t j = p; j >= r; --j) {
				auto i = span - p + j;
				double denominator = spline.knots[i + p - r + 1] - spline.knots[i];
				double alpha = denominator ? (t - spline.knots[i]) / denominator : 0.;
				for (size_t k = 0; k < 4; ++k)
					values[j][k] = std::lerp(values[j - 1][k], values[j][k], alpha);
			}
		auto value = values[p];
		if (std::abs(value[3]) <= std::numeric_limits<double>::min())
			throw std::runtime_error("rational spline has zero homogeneous weight");
		return Checked({value[0] / value[3], value[1] / value[3], value[2] / value[3]});
	}

	// Interpolate fit points in the documented chord/sqrt/uniform parameterization.
	// End tangent constraints are included when present. DWG does not retain the
	// originating application's fit solver, so callers report this reconstruction.
	inline entities::sSpline Interpolate(entities::sSpline const& source) {
		auto count = source.fitPoints.size();
		if (count < 2 || count > 256)
			throw std::runtime_error("fit spline requires 2..256 points for bounded interpolation");
		bool first = Length(source.startTangent) > 0, last = Length(source.endTangent) > 0;
		size_t n = count + first + last, p = source.degree;
		if (!p || p >= n || p > 16)
			throw std::runtime_error("unsupported fit spline degree");
		std::vector<double> parameters(count);
		double total{};
		for (size_t i = 1; i < count; ++i) {
			double distance = Length(Add(source.fitPoints[i], Scale(source.fitPoints[i - 1], -1)));
			if (!(distance > 0))
				throw std::runtime_error("repeated consecutive spline fit points");
			total += source.knotParameter == 2 ? 1. : source.knotParameter == 1 ? std::sqrt(distance) : distance;
			parameters[i] = total;
		}
		for (auto& value : parameters)
			value /= total;
		std::vector<double> expanded = parameters;
		if (first)
			expanded.insert(expanded.begin(), 0.);
		if (last)
			expanded.push_back(1.);
		entities::sSpline result;
		result.scenario = 1;
		result.degree = source.degree;
		result.closed = source.closed;
		result.periodic = source.periodic;
		result.knots.resize(n + p + 1, 1.);
		std::fill_n(result.knots.begin(), p + 1, 0.);
		for (size_t j = 1; j < n - p; ++j) {
			double sum{};
			for (size_t k = j; k < j + p; ++k)
				sum += expanded[k];
			result.knots[j + p] = sum / p;
		}
		std::vector<std::vector<double>> matrix(n, std::vector<double>(n + 3));
		result.controlPoints.resize(n);
		for (size_t row = 0; row < count; ++row) {
			for (size_t col = 0; col < n; ++col) {
				result.controlPoints[col].x = 1.;
				matrix[row][col] = SplinePoint(result, parameters[row]).x;
				result.controlPoints[col].x = 0.;
			}
			auto point = source.fitPoints[row];
			matrix[row][n] = point.x;
			matrix[row][n + 1] = point.y;
			matrix[row][n + 2] = point.z;
		}
		size_t row = count;
		auto tangent = [&](bool atStart, point_t direction) {
			double denominator = atStart ? result.knots[p + 1] : 1. - result.knots[n - 1];
			if (!(denominator > 0))
				throw std::runtime_error("singular fit tangent constraint");
			if (atStart) {
				matrix[row][0] = -static_cast<double>(p) / denominator;
				matrix[row][1] = p / denominator;
			} else {
				matrix[row][n - 2] = -static_cast<double>(p) / denominator;
				matrix[row][n - 1] = p / denominator;
			}
			direction = Scale(direction, total / Length(direction));
			matrix[row][n] = direction.x;
			matrix[row][n + 1] = direction.y;
			matrix[row][n + 2] = direction.z;
			++row;
		};
		if (first)
			tangent(true, source.startTangent);
		if (last)
			tangent(false, source.endTangent);
		for (size_t col = 0; col < n; ++col) {
			size_t pivot = col;
			for (size_t r = col + 1; r < n; ++r)
				if (std::abs(matrix[r][col]) > std::abs(matrix[pivot][col]))
					pivot = r;
			if (std::abs(matrix[pivot][col]) < 1e-12)
				throw std::runtime_error("singular fit spline interpolation");
			std::swap(matrix[pivot], matrix[col]);
			for (size_t r = col + 1; r < n; ++r) {
				double factor = matrix[r][col] / matrix[col][col];
				if (!factor)
					continue;
				for (size_t c = col; c < n + 3; ++c)
					matrix[r][c] -= factor * matrix[col][c];
			}
		}
		for (size_t r = n; r-- > 0;) {
			std::array<double, 3> point{};
			for (size_t axis = 0; axis < 3; ++axis) {
				double value = matrix[r][n + axis];
				for (size_t col = r + 1; col < n; ++col) {
					auto pnt = result.controlPoints[col];
					value -= matrix[r][col] * (axis == 0 ? pnt.x : axis == 1 ? pnt.y : pnt.z);
				}
				point[axis] = value / matrix[r][r];
			}
			result.controlPoints[r] = Checked({point[0], point[1], point[2]});
		}
		return result;
	}

	inline std::vector<point_t> Tessellate(entities::sSpline const& spline, sTransform const& transform,
	                                       double tolerance, size_t limit) {
		if (!(tolerance > 0) || !std::isfinite(tolerance) || !limit)
			throw std::runtime_error("invalid curve sampling options");
		auto n = spline.controlPoints.size(), p = static_cast<size_t>(spline.degree);
		if (p >= n || spline.knots.size() != n + p + 1)
			throw std::runtime_error("invalid spline knot vector");
		std::vector<point_t> points;
		auto evaluate = [&](double t) { return transform.Point(SplinePoint(spline, t)); };
		std::function<void(double, double, point_t, point_t, unsigned)> split;
		split = [&](double a, double b, point_t start, point_t end, unsigned depth) {
			double mid = std::midpoint(a, b);
			auto center = evaluate(mid);
			double error{};
			for (double fraction : {.25, .5, .75}) {
				auto actual = evaluate(std::lerp(a, b, fraction));
				auto line = Add(Scale(start, 1 - fraction), Scale(end, fraction));
				error = std::max(error, Length(Add(actual, Scale(line, -1))));
			}
			if (error > tolerance) {
				if (depth >= 20 || mid == a || mid == b)
					throw std::runtime_error("curve tolerance exceeds subdivision limit");
				split(a, mid, start, center, depth + 1);
				split(mid, b, center, end, depth + 1);
			} else {
				if (points.size() > limit)
					throw std::runtime_error("curve segment limit reached");
				points.push_back(end);
			}
		};
		points.push_back(evaluate(spline.knots[p]));
		for (size_t i = p; i < n; ++i)
			if (spline.knots[i] < spline.knots[i + 1])
				split(spline.knots[i], spline.knots[i + 1], evaluate(spline.knots[i]), evaluate(spline.knots[i + 1]),
				      0);
		return points;
	}
} // namespace gtl::dwg::detail

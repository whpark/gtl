#pragma once

//////////////////////////////////////////////////////////////////////
//
// unit
//
// PWH
//
// 2021.05.12. from Mocha
//
//////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <cmath>
#include <numbers>
#include "gtl/concepts.h"

namespace gtl {

	// length - mm

	class length_mm_t {	// represents length in mm
	public:
		double dValue = 0.0;

		length_mm_t(double v) : dValue(v) { }

		operator double& () { return dValue; };
		operator double () const { return dValue; };

		auto operator <=> (const length_mm_t& ) const = default;
		auto operator <=> (double b) const { return dValue <=> b; }
	};
	using mm_t = length_mm_t;

	namespace literals {
		inline length_mm_t operator "" _nm (long double v)					{ return length_mm_t(v * 1.e-6); }
		inline length_mm_t operator "" _um (long double v)					{ return length_mm_t(v * 1.e-3); }
		inline length_mm_t operator "" _mm (long double v)					{ return length_mm_t(v); }
		inline length_mm_t operator "" _cm (long double v)					{ return length_mm_t(v * 1.e1); }
		inline length_mm_t operator ""  _m (long double v)					{ return length_mm_t(v * 1.e3); }
		inline length_mm_t operator "" _km (long double v)					{ return length_mm_t(v * 1.e6); }
		inline length_mm_t operator "" _nm (unsigned long long v)			{ return length_mm_t(v * 1.e-6); }
		inline length_mm_t operator "" _um (unsigned long long v)			{ return length_mm_t(v * 1.e-3); }
		inline length_mm_t operator "" _mm (unsigned long long v)			{ return length_mm_t((double)v); }
		inline length_mm_t operator "" _cm (unsigned long long v)			{ return length_mm_t(v * 1.e1); }
		inline length_mm_t operator ""  _m (unsigned long long v)			{ return length_mm_t(v * 1.e3); }
		inline length_mm_t operator "" _km (unsigned long long v)			{ return length_mm_t(v * 1.e6); }

		inline unsigned long long operator "" _KB (unsigned long long v)	{ return v * 1024; }
		inline unsigned long long operator "" _MB (unsigned long long v)	{ return v * 1024_KB; }
		inline unsigned long long operator "" _GB (unsigned long long v)	{ return v * 1024_MB; }

		// Percent - %
		inline double operator "" _percent (long double v)					{ return double(v * 1.e-2); }
		inline double operator "" _percent (unsigned long long v)			{ return double(v * 1.e-2); }
	}


	template < std::floating_point T >
	constexpr T rad2deg(T x) { 
		return x * 180. / std::numbers::pi;
	};
	template < std::floating_point T >
	constexpr T deg2rad(T x) { 
		return x * std::numbers::pi / 180.;
	};


	// angle - rad, deg
	class angle_deg_t;
	class angle_rad_t;
	class angle_rad_t {	// represents angle in rad
	public:
		double dValue = 0.0;

		angle_rad_t() = default;
		angle_rad_t(angle_rad_t const&) = default;
		angle_rad_t(angle_rad_t&&) = default;
		angle_rad_t(angle_deg_t const& b);
		explicit angle_rad_t(double v) : dValue(v) { }

		operator double& () { return dValue; };
		operator double () const { return dValue; };
		//operator angle_deg_t() const;
		angle_rad_t& operator = (angle_rad_t const&) = default;
		angle_rad_t& operator = (double const& b) { dValue = b; return *this; }
		angle_rad_t& operator = (angle_deg_t const& b);
		angle_rad_t operator - () const { return angle_rad_t(-dValue); }
		auto operator <=> (angle_rad_t const&) const = default;
		auto operator <=> (angle_deg_t const& deg) const { return *this <=> (angle_rad_t)deg; }
		auto operator <=> (double rad) const { return dValue <=> rad; }

		angle_rad_t operator + (angle_rad_t const& b) const { return angle_rad_t(dValue + b.dValue); }
		angle_rad_t operator - (angle_rad_t const& b) const { return angle_rad_t(dValue - b.dValue); }
		angle_rad_t operator * (angle_rad_t const& b) const { return angle_rad_t(dValue * b.dValue); }
		angle_rad_t operator / (angle_rad_t const& b) const { return angle_rad_t(dValue / b.dValue); }
		angle_rad_t& operator += (angle_rad_t const& b) { dValue += b.dValue; return *this; }
		angle_rad_t& operator -= (angle_rad_t const& b) { dValue -= b.dValue; return *this; }
		angle_rad_t& operator *= (angle_rad_t const& b) { dValue *= b.dValue; return *this; }
		angle_rad_t& operator /= (angle_rad_t const& b) { dValue /= b.dValue; return *this; }

		angle_rad_t operator + (angle_deg_t const& b) const;
		angle_rad_t operator - (angle_deg_t const& b) const;
		angle_rad_t operator * (angle_deg_t const& b) const;
		angle_rad_t operator / (angle_deg_t const& b) const;
		angle_rad_t& operator += (angle_deg_t const& b);
		angle_rad_t& operator -= (angle_deg_t const& b);
		angle_rad_t& operator *= (angle_deg_t const& b);
		angle_rad_t& operator /= (angle_deg_t const& b);

		angle_rad_t operator + (double const b) const;
		angle_rad_t operator - (double const b) const;
		angle_rad_t operator * (double const b) const;
		angle_rad_t operator / (double const b) const;
		angle_rad_t& operator += (double const b);
		angle_rad_t& operator -= (double const b);
		angle_rad_t& operator *= (double const b);
		angle_rad_t& operator /= (double const b);

		static angle_rad_t atan(double v)				{ return angle_rad_t(::atan(v)); }
		static angle_rad_t atan2(double y, double x)	{ return angle_rad_t(::atan2(y, x)); }
		static angle_rad_t asin(double v)				{ return angle_rad_t(::asin(v)); }
		static angle_rad_t acos(double v)				{ return angle_rad_t(::acos(v)); }
	};
	class __declspec(novtable) angle_deg_t {	// represents angle in deg
	public:
		double dValue = 0.0;

		angle_deg_t() = default;
		angle_deg_t(angle_deg_t const&) = default;
		angle_deg_t(angle_deg_t&&) = default;
		angle_deg_t(angle_rad_t const& b);
		explicit angle_deg_t(double v) : dValue(v) { }

		operator double& () { return dValue; };
		operator double () const { return dValue; };
		//operator angle_rad_t() const;
		angle_deg_t& operator = (angle_deg_t const&) = default;
		angle_deg_t& operator = (double const& b) { dValue = b; return *this; }
		angle_deg_t& operator = (angle_rad_t const& b);
		angle_deg_t operator - () const { return angle_deg_t(-dValue); }
		auto operator <=> (angle_deg_t const&) const = default;
		auto operator <=> (angle_rad_t const& deg) const { return *this <=> (angle_deg_t)deg; }
		auto operator <=> (double deg) const { return dValue <=> deg; }
		angle_deg_t operator + (angle_deg_t const& b) const { return angle_deg_t(dValue + b.dValue); }
		angle_deg_t operator - (angle_deg_t const& b) const { return angle_deg_t(dValue - b.dValue); }
		angle_deg_t operator * (angle_deg_t const& b) const { return angle_deg_t(dValue * b.dValue); }
		angle_deg_t operator / (angle_deg_t const& b) const { return angle_deg_t(dValue / b.dValue); }
		angle_deg_t& operator += (angle_deg_t const& b) { dValue += b.dValue; return *this; }
		angle_deg_t& operator -= (angle_deg_t const& b) { dValue -= b.dValue; return *this; }
		angle_deg_t& operator *= (angle_deg_t const& b) { dValue *= b.dValue; return *this; }
		angle_deg_t& operator /= (angle_deg_t const& b) { dValue /= b.dValue; return *this; }

		angle_deg_t operator + (angle_rad_t const& b) const;
		angle_deg_t operator - (angle_rad_t const& b) const;
		angle_deg_t operator * (angle_rad_t const& b) const;
		angle_deg_t operator / (angle_rad_t const& b) const;
		angle_deg_t& operator += (angle_rad_t const& b);
		angle_deg_t& operator -= (angle_rad_t const& b);
		angle_deg_t& operator *= (angle_rad_t const& b);
		angle_deg_t& operator /= (angle_rad_t const& b);

		angle_deg_t operator + (double const b) const;
		angle_deg_t operator - (double const b) const;
		angle_deg_t operator * (double const b) const;
		angle_deg_t operator / (double const b) const;
		angle_deg_t& operator += (double const b);
		angle_deg_t& operator -= (double const b);
		angle_deg_t& operator *= (double const b);
		angle_deg_t& operator /= (double const b);

		static angle_deg_t atan(double v)				{ return angle_rad_t::atan(v); }
		static angle_deg_t atan2(double y, double x)	{ return angle_rad_t::atan2(y, x); }
		static angle_deg_t asin(double v)				{ return angle_rad_t::asin(v); }
		static angle_deg_t acos(double v)				{ return angle_rad_t::acos(v); }
	};
	inline angle_rad_t::angle_rad_t(angle_deg_t const& b) : dValue(deg2rad(b.dValue)) {}
	inline angle_deg_t::angle_deg_t(angle_rad_t const& b) : dValue(rad2deg(b.dValue)) {}
	inline angle_rad_t& angle_rad_t::operator = (angle_deg_t const& b) { dValue = deg2rad(b.dValue); return *this; };
	inline angle_deg_t& angle_deg_t::operator = (angle_rad_t const& b) { dValue = rad2deg(b.dValue); return *this; };
	//inline angle_rad_t::operator angle_deg_t() const { return angle_deg_t(rad2deg(dValue)); }
	//inline angle_deg_t::operator angle_rad_t() const { return angle_rad_t(deg2rad(dValue)); }

	inline angle_rad_t operator "" _rad (long double v)					{ return angle_rad_t(v); }
	inline angle_rad_t operator "" _rad (unsigned long long v)			{ return angle_rad_t((double) v); }
	inline angle_deg_t operator "" _deg (long double v)					{ return angle_deg_t(v); }
	inline angle_deg_t operator "" _deg (unsigned long long v)			{ return angle_deg_t((double) v); }

	inline angle_rad_t angle_rad_t::operator + (angle_deg_t const& b) const { return angle_rad_t(dValue + (angle_rad_t)b); }
	inline angle_rad_t angle_rad_t::operator - (angle_deg_t const& b) const { return angle_rad_t(dValue - (angle_rad_t)b); }
	inline angle_rad_t angle_rad_t::operator * (angle_deg_t const& b) const { return angle_rad_t(dValue * (angle_rad_t)b); }
	inline angle_rad_t angle_rad_t::operator / (angle_deg_t const& b) const { return angle_rad_t(dValue / (angle_rad_t)b); }
	inline angle_rad_t& angle_rad_t::operator += (angle_deg_t const& b) { dValue += (angle_rad_t)b; return *this; }
	inline angle_rad_t& angle_rad_t::operator -= (angle_deg_t const& b) { dValue -= (angle_rad_t)b; return *this; }
	inline angle_rad_t& angle_rad_t::operator *= (angle_deg_t const& b) { dValue *= (angle_rad_t)b; return *this; }
	inline angle_rad_t& angle_rad_t::operator /= (angle_deg_t const& b) { dValue /= (angle_rad_t)b; return *this; }

	inline angle_rad_t angle_rad_t::operator + (double const b) const { return angle_rad_t(dValue + b); }
	inline angle_rad_t angle_rad_t::operator - (double const b) const { return angle_rad_t(dValue - b); }
	inline angle_rad_t angle_rad_t::operator * (double const b) const { return angle_rad_t(dValue * b); }
	inline angle_rad_t angle_rad_t::operator / (double const b) const { return angle_rad_t(dValue / b); }
	inline angle_rad_t& angle_rad_t::operator += (double const b) { dValue += b; return *this; }
	inline angle_rad_t& angle_rad_t::operator -= (double const b) { dValue -= b; return *this; }
	inline angle_rad_t& angle_rad_t::operator *= (double const b) { dValue *= b; return *this; }
	inline angle_rad_t& angle_rad_t::operator /= (double const b) { dValue /= b; return *this; }

	inline angle_deg_t angle_deg_t::operator + (angle_rad_t const& b) const { return angle_deg_t(dValue + (angle_deg_t)b); }
	inline angle_deg_t angle_deg_t::operator - (angle_rad_t const& b) const { return angle_deg_t(dValue - (angle_deg_t)b); }
	inline angle_deg_t angle_deg_t::operator * (angle_rad_t const& b) const { return angle_deg_t(dValue * (angle_deg_t)b); }
	inline angle_deg_t angle_deg_t::operator / (angle_rad_t const& b) const { return angle_deg_t(dValue / (angle_deg_t)b); }
	inline angle_deg_t& angle_deg_t::operator += (angle_rad_t const& b) { dValue += (angle_deg_t)b; return *this; }
	inline angle_deg_t& angle_deg_t::operator -= (angle_rad_t const& b) { dValue -= (angle_deg_t)b; return *this; }
	inline angle_deg_t& angle_deg_t::operator *= (angle_rad_t const& b) { dValue *= (angle_deg_t)b; return *this; }
	inline angle_deg_t& angle_deg_t::operator /= (angle_rad_t const& b) { dValue /= (angle_deg_t)b; return *this; }

	inline angle_deg_t angle_deg_t::operator + (double const b) const { return angle_deg_t(dValue + b); }
	inline angle_deg_t angle_deg_t::operator - (double const b) const { return angle_deg_t(dValue - b); }
	inline angle_deg_t angle_deg_t::operator * (double const b) const { return angle_deg_t(dValue * b); }
	inline angle_deg_t angle_deg_t::operator / (double const b) const { return angle_deg_t(dValue / b); }
	inline angle_deg_t& angle_deg_t::operator += (double const b) { dValue += b; return *this; }
	inline angle_deg_t& angle_deg_t::operator -= (double const b) { dValue -= b; return *this; }
	inline angle_deg_t& angle_deg_t::operator *= (double const b) { dValue *= b; return *this; }
	inline angle_deg_t& angle_deg_t::operator /= (double const b) { dValue /= b; return *this; }

	//typedef angle_rad_t rad_t;
	//typedef angle_deg_t deg_t;

	using rad_t = angle_rad_t;
	using deg_t = angle_deg_t;

	inline double cos(angle_rad_t t)				{ return ::cos((double)t); }
	inline double sin(angle_rad_t t)				{ return ::sin((double)t); }
	inline double tan(angle_rad_t t)				{ return ::tan((double)t); }
	inline double cos(angle_deg_t t)				{ return ::cos((double)(rad_t)t); }
	inline double sin(angle_deg_t t)				{ return ::sin((double)(rad_t)t); }
	inline double tan(angle_deg_t t)				{ return ::tan((double)(rad_t)t); }

}	// namespace gtl

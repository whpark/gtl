//////////////////////////////////////////////////////////////////////
//
// unit
//
// PWH
//
// 2021.05.12. from Mocha
//
//////////////////////////////////////////////////////////////////////

module;

#include <cstdint>
#include <cmath>
#include <numbers>
#include <compare>

#include "gtl/_config.h"
#include "gtl/_macro.h"

export module gtl:unit;
import :concepts;

export namespace gtl {

	// length - mm

	template < gtlc::arithmetic T >
	class tlength_mm_t {	// represents length in mm
	public:
		using value_type = T;
		T dValue{};

		tlength_mm_t(T v) : dValue(v) { }

		operator T& () { return dValue; };
		operator T () const { return dValue; };

		[[ nodiscard ]] auto operator <=> (tlength_mm_t const& ) const = default;
		[[ nodiscard ]] auto operator <=> (T b) const { return dValue <=> b; }

		template < typename archive >
		friend void serialize(archive& ar, tlength_mm_t& len, unsigned int const file_version) {
			ar & len.dValue;
		}

	};
	using mm_t = tlength_mm_t<double>;

	namespace literals {
		inline mm_t operator "" _nm (long double v)					{ return mm_t(v * 1.e-6); }
		inline mm_t operator "" _um (long double v)					{ return mm_t(v * 1.e-3); }
		inline mm_t operator "" _mm (long double v)					{ return mm_t(v); }
		inline mm_t operator "" _cm (long double v)					{ return mm_t(v * 1.e1); }
		inline mm_t operator ""  _m (long double v)					{ return mm_t(v * 1.e3); }
		inline mm_t operator "" _km (long double v)					{ return mm_t(v * 1.e6); }
		inline mm_t operator "" _nm (unsigned long long v)			{ return mm_t(v * 1.e-6); }
		inline mm_t operator "" _um (unsigned long long v)			{ return mm_t(v * 1.e-3); }
		inline mm_t operator "" _mm (unsigned long long v)			{ return mm_t((double)v); }
		inline mm_t operator "" _cm (unsigned long long v)			{ return mm_t(v * 1.e1); }
		inline mm_t operator ""  _m (unsigned long long v)			{ return mm_t(v * 1.e3); }
		inline mm_t operator "" _km (unsigned long long v)			{ return mm_t(v * 1.e6); }

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


	template < std::floating_point tvalue_t > class tangle_rad_t;
	template < std::floating_point tvalue_t > class tangle_deg_t;

	// angle - rad, deg
	template < std::floating_point tvalue_t >
	class tangle_rad_t {	// represents angle in rad
	public:
		using value_type = tvalue_t;
		value_type dValue{};

		tangle_rad_t() = default;
		tangle_rad_t(tangle_rad_t const&) = default;
		tangle_rad_t(tangle_rad_t&&) = default;
		tangle_rad_t(tangle_deg_t<value_type> const& b);
		explicit tangle_rad_t(value_type v) : dValue(v) { }

		operator value_type& () { return dValue; };
		operator value_type () const { return dValue; };
		//operator tangle_deg_t() const;
		tangle_rad_t& operator = (tangle_rad_t const&) = default;
		tangle_rad_t& operator = (value_type const& b) { dValue = b; return *this; }
		tangle_rad_t& operator = (tangle_deg_t<value_type> const& b);
		tangle_rad_t operator - () const { return tangle_rad_t(-dValue); }
		[[ nodiscard ]] auto operator <=> (tangle_rad_t const&) const = default;
		[[ nodiscard ]] auto operator <=> (tangle_deg_t<value_type> const& deg) const { return *this <=> (tangle_rad_t)deg; }
		[[ nodiscard ]] auto operator <=> (value_type rad) const { return dValue <=> rad; }

		tangle_rad_t operator + (tangle_rad_t const& b) const { return tangle_rad_t(dValue + b.dValue); }
		tangle_rad_t operator - (tangle_rad_t const& b) const { return tangle_rad_t(dValue - b.dValue); }
		tangle_rad_t operator * (tangle_rad_t const& b) const { return tangle_rad_t(dValue * b.dValue); }
		tangle_rad_t operator / (tangle_rad_t const& b) const { return tangle_rad_t(dValue / b.dValue); }
		tangle_rad_t& operator += (tangle_rad_t const& b) { dValue += b.dValue; return *this; }
		tangle_rad_t& operator -= (tangle_rad_t const& b) { dValue -= b.dValue; return *this; }
		tangle_rad_t& operator *= (tangle_rad_t const& b) { dValue *= b.dValue; return *this; }
		tangle_rad_t& operator /= (tangle_rad_t const& b) { dValue /= b.dValue; return *this; }

		tangle_rad_t operator + (tangle_deg_t<value_type> const& b) const;
		tangle_rad_t operator - (tangle_deg_t<value_type> const& b) const;
		tangle_rad_t operator * (tangle_deg_t<value_type> const& b) const;
		tangle_rad_t operator / (tangle_deg_t<value_type> const& b) const;
		tangle_rad_t& operator += (tangle_deg_t<value_type> const& b);
		tangle_rad_t& operator -= (tangle_deg_t<value_type> const& b);
		tangle_rad_t& operator *= (tangle_deg_t<value_type> const& b);
		tangle_rad_t& operator /= (tangle_deg_t<value_type> const& b);

		tangle_rad_t operator + (value_type const b) const;
		tangle_rad_t operator - (value_type const b) const;
		tangle_rad_t operator * (value_type const b) const;
		tangle_rad_t operator / (value_type const b) const;
		tangle_rad_t& operator += (value_type const b);
		tangle_rad_t& operator -= (value_type const b);
		tangle_rad_t& operator *= (value_type const b);
		tangle_rad_t& operator /= (value_type const b);

		static tangle_rad_t atan(value_type v)				{ return tangle_rad_t(::atan(v)); }
		static tangle_rad_t atan2(value_type y, value_type x)	{ return tangle_rad_t(::atan2(y, x)); }
		static tangle_rad_t asin(value_type v)				{ return tangle_rad_t(::asin(v)); }
		static tangle_rad_t acos(value_type v)				{ return tangle_rad_t(::acos(v)); }

		template < typename archive >
		friend void serialize(archive& ar, tangle_rad_t& len, unsigned int const file_version) {
			ar & len.dValue;
		}

	};

	template < std::floating_point tvalue_t >
	class tangle_deg_t {	// represents angle in deg
	public:
		using value_type = tvalue_t;
		value_type dValue{};

		tangle_deg_t() = default;
		tangle_deg_t(tangle_deg_t const&) = default;
		tangle_deg_t(tangle_deg_t&&) = default;
		tangle_deg_t(tangle_rad_t<value_type> const& b);
		explicit tangle_deg_t(value_type v) : dValue(v) { }

		operator value_type& () { return dValue; };
		operator value_type () const { return dValue; };
		//operator tangle_rad_t<value_type>() const;
		tangle_deg_t& operator = (tangle_deg_t const&) = default;
		tangle_deg_t& operator = (value_type const& b) { dValue = b; return *this; }
		tangle_deg_t& operator = (tangle_rad_t<value_type> const& b);
		tangle_deg_t operator - () const { return tangle_deg_t(-dValue); }
		[[ nodiscard ]] auto operator <=> (tangle_deg_t const&) const = default;
		[[ nodiscard ]] auto operator <=> (tangle_rad_t<value_type> const& deg) const { return *this <=> (tangle_deg_t)deg; }
		[[ nodiscard ]] auto operator <=> (value_type deg) const { return dValue <=> deg; }
		tangle_deg_t operator + (tangle_deg_t const& b) const { return tangle_deg_t(dValue + b.dValue); }
		tangle_deg_t operator - (tangle_deg_t const& b) const { return tangle_deg_t(dValue - b.dValue); }
		tangle_deg_t operator * (tangle_deg_t const& b) const { return tangle_deg_t(dValue * b.dValue); }
		tangle_deg_t operator / (tangle_deg_t const& b) const { return tangle_deg_t(dValue / b.dValue); }
		tangle_deg_t& operator += (tangle_deg_t const& b) { dValue += b.dValue; return *this; }
		tangle_deg_t& operator -= (tangle_deg_t const& b) { dValue -= b.dValue; return *this; }
		tangle_deg_t& operator *= (tangle_deg_t const& b) { dValue *= b.dValue; return *this; }
		tangle_deg_t& operator /= (tangle_deg_t const& b) { dValue /= b.dValue; return *this; }

		tangle_deg_t operator + (tangle_rad_t<value_type> const& b) const;
		tangle_deg_t operator - (tangle_rad_t<value_type> const& b) const;
		tangle_deg_t operator * (tangle_rad_t<value_type> const& b) const;
		tangle_deg_t operator / (tangle_rad_t<value_type> const& b) const;
		tangle_deg_t& operator += (tangle_rad_t<value_type> const& b);
		tangle_deg_t& operator -= (tangle_rad_t<value_type> const& b);
		tangle_deg_t& operator *= (tangle_rad_t<value_type> const& b);
		tangle_deg_t& operator /= (tangle_rad_t<value_type> const& b);

		tangle_deg_t operator + (value_type const b) const;
		tangle_deg_t operator - (value_type const b) const;
		tangle_deg_t operator * (value_type const b) const;
		tangle_deg_t operator / (value_type const b) const;
		tangle_deg_t& operator += (value_type const b);
		tangle_deg_t& operator -= (value_type const b);
		tangle_deg_t& operator *= (value_type const b);
		tangle_deg_t& operator /= (value_type const b);

		static tangle_deg_t atan(value_type v)				{ return tangle_rad_t<value_type>::atan(v); }
		static tangle_deg_t atan2(value_type y, value_type x)	{ return tangle_rad_t<value_type>::atan2(y, x); }
		static tangle_deg_t asin(value_type v)				{ return tangle_rad_t<value_type>::asin(v); }
		static tangle_deg_t acos(value_type v)				{ return tangle_rad_t<value_type>::acos(v); }

		template < typename archive >
		friend void serialize(archive& ar, tangle_deg_t& len, unsigned int const file_version) {
			ar & len.dValue;
		}

	};
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>::tangle_rad_t(tangle_deg_t<tvalue_t> const& b) : dValue(deg2rad(b.dValue)) {}
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>::tangle_deg_t(tangle_rad_t<tvalue_t> const& b) : dValue(rad2deg(b.dValue)) {}
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>& tangle_rad_t<tvalue_t>::operator = (tangle_deg_t<tvalue_t> const& b) { dValue = deg2rad(b.dValue); return *this; };
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>& tangle_deg_t<tvalue_t>::operator = (tangle_rad_t<tvalue_t> const& b) { dValue = rad2deg(b.dValue); return *this; };
	//inline tangle_rad_t::operator tangle_deg_t() const { return tangle_deg_t(rad2deg(dValue)); }
	//inline tangle_deg_t::operator tangle_rad_t() const { return tangle_rad_t(deg2rad(dValue)); }

	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>  tangle_rad_t<tvalue_t>::operator +  (tangle_deg_t<tvalue_t> const& b) const { return tangle_rad_t<tvalue_t>(dValue + (tangle_rad_t<tvalue_t>)b); }
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>  tangle_rad_t<tvalue_t>::operator -  (tangle_deg_t<tvalue_t> const& b) const { return tangle_rad_t<tvalue_t>(dValue - (tangle_rad_t<tvalue_t>)b); }
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>  tangle_rad_t<tvalue_t>::operator *  (tangle_deg_t<tvalue_t> const& b) const { return tangle_rad_t<tvalue_t>(dValue * (tangle_rad_t<tvalue_t>)b); }
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>  tangle_rad_t<tvalue_t>::operator /  (tangle_deg_t<tvalue_t> const& b) const { return tangle_rad_t<tvalue_t>(dValue / (tangle_rad_t<tvalue_t>)b); }
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>& tangle_rad_t<tvalue_t>::operator += (tangle_deg_t<tvalue_t> const& b) { dValue += (tangle_rad_t<tvalue_t>)b; return *this; }
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>& tangle_rad_t<tvalue_t>::operator -= (tangle_deg_t<tvalue_t> const& b) { dValue -= (tangle_rad_t<tvalue_t>)b; return *this; }
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>& tangle_rad_t<tvalue_t>::operator *= (tangle_deg_t<tvalue_t> const& b) { dValue *= (tangle_rad_t<tvalue_t>)b; return *this; }
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>& tangle_rad_t<tvalue_t>::operator /= (tangle_deg_t<tvalue_t> const& b) { dValue /= (tangle_rad_t<tvalue_t>)b; return *this; }

	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>  tangle_rad_t<tvalue_t>::operator +  (tvalue_t const b) const { return tangle_rad_t<tvalue_t>(dValue + b); }
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>  tangle_rad_t<tvalue_t>::operator -  (tvalue_t const b) const { return tangle_rad_t<tvalue_t>(dValue - b); }
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>  tangle_rad_t<tvalue_t>::operator *  (tvalue_t const b) const { return tangle_rad_t<tvalue_t>(dValue * b); }
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>  tangle_rad_t<tvalue_t>::operator /  (tvalue_t const b) const { return tangle_rad_t<tvalue_t>(dValue / b); }
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>& tangle_rad_t<tvalue_t>::operator += (tvalue_t const b) { dValue += b; return *this; }
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>& tangle_rad_t<tvalue_t>::operator -= (tvalue_t const b) { dValue -= b; return *this; }
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>& tangle_rad_t<tvalue_t>::operator *= (tvalue_t const b) { dValue *= b; return *this; }
	template < std::floating_point tvalue_t >
	inline tangle_rad_t<tvalue_t>& tangle_rad_t<tvalue_t>::operator /= (tvalue_t const b) { dValue /= b; return *this; }

	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>  tangle_deg_t<tvalue_t>::operator +  (tangle_rad_t<tvalue_t> const& b) const { return tangle_deg_t<tvalue_t>(dValue + (tangle_deg_t<tvalue_t>)b); }
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>  tangle_deg_t<tvalue_t>::operator -  (tangle_rad_t<tvalue_t> const& b) const { return tangle_deg_t<tvalue_t>(dValue - (tangle_deg_t<tvalue_t>)b); }
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>  tangle_deg_t<tvalue_t>::operator *  (tangle_rad_t<tvalue_t> const& b) const { return tangle_deg_t<tvalue_t>(dValue * (tangle_deg_t<tvalue_t>)b); }
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>  tangle_deg_t<tvalue_t>::operator /  (tangle_rad_t<tvalue_t> const& b) const { return tangle_deg_t<tvalue_t>(dValue / (tangle_deg_t<tvalue_t>)b); }
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>& tangle_deg_t<tvalue_t>::operator += (tangle_rad_t<tvalue_t> const& b) { dValue += (tangle_deg_t<tvalue_t>)b; return *this; }
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>& tangle_deg_t<tvalue_t>::operator -= (tangle_rad_t<tvalue_t> const& b) { dValue -= (tangle_deg_t<tvalue_t>)b; return *this; }
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>& tangle_deg_t<tvalue_t>::operator *= (tangle_rad_t<tvalue_t> const& b) { dValue *= (tangle_deg_t<tvalue_t>)b; return *this; }
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>& tangle_deg_t<tvalue_t>::operator /= (tangle_rad_t<tvalue_t> const& b) { dValue /= (tangle_deg_t<tvalue_t>)b; return *this; }

	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>  tangle_deg_t<tvalue_t>::operator + (tvalue_t const b) const { return tangle_deg_t<tvalue_t>(dValue + b); }
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>  tangle_deg_t<tvalue_t>::operator - (tvalue_t const b) const { return tangle_deg_t<tvalue_t>(dValue - b); }
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>  tangle_deg_t<tvalue_t>::operator * (tvalue_t const b) const { return tangle_deg_t<tvalue_t>(dValue * b); }
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>  tangle_deg_t<tvalue_t>::operator / (tvalue_t const b) const { return tangle_deg_t<tvalue_t>(dValue / b); }
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>& tangle_deg_t<tvalue_t>::operator += (tvalue_t const b) { dValue += b; return *this; }
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>& tangle_deg_t<tvalue_t>::operator -= (tvalue_t const b) { dValue -= b; return *this; }
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>& tangle_deg_t<tvalue_t>::operator *= (tvalue_t const b) { dValue *= b; return *this; }
	template < std::floating_point tvalue_t >
	inline tangle_deg_t<tvalue_t>& tangle_deg_t<tvalue_t>::operator /= (tvalue_t const b) { dValue /= b; return *this; }

	//typedef tangle_rad_t rad_t;
	//typedef tangle_deg_t deg_t;

	template < std::floating_point tvalue_t > inline double cos(tangle_rad_t<tvalue_t> t)	{ return ::cos((tvalue_t)t); }
	template < std::floating_point tvalue_t > inline double sin(tangle_rad_t<tvalue_t> t)	{ return ::sin((tvalue_t)t); }
	template < std::floating_point tvalue_t > inline double tan(tangle_rad_t<tvalue_t> t)	{ return ::tan((tvalue_t)t); }
	template < std::floating_point tvalue_t > inline double cos(tangle_deg_t<tvalue_t> t)	{ return ::cos((tvalue_t)(tangle_rad_t<tvalue_t>)t); }
	template < std::floating_point tvalue_t > inline double sin(tangle_deg_t<tvalue_t> t)	{ return ::sin((tvalue_t)(tangle_rad_t<tvalue_t>)t); }
	template < std::floating_point tvalue_t > inline double tan(tangle_deg_t<tvalue_t> t)	{ return ::tan((tvalue_t)(tangle_rad_t<tvalue_t>)t); }

	using rad_t = tangle_rad_t<double>;
	using deg_t = tangle_deg_t<double>;
	namespace literals {
		inline rad_t operator "" _rad (long double v)				{ return rad_t(v); }
		inline rad_t operator "" _rad (unsigned long long v)		{ return rad_t((double) v); }
		inline deg_t operator "" _deg (long double v)				{ return deg_t(v); }
		inline deg_t operator "" _deg (unsigned long long v)		{ return deg_t((double) v); }
	}

}	// namespace gtl

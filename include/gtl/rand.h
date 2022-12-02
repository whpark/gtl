#pragma once

//////////////////////////////////////////////////////////////////////
//
// rand.h:
//
// PWH
// 2019.07.27.
//
//	random number generator helper
//
//////////////////////////////////////////////////////////////////////

#include <random>

namespace gtl {

	//=============================================================================================================================
	// General Random Distribution
	//
	template < typename T, typename T_ENGINE, typename T_DIST, typename T_RANDOM_DEVICE = std::random_device > requires std::is_arithmetic_v<T>
	class TRand {
	protected:
		T_RANDOM_DEVICE m_rd {};
		T_ENGINE m_engine;
		T_DIST m_distribution;

	public:
		template <typename ... Param >
		TRand(Param &&... args) : m_engine(m_rd()), m_distribution(args...) {
		}

		// Get One Value
		T operator() () {
			return m_distribution(m_engine);
		}

		// Get Series of random values
		std::vector<T> operator() (int n) {
			std::vector<T> lst;
			lst.reserve(n);
			for (int i = 0; i < n; i++) {
				lst.push_back(m_distribution(m_engine));
			}
			return lst;
		}

		T_ENGINE const& GetEngine() const { return m_engine; }
		T_DIST const& GetDitributor() const { return m_distribution; }

	};

	//-----------------------------------------------------------------------------------------------------------------------------
	// Random Uniform Distribution for int, int64_t, float, double
	template < typename T_ENGINE = std::mt19937, typename T_RANDOM_DEVICE = std::random_device >
	class xRandUniI : public TRand<int, T_ENGINE, std::uniform_int_distribution<int>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<int, T_ENGINE, std::uniform_int_distribution<int>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		xRandUniI(int min, int max) : base_t(min, max) {}
	};

	template < typename T_ENGINE = std::mt19937_64, typename T_RANDOM_DEVICE = std::random_device >
	class xRandUniI64 : public TRand<int64_t, T_ENGINE, std::uniform_int_distribution<int64_t>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<int64_t, T_ENGINE, std::uniform_int_distribution<int64_t>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		xRandUniI64(int64_t min, int64_t max) : base_t(min, max) {}
	};

	template < typename T_ENGINE = std::mt19937, typename T_RANDOM_DEVICE = std::random_device >
	class xRandUniF : public TRand<float, T_ENGINE, std::uniform_real_distribution<float>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<float, T_ENGINE, std::uniform_real_distribution<float>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		xRandUniF(float min, float max) : base_t(min, max) {}
	};

	template < typename T_ENGINE = std::mt19937, typename T_RANDOM_DEVICE = std::random_device >
	class xRandUniD : public TRand<double, T_ENGINE, std::uniform_real_distribution<double>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<double, T_ENGINE, std::uniform_real_distribution<double>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		xRandUniD(double min, double max) : base_t(min, max) {}
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	// Normal Distribution for int, int64_t, float, double
	template < typename T_ENGINE = std::mt19937, typename T_RANDOM_DEVICE = std::random_device >
	class xRandNormI : public TRand<int, T_ENGINE, std::normal_distribution<int>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<int, T_ENGINE, std::normal_distribution<int>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		xRandNormI(int mean, int sigma) : base_t(mean, sigma) {}
	};
	template < typename T_ENGINE = std::mt19937_64, typename T_RANDOM_DEVICE = std::random_device >
	class xRandNormI64 : public TRand<int64_t, T_ENGINE, std::normal_distribution<int64_t>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<int64_t, T_ENGINE, std::normal_distribution<int64_t>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		xRandNormI64(int64_t mean, int64_t sigma) : base_t(mean, sigma) {}
	};

	template < typename T_ENGINE = std::mt19937, typename T_RANDOM_DEVICE = std::random_device >
	class xRandNormF : public TRand<float, T_ENGINE, std::normal_distribution<float>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<float, T_ENGINE, std::normal_distribution<float>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		xRandNormF(float mean, double sigma) : base_t(mean, sigma) {}
	};

	template < typename T_ENGINE = std::mt19937, typename T_RANDOM_DEVICE = std::random_device >
	class xRandNormD : public TRand<double, T_ENGINE, std::normal_distribution<double>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<double, T_ENGINE, std::normal_distribution<double>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		xRandNormD(double mean, double sigma) : base_t(mean, sigma) {}
	};

} // namespace gtl;

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

module;

#include <random>

#include "gtl/_config.h"
#include "gtl/_macro.h"

export module gtl:rand;

export namespace gtl {

	//=============================================================================================================================
	// General Random Distribution
	//
	template < typename T, typename T_ENGINE, typename T_DIST, typename T_RANDOM_DEVICE = std::random_device, typename = std::is_arithmetic<T> >
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

		const T_ENGINE GetEngine() const { return m_engine; }
		const T_DIST GetDitributor() const { return m_distribution; }

	};

	//-----------------------------------------------------------------------------------------------------------------------------
	// Random Uniform Distribution for int, int64_t, float, double
	template < typename T_ENGINE = std::mt19937, typename T_RANDOM_DEVICE = std::random_device >
	class CRandUniI : public TRand<int, T_ENGINE, std::uniform_int_distribution<int>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<int, T_ENGINE, std::uniform_int_distribution<int>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		CRandUniI(int min, int max) : TRand(min, max) {}
	};

	template < typename T_ENGINE = std::mt19937_64, typename T_RANDOM_DEVICE = std::random_device >
	class CRandUniI64 : public TRand<int64_t, T_ENGINE, std::uniform_int_distribution<int64_t>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<int64_t, T_ENGINE, std::uniform_int_distribution<int64_t>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		CRandUniI64(int64_t min, int64_t max) : TRand(min, max) {}
	};

	template < typename T_ENGINE = std::mt19937, typename T_RANDOM_DEVICE = std::random_device >
	class CRandUniF : public TRand<float, T_ENGINE, std::uniform_real_distribution<float>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<float, T_ENGINE, std::uniform_real_distribution<float>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		CRandUniF(float min, float max) : TRand(min, max) {}
	};

	template < typename T_ENGINE = std::mt19937, typename T_RANDOM_DEVICE = std::random_device >
	class CRandUniD : public TRand<double, T_ENGINE, std::uniform_real_distribution<double>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<double, T_ENGINE, std::uniform_real_distribution<double>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		CRandUniD(double min, double max) : TRand(min, max) {}
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	// Normal Distribution for int, int64_t, float, double
	template < typename T_ENGINE = std::mt19937, typename T_RANDOM_DEVICE = std::random_device >
	class CRandNormI : public TRand<int, T_ENGINE, std::normal_distribution<int>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<int, T_ENGINE, std::normal_distribution<int>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		CRandNormI(int mean, int sigma) : TRand(mean, sigma) {}
	};
	template < typename T_ENGINE = std::mt19937_64, typename T_RANDOM_DEVICE = std::random_device >
	class CRandNormI64 : public TRand<int64_t, T_ENGINE, std::normal_distribution<int64_t>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<int64_t, T_ENGINE, std::normal_distribution<int64_t>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		CRandNormI64(int64_t mean, int64_t sigma) : TRand(mean, sigma) {}
	};

	template < typename T_ENGINE = std::mt19937, typename T_RANDOM_DEVICE = std::random_device >
	class CRandNormF : public TRand<float, T_ENGINE, std::normal_distribution<float>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<float, T_ENGINE, std::normal_distribution<float>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		CRandNormF(float mean, double sigma) : TRand(mean, sigma) {}
	};

	template < typename T_ENGINE = std::mt19937, typename T_RANDOM_DEVICE = std::random_device >
	class CRandNormD : public TRand<double, T_ENGINE, std::normal_distribution<double>, T_RANDOM_DEVICE > {
	public:
		using base_t = TRand<double, T_ENGINE, std::normal_distribution<double>, T_RANDOM_DEVICE >;
		using base_t::base_t;
		CRandNormD(double mean, double sigma) : TRand(mean, sigma) {}
	};

} // namespace gtl;

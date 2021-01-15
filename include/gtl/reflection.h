#pragma once


//////////////////////////////////////////////////////////////////////
//
// reflection.h: reflection
//
// PWH. 2020.01.03.
//
// 2021.01.04. member_wise.h -> reflection.h
//
//////////////////////////////////////////////////////////////////////


#include "gtl/_default.h"
#include "gtl/misc.h"
#include "gtl/string.h"


namespace gtl {
#pragma pack(push, 8)

	#define MW__BEGIN_TBL(THIS_CLASS)\
		using mw_this_t = THIS_CLASS;\
		constexpr inline static const std::tuple member_tuple_s {\

	#define MW__ADD_MEMBER(var)\
			gtl::internal::pair{ (const char*)#var, &mw_this_t::var },

	#define MW__END_TBL()\
		};\

	#define MW__MEMBER_EX(var, expr)	MW__ADD_MEMBER(var) expr

	#define MW__MEMBER_1(var)			MW__ADD_MEMBER(var)
	#define MW__MEMBER_2(var1, var2)    MW__ADD_MEMBER(var1) MW__ADD_MEMBER(var2)
	#define MW__MEMBER_3(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_2(__VA_ARGS__))
	#define MW__MEMBER_4(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_3(__VA_ARGS__))
	#define MW__MEMBER_5(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_4(__VA_ARGS__))
	#define MW__MEMBER_6(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_5(__VA_ARGS__))
	#define MW__MEMBER_7(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_6(__VA_ARGS__))
	#define MW__MEMBER_8(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_7(__VA_ARGS__))
	#define MW__MEMBER_9(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_8(__VA_ARGS__))
	#define MW__MEMBER_10(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_9(__VA_ARGS__))
	#define MW__MEMBER_11(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_10(__VA_ARGS__))
	#define MW__MEMBER_12(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_11(__VA_ARGS__))
	#define MW__MEMBER_13(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_12(__VA_ARGS__))
	#define MW__MEMBER_14(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_13(__VA_ARGS__))
	#define MW__MEMBER_15(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_14(__VA_ARGS__))
	#define MW__MEMBER_16(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_15(__VA_ARGS__))
	#define MW__MEMBER_17(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_16(__VA_ARGS__))
	#define MW__MEMBER_18(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_17(__VA_ARGS__))
	#define MW__MEMBER_19(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_18(__VA_ARGS__))
	#define MW__MEMBER_20(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_19(__VA_ARGS__))
	#define MW__MEMBER_21(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_20(__VA_ARGS__))
	#define MW__MEMBER_22(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_21(__VA_ARGS__))
	#define MW__MEMBER_23(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_22(__VA_ARGS__))
	#define MW__MEMBER_24(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_23(__VA_ARGS__))
	#define MW__MEMBER_25(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_24(__VA_ARGS__))
	#define MW__MEMBER_26(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_25(__VA_ARGS__))
	#define MW__MEMBER_27(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_26(__VA_ARGS__))
	#define MW__MEMBER_28(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_27(__VA_ARGS__))
	#define MW__MEMBER_29(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_28(__VA_ARGS__))
	#define MW__MEMBER_30(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_29(__VA_ARGS__))
	#define MW__MEMBER_31(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_30(__VA_ARGS__))
	#define MW__MEMBER_32(var, ...)		MW__MEMBER_EX(var, MW__MEMBER_31(__VA_ARGS__))

	#define MW__MEMBERS_F2(N, ...) MW__MEMBER_##N (__VA_ARGS__)
	#define MW__MEMBERS_F(N, ...) MW__MEMBERS_F2(N, __VA_ARGS__)
	#define MW__MEMBERS(...)      MW__MEMBERS_F(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)


	//-----------------------------------------------------------------------------
	// IMemberWise
	// 
#define DECL_MEMBER_WISE_BASE(THIS_CLASS)\
	using mw_base_t = THIS_CLASS;\
	friend void from_json(nlohmann::json const& j, THIS_CLASS& var) {\
		std::apply([&var, &j](auto& ... args) { ((var.*(args.second) = j[args.first]), ...); }, THIS_CLASS::member_tuple_s);\
	}\
	friend void to_json(nlohmann::json& j, THIS_CLASS const& var) {\
		j.clear();\
		std::apply([&var, &j](auto const& ... args) { ((j[args.first] = var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);\
	}\
	template < typename Archive >\
	friend Archive& operator & (Archive& ar, THIS_CLASS& var) {\
		std::apply([&var, &ar](auto& ... args) { ((ar & var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);\
		return ar;\
	}\
	template < typename Archive >\
	friend Archive& operator & (Archive& ar, THIS_CLASS const& var) {\
		std::apply([&var, &ar](auto const& ... args) { ((ar & var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);\
		return ar;\
	}\


#define DECL_MEMBER_WISE_DERIVED(THIS_CLASS, PARENT_CLASS)\
	friend void from_json(nlohmann::json const& j, THIS_CLASS& var) {\
		if constexpr (std::is_same_v<THIS_CLASS, PARENT_CLASS>) {\
		} else if constexpr (std::is_base_of_v<PARENT_CLASS, THIS_CLASS>) {\
			from_json(j, (PARENT_CLASS&)var);\
		}\
		std::apply([&var, &j](auto& ... args) { ((var.*(args.second) = j[args.first]), ...); }, THIS_CLASS::member_tuple_s);\
	}\
	friend void to_json(nlohmann::json& j, THIS_CLASS const& var) {\
		if constexpr (std::is_same_v<THIS_CLASS, PARENT_CLASS>) {\
			j.clear();\
		} else if constexpr (std::is_base_of_v<PARENT_CLASS, THIS_CLASS>) {\
			to_json(j, (PARENT_CLASS const&)var);\
		}\
		std::apply([&var, &j](auto const& ... args) { ((j[args.first] = var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);\
	}\
	template < typename Archive >\
	friend Archive& operator & (Archive& ar, THIS_CLASS& var) {\
		if constexpr (std::is_same_v<THIS_CLASS, PARENT_CLASS>) {\
		} else if constexpr (std::is_base_of_v<PARENT_CLASS, THIS_CLASS>) {\
			ar & (PARENT_CLASS&)var;\
		}\
		std::apply([&var, &ar](auto& ... args) { ((ar & var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);\
		return ar;\
	}\
	template < typename Archive >\
	friend Archive& operator & (Archive& ar, THIS_CLASS const& var) {\
		if constexpr (std::is_same_v<THIS_CLASS, PARENT_CLASS>) {\
		} else if constexpr (std::is_base_of_v<PARENT_CLASS, THIS_CLASS>) {\
			ar & (PARENT_CLASS const&)var;\
		}\
		std::apply([&var, &ar](auto const& ... args) { ((ar & var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);\
		return ar;\
	}\


	//-----------------------------------------------------------------------------
	// IMemberWiseV : with virtual function
#define DECL_MEMBER_WISE_VIRTUAL_BASE(THIS_CLASS)\
	DECL_MEMBER_WISE(THIS_CLASS)\
	virtual void FromJson(nlohmann::json const& j) { from_json(j, *(THIS_CLASS*)this); }\
	virtual void ToJson(nlohmann::json& j) const { to_json(j, *(THIS_CLASS*)this); }\
	virtual bool Compare(THIS_CLASS const& B) const {\
		if (!dynamic_cast<THIS_CLASS const*>(&B))\
			return false;\
		return *this == B;\
	}\


#define DECL_MEMBER_WISE_VIRTUAL_DERIVED(THIS_CLASS, PARENT_CLASS)\
	DECL_MEMBER_WISE_DERIVED(THIS_CLASS, PARENT_CLASS)\
	void FromJson(nlohmann::json const& j) override { from_json(j, *(THIS_CLASS*)this); }\
	void ToJson(nlohmann::json& j) const override { to_json(j, *(THIS_CLASS*)this); }\
	bool Compare(typename THIS_CLASS::mw_base_t const& B) const override {\
		if (!dynamic_cast<THIS_CLASS const*>(&B))\
			return false;\
		return (*this) == (THIS_CLASS const&)B;\
	}\


	/// @brief CRTP reflection class. (not good. better to use MACROS)
	/// @tparam 
	template < typename THIS_CLASS >
	class IMemberWise {
	public:
		using mw_base_t = THIS_CLASS;
		//using mw_this_t = THIS_CLASS;

		template < gtlc::json_container tjson >
		friend void from_json(tjson const& j, THIS_CLASS& var) {
		#if 0
			for...(auto& [name, pVar] : THIS_CLASS::member_tuple_s) {
				var.*pVar = j[name];
			}
		#else
			std::apply([&var, &j](auto& ... args) { ((var.*(args.second) = j[args.first]), ...); }, THIS_CLASS::member_tuple_s);
		#endif
		}

		template < typename tjson >
		friend void to_json(tjson& j, THIS_CLASS const& var) {
			j.clear();

		#if 0
			for...(auto& [name, pVar] : THIS_CLASS::member_tuple_s) {
				j[name] = var.*pVar;
			}
		#else
			std::apply([&var, &j](auto const& ... args) { ((j[args.first] = var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);
		#endif
		}
		template < typename Archive >
		friend Archive& operator & (Archive& ar, THIS_CLASS& var) {
		#if 0
			for...(auto& [name, pVar] : THIS_CLASS::member_tuple_s) {
				ar & var.*pVar;
			}
		#else
			std::apply([&var, &ar](auto& ... args) { ((ar & var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);
		#endif
			return ar;
		}
		template < typename Archive >
		friend Archive& operator & (Archive& ar, THIS_CLASS const& var) {
		#if 0
			for...(auto const& [name, pVar] : THIS_CLASS::member_tuple_s) {
				ar & var.*pVar;
			}
		#else
			std::apply([&var, &ar](auto const& ... args) { ((ar & var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);
		#endif
			return ar;
		}

	public:
		auto operator <=>(IMemberWise const&) const = default;
	};

	template < typename PARENT_CLASS, typename THIS_CLASS >
	class IMemberWiseDerived {
	public:
		//using mw_this_t = THIS_CLASS;
		//using mw_base_t = typename PARENT_CLASS::base_t;

		// from_json
		friend void from_json(nlohmann::json const& j, THIS_CLASS& var) {
			if constexpr (std::is_same_v<THIS_CLASS, PARENT_CLASS>) {
			} else if constexpr (std::is_base_of_v<PARENT_CLASS, THIS_CLASS>) {
				from_json(j, (PARENT_CLASS&)var);
			}

		#if 0
			for...(auto& [name, pVar] : THIS_CLASS::member_tuple_s) {
				var.*pVar = j[name];
			}
		#else
			std::apply([&var, &j](auto& ... args) { ((var.*(args.second) = j[args.first]), ...); }, THIS_CLASS::member_tuple_s);
		#endif
		}
		// to_json
		friend void to_json(nlohmann::json& j, THIS_CLASS const& var) {
			if constexpr (std::is_same_v<THIS_CLASS, PARENT_CLASS>) {
				j.clear();
			} else if constexpr (std::is_base_of_v<PARENT_CLASS, THIS_CLASS>) {
				to_json(j, (PARENT_CLASS const&)var);
			}

		#if 0
			for...(auto& [name, pVar] : THIS_CLASS::member_tuple_s) {
				j[name] = var.*pVar;
			}
		#else
			std::apply([&var, &j](auto const& ... args) { ((j[args.first] = var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);
		#endif
		}
		// archive & var
		template < typename Archive >
		friend Archive& operator & (Archive& ar, THIS_CLASS& var) {
			if constexpr (std::is_same_v<THIS_CLASS, PARENT_CLASS>) {
			} else if constexpr (std::is_base_of_v<PARENT_CLASS, THIS_CLASS>) {
				ar & (PARENT_CLASS&)var;
			}

		#if 0
			for...(auto& [name, pVar] : THIS_CLASS::member_tuple_s) {
				ar & var.*pVar;
			}
		#else
			std::apply([&var, &ar](auto& ... args) { ((ar & var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);
		#endif
			return ar;
		}
		// archive & var
		template < typename Archive >
		friend Archive& operator & (Archive& ar, THIS_CLASS const& var) {
			if constexpr (std::is_same_v<THIS_CLASS, PARENT_CLASS>) {
			} else if constexpr (std::is_base_of_v<PARENT_CLASS, THIS_CLASS>) {
				ar & (PARENT_CLASS const&)var;
			}

		#if 0
			for...(auto& [name, pVar] : THIS_CLASS::member_tuple_s) {
				ar & var.*pVar;
			}
		#else
			std::apply([&var, &ar](auto const& ... args) { ((ar & var.*(args.second)), ...); }, THIS_CLASS::member_tuple_s);
		#endif
			return ar;
		}

	public:
		auto operator <=>(IMemberWiseDerived const&) const = default;
	};

	//-----------------------------------------------------------------------------
	// IMemberWiseV : with virtual function
	template < typename THIS_CLASS >
	class IMemberWiseV : public IMemberWise<THIS_CLASS> {
	public:
		virtual void FromJson(nlohmann::json const& j) {
			from_json(j, *(THIS_CLASS*)this);
		}
		virtual void ToJson(nlohmann::json& j) const {
			to_json(j, *(THIS_CLASS*)this);
		}
		virtual bool Compare(THIS_CLASS const& B) const {
			if (!dynamic_cast<THIS_CLASS const*>(&B))
				return false;
			return *this == B;
		}

	public:
		auto operator <=>(IMemberWiseV const&) const = default;
	};
	template < typename PARENT_CLASS, typename THIS_CLASS >
	class IMemberWiseDerivedV : public IMemberWiseDerived<PARENT_CLASS, THIS_CLASS> {
	public:
		void FromJson(nlohmann::json const& j) override {
			from_json(j, *(THIS_CLASS*)this);
		}
		void ToJson(nlohmann::json& j) const override {
			to_json(j, *(THIS_CLASS*)this);
		}
		bool Compare(typename THIS_CLASS::mw_base_t const& B) const override {
			if (!dynamic_cast<THIS_CLASS const*>(&B))
				return false;
			return (*this) == (THIS_CLASS const&)B;
		}

	public:
		auto operator <=>(IMemberWiseDerivedV const&) const = default;
	};


#if 0
	namespace example {

		namespace CRTP {
			struct CBase : public IMemberWise<CBase> {
				int i, j;
				double d;

				MW__BEGIN_TBL(CBase)
					MW__MEMBERS(i, j)
					MW__MEMBERS(d)
				MW__END_TBL()

			public:
				auto operator <=> (CBase const&) const = default;
			};

			struct CDerivedA : public CBase, public IMemberWiseDerived<CBase, CDerivedA> {
				std::wstring name;

				MW__BEGIN_TBL(CDerivedA)
					MW__MEMBERS(name)
				MW__END_TBL()

			public:
				auto operator <=> (CDerivedA const&) const = default;
			};
		};

		namespace MACRO {
			struct CBase {

				int i, j;
				double d;

				DECL_MEMBER_WISE_BASE(CBase)
				MW__BEGIN_TBL(CBase)
					MW__MEMBERS(i, j)
					MW__MEMBERS(d)
				MW__END_TBL()

			public:
				auto operator <=> (CBase const&) const = default;
			};

			struct CDerivedA : public CBase {
				std::wstring name;

				DECL_MEMBER_WISE_DERIVED(CDerivedA, CBase)
				MW__BEGIN_TBL(CDerivedA)
					MW__MEMBERS(name)
				MW__END_TBL()

			public:
				auto operator <=> (CDerivedA const&) const = default;
			};
		}

	}
#endif

#pragma pack(pop)
}	// namespace gtl

#pragma once

//////////////////////////////////////////////////////////////////////
//
// dynamic.h: dynamic create, polymorphic serialize
//
// PWH
//
// 2013.
// 2021.01.19.
//
//////////////////////////////////////////////////////////////////////

#include "gtl/_default.h"
#include "gtl/misc.h"
#include "gtl/reflection.h"

namespace gtl {
#pragma pack(push, 8)


	namespace internal {

		template < typename OWNER >
		struct T_DYNAMIC_CREATE_Register {
			T_DYNAMIC_CREATE_Register() {
				OWNER::RegisterDynamicCreate(OWNER::s_identifier_dn, OWNER::NewObject);
			};
		};

	}


#define GTL__DYNAMIC_VIRTUAL_INTERFACE(className)\
	using mw_base_t = className;\
	using mw_this_t = className;\
	virtual std::unique_ptr<mw_base_t> NewObject() const = 0;\
	virtual std::unique_ptr<mw_base_t> NewClone()  const = 0;\
	friend mw_base_t* new_clone(mw_this_t const& r) { return r.NewClone().release(); }

#define GTL__DYNAMIC_VIRTUAL_BASE(className)\
	using mw_base_t = className;\
	using mw_this_t = className;\
	virtual std::unique_ptr<mw_base_t> NewObject() const { return std::make_unique<mw_this_t>(); }\
	virtual std::unique_ptr<mw_base_t> NewClone()  const { return std::make_unique<mw_this_t>(*this); }\
	friend mw_this_t* new_clone(mw_this_t const& r) { return (mw_this_t*)r.NewClone().release(); }

#define GTL__DYNAMIC_VIRTUAL_DERIVED(className)\
	using mw_this_t = className;\
	virtual std::unique_ptr<mw_base_t> NewObject() const override { return std::make_unique<mw_this_t>(); }\
	virtual std::unique_ptr<mw_base_t> NewClone()  const override { return std::make_unique<mw_this_t>(*this); }\
	friend mw_this_t* new_clone(mw_this_t const& r) { return (mw_this_t*)r.NewClone().release(); }


#define GTL__DYNAMIC_VIRTUAL_SERIALIZE_INTERFACE(className, ...)\

#define GTL__DYNAMIC_VIRTUAL_SERIALIZE_BASE(className, ...)\

#define GTL__DYNAMIC_VIRTUAL_SERIALIZE_DERIVED(className, ...)\

	//-----------------------------------------------------------------------------
	// Dynamic Creation. (Create using IDENTIFIER_DN)
	//
	// 2013.12.05. s_DYNAMIC_CREATE 변수의 위치를 CDYNAMIC_CREATE의 멤버변수로 두었다가 BaseClass 의 멤버로 옮김. DLL 로 만들 때, CLASS 내부의 static 변수는 Export 되지 않음.
	// 2018.12.    static 함수내부의 static 변수로 옮김. class 내의 static 변수로 있으면.... multiple instatiation 발생. (또는 multiple initialization). static inline 도 동일한 현상 발생.
	//             static 변수의 초기화 순서도 문제가 됨. 일단 static 함수내의 static 변수로 있으면 상관 없음..... 그래서 구현이 복잡해짐.
	// 2019.11.20. MACRO -> CRTP
	// 2021.01.19. renewal. CRTP -> MACRO + nested class
	//


	/// @brief TDynamicCreateHelper
	/// @tparam tobject
	/// @tparam tidentifier
	/// @tparam tcreator
	/// @tparam tmap
	template < typename tobject, typename tidentifier,
		typename tcreator = std::function<std::unique_ptr<tobject>()>,
		typename tmap = std::map<tidentifier, tcreator> >
	class TDynamicCreateBase {
	public:
		using object_t = tobject;
		using identifier_t = tidentifier;
		using creator_t = tcreator;
		using map_t = tmap;

	public:
		static void RegisterDynamicCreator(identifier_t const& id, creator_t Creator) {
			tableDynamicCreate_s[id] = Creator;
		}

		[[nodiscard]] static std::unique_ptr<object_t> CreateObject(identifier_t const& identifier) {
			if (auto const& item = tableDynamicCreate_s.find(identifier); item != tableDynamicCreate_s.end())
				return item->second();
			throw std::invalid_argument(GTL__FUNCSIG "no creator");
			return nullptr;
		};

	private:
		static inline map_t tableDynamicCreate_s;
	};

#define GTL__DYNAMIC_BASE(T_IDENTIFIER)\
	static inline TDynamicCreateBase<this_t, T_IDENTIFIER> dynamicCreateBase_s;


	template < typename tobject, auto id, std::unique_ptr<typename tobject::base_t> (*Creator)() >
	class TDynamicCreateHelper {
	public:
		using object_t = tobject;

	public:
		struct SDynamicCreateRegister {
			SDynamicCreateRegister() {
				object_t::dynamicCreateBase_s.RegisterDynamicCreator(id, Creator);
			};
		};
		static inline SDynamicCreateRegister dynamicCreateRegister_s;
	};

//#define GTL__DYNAMIC_CLASS(ID)\
//	static inline TDynamicCreateHelper<this_t, ID, []() -> std::unique_ptr<mw_base_t>{ return std::make_unique<this_t>(); }> dynamicCreateDerived_s;
#define GTL__DYNAMIC_CLASS(ID)\
	static inline TDynamicCreateHelper<this_t, ID, std::make_unique<this_t>> dynamicCreateDerived_s;
#define GTL__DYNAMIC_CLASS_EMPLACE(ID, ...)\
	static inline TDynamicCreateHelper<this_t, ID, []()->std::unique_ptr<base_t>{ return std::make_unique<this_t>(__VA_ARGS__); }> dynamicCreateDerived_s;


//	//-----------------------------------------------------------------------------
//	// IDynamicCreateDerived
//	//			template < auto& ID >
//	//			template < const char8_t& ID[20] >
//	//			template < const wchar_t& ID[20] >
//
//	template < typename BASE_CLASS, typename THIS_CLASS, auto ... ID >
//	class IDynamicCreateDerived : public INewCloneDerived<BASE_CLASS, THIS_CLASS> {
//	public:
//
//		// ID
//		using identifier_dn_t = typename BASE_CLASS::identifier_dn_t;
//		static inline identifier_dn_t const s_identifier_dn { ID ..., };
//		virtual const identifier_dn_t& GetIdentifierDN() const override { return s_identifier_dn; }
//
//		// New Object
//		using INewObject<THIS_CLASS>::NewObject;
//
//	private:
//		// Register
//		friend internal::T_DYNAMIC_CREATE_Register<IDynamicCreateDerived>;
//		static inline internal::T_DYNAMIC_CREATE_Register<IDynamicCreateDerived> s_DYNAMIC_CREATE_Register;
//
//	};
//
//
//#define DECLARE_DYNAMIC_CREATE_BASE__(THIS_CLASS, IDENTIFIER_DN)\
//	public:\
//		using identifier_dn_t = IDENTIFIER_DN;\
//		using INewObject<THIS_CLASS>::NewObject;\
//	private:\
//		using creator_t = std::function<std::unique_ptr<THIS_CLASS>()>;\
//		static inline std::map<identifier_dn_t, creator_t> s_tableDynamicCreate;\
//		friend internal::T_DYNAMIC_CREATE_Register<THIS_CLASS>;\
//		static inline internal::T_DYNAMIC_CREATE_Register<THIS_CLASS> s_DYNAMIC_CREATE_Register;\
//	protected:\
//		static void RegisterDynamicCreate(const identifier_dn_t& id, creator_t NewObject) {\
//			s_tableDynamicCreate[id] = NewObject;\
//		}\
//	public:\
//		[[nodiscard]] static std::unique_ptr<THIS_CLASS> CreateObject(const identifier_dn_t& identifier) {\
//			const auto& table = s_tableDynamicCreate;\
//			if (const auto& item = table.find(identifier); item != table.end())\
//				return item->second();\
//			throw std::invalid_argument(__FMSG "no creator");\
//			return nullptr;\
//		};
//#define DECLARE_DYNAMIC_CREATE__(ID)\
//	static inline identifier_dn_t const s_identifier_dn { ID };\
//	virtual const identifier_dn_t& GetIdentifierDN() const { return s_identifier_dn; }
//#define DECLARE_DYNAMIC_CREATE_INTERFACE__()\
//	static inline identifier_dn_t const s_identifier_dn { };\
//	virtual const identifier_dn_t& GetIdentifierDN() const = 0;
//
//#define DECLARE_DYNAMIC_CREATE_BASE(THIS_CLASS, IDENTIFIER_DN, ID)\
//	DECLARE_DYNAMIC_CREATE_BASE__(THIS_CLASS, IDENTIFIER_DN)\
//	DECLARE_DYNAMIC_CREATE__(ID)
//#define DECLARE_DYNAMIC_CREATE_BASE_INTERFACE(THIS_CLASS, IDENTIFIER_DN, ID)\
//	DECLARE_DYNAMIC_CREATE_BASE__(THIS_CLASS, IDENTIFIER_DN)\
//	DECLARE_DYNAMIC_CREATE_INTERFACE__(ID)
//#define DECLARE_DYNAMIC_CREATE_DERIVED(ID)\
//	DECLARE_DYNAMIC_CREATE__(ID)
//
//
//	//-----------------------------------------------------------------------------
//	// DynamicSerializeOut, DynamicSerializeIn
//	template < typename ARCHIVE_OUT, typename BASE_CLASS >
//	//requires (requires (BASE_CLASS base) { BASE_CLASS::identifier_dn_t; BASE_CLASS::identifier_dn_serialize_as_t; base.SerializeOut; })
//	//[[nodiscard]]
//	inline bool DynamicSerializeOut(ARCHIVE_OUT& ar, const BASE_CLASS* pObject) {
//		if (!pObject)
//			return false;
//		ar & (typename BASE_CLASS::identifier_dn_serialize_as_t&) pObject->GetIdentifierDN();
//		pObject->SerializeOut(ar);
//		return true;
//	};
//	template < typename ARCHIVE_IN, typename BASE_CLASS >
//	//requires (requires (BASE_CLASS base) { BASE_CLASS::identifier_dn_t; BASE_CLASS::identifier_dn_serialize_as_t; base.SerializeIn; })
//	[[nodiscard]]
//	inline std::unique_ptr<BASE_CLASS> DynamicSerializeIn(ARCHIVE_IN& ar, BASE_CLASS* = nullptr)
//	{
//		typename BASE_CLASS::identifier_dn_serialize_as_t id;
//		ar >> id;
//		auto rObject = BASE_CLASS::CreateObject((const BASE_CLASS::identifier_dn_t&)id);
//		if (rObject)
//			rObject->SerializeIn(ar);
//		return rObject;
//	};
//	//-----------------------------------------------------------------------------
//	// IDynamicSerializeBase
//	//   2019.11.21.
//	//
//	//   THIS_CLASS class must have ( ARCHIVE& operator & (ARCHIVE& ar, Data& value) {} )
//	//
//	template < typename THIS_CLASS,
//		typename IDENTIFIER_DN,
//		typename ARCHIVE_IN, typename ARCHIVE_OUT = ARCHIVE_IN,
//		typename IDENTIFIER_SERIALIZE_AS = IDENTIFIER_DN,
//		template <typename> typename SMART_PTR = std::unique_ptr >
//	class IDynamicSerializeBase : public IDynamicCreateBase<THIS_CLASS, IDENTIFIER_DN> {
//	public:
//		using this_t = THIS_CLASS;
//		using archive_in_t = ARCHIVE_IN;
//		using archive_out_t = ARCHIVE_OUT;
//		using archive_smart_ptr_t = SMART_PTR<THIS_CLASS>;
//		using identifier_dn_serialize_as_t = IDENTIFIER_SERIALIZE_AS;
//
//
//		static archive_smart_ptr_t DynamicSerializeIn(archive_in_t& ar) {
//			return gtl::DynamicSerializeIn<archive_in_t, THIS_CLASS>(ar, (THIS_CLASS*)nullptr);
//		}
//		void DynamicSerializeOut(archive_out_t& ar) {
//			gtl::DynamicSerializeOut<archive_out_t, THIS_CLASS>(ar, (THIS_CLASS*)this);
//		}
//
//		virtual archive_out_t& SerializeOut(archive_out_t& ar) const {
//			// THIS_CLASS class must have the (operator &).
//			return ar & *(const THIS_CLASS*)this;
//		};
//		virtual archive_in_t& SerializeIn(archive_in_t& ar) {
//			// THIS_CLASS class must have the (operator &).
//			return ar & *(THIS_CLASS*)this;
//		};
//
//	};
//
//
//
//	//-----------------------------------------------------------------------------
//	// IDynamicSerializeDerived
//	template < typename BASE_CLASS, typename THIS_CLASS, auto ... ID >
//	class IDynamicSerializeDerived : public IDynamicCreateDerived<BASE_CLASS, THIS_CLASS, ID ...> {
//	public:
//
//		using archive_in_t = typename BASE_CLASS::archive_in_t;
//		using archive_out_t = typename BASE_CLASS::archive_out_t;
//
//		virtual archive_out_t& SerializeOut(archive_out_t& ar) const override {
//			// THIS_CLASS class must have the (operator &).
//			return ar & *(const THIS_CLASS*)this;
//		};
//		virtual archive_in_t& SerializeIn(archive_in_t& ar) override {
//			// THIS_CLASS class must have the (operator &).
//			return ar & *(THIS_CLASS*)this;
//		};
//
//	};
//
//
//#define DECLARE_DYNAMIC_SERIALIZE_BASE__(THIS_CLASS, IDENTIFIER_SERIALIZE_AS)





#pragma pack(pop)
};	// namespace gtl


namespace gtl {

	// from biscuit/memory.ixx

	template < typename T >
	struct TCloner {
		std::unique_ptr<T> operator () (T const& self) const { return self.NewClone(); }
	};
	template < typename T >
	struct TStaticCloner {
		std::unique_ptr<T> operator () (T const& self) const { return std::make_unique<T>(self); }
	};

	template < /*concepts::cloneable */typename T, class CLONER = TCloner<T> >
	class TCloneablePtr : public std::unique_ptr<T> {
	public:
		using base_t = std::unique_ptr<T>;
		using this_t = TCloneablePtr;

		using base_t::base_t;
		//TCloneablePtr(this_t&& other) : base_t(std::move(other)) {}
		using base_t::operator =;
		using base_t::operator *;
		using base_t::operator ->;
		using base_t::operator bool;

		static inline CLONER cloner;

		TCloneablePtr(std::unique_ptr<T>&& other) : base_t(std::move(other)) {}
		TCloneablePtr(this_t&& other) : base_t(std::move(other)) {}
		// copy constructor
		TCloneablePtr(std::unique_ptr<T> const& other) : base_t(other ? cloner(*other) : nullptr) {}
		TCloneablePtr(this_t const& other) : base_t(other ? cloner(*other) : nullptr) {}

		TCloneablePtr& operator = (std::unique_ptr<T>&& other) { base_t::operator = (std::move(other)); return *this; }
		TCloneablePtr& operator = (std::unique_ptr<T> const& other) { base_t::operator = (cloner(*other)); return *this; }
		TCloneablePtr& operator = (this_t&& other) { base_t::operator = (std::move(other)); return *this; }
		TCloneablePtr& operator = (this_t const& other) { base_t::operator = (cloner(*other)); return *this; }

		template < typename U, class CLONER2 >
		TCloneablePtr& operator = (TCloneablePtr<U, CLONER2>&& other) {
			this->reset(other.release());
			return*this;
		}
		template < typename U, class CLONER2 >
		TCloneablePtr& operator = (TCloneablePtr<U, CLONER2> const& other) {
			static CLONER2 cloner2;
			this->reset(other ? cloner2(*other).release() : nullptr);
			return*this;
		}

		auto operator <=> (TCloneablePtr const& other) const {
			bool bEmptyA = !(*this);
			bool bEmptyB = !other;
			if (bEmptyA and bEmptyB) return std::strong_ordering::equal;
			else if (bEmptyA) return std::strong_ordering::less;
			else if (bEmptyB) return std::strong_ordering::greater;
			return (**this) <=> (*other);
		}
		bool operator == (TCloneablePtr const& other) const {
			T const* pA = this->get();
			T const* pB = other.get();
			if (pA == pB)
				return true;
			if (!pA || !pB)
				return false;
			return (*pA) == (*pB);
		}
		//bool operator != (TCloneablePtr const& other) const { return (operator <=> (other)) != 0; }
		//bool operator <  (TCloneablePtr const& other) const { return (operator <=> (other)) < 0; }
		//bool operator <= (TCloneablePtr const& other) const { return (operator <=> (other)) <= 0; }
		//bool operator >  (TCloneablePtr const& other) const { return (operator <=> (other)) > 0; }
		//bool operator >= (TCloneablePtr const& other) const { return (operator <=> (other)) >= 0; }
		//bool operator == (TCloneablePtr const& other) const = default;
		//bool operator != (TCloneablePtr const& other) const = default;
		//bool operator <  (TCloneablePtr const& other) const = default;
		//bool operator <= (TCloneablePtr const& other) const = default;
		//bool operator >  (TCloneablePtr const& other) const = default;
		//bool operator >= (TCloneablePtr const& other) const = default;

	};

}

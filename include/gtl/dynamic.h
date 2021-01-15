#pragma once

//////////////////////////////////////////////////////////////////////
//
// dynamic.h:
//
// PWH
//
//////////////////////////////////////////////////////////////////////

#include "gtl/_default.h"
#include "gtl/misc.h"

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


	//-----------------------------------------------------------------------------
	// New Object
	template < typename THIS_CLASS >
	class INewObject {
	public:
		//using this_t = THIS_CLASS;
		static std::unique_ptr<THIS_CLASS> NewObject() {
			if constexpr (std::is_abstract_v<THIS_CLASS>) {
				return nullptr;
			} else if constexpr (std::is_constructible_v<THIS_CLASS>) {
				return std::make_unique<THIS_CLASS>();
			} else {
				static_assert(false);
			}
		}
	};
#define DECLARE_NEW_OBJECT_INTERFACE(THIS_CLASS)\
	static std::unique_ptr<THIS_CLASS> NewObject() = 0;
#define DECLARE_NEW_OBJECT(THIS_CLASS)\
	static std::unique_ptr<THIS_CLASS> NewObject() {\
		return std::make_unique<THIS_CLASS>();\
	}

	//-----------------------------------------------------------------------------
	// Clone Object
	template < typename THIS_CLASS >
	class INewCloneBase : public INewObject<THIS_CLASS> {
	public:
		using base_t = THIS_CLASS;
		using this_t = THIS_CLASS;

		virtual std::unique_ptr<base_t> NewClone() const {
			if constexpr (std::is_abstract_v<this_t>) {
				// how to make it compile-time-error?
				return nullptr;
			} else {
				return std::make_unique<this_t>(*(this_t*)this);
			}
		}

		using INewObject<THIS_CLASS>::NewObject;
	};
	template < typename BASE_CLASS, typename THIS_CLASS >
	class INewCloneDerived : public BASE_CLASS, public INewObject<THIS_CLASS> {
	public:
		using base_t = typename BASE_CLASS::base_t;
		using this_t = THIS_CLASS;

		virtual std::unique_ptr<base_t> NewClone() const override {
			if constexpr (std::is_abstract_v<this_t>) {
				// how to make it compile-time-error?
				return nullptr;
			} else {
				return std::make_unique<this_t>(*(this_t*)this);
			}
		}

		using INewObject<THIS_CLASS>::NewObject;
	};

#define DECLARE_NEW_CLONE_BASE_INTERFACE(THIS_CLASS)\
	using base_t = THIS_CLASS;\
	virtual std::unique_ptr<THIS_CLASS> NewClone() const = 0;
#define DECLARE_NEW_CLONE_BASE(THIS_CLASS)\
	using base_t = THIS_CLASS;\
	virtual std::unique_ptr<THIS_CLASS> NewClone() const {\
		return std::make_unique<this_t>(*(this_t*)this);\
	}
#define DECLARE_NEW_CLONE_DERIVED(THIS_CLASS)\
	virtual std::unique_ptr<base_t> NewClone() const override {\
		return std::make_unique<THIS_CLASS>(*(THIS_CLASS*)this);\
	}


	//-----------------------------------------------------------------------------
	// Dynamic Create. (Creating with IDENTIFIER_DN)
	//

	// 2013.12.05. s_DYNAMIC_CREATE 변수의 위치를 CDYNAMIC_CREATE의 멤버변수로 두었다가 BaseClass 의 멤버로 옮김. DLL 로 만들 때, CLASS 내부의 static 변수는 Export 되지 않음.
	// 2018.12.    static 함수내부의 static 변수로 옮김. class 내의 static 변수로 있으면.... multiple instatiation 발생. (또는 multiple initialization). inline static 도 동일한 현상 발생.
	//             static 변수의 초기화 순서도 문제가 됨. 일단 static 함수내의 static 변수로 있으면 상관 없음..... 그래서 구현이 복잡해짐.
	// 2019.11.20. CRTP Class 로 변경


	//-----------------------------------------------------------------------------
	// IDynamicCreateBase
	template < typename THIS_CLASS, typename IDENTIFIER_DN >	// ID : 나중에, template argument 로 string 이 넘어가게 된다면, 대충 해결 될것 같긴한데,
	class IDynamicCreateBase : public INewCloneBase<THIS_CLASS> {
	public:

		// ID
		using identifier_dn_t = IDENTIFIER_DN;
		static inline identifier_dn_t const s_identifier_dn { };	// base class must have 'default' id {}.
		virtual const identifier_dn_t& GetIdentifierDN() const { return s_identifier_dn; }

		// New Object
		using INewObject<THIS_CLASS>::NewObject;

	private:
		// register
		using creator_t = std::function<std::unique_ptr<THIS_CLASS>()>;
		static inline std::map<identifier_dn_t, creator_t> s_tableDynamicCreate;
		friend internal::T_DYNAMIC_CREATE_Register<IDynamicCreateBase>;
		static inline internal::T_DYNAMIC_CREATE_Register<IDynamicCreateBase> s_DYNAMIC_CREATE_Register;
	protected:
		static void RegisterDynamicCreate(const identifier_dn_t& id, creator_t NewObject) {
			s_tableDynamicCreate[id] = NewObject;
		}

	public:
		// Create Object from ID
		[[nodiscard]] static std::unique_ptr<THIS_CLASS> CreateObject(const identifier_dn_t& identifier) {
			const auto& table = s_tableDynamicCreate;
			if (const auto& item = table.find(identifier); item != table.end())
				return item->second();
			throw std::invalid_argument(__FMSG "no creator");
			return nullptr;
		}

	};

	//-----------------------------------------------------------------------------
	// IDynamicCreateDerived
	//			template < auto& ID >
	//			template < const char8_t& ID[20] >
	//			template < const wchar_t& ID[20] >

	template < typename BASE_CLASS, typename THIS_CLASS, auto ... ID >
	class IDynamicCreateDerived : public INewCloneDerived<BASE_CLASS, THIS_CLASS> {
	public:

		// ID
		using identifier_dn_t = typename BASE_CLASS::identifier_dn_t;
		static inline identifier_dn_t const s_identifier_dn { ID ..., };
		virtual const identifier_dn_t& GetIdentifierDN() const override { return s_identifier_dn; }

		// New Object
		using INewObject<THIS_CLASS>::NewObject;

	private:
		// Register
		friend internal::T_DYNAMIC_CREATE_Register<IDynamicCreateDerived>;
		static inline internal::T_DYNAMIC_CREATE_Register<IDynamicCreateDerived> s_DYNAMIC_CREATE_Register;

	};


#define DECLARE_DYNAMIC_CREATE_BASE__(THIS_CLASS, IDENTIFIER_DN)\
	public:\
		using identifier_dn_t = IDENTIFIER_DN;\
		using INewObject<THIS_CLASS>::NewObject;\
	private:\
		using creator_t = std::function<std::unique_ptr<THIS_CLASS>()>;\
		static inline std::map<identifier_dn_t, creator_t> s_tableDynamicCreate;\
		friend internal::T_DYNAMIC_CREATE_Register<THIS_CLASS>;\
		static inline internal::T_DYNAMIC_CREATE_Register<THIS_CLASS> s_DYNAMIC_CREATE_Register;\
	protected:\
		static void RegisterDynamicCreate(const identifier_dn_t& id, creator_t NewObject) {\
			s_tableDynamicCreate[id] = NewObject;\
		}\
	public:\
		[[nodiscard]] static std::unique_ptr<THIS_CLASS> CreateObject(const identifier_dn_t& identifier) {\
			const auto& table = s_tableDynamicCreate;\
			if (const auto& item = table.find(identifier); item != table.end())\
				return item->second();\
			throw std::invalid_argument(__FMSG "no creator");\
			return nullptr;\
		};
#define DECLARE_DYNAMIC_CREATE__(ID)\
	static inline identifier_dn_t const s_identifier_dn { ID };\
	virtual const identifier_dn_t& GetIdentifierDN() const { return s_identifier_dn; }
#define DECLARE_DYNAMIC_CREATE_INTERFACE__()\
	static inline identifier_dn_t const s_identifier_dn { };\
	virtual const identifier_dn_t& GetIdentifierDN() const = 0;

#define DECLARE_DYNAMIC_CREATE_BASE(THIS_CLASS, IDENTIFIER_DN, ID)\
	DECLARE_DYNAMIC_CREATE_BASE__(THIS_CLASS, IDENTIFIER_DN)\
	DECLARE_DYNAMIC_CREATE__(ID)
#define DECLARE_DYNAMIC_CREATE_BASE_INTERFACE(THIS_CLASS, IDENTIFIER_DN, ID)\
	DECLARE_DYNAMIC_CREATE_BASE__(THIS_CLASS, IDENTIFIER_DN)\
	DECLARE_DYNAMIC_CREATE_INTERFACE__(ID)
#define DECLARE_DYNAMIC_CREATE_DERIVED(ID)\
	DECLARE_DYNAMIC_CREATE__(ID)


	//-----------------------------------------------------------------------------
	// DynamicSerializeOut, DynamicSerializeIn
	template < typename ARCHIVE_OUT, typename BASE_CLASS >
	//requires (requires (BASE_CLASS base) { BASE_CLASS::identifier_dn_t; BASE_CLASS::identifier_dn_serialize_as_t; base.SerializeOut; })
	//[[nodiscard]]
	inline bool DynamicSerializeOut(ARCHIVE_OUT& ar, const BASE_CLASS* pObject) {
		if (!pObject)
			return false;
		ar & (typename BASE_CLASS::identifier_dn_serialize_as_t&) pObject->GetIdentifierDN();
		pObject->SerializeOut(ar);
		return true;
	};
	template < typename ARCHIVE_IN, typename BASE_CLASS >
	//requires (requires (BASE_CLASS base) { BASE_CLASS::identifier_dn_t; BASE_CLASS::identifier_dn_serialize_as_t; base.SerializeIn; })
	[[nodiscard]]
	inline std::unique_ptr<BASE_CLASS> DynamicSerializeIn(ARCHIVE_IN& ar, BASE_CLASS* = nullptr)
	{
		typename BASE_CLASS::identifier_dn_serialize_as_t id;
		ar >> id;
		auto rObject = BASE_CLASS::CreateObject((const BASE_CLASS::identifier_dn_t&)id);
		if (rObject)
			rObject->SerializeIn(ar);
		return rObject;
	};
	//-----------------------------------------------------------------------------
	// IDynamicSerializeBase
	//   2019.11.21.
	//
	//   THIS_CLASS class must have ( ARCHIVE& operator & (ARCHIVE& ar, Data& value) {} )
	//
	template < typename THIS_CLASS,
		typename IDENTIFIER_DN,
		typename ARCHIVE_IN, typename ARCHIVE_OUT = ARCHIVE_IN,
		typename IDENTIFIER_SERIALIZE_AS = IDENTIFIER_DN,
		template <typename> typename SMART_PTR = std::unique_ptr >
	class IDynamicSerializeBase : public IDynamicCreateBase<THIS_CLASS, IDENTIFIER_DN> {
	public:
		using this_t = THIS_CLASS;
		using archive_in_t = ARCHIVE_IN;
		using archive_out_t = ARCHIVE_OUT;
		using archive_smart_ptr_t = SMART_PTR<THIS_CLASS>;
		using identifier_dn_serialize_as_t = IDENTIFIER_SERIALIZE_AS;


		static archive_smart_ptr_t DynamicSerializeIn(archive_in_t& ar) {
			return gtl::DynamicSerializeIn<archive_in_t, THIS_CLASS>(ar, (THIS_CLASS*)nullptr);
		}
		void DynamicSerializeOut(archive_out_t& ar) {
			gtl::DynamicSerializeOut<archive_out_t, THIS_CLASS>(ar, (THIS_CLASS*)this);
		}

		virtual archive_out_t& SerializeOut(archive_out_t& ar) const {
			// THIS_CLASS class must have the (operator &).
			return ar & *(const THIS_CLASS*)this;
		};
		virtual archive_in_t& SerializeIn(archive_in_t& ar) {
			// THIS_CLASS class must have the (operator &).
			return ar & *(THIS_CLASS*)this;
		};

	};



	//-----------------------------------------------------------------------------
	// IDynamicSerializeDerived
	template < typename BASE_CLASS, typename THIS_CLASS, auto ... ID >
	class IDynamicSerializeDerived : public IDynamicCreateDerived<BASE_CLASS, THIS_CLASS, ID ...> {
	public:

		using archive_in_t = typename BASE_CLASS::archive_in_t;
		using archive_out_t = typename BASE_CLASS::archive_out_t;

		virtual archive_out_t& SerializeOut(archive_out_t& ar) const override {
			// THIS_CLASS class must have the (operator &).
			return ar & *(const THIS_CLASS*)this;
		};
		virtual archive_in_t& SerializeIn(archive_in_t& ar) override {
			// THIS_CLASS class must have the (operator &).
			return ar & *(THIS_CLASS*)this;
		};

	};


#define DECLARE_DYNAMIC_SERIALIZE_BASE__(THIS_CLASS, IDENTIFIER_SERIALIZE_AS)





#pragma pack(pop)
};	// namespace gtl

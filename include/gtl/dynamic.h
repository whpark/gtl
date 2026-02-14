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
	// 2026-01-25. CRTP+MACRO again
	//

	//=============================================================================================================================
	// Dynamically Creatable, Cloneable, Comparable Interface
	//

	// from biscuit/memory.ixx
#define GTL__DYNAMIC_CLASS(KEY, ...) \
public:\
	creator_key_t const& GetObjectKeyName() const override { return s_dynamicCreateHelper.m_id; }\
	std::unique_ptr<base_t> clone() const override { return std::make_unique<this_t>(*this); }\
	bool Equals(base_t const& b) const override { return typeid(*this) == typeid(b) and (*this == (this_t const&)b); }\
	std::partial_ordering Compare(base_t const& b) const override { if (typeid(*this) != typeid(b)) return std::partial_ordering::unordered; return (*this) <=> (this_t const&)b; }\
private:\
	static inline sDynamicCreateRegister s_dynamicCreateHelper{std::move(KEY), []{ return std::make_unique<this_t>(__VA_ARGS__); }};\

	//-------------------------------------------------------------------------
	template < typename tSelf, typename tKey >
	struct TDynamicCreator {
		// Dynamic Create
	public:
		using base_t = tSelf;
	public:
		auto operator <=> (TDynamicCreator const&) const = default;
		bool operator == (TDynamicCreator const&) const = default;
	protected:
		using creator_func_t = std::function<std::unique_ptr<base_t>()>;
		using creator_key_t = tKey;
		using creator_map_t = std::map<creator_key_t, creator_func_t>;

	public:
		virtual creator_key_t const& GetObjectKeyName() const = 0;
		virtual std::unique_ptr<base_t> clone() const = 0;	// "clone" : de facto standard name
		virtual bool Equals(tSelf const&) const = 0;
		virtual std::partial_ordering Compare(tSelf const&) const = 0;

	private:
		static inline creator_map_t& GetDynamicCreatorTable() { static creator_map_t tableDynamicCreate; return tableDynamicCreate; }
	public:
		static inline std::vector<creator_key_t> GetDynamicCreatableObjectKeyNameList() {
			std::vector<creator_key_t> types;
			auto const& map = GetDynamicCreatorTable();
			for (auto const& [key, _] : map)
				types.push_back(key);
			return types;
		}
	protected:
		static void RegisterDynamicCreator(creator_key_t const& id, creator_func_t&& Creator) { GetDynamicCreatorTable()[id] = std::move(Creator); }
	public:
		[[nodiscard]] static std::unique_ptr<base_t> CreateObject(creator_key_t const& identifier) {
			auto const& map = GetDynamicCreatorTable();
			if (auto iter = map.find(identifier); iter != map.end())
				return iter->second();
			return nullptr;
		};
	protected:
		struct sDynamicCreateRegister {
			creator_key_t m_id;
			sDynamicCreateRegister(creator_key_t&& id, creator_func_t&& Creator) : m_id(id) {
				RegisterDynamicCreator(id, std::move(Creator));
			};
		};
	};

	//-------------------------------------------------------------------------
	template < typename T >
	struct TCloner {
		std::unique_ptr<T> operator () (T const& self) const { return self.clone(); }
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

		TCloneablePtr() = default;
		TCloneablePtr(this_t const& other) : base_t(other ? cloner(*other) : nullptr) {}
		TCloneablePtr(this_t&& other) = default;
		TCloneablePtr& operator = (this_t const& other) { base_t::operator = (cloner(*other)); return *this; }
		TCloneablePtr& operator = (this_t&& other) = default;

		// move from unique_ptr
		template < typename U > requires std::is_base_of_v<T, U>
		TCloneablePtr(std::unique_ptr<U>&& other) : base_t(std::move(other)) {}
		template < typename U > requires std::is_base_of_v<T, U>
		TCloneablePtr& operator = (std::unique_ptr<U>&& other) { base_t::operator = (std::move(other)); return *this; }

		// copy from unique_ptr
		template < typename U > requires std::is_base_of_v<T, U>
		TCloneablePtr(std::unique_ptr<U> const& other) : base_t(other ? cloner(*other) : nullptr) {}
		template < typename U > requires std::is_base_of_v<T, U>
		TCloneablePtr& operator = (std::unique_ptr<U> const& other) { base_t::operator = (cloner(*other)); return *this; }

		template < typename U, class CLONER2 > requires std::is_base_of_v<T, U>
		TCloneablePtr& operator = (TCloneablePtr<U, CLONER2>&& other) {
			this->reset(other.release());
			return*this;
		}
		template < typename U, class CLONER2 > requires std::is_base_of_v<T, U>
		TCloneablePtr& operator = (TCloneablePtr<U, CLONER2> const& other) {
			static CLONER2 cloner2;
			this->reset(other ? cloner2(*other).release() : nullptr);
			return*this;
		}

		std::partial_ordering operator <=> (TCloneablePtr const& other) const {
			bool bEmptyA = !(*this);
			bool bEmptyB = !other;
			if (bEmptyA and bEmptyB) return std::partial_ordering::equivalent;
			else if (bEmptyA) return std::partial_ordering::less;
			else if (bEmptyB) return std::partial_ordering::greater;
			if constexpr (requires (T const& a, T const& b) { a.Compare(b); }) {
				return (*this)->Compare(*other);
			}
			else {
				return (**this) <=> (*other);
			}
		}
		bool operator == (TCloneablePtr const& other) const {
			T const* pA = this->get();
			T const* pB = other.get();
			if (pA == pB)
				return true;
			if (!pA || !pB)
				return false;
			if constexpr (requires (T const& a, T const& b) { a.Equals(b); }) {
				return pA->Equals(*pB);
			}
			else {
				return (*pA) == (*pB);
			}
		}
	};

}

///////////////////////////////////////////////////////////////////////////////
//
// ptr_container.h
//
//			GTL : (G)reen (T)ea (L)atte
//
// 2017.07.20.
// 2017.04.23. container : vector<unique_ptr> 추가
// 2017.07.24. container : Sort 제외 IList 기능들 추가.
// 2017.07.28. container : 1 차 완료 예정.
// 2018.03.10. Move Constructor 추가
//             TSmartPtr... --> TUniquePtr... / TSharedPtr... 로 세분화
// 2018.04.01. operator == 추가.
// 2022.04.05. 전체 재작업.
//
// PWH
//
///////////////////////////////////////////////////////////////////////////////

module;

#include <memory>
#include <vector>
#include <deque>

#include "gtl/_config.h"
#include "gtl/_macro.h"

export module gtl:ptr_container;

export namespace gtl {

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	// Iterator ( iterator, const_iterator )
	template < typename base_iterator >
	class TPtrIterator : public base_iterator {
	public:
		using this_t = TPtrIterator;
		using base_t = base_iterator;
		using value_type = typename base_t::value_type::element_type;

	public:
		// constructor
		using base_t::base_t;
		constexpr TPtrIterator(base_t const& base) : base_t(base) { }

		// operators
		using base_t::operator =;
		using base_t::operator +;
		using base_t::operator ++;
		using base_t::operator +=;
		using base_t::operator -;
		using base_t::operator --;
		using base_t::operator -=;
		auto operator <=> (TPtrIterator const&) const = default;

		value_type&			operator * ()			{ return *(base_t::operator *()); }
		value_type const&	operator * () const		{ return *(base_t::operator *()); }
		value_type*			operator -> ()			{ return base_t::operator *(); }
		value_type const*	operator -> () const	{ return base_t::operator *(); }

		auto&		GetPtr()		{ return base_t::operator*(); }
		auto const&	GetPtr() const	{ return base_t::operator*(); }
	};

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	// TPtrContainer
	template < typename TData, template < typename TData > typename TPtr, template < typename TPtr > typename base_container >
	class TPtrContainer : public base_container<TPtr<TData>> {
	public:
		// alias
		using this_t = TPtrContainer;
		using base_t = base_container<TPtr<TData>>;

		using value_type      = TData;
		using reference       = TData&;
		using const_reference = const TData&;

		using iterator					= TPtrIterator<typename base_t::iterator>;
		using const_iterator			= TPtrIterator<typename base_t::const_iterator>;
		using reverse_iterator			= TPtrIterator<typename base_t::reverse_iterator>;
		using const_reverse_iterator	= TPtrIterator<typename base_t::const_reverse_iterator>;

		// constructor
		using base_t::base_t;

		// begin/end
		constexpr iterator					begin()			{ return base_t::begin(); }
		constexpr iterator					end()			{ return base_t::end(); }
		constexpr const_iterator			begin()	const	{ return base_t::cbegin(); }
		constexpr const_iterator			end() const		{ return base_t::cend(); }
		constexpr const_iterator			cbegin() const	{ return base_t::cbegin(); }
		constexpr const_iterator			cend() const	{ return base_t::cend(); }

		constexpr reverse_iterator			rbegin()		{ return base_t::rbegin(); }
		constexpr reverse_iterator			rend()			{ return base_t::rend(); }
		constexpr const_reverse_iterator	rbegin() const	{ return base_t::crbegin(); }
		constexpr const_reverse_iterator	rend() const	{ return base_t::crend(); }
		constexpr const_reverse_iterator	crbegin() const	{ return base_t::crbegin(); }
		constexpr const_reverse_iterator	crend() const	{ return base_t::crend(); }

		// front/back
		constexpr auto&			front()			{ return *base_t::front(); }
		constexpr auto const&	front() const	{ return *base_t::front(); }
		constexpr auto&			back()			{ return *base_t::back(); }
		constexpr auto const&	back() const	{ return *base_t::back(); }

		// at
		constexpr auto&			at(base_t::size_type offset)		{ return *(base_t::operator [](offset)); }
		constexpr auto const&	at(base_t::size_type offset) const	{ return *(base_t::operator [](offset)); }

		// operator []
		auto&		operator [](base_t::size_type offset)		{ return *(base_t::operator [](offset)); }
		auto const& operator [](base_t::size_type offset) const { return *(base_t::operator [](offset)); }

		// Push/Pop
		TPtr<TData> PopFront() requires requires (base_t container) { container.pop_front(); } {
			auto r = std::move(base_t::front());
			base_t::pop_front();
			return r;
		}
		TPtr<TData> PopBack() {
			auto r = std::move(base_t::back());
			base_t::pop_back();
			return r;
		}

		// operator <=>
		auto operator <=> (this_t const&) const = default;
	};


	// alias

	template < typename TData >
	using uptr_vector = TPtrContainer<TData, std::unique_ptr, std::vector>;
	template < typename TData >
	using sptr_vector = TPtrContainer<TData, std::shared_ptr, std::vector>;

	template < typename TData >
	using uptr_deque = TPtrContainer<TData, std::unique_ptr, std::deque>;
	template < typename TData >
	using sptr_deque = TPtrContainer<TData, std::shared_ptr, std::deque>;

}	// namespace gtl

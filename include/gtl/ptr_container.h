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

#pragma once

#include <memory>
#include <vector>
#include <deque>
#include "gtl/mutex.h"

namespace gtl {

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
	template < typename TData, template < typename TData > typename TPtr, template < typename TPtr > typename base_container, typename TMutex = gtl::null_mutex >
	class TPtrContainer : public base_container<TPtr<TData>>, public TMutex {
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

		constexpr static bool has_push_pop_front_ = requires(base_t base) {base.push_back; base.pop_back};

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

		// operator <=>
		auto operator <=> (this_t const&) const = default;

		// Concurrent Operation (Thread Safe operation)
		constexpr void push_front(TPtr<TData>&& r) requires has_push_pop_front_ {
			std::unique_lock lock(*this);
			base_t::push_front(std::move(r));
		}
		constexpr void push_back(TPtr<TData>&& r) {
			std::unique_lock lock(*this);
			base_t::push_back(std::move(r));
		}
		constexpr [[nodiscard]] TPtr<TData> pop_front() requires has_push_pop_front_ {
			std::unique_lock lock(*this);
			auto r = std::move(base_t::front());
			base_t::pop_front();
			return r;
		}
		constexpr [[nodiscard]] TPtr<TData> pop_back() {
			std::unique_lock lock(*this);
			auto r = std::move(base_t::back());
			base_t::pop_back();
			return r;
		}
		constexpr void Insert(auto&& where, TPtr<TData>&& r) {
			std::unique_lock lock(*this);
			base_t::insert(where, std::move(r));
		}
	};


	// alias

	template < typename TData >
	using xUPtrVector = TPtrContainer<TData, std::unique_ptr, std::vector>;
	template < typename TData >
	using xSPtrVector = TPtrContainer<TData, std::shared_ptr, std::vector>;

	template < typename TData >
	using xUPtrDeque = TPtrContainer<TData, std::unique_ptr, std::deque>;
	template < typename TData >
	using xSPtrDeque = TPtrContainer<TData, std::shared_ptr, std::deque>;

	template < typename TData >
	using xConcurrentUPtrVector = TPtrContainer<TData, std::unique_ptr, std::vector, gtl::recursive_shared_mutex>;
	template < typename TData >
	using xConcurrentSPtrVector = TPtrContainer<TData, std::shared_ptr, std::vector, gtl::recursive_shared_mutex>;

	template < typename TData >
	using xConcurrentUPtrDeque = TPtrContainer<TData, std::unique_ptr, std::deque, gtl::recursive_shared_mutex>;
	template < typename TData >
	using xConcurrentSPtrDeque = TPtrContainer<TData, std::shared_ptr, std::deque, gtl::recursive_shared_mutex>;

}	// namespace gtl

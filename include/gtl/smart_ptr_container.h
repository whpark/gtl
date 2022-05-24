///////////////////////////////////////////////////////////////////////////////
//
// smart_ptr_container.h
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
	class TSmartPtrIterator : public base_iterator {
	public:
		using this_t = TSmartPtrIterator;
		using base_t = base_iterator;
		using value_type = typename base_t::value_type::element_type;

	public:
		// constructor
		using base_t::base_t;
		constexpr TSmartPtrIterator(base_t const& base) : base_t(base) { }

		// operators
		using base_t::operator =;
		using base_t::operator +;
		using base_t::operator ++;
		using base_t::operator +=;
		using base_t::operator -;
		using base_t::operator --;
		using base_t::operator -=;
		auto operator <=> (TSmartPtrIterator const&) const = default;

		value_type&			operator * ()			{ return *(base_t::operator *()); }
		value_type const&	operator * () const		{ return *(base_t::operator *()); }
		value_type*			operator -> ()			{ return (base_t::operator *()).get(); }
		value_type const*	operator -> () const	{ return (base_t::operator *()).get(); }

		auto&		GetSmartPtr()		{ return base_t::operator*(); }
		auto const&	GetSmartPtr() const	{ return base_t::operator*(); }
	};

	template < typename TCONTAINER >
	concept has_push_pop_front = requires(TCONTAINER container) {container.push_front; container.pop_front;};
	template < typename T >
	concept value_has_clone = requires (T object) { object.NewClone(); };

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	// TSmartPtrContainer
	template < typename T, template < typename T > typename TSmartPtr, template < typename TSmartPtr > typename base_container, typename TMutex = gtl::null_mutex >
	class TSmartPtrContainer : public base_container<TSmartPtr<T>>, public TMutex {
	public:
		// alias
		using this_t = TSmartPtrContainer;
		using base_t = base_container<TSmartPtr<T>>;

		using value_type      = T;
		using reference       = T&;
		using const_reference = const T&;

		using iterator					= TSmartPtrIterator<typename base_t::iterator>;
		using const_iterator			= TSmartPtrIterator<typename base_t::const_iterator>;
		using reverse_iterator			= TSmartPtrIterator<typename base_t::reverse_iterator>;
		using const_reverse_iterator	= TSmartPtrIterator<typename base_t::const_reverse_iterator>;

		//constexpr static auto base_has_push_pop_front_ = requires(base_t base) {base.push_front; base.pop_front;};
		//constexpr static auto value_has_clone_ = requires (T object) { object.Clone(); };

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
		constexpr auto&						front()			{ return *base_t::front(); }
		constexpr auto const&				front() const	{ return *base_t::front(); }
		constexpr auto&						back()			{ return *base_t::back(); }
		constexpr auto const&				back() const	{ return *base_t::back(); }

		// at
		constexpr auto&						at(base_t::size_type offset)		{ return *(base_t::operator [](offset)); }
		constexpr auto const&				at(base_t::size_type offset) const	{ return *(base_t::operator [](offset)); }

		// operator []
		constexpr auto&						operator [](base_t::size_type offset)		{ return *(base_t::operator [](offset)); }
		constexpr auto const&				operator [](base_t::size_type offset) const { return *(base_t::operator [](offset)); }

		// operator <=>
		auto operator <=> (this_t const&) const = default;

		// Concurrent Operation (Thread Safe operation)
		constexpr auto& push_front(TSmartPtr<T>&& r) requires has_push_pop_front<base_t> {
			std::unique_lock lock(*this);
			base_t::push_front(std::move(r));
			return front();
		}
		constexpr auto& push_front(T* ptr) requires has_push_pop_front<base_t> {
			std::unique_lock lock(*this);
			base_t::emplace_front(ptr);
			return front();
		}
		constexpr auto& push_back(TSmartPtr<T>&& r) {
			std::unique_lock lock(*this);
			base_t::push_back(std::move(r));
			return back();
		}
		constexpr auto& push_back(T* ptr) {
			std::unique_lock lock(*this);
			base_t::emplace_back(ptr);
			return back();
		}
		constexpr [[nodiscard]] TSmartPtr<T> pop_front() requires has_push_pop_front<base_t> {
			std::unique_lock lock(*this);
			if (base_t::empty())
				return {};
			auto r = std::move(base_t::front());
			base_t::pop_front();
			return r;
		}
		constexpr [[nodiscard]] TSmartPtr<T> pop_back() {
			std::unique_lock lock(*this);
			if (base_t::empty())
				return {};
			auto r = std::move(base_t::back());
			base_t::pop_back();
			return r;
		}
		constexpr auto& insert(auto&& where, TSmartPtr<T>&& r) {
			std::unique_lock lock(*this);
			return base_t::emplace(where, std::move(r));
		}
		constexpr auto& insert(size_t index, TSmartPtr<T>&& r) {
			std::unique_lock lock(*this);
			return base_t::emplace(base_t::begin()+index, std::move(r));
		}
		template <class... TValue>
		inline auto& emplace_front(TValue&&... values) requires has_push_pop_front<base_t> {
			std::unique_lock lock(*this);
			return base_t::emplace_front(new T{std::forward<TValue>(values)...});
		}
		template <class... TValue>
		inline auto& emplace_back(TValue&&... values) {
			std::unique_lock lock(*this);
			return emplace_back(new T{std::forward<TValue>(values)...});
		}
		template <class... TValue>
		inline auto& emplace(auto&& where, TValue&&... values) {
			std::unique_lock lock(*this);
			return *base_t::emplace(where, new T{std::forward<TValue>(values)...});
		}

		iterator erase(const_iterator where) {
			std::unique_lock lock(*this);
			return base_t::erase(where);
		}
		iterator erase(const_iterator iter0, const_iterator iter1) {
			std::unique_lock lock(*this);
			return base_t::erase(iter0, iter1);
		}

		void clear() { // erase all
			std::unique_lock lock(*this);
			base_t::clear();
		}

		void swap(this_t&) = delete;

		//void assign(std::initializer_list<T>) = delete;
		//iterator insert(const_iterator, std::initializer_list<T>) = delete;

		// helper
		constexpr base_t&		Base()			{ return *this; }
		constexpr base_t const&	Base() const	{ return *this; }

		template < typename ... Arg >
		constexpr void AddCloneTo(TSmartPtrContainer<Arg...>& container) const requires (value_has_clone<T> || std::is_copy_constructible_v<T>) {
			std::shared_lock lock(*this);
			if constexpr (value_has_clone<T>) {
				for (auto const& obj : *this)
					container.push_back(obj.NewClone());
			}
			else if constexpr (std::is_copy_constructible_v<T>) {
				for (auto const& obj : *this)
					container.push_back(new T{obj});
			}
		}
		template < typename ... Arg >
		constexpr void AddCloneFrom(TSmartPtrContainer const& container) requires (value_has_clone<T> || std::is_copy_constructible_v<T>) {
			std::shared_lock lock(*this);
			if constexpr (value_has_clone<T>) {
				for (auto const& obj : container)
					push_back(obj.NewClone());
			}
			else if constexpr (std::is_copy_constructible_v<T>) {
				for (auto const& obj : container)
					push_back(new T{obj});
			}
		}
		template < typename ... Arg >
		constexpr void AddRefTo(TSmartPtrContainer<Arg...>& container) const requires std::is_convertible_v<TSmartPtr<T>, std::shared_ptr<T>> {
			std::shared_lock lock(*this);
			//std::unique_lock lock2(container);
			for (auto sharedptr : Base())
				container.push_back(std::move(sharedptr));
		}
		template < typename ... Arg >
		constexpr void AddRefFrom(TSmartPtrContainer<Arg...> const& container) requires std::is_convertible_v<TSmartPtr<T>, std::shared_ptr<T>> {
			std::shared_lock lock(*this);
			for (auto sharedptr : container.Base())
				push_back(std::move(sharedptr));
		}

		bool Contains(T const& obj) {
			std::shared_lock lock(*this);
			for (auto const& v : *this) {
				if (obj == v)
					return true;
			}
			return false;
		}
		size_t Find(T const& obj) const {
			std::shared_lock lock(*this);
			for (size_t i{}; i < base_t::size(); i++) {
				if (at(i) == obj)
					return i;
			}
			return (size_t)-1;
		}
		size_t Find(T const* ptr) const {
			std::shared_lock lock(*this);
			for (size_t i{}; i < base_t::size(); i++) {
				if (at(i).ptr() == ptr)
					return i;
			}
			return (size_t)-1;
		}
		auto FindIter(T const& obj) {
			std::shared_lock lock(*this);
			for (auto iter = begin(); iter != end(); iter++) {
				if (*iter == obj)
					return iter;
			}
			return end();
		}
		auto FindIter(T const& obj) const {
			std::shared_lock lock(*this);
			for (auto iter = begin(); iter != end(); iter++) {
				if (*iter == obj)
					return iter;
			}
			return end();
		}
		auto FindIter(T const* ptr) {
			std::shared_lock lock(*this);
			for (auto iter = begin(); iter != end(); iter++) {
				if (iter.GetSmartPtr().get() == ptr)
					return iter;
			}
			return end();
		}
		auto FindIter(T const* ptr) const {
			std::shared_lock lock(*this);
			for (auto iter = begin(); iter != end(); iter++) {
				if (iter.GetSmartPtr().get() == ptr)
					return iter;
			}
			return end();
		}

		template < typename ... TArgs >
		decltype(auto) Push(TArgs&& ...args) {
			return push_back(std::forward<TArgs>(args)...);
		}
		decltype(auto) Pop() {
			return pop_back();
		}

		bool Delete(size_t index) {
			if (index > base_t::size())
				return false;
			std::unique_lock lock(*this);
			base_t::erase(begin()+index);
			return true;
		}
		bool Delete(T const* ptr) {
			std::unique_lock lock(*this);
			if (auto iter = FindIter(ptr); iter != end()) {
				base_t::erase(iter);
				return true;
			}
			return false;
		}

	};


	// alias
	template < typename T >
	using TUPtrVector = TSmartPtrContainer<T, std::unique_ptr, std::vector>;
	template < typename T >
	using TSPtrVector = TSmartPtrContainer<T, std::shared_ptr, std::vector>;

	template < typename T >
	using TUPtrDeque = TSmartPtrContainer<T, std::unique_ptr, std::deque>;
	template < typename T >
	using TSPtrDeque = TSmartPtrContainer<T, std::shared_ptr, std::deque>;

	template < typename T >
	using TConcurrentUPtrVector = TSmartPtrContainer<T, std::unique_ptr, std::vector, gtl::recursive_shared_mutex>;
	template < typename T >
	using TConcurrentSPtrVector = TSmartPtrContainer<T, std::shared_ptr, std::vector, gtl::recursive_shared_mutex>;

	template < typename T >
	using TConcurrentUPtrDeque = TSmartPtrContainer<T, std::unique_ptr, std::deque, gtl::recursive_shared_mutex>;
	template < typename T >
	using TConcurrentSPtrDeque = TSmartPtrContainer<T, std::shared_ptr, std::deque, gtl::recursive_shared_mutex>;

}	// namespace gtl

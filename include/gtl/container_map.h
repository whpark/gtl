///////////////////////////////////////////////////////////////////////////////
//
// vector_map.h
//
//			GTL : (G)reen (T)ea (L)atte
//
// 2023-10-23. a simple map class using std container and std::pair (originally, gtl::TVectorMap)
//
// PWH
//
///////////////////////////////////////////////////////////////////////////////


#pragma once

#include <vector>
#include <deque>
#include <list>
#include <algorithm>

namespace gtl {

	template <template <typename ...TArg> typename TContainer, class TKey, class T, class TKeyEq = std::equal_to<TKey>, class TAlloc = std::allocator<std::pair<TKey const, T>> >
	class TContainerMap : protected TContainer<std::pair<TKey const, T>, TAlloc> {
	public:
		using this_t = TContainerMap;
		using base_t = TContainer<std::pair<TKey const, T>/*, TAlloc*/>;

	public:
		using base_t::base_t;
		using base_t::operator =;

		auto operator <=> (this_t const&) const noexcept = default;

	public:
		base_t& base() noexcept { return *this; }
		base_t const& base() const noexcept { return *this; }
		~TContainerMap() = default;

		using base_t::begin;
		using base_t::end;
		using base_t::cbegin;
		using base_t::cend;
		using base_t::rbegin;
		using base_t::rend;
		using base_t::crbegin;
		using base_t::crend;

		using base_t::front;
		using base_t::back;
		//using base_t::data;
		constexpr decltype(auto) data() const noexcept requires requires (base_t container) { container.data(); } {
			return base_t::data();
		}

		using base_t::empty;
		using base_t::size;
		using base_t::max_size;
		//using base_t::reserve;
		constexpr void reserve(size_t size) requires requires (base_t container) { container.reserve(size); } {
			return base_t::reserve(size);
		}
		//using base_t::capacity;
		constexpr auto capacity() const noexcept requires requires (base_t container) { container.capacity(); } {
			return base_t::capacity();
		}
		//using base_t::shrink_to_fit;
		constexpr void shrink_to_fit() requires requires (base_t container) { container.shrink_to_fit(); } {
			base_t::shrink_to_fit();
		}

		using base_t::clear;
		using base_t::swap;

		template < typename TKey2 > requires (requires(TKey2 key2) { TKey{} = key2; })
		auto& operator[](TKey2 const& key) {
			return find(key)->second;
		}
		template < typename TKey2 > requires (requires(TKey2 key2) { TKey{} = key2; })
		auto& operator[](TKey2 const& key) const {
			if (auto iter = find(key); iter != end()) {
				return iter->second;
			}
			throw std::out_of_range("TDequeMap::operator[]");
		}

		template < typename TKey2 > requires (requires(TKey2 key2) { TKey{} = key2; })
		auto find(TKey2 const& key) const {
			return std::find_if(begin(), end(),
				[&key](auto const& pair) { return TKeyEq{}(pair.first, key); });
		}
		template < typename TKey2 > requires (requires(TKey2 key2) { TKey{} = key2; })
		auto find(TKey2 const& key) {
			if (auto iter = std::find_if(begin(), end(),
				[&key](auto const& pair) { return TKeyEq{}(pair.first, key); });
				iter != end())
			{
				return iter;
			}
			base_t::emplace_back(key, T{});
			auto iter = end();
			return --iter;
		}

		template < typename TKey2 > requires (requires(TKey2 key2) { TKey{} = key2; })
		void insert(TKey2 const& key, T&& value) {
			if (auto it = find(key); it != end()) {
				// Key already exists, update the value
				it->second = std::move(value);
			} else {
				base_t::emplace_back(key, std::move(value));
			}
		}
		template < typename TKey2 > requires (requires(TKey2 key2) { TKey{} = key2; })
		void insert(TKey2 const& key, T const& value) {
			if (auto it = find(key); it != end()) {
				// Key already exists, update the value
				it->second = value;
			} else {
				base_t::emplace_back(key, value);
			}
		}

		template < typename TKey2 > requires (requires(TKey2 key2) { TKey{} = key2; })
		bool erase(TKey2 const& key) {
			if (auto it = find(key); it != end()) {
				base_t::erase(it);
				return true;
			}
			return false;
		}

	};

}	// namespace gtl

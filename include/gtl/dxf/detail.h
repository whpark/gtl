#pragma once

#include "gtl/gtl.h"
#include "gtl/container_map.h"
#include "gtl/dynamic.h"
#include "gtl/shape/shape.h"
#include "gtl/shape/color_table.h"

#include <boost/pfr.hpp>

#include <expected>
#include <span>

namespace gtl {

	template <typename T>
	constexpr std::size_t CountStructMember() noexcept {
		using value_t = std::remove_cvref_t<T>;
		if constexpr (requires { typename value_t::coord_t; })
			return std::tuple_size_v<typename value_t::coord_t>;
		else
			return boost::pfr::tuple_size_v<value_t>;
	}

	template <std::size_t I, typename T>
	constexpr decltype(auto) GetStructMember(T&& value) noexcept {
		using value_t = std::remove_cvref_t<T>;
		if constexpr (requires { typename value_t::coord_t; })
			return std::forward<T>(value).member(I);
		else
			return boost::pfr::get<I>(std::forward<T>(value));
	}

	template <std::size_t I, typename T>
	using struct_member_t = std::remove_cvref_t<decltype(GetStructMember<I>(std::declval<T&>()))>;

	namespace dxf::detail {
		template <std::size_t N, typename Func, std::size_t... I>
		constexpr bool ForEachIntSeq(Func&& func, std::index_sequence<I...>) {
			return (static_cast<bool>(func.template operator()<I>()) || ...);
		}
	}

	template <std::size_t N, typename Func>
	constexpr bool ForEachIntSeq(Func&& func) {
		return dxf::detail::ForEachIntSeq<N>(
			std::forward<Func>(func), std::make_index_sequence<N>{});
	}

} // namespace gtl

#define GTL__DXF_DEFINE_SPACESHIP_OPERATOR(CLASS_NAME) \
	auto operator<=>(CLASS_NAME const&) const = default;

#define GTL__DXF_ENTITY_DERIVED(tEntity, eEntityType, NAME, tSubclasses) \
	using tEntity = TEntityDerived<eEntityType, NAME, tSubclasses>;

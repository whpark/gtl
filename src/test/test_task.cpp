#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/string.h"

using namespace std::literals;
using namespace gtl::literals;

namespace gtl {
	template < typename TFunc >
	class xTask : public std::packaged_task<TFunc> {
	public:
		std::optional<std::jthread> m_worker;
	public:
		using base_t = std::packaged_task<TFunc>;
		using this_t = xTask;

	public:
		using base_t::base_t;
		using base_t::operator();

		xTask() = default;
		xTask(this_t&&) = default;
		this_t& operator=(this_t&&) = default;

		template < typename ...TArgs >
		void Execute(TArgs&&... args) {
			m_worker = std::jthread([&]() {
				(*this)(std::forward<TArgs>(args)...);
				});
		}
		//void Execute(TArgs&&... args) requires requires (base_t task) { task(std::stop_token{}, std::forward<TArgs>(args)...); } {
		//	m_worker = std::jthread([&](std::stop_token stop) {
		//		(*this)(stop, std::forward<TArgs>(args)...);
		//		});
		//}
		decltype(auto) get() {
			return base_t::get_future().get();
		}
		
	};

#define _XTASK_DEDUCTION_GUIDE(CALL_OPT, X1, X2, X3) \
    template <class _Ret, class... _Types>                   \
    xTask(_Ret(CALL_OPT*)(_Types...)) -> xTask<_Ret(_Types...)>; // intentionally discards CALL_OPT

_NON_MEMBER_CALL(_XTASK_DEDUCTION_GUIDE, X1, X2, X3)
#undef _XTASK_DEDUCTION_GUIDE

	//template <class _Fx>
	//xTask(_Fx) -> xTask<typename std::_Deduce_signature<_Fx>::type>;

};


double accum(std::span<double>::iterator beg, std::span<double>::iterator end, double init) {
	fmt::println("dist :{}", std::distance(beg, end));
	auto sum = std::accumulate(beg, end, init);
	return sum;
}

double comp2(std::span<double> v) {
	gtl::xTask pt0{accum};
	gtl::xTask pt1{accum};
	auto beg = v.begin();
	auto mid = v.begin() + v.size()/2;
	auto end = v.end();
	fmt::println("v : count : {}", v.size());
	pt0.Execute(beg, mid, 0.0);
	pt1.Execute(mid, end, 0.0);

	return pt0.get() + pt1.get();
}

TEST(gtl_task, task) {
	std::vector<double> v{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	auto r = comp2(v);
	EXPECT_TRUE(r == 55);
}

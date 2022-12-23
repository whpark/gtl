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


double accum(double* beg, double* end, double init) {
	return std::accumulate(&*beg, &*end, init);
}

double comp2(std::vector<double>& v) {
	gtl::xTask pt0{accum};
	gtl::xTask pt1{accum};
	double* first = &v[0];
	pt0.Execute(first, first+v.size()/2, 0.0);
	pt1.Execute(first+v.size()/2, first+v.size(), 0.0);

	return pt0.get() + pt1.get();
}

TEST(gtl_task, task) {
	std::vector<double> v{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	auto r = comp2(v);
	EXPECT_TRUE(r == 55);
}

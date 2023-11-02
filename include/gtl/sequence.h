#pragma once

//////////////////////////////////////////////////////////////////////
//
// sequence.h: Sequence Loop (using co-routine) Mocha::MIP like ...
//
// PWH
// 2023-10-27
//
//////////////////////////////////////////////////////////////////////

#include <coroutine>
#include <vector>
#include <concepts>
#include <list>
#include <functional>
#include <optional>

#include "glaze/glaze.hpp"

#include "gtl/_lib_gtl.h"
#include "gtl/concepts.h"

#include <format>
#include <debugapi.h>
#undef max
#undef min

namespace gtl::seq::inline v01 {

	using clock_t= std::chrono::high_resolution_clock;
	using ms_t = std::chrono::milliseconds;
	using id_t = std::string;

	//-------------------------------------------------------------------------
	/// @brief sequence function parameter (in/out)
	struct sParam {
		glz::json_t in, out;
	};

	//-------------------------------------------------------------------------
	/// @brief used as co_yield value
	struct sState {
		clock_t::time_point tNextDispatch{};
		clock_t::time_point tNextDispatchChild{clock_t::time_point::max()};
		//bool bDone{false};

		sState(clock_t::time_point t = clock_t::now()) : tNextDispatch(t) {}
		sState(clock_t::duration d) {
			tNextDispatch = (d.count() == 0) ? clock_t::time_point{} : clock_t::now() + d;
		}
		sState(std::suspend_always ) : tNextDispatch(clock_t::time_point::max()) {}
		sState(std::suspend_never ) : tNextDispatch{} {}
		sState(sState const&) = default;
		sState(sState&&) = default;
		sState& operator = (sState const&) = default;
		sState& operator = (sState&&) = default;

		void Clear() { *this = {}; }
	};

	//-------------------------------------------------------------------------
	/// @brief sequence dispatcher
	struct sSequence {
	public:
		struct promise_type;
		using coroutine_handle_t = std::coroutine_handle<promise_type>;
		//-------------------------------------------------------------------------
		/// @brief 
		struct promise_type {
			sState m_state;
			std::exception_ptr m_exception;

			coroutine_handle_t get_return_object() {
				return coroutine_handle_t::from_promise(*this);
			}
			std::suspend_always initial_suspend() { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }
			void unhandled_exception() { m_exception = std::current_exception(); }

			std::suspend_always yield_value(sState state) {
				m_state = state;
				return {};
			}
			void return_void() {}
		};

	protected:
		std::thread::id const m_threadID{std::this_thread::get_id()};
		id_t m_name;
		coroutine_handle_t m_handle;
		clock_t::time_point m_timeout{clock_t::time_point::max()};
		sState m_state;
		std::mutex m_mtxChildren;
		std::list<sSequence> m_children;

	public:
		// constructor
		sSequence(id_t name = "") : m_name(std::move(name)) {}
		sSequence(coroutine_handle_t&& h) : m_handle(std::exchange(h, nullptr)) {}
		sSequence(sSequence const&) = delete;
		sSequence& operator = (sSequence const&) = delete;
		sSequence(sSequence&& b) {
			m_name.swap(b.m_name);
			m_handle = std::exchange(b.m_handle, nullptr);
			m_timeout = std::exchange(b.m_timeout, {});
			m_state = std::exchange(b.m_state, {});
			m_children.swap(b.m_children);
		}
		sSequence& operator = (sSequence&& b) {
			Destroy();
			m_name.swap(b.m_name);
			m_handle = std::exchange(b.m_handle, nullptr);
			m_timeout = std::exchange(b.m_timeout, {});
			m_state = std::exchange(b.m_state, {});
			m_children.swap(b.m_children);
			return *this;
		}
		//sSequence& operator = (coroutine_handle_t&& h) {
		//	Destroy();
		//	m_handle = std::exchange(h, nullptr);
		//	return *this;
		//}
		void SetName(id_t name) {
			this->m_name = std::move(name);
		}

		// destructor
		~sSequence() {
			Destroy();
		}
		inline void Destroy() {
			m_name.clear();
			if (auto h = std::exchange(m_handle, nullptr); h) {
				h.destroy();
			}
		}

		inline bool IsDone() const {
			return m_children.empty() and (!m_handle or m_handle.done());
		}

		/// @brief 
		/// @return Get Next Dispatch Time
		/// this function seems to be very heavy. need to optimize.
		clock_t::time_point GetNextDispatchTime(bool bRefreshChild = false) const {
			auto t = clock_t::time_point::max();
			if (bRefreshChild) {
				for (auto const& child : m_children) {
					t = std::min(t, child.GetNextDispatchTime(bRefreshChild));
				}
			}
			else {
				if (m_children.size())
					t = std::min(t, m_state.tNextDispatchChild);
			}
			if (m_children.empty() and m_handle and !m_handle.done())
				t = std::min(t, m_state.tNextDispatch);
			return t;
		}

		/// @brief reserves next dispatch time. NOT dispatch, NOT reserve dispatch itself.
		bool ReserveResume(clock_t::time_point tWhen = {}) {
			if (!m_handle or m_handle.done())
				return false;
			m_state.tNextDispatch = tWhen;
			return true;
		}
		bool ReserveResume(clock_t::duration dur) { return ReserveResume(dur.count() ? clock_t::now() + dur : clock_t::time_point{}); }

		/// @brief Dispatch.
		/// @return true if need next dispatch
		bool Dispatch(clock_t::time_point& tNextDispatchOut) {
			if (std::this_thread::get_id() != m_threadID) {
				assert(false);
				return false;
			}
			// Dispatch Child Sequences
			for (bool bContinue{true}; bContinue;) {
				bContinue = false;
				do {
					auto const t0 = clock_t::now();
					auto& tNextDispatchChild = m_state.tNextDispatchChild;
					tNextDispatchChild = clock_t::time_point::max();	// send to the future
					//std::scoped_lock lock{m_mtxChildren};
					for (auto iter = m_children.begin(); iter != m_children.end();) {
						tNextDispatchChild = clock_t::time_point::max();	// send to the future
						auto& child = *iter;

						//if (!child.m_handle or child.m_handle.done()) {
						//	iter = m_children.erase(iter);
						//	continue;
						//}

						// Check Time
						if (auto t = child.GetNextDispatchTime(); t > t0) {	// not yet
							tNextDispatchChild = std::min(tNextDispatchChild, t);
							iter++;
							continue;
						}

						// Dispatch Child
						if (child.Dispatch(tNextDispatchChild)) {	// no more child or child done
							iter++;
						}
						else {
							iter = m_children.erase(iter);
							continue;
						}
					}
					if (m_state.tNextDispatchChild > t0)
						break;
				} while (m_children.size());

				// if no more child sequence, Dispatch Self
				if (m_children.empty() and m_handle and !m_handle.done()) {
					m_handle.promise().m_state.tNextDispatch = clock_t::time_point::max();
					m_handle.resume();
					m_state = m_handle.promise().m_state;
					bContinue = !m_children.empty();	// if new child sequence added, continue to dispatch child
				}
			}
			tNextDispatchOut = std::min(tNextDispatchOut, GetNextDispatchTime());
			return !IsDone();
		}

		/// @brief Add Child Sequence
		sSequence& AddChild(id_t name, std::function<sSequence(sSequence&)> func) {
			//std::optional<std::scoped_lock<std::mutex>> lock;
			//if (std::this_thread::get_id() != m_threadID)
			//	lock.emplace(m_mtxChildren);
			m_children.emplace_back();
			m_children.back() = func(m_children.back());
			m_children.back().m_name = std::move(name);
			return m_children.back();
		}

		/// @brief Find Child Sequence
		/// @param name 
		/// @return child sequence. if not found, empty child sequence.
		sSequence& FindChild(id_t const& name) {
			for (auto& child : m_children) {
				if (child.m_name == name)
					return child;
			}
#ifdef _DEBUG
			assert(false);
#endif
			static sSequence dummy(coroutine_handle_t{nullptr});
			return dummy;
		}

	};

	//-------------------------------------------------------------------------
	/// @brief sequence wrapper
	template < typename tSelf >
	class TSequence {
	public:
		using this_t = TSequence;
		using self_t = tSelf;

		template < typename ... targs >
		using tseq_func_t = std::function<sSequence(self_t*, sSequence&, targs&& ...)>;
		using seq_func_t = tseq_func_t<>;
		using seq_handler_t = tseq_func_t<std::shared_ptr<sParam>>;

	public:
		//struct sAwaitable {
		//	bool await_ready() { return false; }
		//	bool await_suspend(handle_t h) {
		//		return false;
		//	}
		//	void await_resume() { }
		//};

	protected:
		id_t m_name;
		sSequence& m_driver;
		std::map<id_t/*sequence name*/, seq_handler_t> m_mapFuncs;

	public:
		//-----------------------------------
		TSequence(sSequence& driver, id_t name) : m_driver(driver), m_name{std::move(name)} {}
		~TSequence() {}
		TSequence(TSequence const&) = delete;
		TSequence& operator = (TSequence const&) = delete;
		TSequence(TSequence&& b) = default;
		TSequence& operator = (TSequence&& b) = default;

		//-----------------------------------
		template < typename ... targs >
		sSequence& CreateSequenceAny(id_t name, sSequence& seqParent, tseq_func_t<targs...> funcSequence, targs&& ... args) {
			tSelf* self = static_cast<tSelf*>(this);
			auto func = [&, this](sSequence& seq) {
				return funcSequence(self, seq, std::forward<targs>(args)...);
			};
			return seqParent.AddChild(std::move(name), func);
		}
		sSequence& CreateSequence(id_t name, sSequence& seqParent, seq_handler_t funcSequence, std::shared_ptr<sParam> params = std::make_shared<sParam>()) {
			tSelf* self = static_cast<tSelf*>(this);
			auto func = [&, this](sSequence& seq) {
				if (!params)
					params = std::make_shared<sParam>();
				return funcSequence(self, seq, std::move(params));
			};
			return seqParent.AddChild(std::move(name), func);
		}

		/// @brief Dispatch
		/// @return next dispatch time
		clock_t::time_point Dispatch() {
			clock_t::time_point tNextDispatch{gtl::seq::v01::clock_t::time_point::max()};
			m_driver.Dispatch(tNextDispatch);
			return tNextDispatch;
		}
	};

};

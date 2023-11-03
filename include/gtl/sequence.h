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
#include <format>

namespace gtl::seq::inline v01 {

	using clock_t= std::chrono::high_resolution_clock;
	using ms_t = std::chrono::milliseconds;
	using id_t = std::string;

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
	struct xSequence {
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
		xSequence* m_parent{};
		coroutine_handle_t m_handle;
		inline thread_local static xSequence* s_seqCurrent{};
		std::thread::id m_threadID{std::this_thread::get_id()};	// NOT const. may be created from other thread (injection)
		id_t m_name;
		//clock_t::time_point m_timeout{clock_t::time_point::max()};
		sState m_state;
		std::list<xSequence> m_children;
	public:
		mutable std::mutex m_mtxChildren;

	public:
		// constructor
		//xSequence(id_t name = "") : m_name(std::move(name)) {}
		xSequence(coroutine_handle_t&& h) : m_handle(std::exchange(h, nullptr)) { }
		xSequence(xSequence const&) = delete;
		xSequence& operator = (xSequence const&) = delete;
		xSequence(xSequence&& b) {
			m_name.swap(b.m_name);
			m_handle = std::exchange(b.m_handle, nullptr);
			//m_timeout = std::exchange(b.m_timeout, {});
			m_state = std::exchange(b.m_state, {});
			m_children.swap(b.m_children);
		}
		xSequence& operator = (xSequence&& b) {
			Destroy();
			m_name.swap(b.m_name);
			m_handle = std::exchange(b.m_handle, nullptr);
			//m_timeout = std::exchange(b.m_timeout, {});
			m_state = std::exchange(b.m_state, {});
			m_children.swap(b.m_children);
			return *this;
		}
		xSequence& operator = (coroutine_handle_t&& h) {
			assert(m_handle == nullptr);
			m_handle = std::exchange(h, nullptr);
			return *this;
		}
		void SetName(id_t name) {
			this->m_name = std::move(name);
		}

		// destructor
		~xSequence() {
			Destroy();
		}
		inline void Destroy() {
			m_name.clear();
			if (auto h = std::exchange(m_handle, nullptr); h) {
				h.destroy();
			}
		}

		/// @brief 
		/// @return 
		inline bool IsDone() const {
			return m_children.empty() and (!m_handle or m_handle.done());
		}

		/// @brief 
		/// @return current running sequence
		static xSequence* GetCurrentSequence() { return s_seqCurrent; }

		/// @brief 
		/// @return working thread id
		auto GetWorkingThreadID() const { return m_threadID; }

		/// @brief 
		/// @return Get Next Dispatch Time
		/// this function seems to be very heavy. need to optimize.
		template <bool bRefreshChild = false>
		clock_t::time_point GetNextDispatchTime() const {
			auto t = clock_t::time_point::max();
			if constexpr (bRefreshChild) {
				for (auto const& child : m_children) {
					t = std::min(t, child.GetNextDispatchTime<bRefreshChild>());
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

		/// @brief update child's next dispatch time
		/// @return shortest next dispatch time
		clock_t::time_point UpdateNextDispatchTime() {
			auto t = clock_t::time_point::max();
			for (auto& child : m_children) {
				t = std::min(t, child.UpdateNextDispatchTime());
			}
			m_state.tNextDispatchChild = t;
			if (m_children.empty() and m_handle and !m_handle.done())
				t = std::min(t, m_state.tNextDispatch);
			return t;
		}

		/// @brief propagate next dispatch time to parent
		void PropagateNextDispatchTime() {
			clock_t::time_point tWhen = GetNextDispatchTime<false>();

			std::optional<std::scoped_lock<std::mutex>> lock;
			if (std::this_thread::get_id() != m_threadID)
				lock.emplace(m_mtxChildren);

			// refresh parent's next dispatch time
			for (auto* parent = m_parent; parent; parent = parent->m_parent) {
				if constexpr (true) {
					// compare parent's next dispatch time is shorter, time and determine earlier break
					if (parent->m_state.tNextDispatchChild <= tWhen)
						break;
					parent->m_state.tNextDispatchChild = std::min(parent->m_state.tNextDispatchChild, tWhen);
				}
				else {
					// no comapre, just update
					parent->m_state.tNextDispatchChild = std::min(parent->m_state.tNextDispatchChild, tWhen);
				}
			}
		}

		/// @brief reserves next dispatch time. NOT dispatch, NOT reserve dispatch itself.
		bool ReserveResume(clock_t::time_point tWhen = {}) {
			if (!m_handle or m_handle.done())
				return false;
			m_state.tNextDispatch = tWhen;

			PropagateNextDispatchTime();
			return true;
		}
		bool ReserveResume(clock_t::duration dur) { return ReserveResume(dur.count() ? clock_t::now() + dur : clock_t::time_point{}); }

		/// @brief 
		/// @param name Task Name
		/// @param func coroutine function
		/// @param ...args for coroutine function. must be moved or copied.
		/// @return 
		template < typename ... targs >
		xSequence& CreateChildSequence(id_t name, std::function<xSequence(targs ...)> func, targs... args) {
			if constexpr (false) {	// todo: do I need this?
				if (std::this_thread::get_id() != m_threadID) {
					throw std::exception("CreateChildSequence() must be called from the same thread as the driver");
				}
			}

			// lock if called from other thread
			std::optional<std::scoped_lock<std::mutex>> lock;
			if (std::this_thread::get_id() != m_threadID)
				lock.emplace(m_mtxChildren);

			// create child sequence
			m_children.emplace_back(func(std::move(args)...));	// coroutine parameters are to be moved (or copied)
			m_children.back().m_parent = this;
			m_children.back().m_threadID = m_threadID;
			m_children.back().m_name = std::move(name);
			return m_children.back();
		}

		/// @brief Find Child Sequence (Direct Child Only)
		/// @param name 
		/// @return child sequence. if not found, empty child sequence.
		auto FindDirectChild(this auto&& self, id_t const& name) -> decltype(&self) {
			std::optional<std::scoped_lock<std::mutex>> lock;
			if (std::this_thread::get_id() != self.m_threadID)
				lock.emplace(self.m_mtxChildren);

			for (auto& child : self.m_children) {
				if (child.m_name == name)
					return &child;
			}
			return nullptr;
		}

		/// @brief Find Child Sequence (Depth First Search)
		/// @param name 
		/// @return child sequence. if not found, empty child sequence.
		auto FindChildDFS(this auto&& self, id_t const& name) -> decltype(&self) {
			// todo: if called from other thread... how? use recursive mutex ?? too expansive
			if (std::this_thread::get_id() != self.m_threadID)
				return nullptr;

			for (auto& child : self.m_children) {
				if (child.m_name == name)
					return &child;
			}
			for (auto& child : self.m_children) {
				if (auto* c = child.FindChildDFS(name))
					return c;
			}
			return nullptr;
		}

		/// @brief main dispatch function
		/// @return next dispatch time
		clock_t::time_point Dispatch() {
			if (std::this_thread::get_id() != m_threadID) [[ unlikely ]] {
				throw std::exception("Dispatch() must be called from the same thread as the driver");
				return {};
			}
			clock_t::time_point tNextDispatch{clock_t::time_point::max()};
			if (Dispatch(tNextDispatch))
				return tNextDispatch;
			return clock_t::time_point::max();
		}

	protected:
		/// @brief Dispatch.
		/// @return true if need next dispatch
		bool Dispatch(clock_t::time_point& tNextDispatchOut) {

			if (s_seqCurrent) [[ unlikely ]] {
				throw std::exception("Dispatch() must NOT be called from Dispatch. !!! No ReEntrance");
				return false;
			}

			// Dispatch Child Sequences
			for (bool bContinue{true}; bContinue;) {
				bContinue = false;
				do {
					auto const t0 = clock_t::now();
					auto& tNextDispatchChild = m_state.tNextDispatchChild;
					tNextDispatchChild = clock_t::time_point::max();	// suspend (do preset for there is no child sequence)
					std::scoped_lock lock{m_mtxChildren};
					for (auto iter = m_children.begin(); iter != m_children.end();) {
						tNextDispatchChild = clock_t::time_point::max();	// suspend
						auto& child = *iter;

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

					// Dispatch
					s_seqCurrent = this;
					m_handle.resume();
					s_seqCurrent = nullptr;

					m_state = m_handle.promise().m_state;
					bContinue = !m_children.empty();	// if new child sequence added, continue to dispatch child
				}
			}
			tNextDispatchOut = std::min(tNextDispatchOut, GetNextDispatchTime());
			return !IsDone();
		}

	};

	//-------------------------------------------------------------------------
	/// @brief sequence map manager
	class xSequenceHandlerMap {
	public:
		struct sParam {
			std::string in, out;
		};

		using this_t = xSequenceHandlerMap;
		using handler_t = std::function<xSequence&(std::shared_ptr<sParam>)>;
		using map_t = std::map<id_t, handler_t>;

	protected:
		id_t m_unit;
		this_t* m_top{};
		this_t* m_parent{};
		std::set<this_t*> m_children;

		map_t m_mapFuncs;
	public:
		// constructors and destructor
		xSequenceHandlerMap(this_t* parent) : m_parent(parent) {
			if (m_parent) {
				m_parent->Register(this);
				m_top = parent->m_top;
			}
		}
		~xSequenceHandlerMap() {
			if (auto* parent = std::exchange(m_parent, nullptr))
				parent->Unregister(this);
		}
		xSequenceHandlerMap(xSequenceHandlerMap const&) = delete;
		xSequenceHandlerMap& operator = (xSequenceHandlerMap const&) = delete;
		xSequenceHandlerMap(xSequenceHandlerMap&& b) {
			if (this == &b)
				return ;

			if (auto* parent = std::exchange(b.m_parent, nullptr)) {
				parent->Unregister(&b);
				m_parent = parent;
				m_top = parent->m_top;
			}
			m_unit = std::exchange(b.m_unit, {});
			m_top = std::exchange(b.m_top, nullptr);
			m_children.swap(b.m_children);

			if (m_parent)
				m_parent->Register(this);
		}
		xSequenceHandlerMap& operator = (xSequenceHandlerMap&&) = delete;	// if has some children, no way to remove children from parent

		//-----------------------------------
		void Register(this_t* child) {
			if (child)
				m_children.insert(child);
		}
		void Unregister(this_t* child) {
			if (child)
				m_children.erase(child);
		}

		//-----------------------------------
		// Find Handler
		inline handler_t FindHandler(id_t const& sequence) const {
			if (auto iter = m_mapFuncs.find(sequence); iter != m_mapFuncs.end())
				return iter->second;
			return nullptr;
		}
		handler_t FindHandlerDFS(id_t const& sequence) const {
			if (auto handler = FindHandler(sequence))
				return handler;
			for (auto* child : m_children) {
				if (auto func = child->FindHandlerDFS(sequence))
					return func;
			}
			return nullptr;
		}

		//-----------------------------------
		// Find Map
		inline auto FindMapDFS(this auto&& self, id_t const& unit) -> decltype(&self) {
			if (m_unit == unit)
				return const_cast<this_t*>(this);
			for (auto* child : self.m_children) {
				if (auto* map = child->FindMapDFS(unit))
					return map;
			}
			return nullptr;
		}

		inline xSequence& CreateChildSequence(xSequence& parent, id_t name, std::shared_ptr<sParam> params = std::make_shared<sParam>()) {
			if (auto handler = FindHandler(name)) {
				return parent.CreateChildSequence<std::shared_ptr<sParam>>(std::move(name), handler, std::move(params));
			}
			throw std::exception(std::format("no handler: {}", name).c_str());
		}

	};

	//-------------------------------------------------------------------------
	/// @brief sequence wrapper, with sequence map
	template < typename tSelf >
	class TSequence : public xSequenceHandlerMap {
	public:
		using this_t = TSequence;
		using self_t = tSelf;

		template < typename ... targs >
		using tseq_handler_t = std::function<xSequence(self_t*, targs&& ...)>;
		using seq_handler_t = tseq_handler_t<std::shared_ptr<sParam>>;

	public:
		//struct sAwaitable {
		//	bool await_ready() { return false; }
		//	bool await_suspend(handle_t h) {
		//		return false;
		//	}
		//	void await_resume() { }
		//};

	public:
		//-----------------------------------
		TSequence() : m_driver(driver), m_name{std::move(name)} {}
		~TSequence() {}
		TSequence(TSequence const&) = delete;
		TSequence& operator = (TSequence const&) = delete;
		TSequence(TSequence&& b) = default;
		TSequence& operator = (TSequence&& b) = default;

		//-----------------------------------
		// Helper functions
		auto const& GetName() const { return m_name; }
		auto const& GetSeqMap() const { return m_mapFuncs; }

		// Connect Sequence
		bool ConnectSequence(gtl::seq::id_t const& name, seq_handler_t func) {
			m_mapFuncs[name] = std::move(func);
		}

		// Find Child Handler
		inline seq_handler_t FindSequenceHandler(id_t const& name) const {
			if (auto iter = m_mapFuncs.find(name); iter == m_mapFuncs.end())
				return iter->second;
			return nullptr;
		}

		// Create Child Sequence
		inline xSequence& CreateChildSequence(xSequence& seqParent, id_t name, seq_handler_t funcSequence, std::shared_ptr<sParam> params = std::make_shared<sParam>()) {
			tSelf* self = static_cast<tSelf*>(this);
			return seqParent.CreateChildSequence<tSelf*, std::shared_ptr<sParam>>(std::move(name), funcSequence, self, std::move(params));
		}
		inline xSequence& CreateChildSequence(id_t name, seq_handler_t funcSequence, std::shared_ptr<sParam> params = std::make_shared<sParam>()) {
			auto* seqParent = m_driver.GetCurrentSequence();
			if (!seqParent)
				throw std::exception("CreateChildSequence() must be called from sequence function");
			return CreateChildSequence(*seqParent, std::move(name), funcSequence, std::move(params));
		}
		inline xSequence& CreateChildSequence(id_t name, std::shared_ptr<sParam> params = std::make_shared<sParam>()) {
			auto funcSequence = FindSequenceHandler(name);
			if (!funcSequence)
				throw std::exception("no such function");
			return CreateChildSequence(std::move(name), funcSequence, std::move(params));
		}
	};

};

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

namespace gtl {

	struct GTL__CLASS xSequenceParam {
		glz::json_t in, out;
	};

	struct GTL__CLASS xSequence {

		template < typename ... targs >
		using seq_func_t = std::function<xSequence(xSequence&, targs&& ...)>;
		struct promise_type;
		using handle_t = std::coroutine_handle<promise_type>;

		struct promise_type {
			bool m_result{};
			std::exception_ptr m_exception;

			xSequence get_return_object() {
				return xSequence(handle_t::from_promise(*this));
			}
			std::suspend_always initial_suspend() { return {}; }
			std::suspend_always final_suspend() noexcept { return {}; }
			void unhandled_exception() { m_exception = std::current_exception(); }

			std::suspend_always yield_value(bool result) {
				m_result = result;
				return {};
			}
			void return_void() {}
		};

		std::optional<handle_t> m_handle;

		xSequence() {}
		xSequence(handle_t handle) : m_handle(handle) {}
		~xSequence() {
			if (auto handle = std::exchange(m_handle, {}); handle and *handle) {
				(*handle).destroy();
			}
		}
		xSequence(xSequence const&) = delete;
		xSequence& operator = (xSequence const&) = delete;
		xSequence(xSequence&& o) {
			m_handle = std::move(std::exchange(o.m_handle, {}));
			m_sequences.swap(o.m_sequences);
		}
		xSequence& operator = (xSequence&& o) {
			m_handle.reset();
			m_handle = std::move(std::exchange(o.m_handle, {}));
			m_sequences.clear();
			m_sequences.swap(o.m_sequences);
			return *this;
		}

		template < typename ... targs >
		std::suspend_always CreateSequenceParams(seq_func_t<targs...> funcSequence, targs&& ... args) {
			m_sequences.emplace_back();
			m_sequences.back() = funcSequence(m_sequences.back(), std::forward<targs>(args)...);
			return {};
		}
		std::suspend_always CreateSequence(std::function<xSequence(xSequence&)> funcSequence) {
			m_sequences.emplace_back();
			m_sequences.back() = funcSequence(m_sequences.back());
			return {};
		}

		bool ProcessSingleStep() {
			bool bContinue = false;
			do {
				bContinue = false;
				for (auto iter = m_sequences.begin(); iter != m_sequences.end(); ) {
					if (iter->ProcessSingleStep()) {
						iter = m_sequences.erase(iter);
					}
					else {
						iter++;
					}
				}
				if (m_sequences.empty() and m_handle) {	// wait for child sequences to finish
					if (!*m_handle or (*m_handle).done())
						break;
					(*m_handle)();
					if ((*m_handle).promise().m_exception)
						std::rethrow_exception((*m_handle).promise().m_exception);
					//bContinue = true;
				}
			} while (bContinue);
			return !m_handle or !*m_handle or (*m_handle).done();
		}
	protected:
		std::list<xSequence> m_sequences;
	};

};

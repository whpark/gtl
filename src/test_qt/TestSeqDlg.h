#pragma once

#include <QDialog>
#include "ui_TestSeqDlg.h"

template<typename T>
struct Generator {
	// The class name 'Generator' is our choice and it is not required for coroutine
	// magic. Compiler recognizes coroutine by the presence of 'co_yield' keyword.
	// You can use name 'MyGenerator' (or any other name) instead as long as you include
	// nested struct promise_type with 'MyGenerator get_return_object()' method.

	struct promise_type;
	using handle_type = std::coroutine_handle<promise_type>;

	struct promise_type // required
	{
		T value_;
		std::exception_ptr exception_;

		Generator get_return_object() {
			return Generator(handle_type::from_promise(*this));
		}
		std::suspend_always initial_suspend() { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }
		void unhandled_exception() { exception_ = std::current_exception(); } // saving
		// exception

		template<std::convertible_to<T> From> // C++20 concept
		std::suspend_always yield_value(From&& from) {
			value_ = std::forward<From>(from); // caching the result in promise
			return {};
		}
		void return_void() {}
	};

	std::optional<handle_type> h_;

	Generator() : h_() {}
	Generator(handle_type h) : h_(h) {}
	~Generator() { if (h_) { if (*h_) (*h_).destroy(); h_.reset(); } }
	Generator(Generator const&) = delete;
	Generator& operator=(Generator const&) = delete;
	Generator(Generator&& o) : h_(std::exchange(o.h_, {})) {}
	Generator& operator=(Generator&& o) {
		if (this != &o) {
			if (h_ and *h_)
				(*h_).destroy();
			h_.reset();
			h_ = std::exchange(o.h_, {});
		}
		return *this;
	}
	explicit operator bool() {
		if (!h_)
			return false;
		fill(); // The only way to reliably find out whether or not we finished coroutine,
		// whether or not there is going to be a next value generated (co_yield)
		// in coroutine via C++ getter (operator () below) is to execute/resume
		// coroutine until the next co_yield point (or let it fall off end).
		// Then we store/cache result in promise to allow getter (operator() below
		// to grab it without executing coroutine).
		return !(*h_).done();
	}
	T operator()() {
		if (!h_)
			throw std::runtime_error("Generator is empty");
		fill();
		full_ = false; // we are going to move out previously cached
		// result to make promise empty again
		return std::move((*h_).promise().value_);
	}

	std::list<Generator<int>> m_sequences;
	std::suspend_always AddSequence(std::function<Generator<int>(Generator<int>&)> funcCreator) {
		m_sequences.emplace_back();
		m_sequences.back() = funcCreator(m_sequences.back());
		return {};
	}
	bool DoSequenceLoop() {
		bool bContinue = false;
		do {
			bContinue = false;
			for (auto iter = m_sequences.begin(); iter != m_sequences.end(); ) {
				if (iter->DoSequenceLoop()) {
					iter = m_sequences.erase(iter);
				}
				else {
					iter++;
				}
			}
			if (m_sequences.empty() and h_) {	// wait for child sequences to finish
				if (*this) {
					auto v = (*this)();
					bool bContinue = true;
				}
			}
		} while (bContinue);
		return !h_ or !*h_ or (*h_).done();
	}
private:
	bool full_ = false;

	void fill() {
		if (!full_) {
			(*h_)();
			if ((*h_).promise().exception_)
				std::rethrow_exception((*h_).promise().exception_);
			// propagate coroutine exception in called context

			full_ = true;
		}
	}
};

class xTestSeqDlg : public QDialog {
	Q_OBJECT
public:
	using this_t = xTestSeqDlg;
	using base_t = QDialog;

public:
	xTestSeqDlg(QWidget* parent = nullptr);
	~xTestSeqDlg();

	Generator<int> m_top;
	std::jthread m_thread;

	Generator<int> Seq1(Generator<int>& self);
	Generator<double> SubSeq();

Q_SIGNALS:
	void sigSequence();

private:
	Ui::xTestSeqDlgClass ui;
};

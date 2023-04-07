///////////////////////////////////////////////////////////////////////////////
//
// lock.h
//
//			GTL : (G)reen (T)ea (L)atte
//
// 2018.01.15. recursive_recursive_shared_mutex (extension of std::recursive_mutex)
// 2019.07.24. QL -> GTL
// 2022.04.07. recursive_shared_mutex - refactoring
//
// PWH
//
///////////////////////////////////////////////////////////////////////////////


#pragma once

#include <mutex>
#include <thread>
#include <atomic>
#include <shared_mutex>
#include <system_error>

#include "gtl/_default.h"
#include "gtl/_lib_gtl.h"


namespace gtl {
#pragma pack(push, 8)

	//class GTL__CLASS null_mutex;
	//class GTL__CLASS recursive_shared_mutex;


	//------------------------------------------------------------------------
	/// @brief null mutex
	class null_mutex {
	public:
		//null_mutex() noexcept {}
		//~null_mutex() noexcept {}

		void lock() const { // lock the mutex
		}
		[[nodiscard]] bool try_lock() const { // try to lock the mutex
			return true;
		}
		void unlock() const { // unlock the mutex
		}

		void lock_shared() const noexcept {}
		bool try_lock_shared() const noexcept {
			return true;
		}
		void unlock_shared() const noexcept {}
	};

#if 1
	//------------------------------------------------------------------------
	/// @brief recursive shared mutex
	class recursive_shared_mutex {
	public:
		using this_t = recursive_shared_mutex;
		//using base_t = std::shared_mutex;
	private:
		std::shared_mutex m_mutex;
		std::atomic<std::thread::id> m_owner;
		size_t m_counter{};

	public:
		//using base_t::base_t;
		recursive_shared_mutex() = default;
		recursive_shared_mutex(recursive_shared_mutex const&) = delete;
		recursive_shared_mutex& operator=(recursive_shared_mutex const&) = delete;

		void lock() noexcept {
			//_Smtx_lock_exclusive(&_Myhandle);
			if (auto idCurrent = std::this_thread::get_id(); idCurrent == m_owner) {
				m_counter++;
			}
			else {
				m_mutex.lock();
				m_owner = idCurrent;
				m_counter = 1;
			}
		}
		void lock_shared() noexcept {
			if (auto idCurrent = std::this_thread::get_id(); idCurrent == m_owner) {
			}
			else {
				m_mutex.lock_shared();
			}
		}
		bool try_lock() noexcept {
			auto idCurrent = std::this_thread::get_id();
			if (idCurrent == m_owner) {
				m_counter++;
				return true;
			}
			if (m_mutex.try_lock()) {
				m_owner = idCurrent;
				m_counter = 1;
			}
		}
		bool try_lock_shared() noexcept {
			if (auto idCurrent = std::this_thread::get_id(); idCurrent == m_owner) {
				m_counter++;
				return true;
			}
			return m_mutex.try_lock_shared();
		}
		void unlock() noexcept {
			if (--m_counter == 0) {
				m_owner.store({});
				m_mutex.unlock();
			}
		}
		void unlock_shared() noexcept {
			if (auto idCurrent = std::this_thread::get_id(); idCurrent == m_owner) {
			} else {
				m_mutex.unlock_shared();
			}
		}

	};

#else
	class recursive_shared_mutex {
	private:
		std::shared_mutex m_mtx;

		std::atomic<std::thread::id> m_idOwner;
		std::uintptr_t m_nLocked = 0;

		std::mutex m_mtxSharedThreadID;
		std::map<std::thread::id, int> m_idsSharedLock;

		std::mutex m_mtxShared;
		std::condition_variable m_cvShared;

	public:
		void lock() noexcept {
			auto idCurrent = std::this_thread::get_id();
			if (idCurrent != m_idOwner) {
				m_mtx.lock();
				assert(m_nLocked == 0);

				// Check Shared Lock
				do {
					if (CheckSharedLock(idCurrent))
						break;

					std::unique_lock lock(m_mtxShared);
					m_cvShared.wait(lock);

				} while(true);

				// Take Ownership
				m_idOwner = idCurrent;
			}
			m_nLocked++;
		}
		[[nodiscard]] bool try_lock() noexcept {
			auto idCurrent = std::this_thread::get_id();
			if (idCurrent == m_idOwner) {
				m_nLocked++;
				return true;
			}

			if (!m_mtx.try_lock())
				return false;

			// Check Shared Lock
			if (!CheckSharedLock(idCurrent)) {
				unlock();
				return false;
			}

			m_idOwner = idCurrent;
			m_nLocked++;

			return true;
		}
		void unlock() noexcept {
			assert( (m_nLocked > 0) && (m_idOwner == std::this_thread::get_id()));
			m_nLocked--;
			if (m_nLocked == 0) {
				m_idOwner = std::thread::id();
				m_mtx.unlock();
			}
		}

		void lock_shared() noexcept {
			RecursiveLock();
			auto id = std::this_thread::get_id();
			{
				std::scoped_lock lock(m_mtxSharedThreadID);
				m_idsSharedLock[id] ++;
			}
			unlock();
		}
		[[nodiscard]] bool try_lock_shared() noexcept {
			if (!try_lock())
				return false;

			auto id = std::this_thread::get_id();
			{
				std::scoped_lock lock(m_mtxSharedThreadID);
				m_idsSharedLock[id] ++;
			}
			unlock();

			return true;
		}
		void unlock_shared() noexcept {
			auto id = std::this_thread::get_id();
			{
				std::scoped_lock lock(m_mtxSharedThreadID);
				if (!--m_idsSharedLock[id])
					m_idsSharedLock.erase(id);
			}
			m_cvShared.notify_one();
		}

	protected:
		void RecursiveLock() noexcept {
			auto idCurrent = std::this_thread::get_id();
			if (idCurrent != m_idOwner) {
				m_mtx.lock();
				assert(m_nLocked == 0);
				m_idOwner = idCurrent;
			}
			m_nLocked++;
		}
		bool CheckSharedLock(std::thread::id idCurrent) noexcept {
			std::scoped_lock lock(m_mtxSharedThreadID);
			int nSharedLock = 0;
			for (const auto& id : m_idsSharedLock) {
				if (id.first != idCurrent) {
					nSharedLock += id.second;
					if (nSharedLock > 0)
						return false;
				}
			}
			return nSharedLock == 0;
		}

	};
#endif

	// Mutex Locker
	// inspired from :
	//			https://www.reddit.com/r/cpp/comments/12def1f/mutexprotected_a_c_pattern_for_easier_concurrency/
	//
	template < typename T, typename TMutex = std::mutex >
	class TMutexLocker {
	protected:
		T m_value;
		mutable TMutex m_mutex;

	public:
		using value_type = T;
		using mutex_type = TMutex;

		template < typename ... TArgs >
		TMutexLocker(TArgs&& ... args) : m_value(std::forward<TArgs>(args) ...) {}

		struct Locker {
			TMutexLocker& m_locker;
			std::unique_lock<TMutex> m_lock;
			Locker(TMutexLocker& locker) : m_locker(locker), m_lock(locker.m_mutex) {}
			Locker(const Locker&) = delete;
			Locker(Locker&&) = delete;
			Locker& operator=(const Locker&) = delete;
			Locker& operator=(Locker&&) = delete;
			T& operator*() = delete;
			const T& operator*() const = delete;
			T* operator->() noexcept { return &m_locker.m_value; }
			const T* operator->() const noexcept { return &m_locker.m_value; }
		};

		Locker Lock() noexcept {
			return Locker(*this);
		}

		T Release() noexcept {
			std::unique_lock lock(m_mutex);
			return std::move(m_value);
		}

	};


#pragma pack(pop)
}	// namespace gtl

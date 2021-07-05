///////////////////////////////////////////////////////////////////////////////
//
// lock.h
//
//			GTL : (G)reen (T)ea (L)atte
//
// 2018.01.15. recursive_shared_mutex (extension of std::recursive_mutex)
// 2019.07.24. QL -> GTL
//
// PWH
//
///////////////////////////////////////////////////////////////////////////////


#pragma once

#include <mutex>
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
		null_mutex() noexcept { }
		~null_mutex() noexcept { }

		void lock() { // lock the mutex
		}
		[[nodiscard]] bool try_lock() { // try to lock the mutex
			return true;
		}
		void unlock() { // unlock the mutex
		}

		void lock_shared() noexcept {}
		bool try_lock_shared() noexcept {
			return true;
		}
		void unlock_shared() noexcept {}
	};


	//------------------------------------------------------------------------
	/// @brief recursive shared mutex
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


#pragma pack(pop)
}	// namespace gtl

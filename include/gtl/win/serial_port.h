#pragma once

//////////////////////////////////////////////////////////////////////
//
// serial_port.h: RS232 Serial Port Wrapper
//
// PWH
// 2019.01.15.
// 2019.07.24. QL -> GTL
// 2021.06.10. gtl->gtl::win_util. NOT TESTED.
// 2023-08-09 win_util -> win
//
//////////////////////////////////////////////////////////////////////

#include "gtl/_default.h"

#include "gtl/time.h"
#include "gtl/log.h"
#include "gtl/mutex.h"

#include "_lib_gtl_win.h"

namespace gtl::win {
#pragma pack(push, 8)

	class GTL__WIN_CLASS xComm {
	public:
		enum eCONTROL_CHAR : uint8_t {
			ACK = 0x06,
			NAK = 0x15,
			ENQ = 0x05,
			EOT = 0x04,
			STX = 0x02,
			ETX = 0x03,

			ASCII_XON = 0x11,
			ASCII_XOFF = 0x13,

		};

		enum eCONTROL : uint32_t {
			CTS  = MS_CTS_ON,
			DSR  = MS_DSR_ON,
			RING = MS_RING_ON,
			DCD  = MS_RLSD_ON,

			FC_DTRDSR = 0x01,
			FC_RTSCTS = 0x02,
			FC_XONXOFF = 0x04,
		};

		enum class ePARITY : uint8_t { no = 0, odd, even, mark, space };
		enum class eSTOP_BITS : uint8_t { one = 0, one5, two = 2 };
		enum class eDTR_CONTROL : uint8_t { disable = 0, enable, handshake };
		enum class eRTS_CONTROL : uint8_t { disable = 0, enable, handshake, toggle };

		struct SETTING {
			uint32_t baud = 9600;
			uint8_t bytesize = 8;
			ePARITY parity = ePARITY::no;
			eSTOP_BITS stopbits = eSTOP_BITS::one;
			eDTR_CONTROL eDTR = eDTR_CONTROL::disable;
			eRTS_CONTROL eRTS = eRTS_CONTROL::disable;
			bool bXOnOff = false;
		};

		constexpr static inline size_t const MAX_RECV_BUF = 1000;
		constexpr static inline size_t const MAX_SEND_BUF = 1000;

		constexpr static inline size_t RECV_ENQ_TIME = 30;

	public:
		xSimpleLog m_log;
		bool m_bLog {};	// if true, leaves log for read/written texts.

	protected:
		HANDLE m_hComm;

		gtl::recursive_shared_mutex m_mtxBuf;
		std::deque<uint8_t> m_bufReceived;

		std::unique_ptr<std::thread> m_threadReader;
		std::mutex m_mtxEventHandler;
		std::condition_variable m_cvEventHandler;
		OVERLAPPED m_overlappedR, m_overlappedW;

		DWORD m_dwError = 0;
		//std::chrono::milliseconds m_durTimeoutTX { 1'000 };	// 1 sec
		//std::chrono::milliseconds m_durTimeoutRX { 1'000 };	// 1 sec
		DWORD m_dwTimeoutTX { 1'000 };	// 1 sec
		DWORD m_dwTimeoutRX { 1'000 };	// 1 sec

	public:
		xComm() = default;

		HANDLE GetCommHandle() { return m_hComm; }

	public:
		bool IsOpen() const { return IsValueNoneOf(m_hComm, nullptr, INVALID_HANDLE_VALUE);}
		bool Open(uint32_t nPort/*1 base*/, const SETTING& setting = SETTING{});
		bool Open(const TCHAR* pszPortName/* \\.\COM1 */, const SETTING& setting = SETTING{});
		bool Close();
		bool Attach(HANDLE hComm);
		HANDLE Detach();

		bool SetCommMask(DWORD dwEvtMask);
		bool EscapeCommFunction(DWORD dwFunc);

		size_t Read(void* buf, DWORD nMaxSize, DWORD dwTimeout = INFINITE);
		size_t Write(const void* buf, DWORD nSize, DWORD dwTimeout = INFINITE);
		bool Purge(DWORD dwFlags = PURGE_RXABORT|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_TXCLEAR);

	public:
		// Enumerates Serial Ports ( Reg. HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIALCOMM )
		static std::vector<std::wstring> FindSerialPorts();

	};


#pragma pack(pop)
}	// namespace gtl::win

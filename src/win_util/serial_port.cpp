#include "pch.h"

//////////////////////////////////////////////////////////////////////
//
// serial_port.cpp: RS232 Serial Port Wrapper
//
// PWH
// 2019.01.15.
// 2019.07.24. QL -> GTL
//
//////////////////////////////////////////////////////////////////////

#include "gtl/mutex.h"
#include "gtl/win_util/serial_port.h"
#include "gtl/win_util/win_util.h"

#include "gtl/string.h"

namespace gtl::win_util {

	using tchar = wchar_t;

	bool XComm::Open(uint32_t nPort/*1 base*/, const SETTING& setting) {
		if (nPort < 1)
			return false;

		return Open(fmt::format(L"\\\\.\\COM{}", nPort).c_str(), setting);
	}

	bool XComm::Open(const tchar* pszPortName, const SETTING& setting) {
		Close();

		m_hComm = CreateFile(pszPortName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

		if (m_hComm == INVALID_HANDLE_VALUE) {
			m_log.LogTag(L"E", L"CANNOT Open - {}", pszPortName);
			return false;
		}
		m_log.Log(L"Port {} Open. baud {}, ByteSize {}, Parity {}, StopBits {}, RTS({}), DTR({}), XOnOff({})",
			pszPortName, setting.baud, setting.bytesize, (uint8_t)setting.parity, (uint8_t)setting.stopbits, (uint8_t)setting.eRTS, (uint8_t)setting.eDTR, setting.bXOnOff);

		// Set Overlapped Structure
		m_overlappedR.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		ResetEvent(m_overlappedR.hEvent);
		m_overlappedW.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		ResetEvent(m_overlappedW.hEvent);

		// Error setting time-outs.

		DCB dcb;
		memset(&dcb, 0, sizeof(dcb));
		dcb.DCBlength = sizeof(dcb);
		dcb.BaudRate = setting.baud;
		dcb.fBinary = 1;
		dcb.fParity = setting.parity != ePARITY::no ? 1 : 0;
		dcb.ByteSize = setting.bytesize;
		dcb.Parity = (uint8_t)setting.parity;
		dcb.StopBits = (uint8_t)setting.stopbits;
		dcb.fOutxCtsFlow = 0;
		dcb.fOutxDsrFlow = 0;
		dcb.fDsrSensitivity = 0;

		dcb.fRtsControl = (uint8_t)setting.eRTS;
		dcb.fDtrControl = (uint8_t)setting.eDTR;

		dcb.fInX = dcb.fOutX = setting.bXOnOff ? 1 : 0;
		dcb.fTXContinueOnXoff = setting.bXOnOff;
		dcb.XonChar = ASCII_XON;
		dcb.XoffChar = ASCII_XOFF;
		dcb.XonLim = 0;
		dcb.XoffLim = 0;

		SetCommState(m_hComm, &dcb);
		//::SetCommMask(m_hComm, EV_RXCHAR|EV_ERR|EV_TXEMPTY);

		COMMTIMEOUTS to;
		ZeroVar(to);
		//to.ReadIntervalTimeout = (DWORD)std::chrono::milliseconds(m_durTimeoutRX).count();
		to.ReadIntervalTimeout = m_dwTimeoutRX;//INFINITE;
		to.ReadTotalTimeoutMultiplier = 1;
		//to.ReadTotalTimeoutConstant = (DWORD)std::chrono::milliseconds(m_durTimeoutRX).count();
		to.ReadTotalTimeoutConstant = m_dwTimeoutRX;//INFINITE;
		to.WriteTotalTimeoutMultiplier = 1;
		//to.WriteTotalTimeoutConstant = (DWORD)std::chrono::milliseconds(m_durTimeoutRX).count();//INFINITE;
		to.WriteTotalTimeoutConstant = m_dwTimeoutRX;
		SetCommTimeouts(m_hComm, &to);

		return true;
	}

	bool XComm::Close() {
		if (IsValueNoneOf(m_hComm, (HANDLE)NULL, INVALID_HANDLE_VALUE)) {
			if (m_overlappedR.hEvent) {
				CloseHandle(m_overlappedR.hEvent);
			}
			if (m_overlappedW.hEvent) {
				CloseHandle(m_overlappedW.hEvent);
			}
			memset(&m_overlappedR, 0, sizeof(m_overlappedR));
			memset(&m_overlappedW, 0, sizeof(m_overlappedW));
			::CloseHandle(m_hComm);
			m_hComm = INVALID_HANDLE_VALUE;
			m_log.Log("Closed");
		}

		return TRUE;
	}

	bool XComm::Attach(HANDLE hComm) {
		if (IsValueNoneOf(m_hComm, (HANDLE)NULL, INVALID_HANDLE_VALUE))
			Close();

		m_hComm = hComm;

		return TRUE;
	}
	HANDLE XComm::Detach() {
		HANDLE hComm = m_hComm;
		m_hComm = INVALID_HANDLE_VALUE;
		return hComm;
	}

	//DWORD XComm::Reader(LPVOID pVoid) {
	//	XComm* pThis = (XComm*)pVoid;

	//	pThis->Log("Reader Started");
	//	try {
	//		if (pThis->m_hComm == INVALID_HANDLE_VALUE)
	//			throw 1;
	//		//		DWORD dwFlag = 0;
	//		//		ClearCommError(pThis->m_hComm, &dwFlag, NULL);
	//		DWORD dwRead = 0;
	//		DWORD dwTimeoutRX = pThis->m_dwTimeoutRX;
	//		OVERLAPPED overlapped;
	//		memset(&overlapped, 0, sizeof(overlapped));
	//		overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//		HANDLE handles[2] = {overlapped.hEvent, pThis->m_eStopReader.m_hObject};
	//		ResetEvent(handles[0]);
	//		BOOL bQuit = FALSE;
	//		BOOL bReRead = TRUE;
	//		do {
	//			{
	//				DWORD dwFlag = 0;
	//				ClearCommError(pThis->m_hComm, &dwFlag, NULL);
	//			}
	//			// Read each byte
	//			char buf[1];
	//			if (bReRead) {
	//				ResetEvent(overlapped.hEvent);
	//				ReadFile(pThis->m_hComm, buf, sizeof(buf), &dwRead, &overlapped);
	//			}
	//			switch (::WaitForMultipleObjects(countof(handles), handles, FALSE, dwTimeoutRX)) {
	//			case WAIT_OBJECT_0 + 0 :
	//				bReRead = TRUE;
	//				GetOverlappedResult(pThis->m_hComm, &overlapped, &dwRead, FALSE);
	//				if (dwRead) {
	//					pThis->m_csReadBuffer.Lock();
	//					pThis->m_buf.push_back(buf[0]);
	//					pThis->m_csReadBuffer.Unlock();

	//					if ((UINT_PTR)pThis->m_buf.size() >= pThis->m_nRThreshold)
	//						pThis->FireReceived(pThis->m_buf.size());
	//				}
	//				break;
	//			case WAIT_TIMEOUT :
	//				if (pThis->m_buf.size()) {
	//					if (pThis->FireReceivingTimeout(pThis->m_buf.size()) == 0) {
	//						pThis->DeleteBuffer();
	//					}
	//				}
	//				bReRead = FALSE;
	//				break;
	//			default :
	//				bQuit = TRUE;
	//				break;
	//			}
	//		} while (!bQuit);
	//		CloseHandle(overlapped.hEvent);
	//	} catch (int e) {
	//		switch (e) {
	//		case 0 :
	//		case 1 :
	//			break;
	//		}
	//		pThis->ErrorLog(__TFMSG(" - Error\n"));
	//	}

	//	return 0;
	//}

	bool XComm::SetCommMask(DWORD dwEvtMask) {
		if (!IsOpen())
			return FALSE;

		::SetCommMask(m_hComm, dwEvtMask);
		return true;

		//if (m_hEventHandler) {
		//	m_eStopEventHandler.SetEvent();
		//	WaitForSingleObject(m_hEventHandler, 1000);
		//	CloseHandle(m_hEventHandler);
		//	m_hEventHandler = NULL;
		//}

		//if (dwEvtMask) {
		//	::SetCommMask(m_hComm, dwEvtMask);
		//	DWORD dwThreadID = 0;
		//	m_hEventHandler = CreateThread(NULL, 0, EventHandler, (LPVOID)this, 0, &dwThreadID);
		//}
		//return TRUE;
	}
	bool XComm::EscapeCommFunction(DWORD dwFunc) {
		if (IsOpen()) {
			return ::EscapeCommFunction(m_hComm, dwFunc);
		}
		return false;
	}


	//DWORD XComm::EventHandler(LPVOID pVoid) {
	//	XComm* pThis = (XComm*)pVoid;

	//	OVERLAPPED overlapped;
	//	memset(&overlapped, 0, sizeof(overlapped));
	//	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//	do {
	//		DWORD dwEvtMask = 0;
	//		ResetEvent(overlapped.hEvent);
	//		if (!::WaitCommEvent(pThis->m_hComm, &dwEvtMask, &overlapped)) {
	//			DWORD dwError = GetLastError();
	//			if (dwError != ERROR_IO_PENDING) {
	//				if (pThis->m_pLog) {
	//					CString str;
	//					str.Format(_T("------ %d : %s"), dwError, GetErrorMessage(dwError));
	//					pThis->Log(str);
	//				}
	//				break;
	//			}
	//		}

	//		HANDLE events[] = {overlapped.hEvent, pThis->m_eStopEventHandler};
	//		DWORD dwResult = WaitForMultipleObjects(2, events, FALSE, INFINITE);
	//		if (dwResult != WAIT_OBJECT_0) {
	//			break;
	//		}
	//		pThis->FireEvt(dwEvtMask);
	//	} while (TRUE);

	//	return 0;
	//}

	size_t XComm::Read(void* _buf, DWORD nMaxSize, DWORD dwTimeout) {
		if (m_hComm == INVALID_HANDLE_VALUE)
			return 0;
		{
			DWORD dwFlag = 0;
			ClearCommError(m_hComm, &dwFlag, NULL);
		}
		DWORD dwTick = GetTickCount();
		BYTE* buf = (BYTE*)_buf;
		DWORD nReadTotal = 0;
		DWORD dwRead = 0;
		BOOL bQuit = FALSE;
		do {
			ReadFile(m_hComm, buf+nReadTotal, nMaxSize-nReadTotal, &dwRead, &m_overlappedR);
			//		WriteLog("Serial : Wait...");
			switch (::WaitForSingleObject(m_overlappedR.hEvent, dwTimeout)) {
			case WAIT_OBJECT_0 :
				//			WriteLog("Serial : GetOverlappedResult...");
				GetOverlappedResult(m_hComm, &m_overlappedR, &dwRead, FALSE);
				if (dwRead) {
					nReadTotal += dwRead;
				} else {
					if ( dwTimeout && (dwTimeout != INFINITE) && (GetTickCount() - dwTick > dwTimeout) ) {
						return FALSE;
					}
					Sleep(10);
				}
				break;
			default :
				//			WriteLog("Serial : Timeout");
			#if _USE_CANCEL_IO_EX
				CancelIoEx(m_hComm, NULL);
			#else
				CancelIo(m_hComm);
			#endif
				bQuit = TRUE;
				break;
			}
		} while ((nReadTotal < nMaxSize) && !bQuit);

		if (m_bLog) {
			auto strs = ConvDataToHexString<wchar_t>({buf, nReadTotal}, 16);
			for (const auto& str : strs) {
				m_log.Log(L"Serial Read    : {}", str);
			}
		}

		return nReadTotal;
	}

	size_t XComm::Write(const void* _buf, DWORD nSize, DWORD dwTimeout) {
		const BYTE* buf = (BYTE*)_buf;
		if (m_hComm == INVALID_HANDLE_VALUE)
			return 0;
		DWORD dwSize = 0;
		// Clear Error Flag
		//DWORD dwFlag = 0;
		//ClearCommError(m_hComm, &dwFlag, NULL);
		//CancelIoEx(m_hComm, NULL);
		// Write
		if (!WriteFile(m_hComm, buf, nSize, &dwSize, &m_overlappedW) && (GetLastError() != ERROR_IO_PENDING)) {
			m_log.LogTag(L"E", L"Error : {}, {}\n", GetLastError(), GetErrorMessage(GetLastError()));
			return 0;
		}

		if (WaitForSingleObject(m_overlappedW.hEvent, dwTimeout) == WAIT_TIMEOUT) {
		#if _USE_CANCEL_IO_EX
			CancelIoEx(m_hComm, NULL);
		#else
			CancelIo(m_hComm);
		#endif

			m_log.LogTag(L"E", L"!!!! Serial FAILED !!!! ");
			return 0;
		}
		GetOverlappedResult(m_hComm, &m_overlappedW, &dwSize, FALSE);

		if (m_bLog) {
			auto strs = ConvDataToHexString<tchar>({(uint8_t*)_buf, (size_t)nSize}, 16, ' ', true, '|');
			for (const auto& str : strs)
				m_log.Log(L"Serial Written : {}", str);
		}

		return dwSize;
	}

	bool XComm::Purge(DWORD dwFlags) {
		if (!m_hComm)
			return FALSE;
		PurgeComm(m_hComm, dwFlags);
		return TRUE;
	}

	std::vector<std::wstring> XComm::FindSerialPorts() {
		using namespace std::literals;

		std::vector<std::wstring> strsPort;

		HKEY hKey = nullptr;
		LONG ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ, &hKey);
		if (ret == ERROR_SUCCESS) {
			for (DWORD dwIndex = 0; ; dwIndex++) {
				std::wstring strName, strValue;
				DWORD dwNameLength = 128, dwValueLength = 128;
				DWORD dwReserved = 0;
				DWORD dwType = 0;
				std::vector<tchar> vStrName(dwNameLength+1, 0);
				std::vector<tchar> vStrValue(dwValueLength+1, 0);
				int ret = RegEnumValue(hKey, dwIndex, vStrName.data(), &dwNameLength, NULL, &dwType, (BYTE*)vStrValue.data(), &dwValueLength);
				if (ret != ERROR_SUCCESS)
					break;
				strName = vStrName.data();
				strValue = vStrValue.data();
				if (dwType != REG_SZ)
					continue;
				gtl::Trim(strValue);
				gtl::TrimLeft(strValue, L"\\."sv);
				if (strValue.empty())
					continue;
				strsPort.emplace_back(std::move(strValue));
			}
		}
		if (hKey)
			RegCloseKey(hKey);
		hKey = nullptr;

		std::sort(strsPort.begin(), strsPort.end(), [](auto const& strA, auto const& strB) { return tdszcmp(strA, strB) < 0; });

		return strsPort;
	}

}


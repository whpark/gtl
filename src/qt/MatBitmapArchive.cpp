#include "pch.h"

#include "gtl/mat_helper.h"
#include "gtl/qt/qt.h"
#include "gtl/qt/util.h"
#include "gtl/qt/MatBitmapArchive.h"
#include "gtl/qt/ProgressDlg.h"

namespace gtl::qt {

	bool SaveBitmapMatProgress(std::filesystem::path const& path, cv::Mat const& img, int nBPP, gtl::xSize2i const& pelsPerMeter, std::span<gtl::color_bgra_t> palette, bool bNoPaletteLookup, bool bBottom2Top) {
		xProgressDlg dlgProgress(nullptr);
		dlgProgress.m_message = std::format(L"Saving : {}", path.wstring());

		dlgProgress.m_rThreadWorker = std::make_unique<std::jthread>(gtl::SaveBitmapMat, path, img, nBPP, pelsPerMeter, palette, bNoPaletteLookup, bBottom2Top, dlgProgress.m_calback);

		auto r = dlgProgress.exec();

		xWaitCursor wc;
		dlgProgress.m_rThreadWorker->join();

		bool bResult = (r == QDialog::Accepted);
		if (!bResult)
			std::filesystem::remove(path);

		return bResult;
	};

	sLoadBitmapMatResult LoadBitmapMatProgress(std::filesystem::path const& path) {
		sLoadBitmapMatResult result;
		xProgressDlg dlgProgress(nullptr);
		dlgProgress.m_message = std::format(L"Loading : {}", path.wstring());

		dlgProgress.m_rThreadWorker = std::make_unique<std::jthread>([&result, &path, &dlgProgress]() {
			result = gtl::LoadBitmapMat(path, dlgProgress.m_calback); });

		auto r = dlgProgress.exec();

		xWaitCursor wc;
		dlgProgress.m_rThreadWorker->join();

		bool bResult = (r == QDialog::Accepted);
		if (!bResult)
			result.img.release();

		return result;
	}

	sLoadBitmapMatPixelArrayResult LoadBitmapMatPixelArrayProgress(std::filesystem::path const& path) {
		sLoadBitmapMatPixelArrayResult result;

		xProgressDlg dlgProgress(nullptr);
		dlgProgress.m_message = std::format(L"Loading : {}", path.wstring());

		dlgProgress.m_rThreadWorker = std::make_unique<std::jthread>([&result, &path, &dlgProgress]() {
			result = gtl::LoadBitmapMatPixelArray(path, dlgProgress.m_calback); });

		auto r = dlgProgress.exec();

		xWaitCursor wc;
		dlgProgress.m_rThreadWorker->join();

		bool bResult = (r == QDialog::Accepted);
		if (!bResult)
			result.img.release();

		return result;
	}

	//=============================================================================
	//

}	// namespace gtl::qt

#include "pch.h"

#include <chrono>
#include <windows.h>

#include <opencv2/cvconfig.h>
#ifdef HAVE_CUDA
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudaimgproc.hpp>
#endif

#include "gtl/mat_helper.h"
#include "FreeImage.h"

namespace gtl {

	bool IsMatEqual(cv::Mat const& a, cv::Mat const& b) {
		if (a.cols != b.cols or a.rows != b.rows or a.size() != b.size() or a.type() != b.type())
			return false;
		if (a.empty())
			return true;
		if (a.isContinuous() and b.isContinuous()) {
			return std::equal(a.datastart, a.dataend, b.datastart);
		} else {
			size_t len = a.cols * a.elemSize();
			for (int y{}; y < a.rows; y++) {
				auto const* pA = a.ptr(y);
				auto const* pB = b.ptr(y);
				if (!std::equal(pA, pA+len, pB))
					return false;
			}
			return true;
		}

		return false;
	}

	void putTextC(cv::InputOutputArray img, std::string const& str, xPoint2i org,
		int fontFace, double fontScale, cv::Scalar color,
		int thickness, int lineType,
		bool bottomLeftOrigin, bool bOutline)
	{
		xSize2i size{ cv::getTextSize(str, fontFace, fontScale, thickness, nullptr) };

		if (bOutline) {
			cv::Scalar crBkgnd;
			crBkgnd = cv::Scalar(255, 255, 255) - color;
			//if (crBkgnd == Scalar(0, 0, 0))
			//	crBkgnd = Scalar(1, 1, 1);
			int iShift = std::max(1, thickness / 2);
			cv::putText(img, str, org - size / 2 + xPoint2i(0, iShift), fontFace, fontScale, crBkgnd, thickness, lineType, bottomLeftOrigin);
			cv::putText(img, str, org - size / 2 + xPoint2i(iShift, 0), fontFace, fontScale, crBkgnd, thickness, lineType, bottomLeftOrigin);
			cv::putText(img, str, org - size / 2 + xPoint2i(-iShift, 0), fontFace, fontScale, crBkgnd, thickness, lineType, bottomLeftOrigin);
			cv::putText(img, str, org - size / 2 + xPoint2i(0, -iShift), fontFace, fontScale, crBkgnd, thickness, lineType, bottomLeftOrigin);
		}

		cv::putText(img, str, org - size / 2, fontFace, fontScale, color, thickness, lineType, bottomLeftOrigin);
	}

	static bool s_bGPUChecked{};
	static bool s_bUseGPU{};

	constexpr static bool const bLoopUnrolling = true;
	constexpr static bool const bMultiThreaded = true;

	bool IsGPUEnabled() {
		return s_bUseGPU;
	}

	bool CheckGPU(bool bUse) {
		if (s_bGPUChecked)
			return s_bUseGPU;

		if (bUse) {
			try {
				cv::Mat img(2, 2, CV_64F);
				cv::cuda::GpuMat mat;
				mat.upload(img);
				s_bUseGPU = true;
			}
			catch (cv::Exception&) {
				s_bUseGPU = false;
			}
		}
		else
			s_bUseGPU = false;

		s_bGPUChecked = true;

		return s_bUseGPU;
	}

	bool ConvertColor(cv::Mat const& imgSrc, cv::Mat& imgDest, int eCode) {
		try {
			//#ifdef HAVE_CUDA
			if (s_bUseGPU) {
				try {
					cv::cuda::GpuMat dst;
					cv::cuda::cvtColor(cv::cuda::GpuMat(imgSrc), dst, eCode);
					dst.download(imgDest);
					return true;
				} catch (...) {
					//TRACE((GTL__FUNCSIG " - Error\n").c_str());
				}
			}
			//#endif
			cvtColor(imgSrc, imgDest, eCode);
		}
		catch (...) {
			return false;
		}
		return true;
	}

	bool ResizeImage(cv::Mat const& imgSrc, cv::Mat& imgDest, double dScale, int eInterpolation) {
		try {
			//#ifdef HAVE_CUDA
			if (s_bUseGPU) {
				try {
					cv::cuda::GpuMat dst;
					cv::cuda::resize(cv::cuda::GpuMat(imgSrc), dst, cv::Size(), dScale, dScale, eInterpolation);
					dst.download(imgDest);
					return true;
				}
				catch (...) {
					//TRACE((GTL__FUNCSIG " - Error\n").c_str());
				}
			}
			//#endif
			resize(imgSrc, imgDest, cv::Size(), dScale, dScale, eInterpolation);
		}
		catch (...) {
			return false;
		}
		return true;
	}

#ifdef HAVE_CUDA
	GTL__API cv::cuda::TemplateMatching& GetCudaTemplateMatching(int mat_type, int method) {
		static std::mutex mtxTemplateMatching;
		static std::map<int, cv::Ptr<cv::cuda::TemplateMatching>> mapTemplateMatching;
		std::lock_guard lock(mtxTemplateMatching);
		auto& ref = mapTemplateMatching[mat_type];
		if (ref.empty())
			ref = cv::cuda::createTemplateMatching(mat_type, method);
		return *ref;
	}
#endif

	bool MatchTemplate(cv::Mat const& img, cv::Mat const& imgTempl, cv::Mat& matResult, int method) {
		try {
			//#ifdef HAVE_CUDA
			if (s_bUseGPU) {
				try {
					cv::cuda::GpuMat mat;
					GetCudaTemplateMatching(img.type(), method).match(cv::cuda::GpuMat(img), cv::cuda::GpuMat(imgTempl), mat);
					mat.download(matResult);
					return true;
				}
				catch (...) {
					//TRACE((GTL__FUNCSIG " - Error\n").c_str());
				}
			}
			//#endif
			cv::matchTemplate(img, imgTempl, matResult, method);
		}
		catch (...) {
			return false;
		}
		return true;
	}

	bool MatchTemplate(cv::Mat const& img, cv::Mat const& imgTempl, int method, xPoint2d& ptBest, double& dMinMax, double& dRate, double dScale, int eInterpolation) {
		try {
			double dMin, dMax;
			cv::Point ptMin, ptMax;
			bool bSuccess{};

			//#ifdef HAVE_CUDA
			if (s_bUseGPU) {
				try {
					cv::cuda::GpuMat imgG;
					cv::cuda::GpuMat imgGTempl;
					if (dScale == 0.0) {
						imgG.upload(img);
						imgGTempl.upload(imgTempl);
					}
					else {
						cv::cuda::resize(cv::cuda::GpuMat(imgG), imgG, cv::Size(), dScale, dScale, eInterpolation);
						cv::cuda::resize(cv::cuda::GpuMat(imgGTempl), imgGTempl, cv::Size(), dScale, dScale, eInterpolation);
					}
					cv::cuda::GpuMat matResult;
					GetCudaTemplateMatching(imgG.type(), method).match(imgG, imgGTempl, matResult);
					cv::cuda::minMaxLoc(matResult, &dMin, &dMax, &ptMin, &ptMax);
					bSuccess = true;
				}
				catch (...) {
					//TRACE((GTL__FUNCSIG " - Error\n").c_str());
				}
			}
			//#endif

			if (!bSuccess) {
				cv::Mat matResult;
				cv::Mat imgG(img), imgGTempl(imgTempl);
				if (dScale != 0.0) {
					cv::resize(imgG, imgG, cv::Size(), dScale, dScale, eInterpolation);
					cv::resize(imgGTempl, imgGTempl, cv::Size(), dScale, dScale, eInterpolation);
				}
				cv::matchTemplate(imgG, imgGTempl, matResult, method);
				cv::minMaxLoc(matResult, &dMin, &dMax, &ptMin, &ptMax);
			}

			switch (method) {
			case cv::TM_SQDIFF :
			case cv::TM_SQDIFF_NORMED :
				ptBest = ptMin;
				break;
			case cv::TM_CCORR :
			case cv::TM_CCORR_NORMED :
			case cv::TM_CCOEFF :
			case cv::TM_CCOEFF_NORMED :
				ptBest = ptMax;
				break;
			}

			switch (method) {
			case cv::TM_SQDIFF :
				dMinMax = dMin;
				dRate = (dMax-dMin) / dMax;
				break;
			case cv::TM_SQDIFF_NORMED :
				dMinMax = dMin;
				dRate = (dMax-dMin) / dMax;
				break;
			case cv::TM_CCORR :
			case cv::TM_CCOEFF :
				dMinMax = dMax;
				dRate = dMax;
				break;
			case cv::TM_CCORR_NORMED :
			case cv::TM_CCOEFF_NORMED :
				dMinMax = dMax;
				dRate = dMax;
				break;
			}

			if (_isnan(dRate))
				dRate = 0;

			if (dScale == 0.0) {
				ptBest.x += imgTempl.cols/2;
				ptBest.y += imgTempl.rows/2;
			} else {
				ptBest.x = ptBest.x / dScale + imgTempl.cols/2;
				ptBest.y = ptBest.y / dScale + imgTempl.rows/2;
			}

		} catch (...) {
			return false;
		}
		return true;
	}


	bool MatToMatTransparent(cv::Mat const& imgSource, cv::Mat& imgTarget, cv::Mat const& matMask) {
		try {
			//std::vector<Mat> mats;
			//split(imgSource, mats);
			//merge(mats, imgSource);
			subtract(imgSource, imgSource, imgSource, matMask);
			add(imgSource, imgTarget, imgSource, matMask);
		}
		catch (...) {
			return false;
		}
		return true;
	}

	bool MatToMatTransparent(cv::Mat const& imgSource, cv::Mat& imgTarget, cv::Scalar const& crTransparent) {
		if (imgTarget.empty() || (imgSource.type() != imgTarget.type()) || (imgSource.size() != imgTarget.size()) ) {
			return false;
		}
		if ( (imgSource.channels() != 3) || (imgSource.depth() != CV_8U) )
			return false;

		cv::Vec3b cr;
		cr[0] = (uint8)crTransparent[0];
		cr[1] = (uint8)crTransparent[1];
		cr[2] = (uint8)crTransparent[2];

		for (int y = 0; y < imgSource.rows; y++) {
			const cv::Vec3b* ptr = imgSource.ptr<cv::Vec3b>(y);
			cv::Vec3b* ptr2 = imgTarget.ptr<cv::Vec3b>(y);
			for (int x = 0; x < imgSource.cols; x++) {
				if (ptr[x] != cr)
					ptr2[x] = ptr[x];
			}
		}

		return false;
	}

	namespace internal {

		template < bool bNoPaletteLookup, bool bBytePacking, typename telement = uint8, bool bLoopUnrolling = true, bool bMultiThreaded = true >
		bool MatToBitmapFile(std::ostream& f, cv::Mat const& img, int nBPP, std::vector<telement> const& pal, callback_progress_t funcCallback) {

			int width32 = (img.cols * nBPP + 31) / 32 * 4;
			int pixel_per_byte = (8/nBPP);
			int nColPixel = pixel_per_byte ? img.cols/ pixel_per_byte * pixel_per_byte : img.cols;

			using Func_PackSingleRow = std::function<void(int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal)>;
			Func_PackSingleRow PackSingleRow;

			if constexpr (bBytePacking) {
				if constexpr (bLoopUnrolling) {

					switch (nBPP) {
					case 1 :
						PackSingleRow = [img_cols = img.cols, nBPP, pixel_per_byte, nColPixel](int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal) {
							int x{};
							for (; x < nColPixel; x += pixel_per_byte) {
								int col = x / pixel_per_byte;
								if constexpr (bNoPaletteLookup) {
									line[col] = (ptr[x + 0] << 7) | (ptr[x + 1] << 6) | (ptr[x + 2] << 5) | (ptr[x + 3] << 4) | (ptr[x + 4] << 3) | (ptr[x + 5] << 2) | (ptr[x + 6] << 1) | (ptr[x + 7]);
								}
								else {
									line[col] = (pal[ptr[x + 0]] << 7) | (pal[ptr[x + 1]] << 6) | (pal[ptr[x + 2]] << 5) | (pal[ptr[x + 3]] << 4) | (pal[ptr[x + 4]] << 3) | (pal[ptr[x + 5]] << 2) | (pal[ptr[x + 6]] << 1) | (pal[ptr[x + 7]]);
								}
							}
							int col = x / pixel_per_byte;
							if (col < line.size())
								line[col] = 0;
							for (int shift{ 8-nBPP }; x < img_cols; x++, shift--) {
								if constexpr (bNoPaletteLookup) {
									line[col] |= ptr[x] << shift;
								}
								else {
									line[col] |= pal[ptr[x]] << shift;
								}
							}
						};
						break;
					case 4 :
						PackSingleRow = [img_cols = img.cols, nBPP, pixel_per_byte, nColPixel](int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal) {
							int x{};
							for (; x < nColPixel; x += pixel_per_byte) {
								int col = x / pixel_per_byte;
								if constexpr (bNoPaletteLookup) {
									line[col] = (ptr[x + 0] << 4) | (ptr[x + 1] << 0);
								}
								else {
									line[col] = (pal[ptr[x + 0]] << 4) | (pal[ptr[x + 1]] << 0);
								}
							}
							int col = x / pixel_per_byte;
							if (col < line.size())
								line[col] = 0;
							for (int shift{ 8-nBPP }; x < img_cols; x++, shift-=nBPP) {
								if constexpr (bNoPaletteLookup) {
									line[col] |= ptr[x] << shift;
								}
								else {
									line[col] |= pal[ptr[x]] << shift;
								}
							}
						};
						break;
					case 8 :
						PackSingleRow = [img_cols = img.cols](int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal) {
							for (int x{}; x < img_cols; x++) {
								if constexpr (bNoPaletteLookup) {
									line[x] = ptr[x];
								}
								else {
									line[x] = pal[ptr[x]];
								}
							}
						};
						break;
					default :
						return false;
					}
				}
				else {
					switch (nBPP) {
					case 1 :
					case 4 :
						PackSingleRow = [img_cols = img.cols, nBPP, pixel_per_byte](int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal) {
							std::memset(line.data(), 0, line.size()*sizeof(line[0]));
							for (int x{}; x < img_cols; x++) {
								int col = x / pixel_per_byte;
								int shift = 8 - (nBPP * ((x%pixel_per_byte)+1));
								if constexpr (bNoPaletteLookup) {
									line[col] |= ptr[x] << shift;
								}
								else {
									line[col] |= pal[ptr[x]] << shift;
								}
							}
						};
						break;
					case 8 :
						PackSingleRow = [img_cols = img.cols](int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal) {
							for (int x{}; x < img_cols; x++) {
								if constexpr (bNoPaletteLookup) {
									line[x] = ptr[x];
								}
								else {
									line[x] = pal[ptr[x]];
								}
							}
						};
						break;
					default :
						return false;
					}
				}
			}	// if constexpr (bBytePacking)
			else {
				if (nBPP == 24) {
					if (sizeof(telement) != 3)
						return false;
					PackSingleRow = [img_cols = img.cols, nBPP, pixel_per_byte](int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal) {
						telement* line3 = (telement*)line.data();
						for (int x{}; x < img_cols; x++) {
							if constexpr (bNoPaletteLookup) {
								line3[x] = ptr[x];
							}
							else {
								line3[x] = pal[ptr[x]];
							}
						}
					};
				}
			}

			if constexpr (bMultiThreaded) {
				auto nCPUDetected = std::thread::hardware_concurrency();
				auto nThread = std::min((uint)img.rows, (nCPUDetected <= 0) ? 2 : nCPUDetected);

				struct BUFFER {
					std::atomic<int> y{-1};
					std::atomic<bool> bReady{false};
					std::vector<uint8> line;
					//std::set<int> idsThread;

					BUFFER() : y{-1} {}
				};
				size_t sizeBuffer = nThread*2;
				std::mutex mtxBuffer;
				std::vector<BUFFER> buffers(sizeBuffer);
				for (auto& buf : buffers)
					buf.line.assign(width32, 0);

				std::atomic<int> yCur, yWritten;
				bool bWritten{};

				auto GetBuffer = [img_rows = img.rows, &mtxBuffer, &buffers, &yCur, &yWritten](std::stop_token stop) -> BUFFER* {
					if (yCur >= img_rows)
						return nullptr;
					std::unique_lock lock(mtxBuffer);
					auto y = yCur.fetch_add(1);
					if (y >= img_rows)
						return nullptr;
					auto& buf = buffers[y % buffers.size()];
					int yOld = buf.y;
					while (yOld >= 0) {
						buf.y.wait(yOld);
						if (stop.stop_requested())
							return nullptr;
						yOld = buf.y;
					}
					buf.y = y;
					buf.y.notify_one();

					return &buf;
				};

				auto PackBuffer = [&img, &buffers, &PackSingleRow, &pal, &yCur, &yWritten/*, &id*/, &GetBuffer](std::stop_token stop) {
					do {
						auto* pBuffer = GetBuffer(stop);
						if (!pBuffer)
							break;
						auto& buf = *pBuffer;

						PackSingleRow(buf.y, buf.line, img.ptr<telement>(buf.y), pal);

						buf.bReady = true;
						buf.bReady.notify_one();
					} while(!stop.stop_requested());
				};

				auto Writer = [&f, &buffers, img_rows = img.rows, &yWritten, &bWritten, &funcCallback]() {
					int iPercent{};
					while (yWritten < img_rows) {
						auto index = yWritten % buffers.size();
						auto& buf = buffers[index];

						while (yWritten != buf.y) {
							buf.y.wait(-1);
						}
						while (!buf.bReady) {
							buf.bReady.wait(false);
						}

						if (!f.write((char*)buf.line.data(), buf.line.size()))
							return;

						buf.bReady = false;
						buf.y = -1;
						buf.y.notify_one();

						yWritten++;
						yWritten.notify_one();

						if (funcCallback) {
							int iPercentNew = yWritten * 100 / img_rows;
							if (iPercent != iPercentNew) {
								iPercent = iPercentNew;
								if (!funcCallback(iPercent, false, false))
									return;
							}
						}
					};
					f.flush();
					bWritten = (bool)f;
				};

				std::thread threadWriter(Writer);
				std::vector<std::jthread> threads;
				threads.reserve(nThread);
				for (uint i{}; i < nThread; i++) {
					threads.emplace_back(PackBuffer);
				}

				threadWriter.join();
				if (!bWritten) {
					for (auto& thread : threads)
						thread.request_stop();
					for (auto& buffer : buffers) {
						buffer.y = -1;
						buffer.y.notify_all();
					}
				}
				return bWritten;
			}
			else {
				int iPercent{};
				std::vector<uint8> line((size_t)width32, 0);
				for (int y{}; y < img.rows; y++) {
					auto const* ptr = img.ptr<telement>(y);
					PackSingleRow(y, line, ptr, pal);
					f.write((char const*)line.data(), width32);

					if (funcCallback) {
						int iPercentNew = y * 100 / img.rows;
						if (iPercent != iPercentNew) {
							iPercent = iPercentNew;
							if (!funcCallback(iPercent, false, false))
								return false;
						}
					}
				}
				return true;
			}
		}

	}	// namespace internal

	bool SaveBitmapMat(std::filesystem::path const& path, cv::Mat const& img, int nBPP, gtl::xSize2i const& pelsPerMeter, std::span<gtl::color_bgra_t> palette, bool bNoPaletteLookup, bool bBottom2Top, callback_progress_t funcCallback) {
		// todo : CV_8UC3 with palette.

		bool bOK{};

		// Trigger notifying it's over.
		xFinalAction fa([&funcCallback, &bOK]{if (funcCallback) funcCallback(-1, true, !bOK);});

		if (img.empty())
			return false;

		auto type = img.type();

		int cx = img.cols;
		int cy = img.rows;
		//if ((cx >= 0xffff) or (cy >= 0xffff))
		//	return false;
		uint64_t lenFileExpect = sizeof(BMP_FILE_HEADER) + sizeof(BITMAP_HEADER);
		lenFileExpect += (nBPP <= 8) ? (uint64_t)cx * cy / (8/nBPP) : (uint64_t)cx*cy*(nBPP/8);
		if (lenFileExpect >= 0xffff'fff0)
			return false;
		int pixel_size = (type == CV_8UC3) ? 3 : ((type == CV_8UC1) ? 1 : 0);
		if (pixel_size <= 0)
			return false;

		BITMAP_HEADER header{};

		header.size = sizeof(header);
		header.width = cx;
		header.height = bBottom2Top ? cy : -cy;
		header.planes = 1;
		header.compression = 0;//BI_RGB;
		header.sizeImage = 0;//cx * cy * pixel_size;
		header.XPelsPerMeter = pelsPerMeter.cx;
		header.YPelsPerMeter = pelsPerMeter.cy;

		cv::Mat img2 = img;
		if (bBottom2Top)
			cv::flip(img, img2, 0);

		if (pixel_size == 3) {
			std::ofstream f(path, std::ios_base::binary);
			if (!f)
				return false;

			BMP_FILE_HEADER fh;

			auto width32 = gtl::AdjustAlign32(cx * 3);
			fh.offsetData = sizeof(fh) + sizeof(header);
			fh.sizeFile = fh.offsetData + width32 * cy;
			header.nBPP = nBPP;
			header.nColorUsed = header.nColorImportant = 0;

			f.write((char const*)&fh, sizeof(fh));
			f.write((char const*)&header, sizeof(header));

			//char redundant[4]{};
			//size_t sr = width32 - (cx * 3);
			//for (int y{}; y < img.rows; y++) {
			//	auto* ptr = img.ptr<cv::Vec3b>(y);
			//	f.write((char const*)ptr, 3 * img.cols);
			//	f.write(redundant, sr);
			//}
			//bOK = true;
			bOK = gtl::internal::MatToBitmapFile<true, false, cv::Vec3b, bLoopUnrolling, bMultiThreaded>(f, img2, nBPP, {}, funcCallback);

			return true;
		}
		else if (type == CV_8UC1) {
			if ((nBPP != 1) and /*(nBPP != 2) and */(nBPP != 4) and (nBPP != 8)) {
				return false;
			}
			if (palette.empty()) {
				return false;
			}

			std::ofstream f(path, std::ios_base::binary);
			if (!f)
				return false;

			BMP_FILE_HEADER fh;

			auto width32 = (cx * nBPP + 31) / 32 * 4;
			fh.offsetData = (uint32_t) ( sizeof(fh) + sizeof(header) + palette.size()/*(0x01 << nBPP)*/ * sizeof(gtl::color_bgra_t) );
			fh.sizeFile = fh.offsetData + width32 * cy;
			header.nBPP = nBPP;
			header.nColorUsed = header.nColorImportant = (uint32_t)palette.size();

			f.write((char const*)&fh, sizeof(fh));
			f.write((char const*)&header, sizeof(header));

			f.write((char const*)palette.data(), palette.size() * sizeof(palette[0]));

			std::vector<uint8> pal((size_t)256, 0);
			if (!bNoPaletteLookup) {
				for (size_t i{}; i < palette.size(); i++) {
					pal[palette[i].r] = (uint8)i;
				}
			}

			if (bNoPaletteLookup)
				bOK = gtl::internal::MatToBitmapFile<true, true, uint8, bLoopUnrolling, bMultiThreaded>(f, img2, nBPP, pal, funcCallback);
			else
				bOK = gtl::internal::MatToBitmapFile<false, true, uint8, bLoopUnrolling, bMultiThreaded>(f, img2, nBPP, pal, funcCallback);
			return bOK;
		}

		return false;
	}

	namespace internal {

		template < typename telement = cv::Vec3b, bool bLoopUnrolling = true, bool bMultiThreaded = false >
		bool MatFromBitmapFile(std::istream& f, cv::Mat& img, int nBPP, std::vector<telement> palette, callback_progress_t funcCallback) {

			if ((nBPP != 1) and (nBPP != 4) and (nBPP != 8) and (nBPP != 24) and (nBPP != 32) )
				return false;

			//auto type = img.type();
			//if ( (type != CV_8UC3) and (type != CV_8UC4) and (type != CV_8UC1) )
			//	return false;
			if (sizeof(telement) != img.elemSize())
				return false;

			int width32 = (img.cols * nBPP + 31) / 32 * 4;
			int pixel_per_byte = (8/nBPP);
			int nColPixel = pixel_per_byte ? img.cols/ pixel_per_byte * pixel_per_byte : img.cols;

			using Func_UnPackSingleRow = std::function<void(int y, std::vector<uint8> const& line, telement* ptr, std::vector<telement> const& palette)>;
			Func_UnPackSingleRow UnPackSingleRow;


			using Func_UnpackLine = std::function<void()>;

			if constexpr (bLoopUnrolling) {
				if (nBPP == 1) {
					if (palette.size() < 2)
						return false;
					UnPackSingleRow = [img_cols = img.cols, &nBPP, &pixel_per_byte, &nColPixel](int y, std::vector<uint8> const& line, telement* ptr, std::vector<telement> const& palette) {
						int x{};
						for (; x < nColPixel; x += pixel_per_byte) {
							int col = x / pixel_per_byte;
							ptr[x + 0] = palette[(line[col] >> 7) & 0b0000'0001];
							ptr[x + 1] = palette[(line[col] >> 6) & 0b0000'0001];
							ptr[x + 2] = palette[(line[col] >> 5) & 0b0000'0001];
							ptr[x + 3] = palette[(line[col] >> 4) & 0b0000'0001];
							ptr[x + 4] = palette[(line[col] >> 3) & 0b0000'0001];
							ptr[x + 5] = palette[(line[col] >> 2) & 0b0000'0001];
							ptr[x + 6] = palette[(line[col] >> 1) & 0b0000'0001];
							ptr[x + 7] = palette[(line[col] >> 0) & 0b0000'0001];
						}
						int col = x / pixel_per_byte;
						for (int shift{ 8-nBPP }; x < img_cols; x++, shift -= nBPP) {
							ptr[x] = palette[(line[col] >> shift) & 0b0000'0001];
						}
					};
				}
				else if (nBPP == 4) {
					if (palette.size() < 16)
						return false;
					UnPackSingleRow = [img_cols = img.cols, &nBPP, &pixel_per_byte, &nColPixel](int y, std::vector<uint8> const& line, telement* ptr, std::vector<telement> const& palette) {
						int x{};
						for (; x < nColPixel; x += 2) {
							int col = x / 2;
							ptr[x + 0] = palette[(line[col] >> 4) & 0b0000'1111];
							ptr[x + 1] = palette[(line[col] >> 0) & 0b0000'1111];
						}
						int col = x / 2;
						for (int shift{ 4 }; x < img_cols; x++, shift -= 4) {
							ptr[x] = palette[(line[col] >> shift) & 0b0000'1111];
						}
					};
				}
				else if (nBPP == 8) {
					if (palette.size() < 256)
						return false;
					UnPackSingleRow = [img_cols = img.cols](int y, std::vector<uint8> const& line, telement* ptr, std::vector<telement> const& palette) {
						for (int x{}; x < img_cols; x++) {
							ptr[x] = palette[line[x]];
						}
					};
				}
				else if ((nBPP == 24) or (nBPP == 32)) {
					// nothing here.
				}
				else
					return false;
			}
			else {
				if ( (nBPP <= 8) and (palette.size() < (0x01 << nBPP)) )
					return false;

				if ( (nBPP == 1) or (nBPP == 4) ) {
					uint8 mask = (0x01 << nBPP) - 1;
					UnPackSingleRow = [img_cols = img.cols, &nBPP, &pixel_per_byte, mask](int y, std::vector<uint8> const& line, telement* ptr, std::vector<telement> const& palette) {
						for (int x{}; x < img_cols; x++) {
							int col = x / pixel_per_byte;
							int shift = 8 - (nBPP * ((x%pixel_per_byte)+1));
							ptr[x] = palette[(line[col] >> shift) & mask];
						}
					};
				} else if (nBPP == 8) {
					UnPackSingleRow = [img_cols = img.cols](int y, std::vector<uint8> const& line, telement* ptr, std::vector<telement> const& palette) {
						for (int x{}; x < img_cols; x++) {
							ptr[x] = palette[line[x]];
						}
					};
				}
				else if ((nBPP == 24) or (nBPP == 32)) {
					// nothing here.
				}
				else
					return false;
			}

			if ( (nBPP == 24) or (nBPP == 32) ) {
				UnPackSingleRow = [img_cols = img.cols](int y, std::vector<uint8> const& line, telement* ptr, std::vector<telement> const& palette) {
					for (int x{}; x < img_cols; x++) {
						int xc = x * sizeof(telement);
						if constexpr (sizeof(telement) == sizeof(cv::Vec3b)) {
							ptr[x][0] = line[xc + 0];
							ptr[x][1] = line[xc + 1];
							ptr[x][2] = line[xc + 2];
						}
					}
				};
			}

			if constexpr (bMultiThreaded) {
				auto nCPUDetected = std::thread::hardware_concurrency();
				auto nThread = std::min((uint)img.rows, (nCPUDetected <= 0) ? 2 : nCPUDetected);

				struct BUFFER {
					std::atomic<int> y{-1};
					std::vector<uint8> line;
				};
				std::vector<BUFFER> buffers((size_t)nThread*2);
				for (auto& buf : buffers)
					buf.line.assign((size_t)width32, (uint8)0);

				std::condition_variable cv;
				std::mutex mtxQ;
				std::queue<BUFFER*> q;
				bool bError{};

				auto Reader = [&f, &buffers, img_rows = img.rows, &q, &mtxQ, &cv, &bError, &funcCallback]() {
					int y{};
					int iPercent{};
					for (; y < img_rows; y++) {
						auto index = y % buffers.size();
						auto& buf = buffers[index];
						int yOld = buf.y;
						while (yOld >= 0) {
							buf.y.wait(yOld);
							yOld = buf.y;
						}
						if (!f.read((char*)buf.line.data(), buf.line.size())) {
							bError = true;
							break;
						}
						buf.y = y;
						//buf.y.notify_one();
						{
							std::unique_lock lock(mtxQ);
							q.push(&buf);
						}
						cv.notify_one();

						if (funcCallback) {
							int iPercentNew = y * 100 / img_rows;
							if (iPercent != iPercentNew) {
								iPercent = iPercentNew;
								if (!funcCallback(iPercent, false, false)) {
									bError = true;
									break;
								}
							}
						}
					}
				};

				auto UnpackBuffer = [&q, &mtxQ, &cv, &UnPackSingleRow, &img, &palette](std::stop_token tk) {
					do {
						BUFFER* pBuf{};
						while (!pBuf) {
							std::unique_lock lock(mtxQ);
							if (q.empty()) {
								if (tk.stop_requested())
									return;
								cv.wait(lock);
							}
							else {
								pBuf = q.front();
								q.pop();
							}
						}

						UnPackSingleRow(pBuf->y, pBuf->line, img.ptr<telement>(pBuf->y), palette);
						pBuf->y = -1;
						pBuf->y.notify_one();

					} while (true);
				};

				std::thread reader{Reader};
				std::stop_source ss;
				std::vector<std::jthread> threads;
				threads.reserve(nThread);
				for (uint i{}; i < nThread; i++) {
					threads.emplace_back(UnpackBuffer, ss.get_token());
				}

				reader.join();
				ss.request_stop();
				cv.notify_all();

				return !bError;

			}
			else {
				std::vector<uint8> line((size_t)width32, 0);
				int iPercent{};
				for (int y{}; y < img.rows; y++) {
					if (!f.read((char*)line.data(), line.size()))
						return false;
					auto* ptr = img.ptr<telement>(y);

					UnPackSingleRow(y, line, ptr, palette);

					if (funcCallback) {
						int iPercentNew = y * 100 / img.rows;
						if (iPercent != iPercentNew) {
							iPercent = iPercentNew;
							if (!funcCallback(iPercent, false, false))
								return false;
						}
					}
				}
			}

			return true;
		}

		template < typename telement = cv::Vec3b, bool bLoopUnrolling = true, bool bMultiThreaded = false >
		bool MatFromBitmapFilePixelArray(std::istream& f, cv::Mat& img, int nBPP, callback_progress_t funcCallback) {

			if ((nBPP != 1) and (nBPP != 4) and (nBPP != 8) and (nBPP != 24) and (nBPP != 32) )
				return false;

			if (sizeof(telement) != img.elemSize())
				return false;

			int width32 = (img.cols * nBPP + 31) / 32 * 4;
			int pixel_per_byte = (8/nBPP);
			int nColPixel = pixel_per_byte ? img.cols/ pixel_per_byte * pixel_per_byte : img.cols;

			using Func_UnPackSingleRow = std::function<void(int y, std::vector<uint8> const& line, telement* ptr)>;
			Func_UnPackSingleRow UnPackSingleRow;

			using Func_UnpackLine = std::function<void()>;

			if (nBPP == 1) {
				UnPackSingleRow = [img_cols = img.cols, &nBPP, &pixel_per_byte, &nColPixel](int y, std::vector<uint8> const& line, telement* ptr) {
					int x{};
					for (; x < nColPixel; x += pixel_per_byte) {
						int col = x / pixel_per_byte;
						ptr[x + 0] = (line[col] >> 7) & 0b0000'0001;
						ptr[x + 1] = (line[col] >> 6) & 0b0000'0001;
						ptr[x + 2] = (line[col] >> 5) & 0b0000'0001;
						ptr[x + 3] = (line[col] >> 4) & 0b0000'0001;
						ptr[x + 4] = (line[col] >> 3) & 0b0000'0001;
						ptr[x + 5] = (line[col] >> 2) & 0b0000'0001;
						ptr[x + 6] = (line[col] >> 1) & 0b0000'0001;
						ptr[x + 7] = (line[col] >> 0) & 0b0000'0001;
					}
					int col = x / pixel_per_byte;
					for (int shift{ 8-nBPP }; x < img_cols; x++, shift -= nBPP) {
						ptr[x] = (line[col] >> shift) & 0b0000'0001;
					}
				};
			}
			else if (nBPP == 4) {
				UnPackSingleRow = [img_cols = img.cols, &nBPP, &pixel_per_byte, &nColPixel](int y, std::vector<uint8> const& line, telement* ptr) {
					int x{};
					for (; x < nColPixel; x += 2) {
						int col = x / 2;
						ptr[x + 0] = (line[col] >> 4) & 0b0000'1111;
						ptr[x + 1] = (line[col] >> 0) & 0b0000'1111;
					}
					int col = x / 2;
					for (int shift{ 4 }; x < img_cols; x++, shift -= 4) {
						ptr[x] = (line[col] >> shift) & 0b0000'1111;
					}
				};
			}
			else if (nBPP == 8) {
				UnPackSingleRow = [img_cols = img.cols](int y, std::vector<uint8> const& line, telement* ptr) {
					for (int x{}; x < img_cols; x++) {
						ptr[x] = line[x];
					}
				};
			}
			else if ((nBPP == 24) or (nBPP == 32)) {
				// nothing here.
			}
			else {
				return false;
			}

			if ( (nBPP == 24) or (nBPP == 32) ) {
				UnPackSingleRow = [img_cols = img.cols](int y, std::vector<uint8> const& line, telement* ptr) {
					for (int x{}; x < img_cols; x++) {
						int xc = x * sizeof(telement);
						if constexpr (sizeof(telement) == sizeof(cv::Vec3b)) {
							ptr[x][0] = line[xc + 0];
							ptr[x][1] = line[xc + 1];
							ptr[x][2] = line[xc + 2];
						} else if constexpr (sizeof(telement) == sizeof(cv::Vec4b)) {
							ptr[x][0] = line[xc + 0];
							ptr[x][1] = line[xc + 1];
							ptr[x][2] = line[xc + 2];
							ptr[x][3] = line[xc + 3];
						}
					}
				};
			}

			auto nCPUDetected = std::thread::hardware_concurrency();
			auto nThread = std::min((uint)img.rows, (nCPUDetected <= 0) ? 2 : nCPUDetected);

			struct BUFFER {
				std::atomic<int> y{-1};
				std::vector<uint8> line;
			};
			std::vector<BUFFER> buffers((size_t)nThread*2);
			for (auto& buf : buffers)
				buf.line.assign((size_t)width32, (uint8)0);

			std::condition_variable cv;
			std::mutex mtxQ;
			std::queue<BUFFER*> q;
			bool bError{};

			auto Reader = [&f, &buffers, img_rows = img.rows, &q, &mtxQ, &cv, &bError, &funcCallback]() {
				int y{};
				int iPercent{};
				for (; y < img_rows; y++) {
					auto index = y % buffers.size();
					auto& buf = buffers[index];
					int yOld = buf.y;
					while (yOld >= 0) {
						buf.y.wait(yOld);
						yOld = buf.y;
					}
					if (!f.read((char*)buf.line.data(), buf.line.size())) {
						bError = true;
						break;
					}
					buf.y = y;
					//buf.y.notify_one();
					{
						std::unique_lock lock(mtxQ);
						q.push(&buf);
					}
					cv.notify_one();

					if (funcCallback) {
						int iPercentNew = y * 100 / img_rows;
						if (iPercent != iPercentNew) {
							iPercent = iPercentNew;
							if (!funcCallback(iPercent, false, false)) {
								bError = true;
								break;
							}
						}
					}
				}
			};

			auto UnpackBuffer = [&q, &mtxQ, &cv, &UnPackSingleRow, &img](std::stop_token tk) {
				do {
					BUFFER* pBuf{};
					while (!pBuf) {
						std::unique_lock lock(mtxQ);
						if (q.empty()) {
							if (tk.stop_requested())
								return;
							cv.wait(lock);
						}
						else {
							pBuf = q.front();
							q.pop();
						}
					}

					UnPackSingleRow(pBuf->y, pBuf->line, img.ptr<telement>(pBuf->y));
					pBuf->y = -1;
					pBuf->y.notify_one();

				} while (true);
			};

			std::thread reader{Reader};
			std::stop_source ss;
			std::vector<std::jthread> threads;
			threads.reserve(nThread);
			for (uint i{}; i < nThread; i++) {
				threads.emplace_back(UnpackBuffer, ss.get_token());
			}

			reader.join();
			ss.request_stop();
			cv.notify_all();

			return !bError;

		}

	}	// namespace internal

	bool LoadBitmapHeader(std::istream& is, BMP_FILE_HEADER& fileHeader, BITMAP_V5_HEADER& header) try {
		if (!is)
			return false;
		if (!is.read((char*)&fileHeader, sizeof(fileHeader)))
			return false;
		uint32_t sizeHeader{};
		if (!is.read((char*)&sizeHeader, sizeof(sizeHeader)))
			return false;
		header.size = sizeHeader;
		return (bool)is.read((char*)&header + sizeof(sizeHeader), sizeHeader - sizeof(sizeHeader));
	} catch (...) {
		return false;
	}
	bool LoadBitmapHeader(std::filesystem::path const& path, BMP_FILE_HEADER& fileHeader, BITMAP_V5_HEADER& header) {
		std::ifstream is(path, std::ios_base::binary);
		return LoadBitmapHeader(is, fileHeader, header);
	}
	bool LoadBitmapHeader(std::istream& is, BMP_FILE_HEADER& fileHeader, variant_BITMAP_HEADER& header) try {
		if (!is.read((char*)&fileHeader, sizeof(fileHeader)))
			return false;
		uint32_t sizeHeader{};
		if (!is.read((char*)&sizeHeader, sizeof(sizeHeader)))
			return false;
		char* pos {};
		switch (sizeHeader) {
		case sizeof(BITMAP_HEADER)		: header.emplace<0, BITMAP_HEADER>({});		pos = (char*)&std::get<BITMAP_HEADER>(header); break;
		case sizeof(BITMAP_V4_HEADER)	: header.emplace<1, BITMAP_V4_HEADER>({});	pos = (char*)&std::get<BITMAP_V4_HEADER>(header); break;
		case sizeof(BITMAP_V5_HEADER)	: header.emplace<2, BITMAP_V5_HEADER>({});	pos = (char*)&std::get<BITMAP_V5_HEADER>(header); break;
		default:
			return false;
		}
		*(uint32_t*)pos = sizeHeader;
		return (bool)is.read(pos + sizeof(sizeHeader), sizeHeader - sizeof(sizeHeader));
	}
	catch (...) {
		return false;
	}
	bool LoadBitmapHeader(std::filesystem::path const& path, BMP_FILE_HEADER& fileHeader, variant_BITMAP_HEADER& header) {
		std::ifstream f(path, std::ios_base::binary);
		return LoadBitmapHeader(f, fileHeader, header);
	}
	sLoadBitmapHeaderResult LoadBitmapHeader(std::istream& is) {
		sLoadBitmapHeaderResult result{};
		result.result = LoadBitmapHeader(is, result.fileHeader, result.header);
		return result;
	}
	sLoadBitmapHeaderResult LoadBitmapHeader(std::filesystem::path const& path) {
		std::ifstream is(path, std::ios_base::binary);
		return LoadBitmapHeader(is);
	}

	/// @brief Load Image from BITMAP file. Image is COLOR or GRAY level image.
	/// @param path 
	/// @param img : CV_8UC1 : gray scale, CV_8UC3 : color (no palette supported), for CV8UC3, palette is not used.
	/// @param nBPP 
	/// @param palette 
	/// @return 
	cv::Mat LoadBitmapMat(std::istream& is, gtl::xSize2i& pelsPerMeter, callback_progress_t funcCallback) {
		bool bOK{};

		// Trigger notifying it's over.
		xFinalAction fa([&funcCallback, &bOK] {if (funcCallback) funcCallback(-1, true, !bOK); });

		cv::Mat img;
		if (!is)
			return img;

		try {

			BMP_FILE_HEADER fh;
			variant_BITMAP_HEADER varHeader{};
			if (!LoadBitmapHeader(is, fh, varHeader))
				return img;

			BITMAP_HEADER* pos {};
			{
				pos = std::get_if<BITMAP_HEADER>(&varHeader);
				if (!pos)
					pos = (BITMAP_HEADER*)std::get_if<BITMAP_V4_HEADER>(&varHeader);
				if (!pos)
					pos = (BITMAP_HEADER*)std::get_if<BITMAP_V5_HEADER>(&varHeader);
				if (!pos)
					return img;
			}
			BITMAP_HEADER& header = *pos;

			if (header.compression or (header.planes != 1))
				return img;

			pelsPerMeter.cx = header.XPelsPerMeter;
			pelsPerMeter.cy = header.YPelsPerMeter;

			int cx = header.width;
			int cy = header.height;
			bool bFlipY{};
			if (cy < 0) {
				cy = -cy;
			}
			else {
				bFlipY = true;
			}
			if ((cx <= 0) or (cy <= 0) /*or ((uint64_t)cx * (uint64_t)cy >= 0xffff'ff00ull)*/)
				return img;

			std::vector<cv::Vec3b> palette;

			// Load Palette
			ptrdiff_t sizePalette = fh.offsetData - sizeof(fh) - header.size;
			for (; sizePalette >= 4; sizePalette -= sizeof(gtl::color_bgra_t)) {
				gtl::color_bgra_t color{};
				if (!is.read((char*)&color, sizeof(color)))
					return img;
				palette.push_back(cv::Vec3b(color.b, color.g, color.r));
			}
			if (!is.seekg(fh.offsetData))
				return img;

			bool bGrayScale {};
			if (header.nBPP <= 8) {
				bGrayScale = true;
				for (auto const& v : palette) {
					if ((v[0] != v[1]) and (v[1] != v[2])) {
						bGrayScale = false;
						break;
					}
				}
			}

			img = cv::Mat::zeros(cv::Size(cx, cy), bGrayScale ? CV_8UC1 : CV_8UC3);
			if (bGrayScale) {
				std::vector<uint8> paletteG(palette.size(), 0);
				for (size_t i{}; i < palette.size(); i++)
					paletteG[i] = palette[i][0];

				bOK = gtl::internal::MatFromBitmapFile<uint8, bLoopUnrolling, bMultiThreaded>(is, img, header.nBPP, paletteG, funcCallback);
			}
			else {
				bOK = gtl::internal::MatFromBitmapFile<cv::Vec3b, bLoopUnrolling, bMultiThreaded>(is, img, header.nBPP, palette, funcCallback);
			}
			if (!bOK)
				img.release();

			if (!img.empty() and bFlipY) {
				cv::flip(img, img, 0);
			}
		} catch (...) {
			img.release();
		}
		return img;
	}
	cv::Mat LoadBitmapMat(std::filesystem::path const& path, gtl::xSize2i& pelsPerMeter, callback_progress_t funcCallback) {
		std::ifstream f(path, std::ios_base::binary);
		return LoadBitmapMat(f, pelsPerMeter, funcCallback);
	}

	sLoadBitmapMatResult LoadBitmapMat(std::istream& is, callback_progress_t funcCallback) {
		sLoadBitmapMatResult result;
		result.img = LoadBitmapMat(is, result.pelsPerMeter, funcCallback);
		return result;
	}
	sLoadBitmapMatResult LoadBitmapMat(std::filesystem::path const& path, callback_progress_t funcCallback) {
		std::ifstream is(path, std::ios::binary);
		return LoadBitmapMat(is, funcCallback);
	}

	/// @brief Load Image into Mat. Image is Pixel ColorIndex.
	/// @param path 
	/// @param img : CV_8UC1 : pixel palette index, CV_8UC3 : color (no palette supported), for CV8UC3, palette is not used.
	/// @param nBPP 
	/// @param palette 
	/// @return 
	cv::Mat LoadBitmapMatPixelArray(std::istream& is, gtl::xSize2i& pelsPerMeter, std::vector<gtl::color_bgra_t>& palette, callback_progress_t funcCallback) {
		bool bOK{};

		// Trigger notifying it's over.
		xFinalAction fa([&funcCallback, &bOK] {if (funcCallback) funcCallback(-1, true, !bOK); });


		cv::Mat img;
		if (!is)
			return img;

		try {

			BMP_FILE_HEADER fh;
			variant_BITMAP_HEADER varHeader{};
			if (!LoadBitmapHeader(is, fh, varHeader))
				return img;

			BITMAP_HEADER* pos {};
			{
				pos = std::get_if<BITMAP_HEADER>(&varHeader);
				if (!pos)
					pos = (BITMAP_HEADER*)std::get_if<BITMAP_V4_HEADER>(&varHeader);
				if (!pos)
					pos = (BITMAP_HEADER*)std::get_if<BITMAP_V5_HEADER>(&varHeader);
				if (!pos)
					return img;
			}
			BITMAP_HEADER& header = *pos;

			if (header.compression or (header.planes != 1))
				return img;

			pelsPerMeter.cx = header.XPelsPerMeter;
			pelsPerMeter.cy = header.YPelsPerMeter;

			int cx = header.width;
			int cy = header.height;
			bool bFlipY{};
			if (cy < 0) {
				cy = -cy;
			}
			else {
				bFlipY = true;
			}
			if ((cx <= 0) or (cy <= 0) /*or ((uint64_t)cx * (uint64_t)cy >= 0xffff'ff00ull)*/)
				return img;

			palette.clear();
			// Load Palette
			ptrdiff_t sizePalette = fh.offsetData - sizeof(fh) - header.size;
			if (sizePalette/4 > 0) {
				palette.reserve(sizePalette/4);
				for (; sizePalette >= 4; sizePalette -= sizeof(gtl::color_bgra_t)) {
					gtl::color_bgra_t color{};
					if (!is.read((char*)&color, sizeof(color)))
						return img;
					palette.push_back(color);
				}
			}
			if (!is.seekg(fh.offsetData))
				return img;

			if (header.nBPP <= 8) {
				img = cv::Mat::zeros(cv::Size(cx, cy), CV_8UC1);
				bOK = gtl::internal::MatFromBitmapFilePixelArray<uint8, bLoopUnrolling, bMultiThreaded>(is, img, header.nBPP, funcCallback);
			} else if (header.nBPP <= 16) {
				img = cv::Mat::zeros(cv::Size(cx, cy), CV_16UC1);
				bOK = gtl::internal::MatFromBitmapFilePixelArray<uint16, bLoopUnrolling, bMultiThreaded>(is, img, header.nBPP, funcCallback);
			} else if (header.nBPP <= 24) {
				img = cv::Mat::zeros(cv::Size(cx, cy), CV_8UC3);
				bOK = gtl::internal::MatFromBitmapFilePixelArray<cv::Vec3b, bLoopUnrolling, bMultiThreaded>(is, img, header.nBPP, funcCallback);
			} else if (header.nBPP <= 32) {
				img = cv::Mat::zeros(cv::Size(cx, cy), CV_8UC4);
				bOK = gtl::internal::MatFromBitmapFilePixelArray<cv::Vec4b, bLoopUnrolling, bMultiThreaded>(is, img, header.nBPP, funcCallback);
			}

			if (!bOK)
				img.release();

			if (!img.empty() and bFlipY) {
				cv::flip(img, img, 0);
			}
		}
		catch (...) {
			img.release();
		}
		return img;
	}
	cv::Mat LoadBitmapMatPixelArray(std::filesystem::path const& path, gtl::xSize2i& pelsPerMeter, std::vector<gtl::color_bgra_t>& palette, callback_progress_t funcCallback) {
		std::ifstream f(path, std::ios_base::binary);
		return LoadBitmapMatPixelArray(f, pelsPerMeter, palette, funcCallback);
	}
	sLoadBitmapMatPixelArrayResult LoadBitmapMatPixelArray(std::istream& is, callback_progress_t funcCallback) {
		sLoadBitmapMatPixelArrayResult result;
		result.img = LoadBitmapMatPixelArray(is, result.pelsPerMeter, result.palette, funcCallback);
		return result;
	}
	sLoadBitmapMatPixelArrayResult LoadBitmapMatPixelArray(std::filesystem::path const& path, callback_progress_t funcCallback) {
		std::ifstream is(path, std::ios::binary);
		return LoadBitmapMatPixelArray(is, funcCallback);
	}


	//! @brief Copy smaller Mat to larger Mat. (to dest xy)
	//! @param src 
	//! @param dest 
	//! @param ptDestTopLeft 
	//! @param mask 
	//! @return 
	bool CopyMatToXY(cv::Mat const& src, cv::Mat& dest, gtl::xPoint2i ptDestTopLeft, cv::Mat const* pMask) {
		if (src.type() != dest.type())
			return false;
		//if ((src.cols > dest.cols) or (src.rows > dest.rows))
		//	return false;
		cv::Rect rcROI(ptDestTopLeft.x, ptDestTopLeft.y, src.cols, src.rows);
		cv::Rect rcSafeROI = gtl::GetSafeROI(rcROI, dest.size());
		if (rcROI == rcSafeROI) {
			if (pMask and pMask->size() == src.size())
				src.copyTo(dest(rcSafeROI), *pMask);
			else
				src.copyTo(dest(rcSafeROI));
			return true;
		}
		else {
			if (rcSafeROI.empty())
				return false;
			cv::Rect rcSrc(0, 0, src.cols, src.rows);
			if (rcROI.x < 0) {
				rcSrc.width += rcROI.x;
				rcSrc.x -= rcROI.x;
				rcROI.width += rcROI.x;
				rcROI.x = 0;
			}
			if (rcROI.y < 0) {
				rcSrc.height += rcROI.y;
				rcSrc.y -= rcROI.y;
				rcROI.height += rcROI.y;
				rcROI.y = 0;
			}
			if (rcROI.x + rcROI.width > dest.cols) {
				rcSrc.width = rcROI.width = dest.cols - rcROI.x;
			}
			if (rcROI.y + rcROI.height > dest.rows) {
				rcSrc.height = rcROI.height = dest.rows - rcROI.y;
			}
			if (gtl::IsROI_Valid(rcSrc, src.size())) {
				if (pMask and pMask->size() == src.size())
					src(rcSrc).copyTo(dest(rcROI), (*pMask)(rcSrc));
				else
					src(rcSrc).copyTo(dest(rcROI));
			}
			return true;
		}
	}

	//! @brief Draw gridlines and pixel value of Mat to canvas.
	bool DrawPixelValue(cv::Mat& canvas, cv::Mat const& imgOriginal, cv::Rect roi, gtl::xCoordTrans2d const& ctCanvasFromImage, double const minTextHeight) {
		using xPoint2d = gtl::xPoint2d;

		// Draw Grid / pixel value
		if (ctCanvasFromImage.m_scale < 4)
			return false;
		cv::Scalar cr{127, 127, 127, 255};
		// grid - horizontal
		for (int y{roi.y}, y1{roi.y+roi.height}; y < y1; y++) {
			auto pt0 = ctCanvasFromImage(xPoint2d{roi.x, y});
			auto pt1 = ctCanvasFromImage(xPoint2d{roi.x+roi.width, y});
			cv::line(canvas, pt0, pt1, cr);
		}
		// grid - vertical
		for (int x{roi.x}, x1{roi.x+roi.width}; x < x1; x++) {
			auto pt0 = ctCanvasFromImage(xPoint2d{x, roi.y});
			auto pt1 = ctCanvasFromImage(xPoint2d{x, roi.y+roi.height});
			cv::line(canvas, pt0, pt1, cr);
		}

		// Pixel Value
		auto nChannel = imgOriginal.channels();
		auto depth = imgOriginal.depth();

		if ( ctCanvasFromImage.m_scale < ((nChannel+1.0)*minTextHeight) )
			return false;
		double heightFont = std::clamp(ctCanvasFromImage.m_scale/(nChannel+1.0), 1., 40.) / 40.;
		//auto t0 = stdc::steady_clock::now();
		for (int y{roi.y}, y1{roi.y+roi.height}; y < y1; y++) {
			auto* ptr = imgOriginal.ptr(y);
			int x1{roi.x+roi.width};
			//#pragma omp parallel for --------- little improvement
			for (int x{roi.x}; x < x1; x++) {
				auto pt = ctCanvasFromImage(xPoint2d{x, y});
				//auto p = SkPoint::Make(pt.x, pt.y);
				auto v = GetMatValue(ptr, depth, nChannel, y, x);
				auto avg = (v[0] + v[1] + v[2]) / nChannel;
				auto cr = (avg > 128) ? cv::Scalar{0, 0, 0, 255} : cv::Scalar{255, 255, 255, 255};
				for (int ch{}; ch < nChannel; ch++) {
					auto str = std::format("{:3}", v[ch]);
					cv::putText(canvas, str, cv::Point(pt.x, pt.y+(ch+1)*heightFont*40), cv::FONT_HERSHEY_DUPLEX, heightFont, cr, 1, true);
				}
			}
		}
		//auto t1 = stdc::steady_clock::now();
		//auto dur = stdc::duration_cast<stdc::milliseconds>(t1-t0).count();
		//OutputDebugString(std::format(L"{} ms\n", dur).c_str());

		return true;
	}

	std::optional<cv::Mat> ConvertFI2Mat(FIBITMAP* src, bool bRGBtoBGR) {
		int type{-1}, bpp{-1};
		auto eImageType = FreeImage_GetImageType(src);
		switch (eImageType) {
		case FIT_UINT16:	type = CV_16UC1; break;
		case FIT_INT16:		type = CV_16SC1; break;
			//case FIT_UINT32:	type = CV_32SC1; break;
		case FIT_INT32:		type = CV_32SC1; break;
		case FIT_FLOAT:		type = CV_32FC1; break;
		case FIT_DOUBLE:	type = CV_64FC1; break;
		case FIT_COMPLEX:	type = CV_64FC2; break;
		case FIT_RGB16:		type = CV_16UC3; break;
		case FIT_RGBA16:	type = CV_16UC4; break;
		case FIT_RGBF:		type = CV_32FC3; break;
		case FIT_RGBAF:		type = CV_32FC4; break;
		case FIT_BITMAP:
			bpp = FreeImage_GetBPP(src);
			switch (bpp) {
			case 1:
			case 2:	// probably there might not be 2bpp image
			case 4:
			case 8: type = CV_8UC1; break;	// To Be Determined
			case 16: type = CV_8UC2; break;	// To Be Determined
			case 24: type = CV_8UC3; break;
			case 32: type = CV_8UC4; break;
			}
			break;
		}
		if (type < 0)
			return {};

		//auto* info = FreeImage_GetInfo(src);
		//info->bmiHeader;

		bool flip{true};
		FIBITMAP* converted{};
		FIBITMAP* fib = src;
		if (eImageType == FIT_BITMAP) {	// Standard image type
			// first check if grayscale image
			if (bpp <= 16) {
				bool bColor{};
				bool bAlpha{};
				// get palette
				if (auto* palette = FreeImage_GetPalette(src)) {
					for (auto nPalette = FreeImage_GetColorsUsed(src), i{ 0u }; i < nPalette; i++) {
						if (auto c = palette[i]; c.rgbBlue != c.rgbGreen or c.rgbGreen != c.rgbRed) {	// todo: alpha channel?
							bColor = true;
							break;
						}
					}
					for (auto nPalette = FreeImage_GetColorsUsed(src), i{ 0u }; i < nPalette; i++) {
						if (auto c = palette[i]; c.rgbReserved) {	// todo: alpha channel?
							bAlpha = true;
							break;
						}
					}
				}
				if (bAlpha) {
					converted = FreeImage_ConvertTo32Bits(src);
					type = CV_8UC4;
				} else if (bColor or (bpp == 16)) {
					converted = FreeImage_ConvertTo24Bits(src);
					type = CV_8UC3;
				} else {
					converted = FreeImage_ConvertToGreyscale(src);
					type = CV_8UC1;
				}
				fib = converted;
			}
		}
		else {
			flip = true;	// don't know if image needs be flipped. so just flip it. :)
		}

		std::optional<gtl::xFinalAction> faConverted;
		if (converted) {
			faConverted.emplace([converted]{FreeImage_Unload(converted);});
		}

		if (!fib)
			return {};

		auto data = FreeImage_GetBits(fib);
		auto step = FreeImage_GetPitch(fib);
		auto size = cv::Size2i(FreeImage_GetWidth(fib), FreeImage_GetHeight(fib));
		if (!data or !step or size.empty())
			return {};

		cv::Mat img;
		{
			cv::Mat imgTemp = cv::Mat(size, type, data, step);	// !!! CAUTION no memory allocation.
			if (flip)
				cv::flip(imgTemp, img, 0);
			else
				imgTemp.copyTo(img);
		}

		int cvt = bRGBtoBGR ? [&]() -> int {
			switch (img.channels()) {
			case 3: return cv::COLOR_RGB2BGR;
			case 4: return cv::COLOR_RGBA2BGRA;
			}
			return -1;
		}() : -1;
		if (cvt > 0)
			cvtColor(img, img, cvt);

		return std::move(img);
	}


	//=============================================================================
	//

}	// namespace gtl


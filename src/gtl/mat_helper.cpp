#include "pch.h"

#include <chrono>

#include "gtl/mat_helper.h"

#include <windows.h>

namespace gtl {


	static bool s_bGPUChecked{};
	static bool s_bUseGPU{};

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
					cv::cvtColor(cv::cuda::GpuMat(imgSrc), dst, eCode);
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
					cv::resize(cv::cuda::GpuMat(imgSrc), dst, cv::Size(), dScale, dScale, eInterpolation);
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

	bool MatchTemplate(cv::Mat const& img, cv::Mat const& imgTempl, cv::Mat& matResult, int method) {
		try {
			//#ifdef HAVE_CUDA
			if (s_bUseGPU) {
				try {
					cv::cuda::GpuMat mat;
					cv::matchTemplate(cv::cuda::GpuMat(img), cv::cuda::GpuMat(imgTempl), mat, method);
					mat.download(matResult);
					return true;
				}
				catch (...) {
					//TRACE((GTL__FUNCSIG " - Error\n").c_str());
				}
			}
			//#endif
			matchTemplate(img, imgTempl, matResult, method);
		}
		catch (...) {
			return false;
		}
		return true;
	}

	bool MatchTemplate(cv::Mat const& img, cv::Mat const& imgTempl, int method, CPoint2d& ptBest, double& dMinMax, double& dRate, double dScale, int eInterpolation) {
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
						cv::resize(cv::cuda::GpuMat(imgG), imgG, cv::Size(), dScale, dScale, eInterpolation);
						cv::resize(cv::cuda::GpuMat(imgGTempl), imgGTempl, cv::Size(), dScale, dScale, eInterpolation);
					}
					cv::cuda::GpuMat matResult;
					cv::matchTemplate(imgG, imgGTempl, matResult, method);
					cv::minMaxLoc(matResult, &dMin, &dMax, &ptMin, &ptMax);
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

		template < bool bPixelIndex, typename telement = uint8, bool bMultiThreaded = true, bool bLoopUnrolling = true >
		bool MatToBitmapFile(std::ostream& f, cv::Mat const& img, int nBPP, std::vector<telement> const& pal) {

			int width32 = (img.cols * nBPP + 31) / 32 * 4;
			int pixel_per_byte = (8/nBPP);
			int nColPixel = img.cols/ pixel_per_byte * pixel_per_byte;

			using Func_PackSingleRow = std::function<void(int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal)>;
			Func_PackSingleRow PackSingleRow;

			if constexpr (bLoopUnrolling) {

				switch (nBPP) {
				case 1 :
					PackSingleRow = [img_cols = img.cols, nBPP, pixel_per_byte, nColPixel](int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal) {
						int x{};
						for (; x < nColPixel; x += pixel_per_byte) {
							int col = x / pixel_per_byte;
							if constexpr (bPixelIndex) {
								line[col] = (ptr[x + 0] << 7) | (ptr[x + 1] << 6) | (ptr[x + 2] << 5) | (ptr[x + 3] << 4) | (ptr[x + 4] << 3) | (ptr[x + 5] << 2) | (ptr[x + 6] << 1) | (ptr[x + 7]);
							}
							else {
								line[col] = (pal[ptr[x + 0]] << 7) | (pal[ptr[x + 1]] << 6) | (pal[ptr[x + 2]] << 5) | (pal[ptr[x + 3]] << 4) | (pal[ptr[x + 4]] << 3) | (pal[ptr[x + 5]] << 2) | (pal[ptr[x + 6]] << 1) | (pal[ptr[x + 7]]);
							}
						}
						int col = x / pixel_per_byte;
						for (int shift{ 8-nBPP }; x < img_cols; x++, shift--) {
							if constexpr (bPixelIndex) {
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
							if constexpr (bPixelIndex) {
								line[col] = (ptr[x + 0] << 4) | (ptr[x + 1] << 0);
							}
							else {
								line[col] = (pal[ptr[x + 0]] << 4) | (pal[ptr[x + 1]] << 0);
							}
						}
						int col = x / pixel_per_byte;
						for (int shift{ 8-nBPP }; x < img_cols; x++, shift-=nBPP) {
							if constexpr (bPixelIndex) {
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
							if constexpr (bPixelIndex) {
								line[x] = ptr[x];
							}
							else {
								line[x] = pal[ptr[x]];
							}
						}
					};
					break;
				case 24 :
					//follows
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
						for (int x{}; x < img_cols; x++) {
							int col = x / pixel_per_byte;
							int shift = 8 - ((x * nBPP) % 8);
							if constexpr (bPixelIndex) {
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
							if constexpr (bPixelIndex) {
								line[x] = ptr[x];
							}
							else {
								line[x] = pal[ptr[x]];
							}
						}
					};
					break;
				case 24 :
					//follows
					break;
				default :
					return false;
				}
			}

			if (nBPP == 24) {
				if (sizeof(telement) != 3)
					return false;
				PackSingleRow = [img_cols = img.cols, nBPP, pixel_per_byte](int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal) {
					telement* line3 = (telement*)line.data();
					for (int x{}; x < img_cols; x++) {
						if constexpr (bPixelIndex) {
							line3[x] = ptr[x];
						}
						else {
							line3[x] = pal[ptr[x]];
						}
					}
				};
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

				auto GetBuffer = [img_rows = img.rows, &mtxBuffer, &buffers, &yCur, &yWritten]() -> BUFFER* {
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
						yOld = buf.y;
					}
					buf.y = y;
					buf.y.notify_one();

					return &buf;
				};

				auto PackBuffer = [&yCur, &img, &buffers, &PackSingleRow, &pal, &yWritten/*, &id*/, &GetBuffer]() {
					do {
						auto* pBuffer = GetBuffer();
						if (!pBuffer)
							break;
						auto& buf = *pBuffer;

						PackSingleRow(buf.y, buf.line, img.ptr(buf.y), pal);

						buf.bReady = true;
						buf.bReady.notify_one();
					} while(true);
				};

				auto Writer = [&f, &buffers, rows = img.rows, &yWritten, &bWritten](std::stop_token stop_token) {
					while (yWritten < rows /*and !stop_token.stop_requested()*/) {
						auto index = yWritten % buffers.size();
						auto& buf = buffers[index];

						while (yWritten != buf.y) {
							buf.y.wait(-1);
						}
						while (!buf.bReady)
							buf.bReady.wait(false);

						if (!f.write((char*)buf.line.data(), buf.line.size()))
							return;

						buf.bReady = false;
						buf.y = -1;
						buf.y.notify_one();

						yWritten++;
						yWritten.notify_one();
					
					};
					f.flush();
					bWritten = (bool)f;
				};

				std::vector<std::jthread> threads;
				threads.reserve(nThread+1);
				threads.emplace_back(Writer);
				for (uint i{}; i < nThread; i++) {
					threads.emplace_back(PackBuffer);
				}

				threads.front().join();
				threads.clear();
				return bWritten;
			}
			else {
				std::vector<uint8> line((size_t)width32, 0);
				for (int y{}; y < img.rows; y++) {
					auto const* ptr = img.ptr<telement>(y);
					PackSingleRow(y, line, ptr, pal);
					f.write((char const*)line.data(), width32);
				}
				return true;
			}
		}

	}	// namespace internal


	bool SaveBitmapMat(std::filesystem::path const& path, cv::Mat const& img, int nBPP, std::span<gtl::color_bgra_t> palette, bool bPixelIndex) {
		// todo : CV_8UC3 with palette.

		if (img.empty())
			return false;

		auto type = img.type();

		int cx = img.cols;
		int cy = img.rows;
		if ((cx >= 0xffff) or (cy >= 0xffff))
			return false;
		if ((ptrdiff_t)cx * cy >= (ptrdiff_t)(0xffff'ffff - sizeof(BMP_FILE_HEADER) - sizeof(BITMAP_HEADER)))
			return false;
		int pixel_size = (type == CV_8UC3) ? 3 : ((type == CV_8UC1) ? 1 : 0);
		if (pixel_size <= 0)
			return false;

		BITMAP_HEADER header{};

		header.size = sizeof(header);
		header.width = cx;
		header.height = -cy;
		header.planes = 1;
		header.compression = 0;//BI_RGB;
		header.sizeImage = 0;//cx * cy * pixel_size;
		header.XPelsPerMeter = 0;
		header.YPelsPerMeter = 0;

		if (pixel_size == 3) {
			std::ofstream f(path, std::ios_base::binary);
			if (!f)
				return false;

			BMP_FILE_HEADER fh;

			auto width32 = gtl::AdjustAlign32(cx * 3);
			fh.offsetData = sizeof(fh) + sizeof(header);
			fh.sizeFile = fh.offsetData + width32 * cy;
			header.nBPP = 8 * pixel_size;
			header.nColorUsed = header.nColorImportant = 0;

			f.write((char const*)&fh, sizeof(fh));
			f.write((char const*)&header, sizeof(header));
			char redundant[4]{};
			size_t sr = width32 - (cx * 3);
			for (int y{}; y < img.rows; y++) {
				auto* ptr = img.ptr<cv::Vec3b>(y);
				f.write((char const*)ptr, 3 * img.cols);
				f.write(redundant, sr);
			}

			//constexpr bool bLoopUnrolling = true;
			//constexpr bool bMultiThreaded = true;
			//return MatToBitmapFile<true, cv::Vec3b, bMultiThreaded, bLoopUnrolling>(f, img, nBPP, {});

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
			fh.offsetData = sizeof(fh) + sizeof(header) + (0x01 << nBPP) * sizeof(gtl::color_bgra_t);
			fh.sizeFile = fh.offsetData + width32 * cy;
			header.nBPP = nBPP;
			header.nColorUsed = header.nColorImportant = (uint32_t)palette.size();

			f.write((char const*)&fh, sizeof(fh));
			f.write((char const*)&header, sizeof(header));

			f.write((char const*)palette.data(), palette.size() * sizeof(palette[0]));

			std::vector<uint8> pal((size_t)256, 0);
			if (!bPixelIndex) {
				for (size_t i{}; i < palette.size(); i++) {
					pal[palette[i].r] = (uint8)i;
				}
			}

			constexpr bool bLoopUnrolling = true;
			constexpr bool bMultiThreaded = true;
			if (bPixelIndex)
				return gtl::internal::MatToBitmapFile<true, uint8, bMultiThreaded, bLoopUnrolling>(f, img, nBPP, pal);
			else
				return gtl::internal::MatToBitmapFile<false, uint8, bMultiThreaded, bLoopUnrolling>(f, img, nBPP, pal);
		}

		return false;
	}

	namespace internal {

		template < bool bLoopUnrolling = true, bool bMultiThreaded = false >
		bool MatFromBitmapFile(std::istream& f, cv::Mat& img, int nBPP, std::vector<cv::Vec3b> palette) {
			if (img.type() != CV_8UC3)
				return false;

			int width32 = (img.cols * nBPP + 31) / 32 * 4;
			int pixel_per_byte = (8/nBPP);
			int nColPixel = img.cols/ pixel_per_byte * pixel_per_byte;

			using Func_UnPackSingleRow = std::function<void(int y, std::vector<uint8> const& line, cv::Vec3b* ptr, std::vector<cv::Vec3b> const& palette)>;
			Func_UnPackSingleRow UnPackSingleRow;

			if ((nBPP == 24) or (nBPP == 32)) {
				std::vector<uint8> line((size_t)width32, 0);
				int nByte = nBPP / 8;
				for (int y{}; y < img.rows; y++) {
					if (!f.read((char*)line.data(), line.size()))
						return false;
					auto* ptr = img.ptr<cv::Vec3b>(y);

					for (int x{}; x < img.cols; x++) {
						int xc = x * nByte;
						ptr[x][0] = line[xc + 0];
						ptr[x][1] = line[xc + 1];
						ptr[x][2] = line[xc + 2];
					}

				}
				return true;
			}

			using Func_UnpackLine = std::function<void()>;

			if constexpr (bLoopUnrolling) {
				if (nBPP == 1) {
					if (palette.size() < 2)
						return false;
					UnPackSingleRow = [img_cols = img.cols, &nBPP, &pixel_per_byte, &nColPixel](int y, std::vector<uint8> const& line, cv::Vec3b* ptr, std::vector<cv::Vec3b> const& palette) {
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
					UnPackSingleRow = [img_cols = img.cols, &nBPP, &pixel_per_byte, &nColPixel](int y, std::vector<uint8> const& line, cv::Vec3b* ptr, std::vector<cv::Vec3b> const& palette) {
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
					UnPackSingleRow = [img_cols = img.cols](int y, std::vector<uint8> const& line, cv::Vec3b* ptr, std::vector<cv::Vec3b> const& palette) {
						for (int x{}; x < img_cols; x++) {
							ptr[x] = palette[line[x]];
						}
					};
				}
				else
					return false;
			}
			else {
				if ((nBPP != 1) and (nBPP != 4) and (nBPP != 8))
					return false;
				if ( palette.size() < (0x01 << nBPP) )
					return false;

				if (nBPP == 8) {
					UnPackSingleRow = [img_cols = img.cols](int y, std::vector<uint8> const& line, cv::Vec3b* ptr, std::vector<cv::Vec3b> const& palette) {
						for (int x{}; x < img_cols; x++) {
							ptr[x] = palette[line[x]];
						}
					};
				} else {
					uint8 mask = (0x01 << nBPP) - 1;
					UnPackSingleRow = [img_cols = img.cols, &nBPP, &pixel_per_byte, mask](int y, std::vector<uint8> const& line, cv::Vec3b* ptr, std::vector<cv::Vec3b> const& palette) {
						for (int x{}; x < img_cols; x++) {
							int col = x / pixel_per_byte;
							int shift = 8 - ((x * nBPP) % 8);
							ptr[x] = palette[(line[col] >> shift) & mask];
						}
					};
				}
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

				auto Reader = [&f, &buffers, img_rows = img.rows, &q, &mtxQ, &cv, &bError]() {
					int y{};
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

						UnPackSingleRow(pBuf->y, pBuf->line, img.ptr<cv::Vec3b>(pBuf->y), palette);
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
				for (int y{}; y < img.rows; y++) {
					if (!f.read((char*)line.data(), line.size()))
						return false;
					auto* ptr = img.ptr<cv::Vec3b>(y);

					UnPackSingleRow(y, line, ptr, palette);
				}
			}

			return true;
		}

	}	// namespace internal

	/// @brief Save Image to BITMAP. Image is COLOR or GRAY level image.
	/// @param path 
	/// @param img : CV_8UC1 : gray scale, CV_8UC3 : color (no palette supported), for CV8UC3, palette is not used.
	/// @param nBPP 
	/// @param palette 
	/// @param bPixelIndex if true, img value is NOT a pixel but a palette index. a full palette must be given.
	/// @return 
	cv::Mat LoadBitmapMat(std::filesystem::path const& path) {
		cv::Mat img;

		std::ifstream f(path, std::ios_base::binary);
		if (!f)
			return img;

		BMP_FILE_HEADER fh;
		BITMAP_V5_HEADER header{};

		if (!f.read((char*)&fh, sizeof(fh)))
			return img;
		uint32_t sizeHeader{};
		if (!f.read((char*)&sizeHeader, sizeof(sizeHeader)))
			return img;
		switch (sizeHeader) {
		case sizeof(BITMAP_HEADER) :
		case sizeof(BITMAP_V4_HEADER) :
		case sizeof(BITMAP_V5_HEADER) :
			header.size = sizeHeader;
			f.read(((char*)&header) + sizeof(sizeHeader), sizeHeader - sizeof(sizeHeader));
			break;
		default:
			return img;
		}

		if (header.compression or (header.planes != 1))
			return img;

		int cx = header.width;
		int cy = header.height;
		bool bFlipY{};
		if (cy < 0) {
			cy = -cy;
		}
		else {
			bFlipY = true;
		}
		if ((cx <= 0) or (cy <= 0) or (cx * (uint64_t)cy >= 0xffff'ff00ull))
			return img;

		std::vector<cv::Vec3b> palette;

		// Load Palette
		ptrdiff_t sizePalette = fh.offsetData - sizeof(fh) - header.size;
		for (; sizePalette >= 4; sizePalette -= sizeof(gtl::color_bgra_t)) {
			gtl::color_bgra_t color{};
			if (!f.read((char*)&color, sizeof(color)))
				return img;
			palette.push_back(cv::Vec3b(color.b, color.g, color.r));
		}
		if (!f.seekg(fh.offsetData))
			return img;

		img = cv::Mat::zeros(cv::Size(cx, cy), CV_8UC3);
		constexpr bool bLoopUnrolling = true;
		constexpr bool bMultiThreaded = true;
		if (!gtl::internal::MatFromBitmapFile<bLoopUnrolling, bMultiThreaded>(f, img, header.nBPP, palette)) {
			img.release();
		}
		if (bFlipY) {
			cv::flip(img, img, 0);
		}

		return img;
	}

	//=============================================================================
	//

}	// namespace gtl


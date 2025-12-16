#include "pch.h"
#include "gtl/_config.h"
#include "gtl/archive.h"
#include <archive.h>
#include <archive_entry.h>

//////////////////////////////////////////////////////////////////////
//
// archive.cpp
//
// PWH
// 2025-12-16 using GPT
//
//////////////////////////////////////////////////////////////////////

namespace gtl {

	namespace fs = std::filesystem;

	static void throw_archive(struct archive* a, const char* msg) {
		std::string e = msg;
		if (a && archive_error_string(a)) {
			e += ": ";
			e += archive_error_string(a);
		}
		throw std::runtime_error(e);
	}

	void add_file_to_zip(struct archive* a, const fs::path& filePath, const fs::path& baseDir) {
		// ZIP 내부 경로 (baseDir 기준 상대경로)
		fs::path relative = fs::relative(filePath, baseDir);

		fs::directory_entry de(filePath);
		fs::file_time_type tLastWriteTime = de.last_write_time();
		std::chrono::system_clock::time_point sctp = std::chrono::clock_cast<std::chrono::system_clock>(tLastWriteTime);

		struct archive_entry* entry = archive_entry_new();
		archive_entry_set_pathname_utf8(entry, (char const*)relative.generic_u8string().c_str());
		archive_entry_set_filetype(entry, AE_IFREG);
		archive_entry_set_perm(entry, 0644);
		archive_entry_set_size(entry, fs::file_size(filePath));
		archive_entry_set_mtime(entry, std::chrono::system_clock::to_time_t(sctp), 0);

		if (archive_write_header(a, entry) != ARCHIVE_OK) {
			archive_entry_free(entry);
			throw_archive(a, "archive_write_header failed");
		}

		std::ifstream ifs(filePath, std::ios::binary);
		if (!ifs)
			throw std::runtime_error("failed to open file: " + filePath.string());

		std::vector<char> buffer(8192);
		while (ifs) {
			ifs.read(buffer.data(), buffer.size());
			std::streamsize n = ifs.gcount();
			if (n > 0) {
				if (archive_write_data(a, buffer.data(), n) < 0) {
					archive_entry_free(entry);
					throw_archive(a, "archive_write_data failed");
				}
			}
		}

		archive_entry_free(entry);
	}

	void add_directory_to_zip(struct archive* a, const fs::path& dirPath, const fs::path& baseDir) {
		fs::path relative = fs::relative(dirPath, baseDir);

		fs::directory_entry de(dirPath);
		fs::file_time_type tLastWriteTime = de.last_write_time();
		std::chrono::system_clock::time_point sctp = std::chrono::clock_cast<std::chrono::system_clock>(tLastWriteTime);

		struct archive_entry* entry = archive_entry_new();
		archive_entry_set_pathname_utf8(entry, (char const*)(relative.generic_u8string() + u8"/").c_str());
		archive_entry_set_filetype(entry, AE_IFDIR);
		archive_entry_set_perm(entry, 0755);
		archive_entry_set_mtime(entry, std::chrono::system_clock::to_time_t(sctp), 0);

		archive_write_header(a, entry);
		archive_entry_free(entry);
	}

	std::expected<bool, std::string> ZipFolder(std::filesystem::path const& pathZip, std::filesystem::path const& folderSource) try {
		std::locale locOld;
		std::setlocale(LC_ALL, "en_us.utf8");
		gtl::xFinalAction onExit([&]() {
			std::locale::global(locOld);
		});

		if (!fs::is_directory(folderSource))
			throw std::runtime_error("folderSource is not a directory");

		struct archive* a = archive_write_new();
		if (!a) throw std::runtime_error("archive_write_new failed");

		archive_write_set_format_filter_by_ext(a, pathZip.extension().string().c_str());
		archive_write_set_options(a, "compression=deflate");

		if (archive_write_open_filename_w(a, pathZip.wstring().c_str()) != ARCHIVE_OK) {
			archive_write_free(a);
			throw_archive(a, "archive_write_open_filename failed");
		}

		for (auto& entry : fs::recursive_directory_iterator(folderSource)) {
			auto const& path = entry.path();
			if (fs::is_directory(entry)) {
				//auto const fname = path.filename();
				//if (path == folderSource or fname == "." or fname == "..")
				//	continue;
				add_directory_to_zip(a, path, folderSource);
			}
			else if (fs::is_regular_file(entry)) {
				add_file_to_zip(a, path, folderSource);
			}
		}

		archive_write_close(a);
		archive_write_free(a);

		return true;
	}
	catch (std::exception& e) {
		return std::unexpected{std::format("ZipFolder exception: {}", e.what())};
	}

	std::expected<bool, std::string> UnzipFolder(std::filesystem::path const& pathZip, std::filesystem::path const& folder) try {
		std::locale locOld;
		std::setlocale(LC_ALL, "en_us.utf8");

		struct archive* a = archive_read_new();
		struct archive* ext = archive_write_disk_new();
		struct archive_entry* entry{};

		gtl::xFinalAction onExit([&]() {
			std::locale::global(locOld);
			if (a) {
				archive_read_close(a);
				archive_read_free(a);
			}
			if (ext) {
				archive_write_close(ext);
				archive_write_free(ext);
			}
		});

		if (!a || !ext)
			throw std::runtime_error("archive allocation failed");

		// ZIP + 기타 포맷 자동 인식
		archive_read_support_format_all(a);
		archive_read_support_filter_all(a);

		// 디스크 쓰기 옵션
		archive_write_disk_set_options(
			ext,
			ARCHIVE_EXTRACT_TIME     | // 파일 시간
			ARCHIVE_EXTRACT_PERM     | // 권한
			ARCHIVE_EXTRACT_ACL      |
			ARCHIVE_EXTRACT_FFLAGS
		);

		archive_write_disk_set_standard_lookup(ext);

		if (archive_read_open_filename_w(a, pathZip.generic_wstring().c_str(), 10240) != ARCHIVE_OK)
			throw_archive(a, "archive_read_open_filename failed");

		while (true) {
			int r = archive_read_next_header(a, &entry);
			if (r == ARCHIVE_EOF)
				break;
			if (r < ARCHIVE_OK)
				throw_archive(a, "archive_read_next_header failed");

			// 출력 경로 변경 (ZIP 내부 상대경로 유지)
			const char* currentPath = archive_entry_pathname_utf8(entry);
			fs::path fullPath = folder / currentPath;
			archive_entry_set_pathname(entry, (char const*)fullPath.generic_u8string().c_str());

			r = archive_write_header(ext, entry);
			if (r < ARCHIVE_OK)
				throw_archive(ext, "archive_write_header failed");

			if (archive_entry_size(entry) > 0) {
				const void* buff;
				size_t size;
				la_int64_t offset;

				while (true) {
					r = archive_read_data_block(
						a, &buff, &size, &offset);
					if (r == ARCHIVE_EOF)
						break;
					if (r < ARCHIVE_OK)
						throw_archive(a, "archive_read_data_block failed");

					r = archive_write_data_block(
						ext, buff, size, offset);
					if (r < ARCHIVE_OK)
						throw_archive(ext, "archive_write_data_block failed");
				}
			}
		}

		return true;
	}
	catch (std::exception& e) {
		return std::unexpected{std::format("ZipFolder exception: {}", e.what())};
	}

} // namespace gtl

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

	std::expected<bool, std::string> ZipFolder(std::filesystem::path const& pathZip, std::filesystem::path const& folderSource, std::string const& strFormat) try {
		std::locale locOld;
		std::setlocale(LC_ALL, "en_us.utf8");
		gtl::xFinalAction onExit([&]() {
			std::locale::global(locOld);
		});

		if (!fs::is_directory(folderSource))
			throw std::runtime_error("folderSource is not a directory");

		struct archive* a = archive_write_new();
		if (!a) throw std::runtime_error("archive_write_new failed");

		archive_write_set_format_filter_by_ext(a, strFormat.empty() ? pathZip.extension().string().c_str() : strFormat.c_str());
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

	std::expected<bool, std::string> UnzipFolder(std::filesystem::path const& pathZip, std::filesystem::path const& folder, std::string const& strFormat) try {
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

	//-----------------------------------------------------------------------------
	// Archive reading

	namespace {

		/// @brief libarchive converts entry names into LC_CTYPE charset.
		///  UTF-8 locale (per thread, not to touch global locale) : any name can be converted. (source charset : see OpenArchiveForRead())
		class xUtf8CTypeScope {
			int m_eThreadLocaleOld{};
			std::string m_strLocaleOld;
		public:
			xUtf8CTypeScope() {
				m_eThreadLocaleOld = _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
				if (auto* p = std::setlocale(LC_CTYPE, nullptr))
					m_strLocaleOld = p;
				std::setlocale(LC_CTYPE, ".UTF-8");
			}
			~xUtf8CTypeScope() {
				std::setlocale(LC_CTYPE, m_strLocaleOld.empty() ? "C" : m_strLocaleOld.c_str());
				_configthreadlocale(m_eThreadLocaleOld);
			}
		};

		struct sArchiveReadDeleter {
			void operator () (struct archive* a) const { archive_read_free(a); }
		};
		using archive_read_ptr = std::unique_ptr<struct archive, sArchiveReadDeleter>;

		std::string ArchiveError(struct archive* a, std::string_view msg) {
			if (a) {
				if (auto const* err = archive_error_string(a))
					return std::format("{}: {}", msg, err);
			}
			return std::string(msg);
		}

		/// @brief OEM codepage of user locale. (ex, "CP949")
		///  GetOEMCP() can be 65001 when "Beta: Use Unicode UTF-8 for worldwide language support" is on.
		std::string GetLegacyCharset() {
		#if (GTL__USE_WINDOWS_API)
			wchar_t buf[16]{};
			if (GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_IDEFAULTCODEPAGE, buf, (int)std::size(buf)) > 0) {
				if (int cp = _wtoi(buf); cp > 0 and cp != CP_UTF8)
					return std::format("CP{}", cp);
			}
		#endif
			return {};
		}

		/// @param bLegacyNames : zip names without utf-8 flag are in OEM codepage (ex, CP949). otherwise, UTF-8.
		std::expected<archive_read_ptr, std::string> OpenArchiveForRead(fs::path const& pathArchive, bool bLegacyNames) {
			archive_read_ptr a(archive_read_new());
			if (!a)
				return std::unexpected{std::string("archive_read_new failed")};
			archive_read_support_format_all(a.get());
			archive_read_support_filter_all(a.get());
			if (!bLegacyNames)
				archive_read_set_options(a.get(), "zip:hdrcharset=UTF-8");
			else if (static auto const charset = GetLegacyCharset(); !charset.empty())
				archive_read_set_options(a.get(), std::format("zip:hdrcharset={}", charset).c_str());
			if (archive_read_open_filename_w(a.get(), pathArchive.c_str(), 64*1024) != ARCHIVE_OK)
				return std::unexpected{ArchiveError(a.get(), "archive_read_open_filename failed")};
			return a;
		}

		/// @brief iterates entries. tries UTF-8 names first, then legacy (OEM codepage) names if any name is not valid UTF-8.
		/// @param fnReset : called on (re)start
		/// @param fn (archive*, archive_entry*) -> true to stop
		/// @return true if stopped by fn
		template < typename TReset, typename TFunc >
		std::expected<bool, std::string> ForEachEntry(fs::path const& pathArchive, TReset&& fnReset, TFunc&& fn) {
			for (bool bLegacyNames : {false, true}) {
				auto a = OpenArchiveForRead(pathArchive, bLegacyNames);
				if (!a)
					return std::unexpected{std::move(a.error())};
				fnReset();
				bool bRetry{};
				struct archive_entry* entry{};
				while (true) {
					int r = archive_read_next_header(a->get(), &entry);
					if (r == ARCHIVE_EOF)
						break;
					if (r == ARCHIVE_WARN and !bLegacyNames) {	// "Pathname cannot be converted from UTF-8 to current locale."
						bRetry = true;
						break;
					}
					if (r < ARCHIVE_WARN)
						return std::unexpected{ArchiveError(a->get(), "archive_read_next_header failed")};
					if (fn(a->get(), entry))
						return true;
				}
				if (!bRetry)
					break;
			}
			return false;
		}

		/// @brief entry name -> generic path without leading "./", "/" and trailing "/"
		fs::path GetEntryPath(struct archive_entry* entry) {
			std::wstring str;
			if (auto const* w = archive_entry_pathname_w(entry))
				str = w;
			else if (auto const* u8 = archive_entry_pathname_utf8(entry))
				str = fs::path((char8_t const*)u8).wstring();
			else if (auto const* s = archive_entry_pathname(entry))
				str = fs::path(s).wstring();
			std::ranges::replace(str, L'\\', L'/');
			std::wstring_view sv = str;
			while (sv.starts_with(L"./"))
				sv.remove_prefix(2);
			while (sv.starts_with(L'/'))
				sv.remove_prefix(1);
			while (sv.ends_with(L'/'))
				sv.remove_suffix(1);
			return fs::path(sv);
		}

	}	// anonymous namespace

	bool IsArchiveFile(std::filesystem::path const& path) {
		auto ext = path.extension().wstring();
		return (gtl::tszicmp<wchar_t>(ext, std::wstring_view(L".zip")) == 0) or (gtl::tszicmp<wchar_t>(ext, std::wstring_view(L".7z")) == 0);
	}

	std::optional<std::pair<std::filesystem::path, std::filesystem::path>> SplitArchivePath(std::filesystem::path const& path) {
		std::error_code ec;
		fs::path pathArchive;
		auto iter = path.begin();
		for (; iter != path.end(); iter++) {
			pathArchive /= *iter;
			if (!IsArchiveFile(*iter))
				continue;
			if (fs::is_regular_file(pathArchive, ec)) {
				iter++;
				break;
			}
		}
		if (pathArchive.empty() or !IsArchiveFile(pathArchive) or !fs::is_regular_file(pathArchive, ec))
			return std::nullopt;
		fs::path inner;
		for (; iter != path.end(); iter++) {
			if (!iter->empty())
				inner /= *iter;
		}
		return std::pair{std::move(pathArchive), fs::path(inner.generic_wstring())};
	}

	std::expected<std::vector<sArchiveEntry>, std::string> ListArchive(std::filesystem::path const& pathArchive) try {
		xUtf8CTypeScope localeScope;

		std::vector<sArchiveEntry> entries;
		auto r = ForEachEntry(pathArchive, [&] { entries.clear(); }, [&](struct archive*, struct archive_entry* entry) {
			sArchiveEntry e;
			e.path = GetEntryPath(entry);
			if (e.path.empty())
				return false;
			e.bDir = archive_entry_filetype(entry) == AE_IFDIR;
			e.size = archive_entry_size_is_set(entry) ? (uint64_t)archive_entry_size(entry) : 0;
			if (archive_entry_mtime_is_set(entry)) {
				e.tLastWrite = std::chrono::system_clock::from_time_t(archive_entry_mtime(entry))
					+ std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::nanoseconds(archive_entry_mtime_nsec(entry)));
			}
			entries.push_back(std::move(e));
			return false;	// no archive_read_data_skip() needed. next_header skips (lazily for 7z)
		});
		if (!r)
			return std::unexpected{std::move(r.error())};
		return entries;
	}
	catch (std::exception& e) {
		return std::unexpected{std::format("ListArchive exception: {}", e.what())};
	}

	std::expected<std::vector<uint8_t>, std::string> ReadArchiveEntry(std::filesystem::path const& pathArchive, std::filesystem::path const& pathEntry,
		std::function<bool(uint64_t read, uint64_t total)> const& fnProgress) try
	{
		xUtf8CTypeScope localeScope;

		auto const target = pathEntry.generic_wstring();
		std::vector<uint8_t> buffer;
		std::string error;
		auto r = ForEachEntry(pathArchive, [] {}, [&](struct archive* a, struct archive_entry* entry) {
			if (archive_entry_filetype(entry) != AE_IFREG)
				return false;
			if (GetEntryPath(entry).generic_wstring() != target)
				return false;

			if (archive_entry_is_encrypted(entry)) {
				error = "encrypted entry is not supported";
				return true;
			}
			uint64_t const total = archive_entry_size_is_set(entry) ? (uint64_t)archive_entry_size(entry) : 0;
			buffer.reserve(total);
			std::vector<uint8_t> block(1024*1024);
			while (true) {
				auto n = archive_read_data(a, block.data(), block.size());
				if (n == 0)
					break;
				if (n < 0) {
					error = ArchiveError(a, "archive_read_data failed");
					break;
				}
				buffer.insert(buffer.end(), block.begin(), block.begin() + (size_t)n);
				if (fnProgress and !fnProgress(buffer.size(), total)) {
					error = "canceled";
					break;
				}
			}
			return true;
		});
		if (!r)
			return std::unexpected{std::move(r.error())};
		if (!*r)
			return std::unexpected{std::format("entry not found: {}", gtl::WtoU8A(target))};
		if (!error.empty())
			return std::unexpected{std::move(error)};
		return buffer;
	}
	catch (std::exception& e) {
		return std::unexpected{std::format("ReadArchiveEntry exception: {}", e.what())};
	}

} // namespace gtl

#include "container.h"
#include "reader.h"
#include "bit_stream.h"
#include <algorithm>
#include <bit>
#include <set>

namespace gtl::dwg::detail {
	namespace {
		using bytes_t = std::span<std::uint8_t const>;
		using buffer_t = std::vector<std::uint8_t>;
		bytes_t Slice(bytes_t b, std::uint64_t at, std::uint64_t count) {
			if (at > b.size() || count > b.size() - at)
				throw xParseError("container range outside input: offset " + std::to_string(at) + ", length " +
								  std::to_string(count) + ", buffer " + std::to_string(b.size()));
			return b.subspan(static_cast<size_t>(at), static_cast<size_t>(count));
		}
		std::uint64_t LE(bytes_t b, size_t at, unsigned count = 4) {
			auto s = Slice(b, at, count);
			std::uint64_t v{};
			for (unsigned i = 0; i < count; ++i)
				v |= std::uint64_t{s[i]} << (i * 8);
			return v;
		}
		struct Cursor {
			bytes_t data;
			size_t at{};
			unsigned Byte() {
				if (at == data.size())
					throw xParseError("truncated compressed stream");
				return data[at++];
			}
			std::uint64_t Read(unsigned n = 4) {
				auto v = LE(data, at, n);
				at += n;
				return v;
			}
			bytes_t Take(size_t n) {
				auto b = Slice(data, at, n);
				at += n;
				return b;
			}
		};
		void Copy(buffer_t &out, size_t distance, size_t count, size_t limit) {
			if (!distance || distance > out.size() || count > limit - out.size())
				throw xParseError("invalid compression back reference");
			for (size_t i = 0; i < count; ++i)
				out.push_back(out[out.size() - distance]);
		}
		std::uint32_t Checksum(bytes_t b, std::uint32_t seed = 0) {
			std::uint32_t a = seed & 65535, beta = seed >> 16;
			for (size_t pos = 0; pos < b.size();) {
				auto end = std::min(b.size(), pos + 0x15b0);
				for (; pos < end; ++pos) {
					a += b[pos];
					beta += a;
				}
				a %= 65521;
				beta %= 65521;
			}
			return a | (beta << 16);
		}
		std::uint32_t CRC32(bytes_t b) {
			std::uint32_t crc = ~0u;
			for (auto c : b) {
				crc ^= c;
				for (int i = 0; i < 8; ++i)
					crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320u : 0);
			}
			return ~crc;
		}
		struct Budget {
			sContainerOptions const &options;
			size_t used{}, pages{};
			size_t Bytes(std::uint64_t n) {
				if (n > options.maxDecodedBytes - used)
					throw std::length_error("DWG decoded byte limit exceeded");
				used += static_cast<size_t>(n);
				return static_cast<size_t>(n);
			}
			void Pages(std::uint64_t n) {
				if (n > options.maxPages - pages)
					throw std::length_error("DWG page limit exceeded");
				pages += static_cast<size_t>(n);
			}
		};
		struct Page {
			std::uint64_t offset{}, size{};
		};
		buffer_t ReedSolomon(bytes_t input, size_t blocks, unsigned k, bool interleaved, size_t storedData = 0) {
			if (!storedData)
				storedData = blocks * k;
			Slice(input, 0, interleaved ? std::uint64_t{blocks} * 255 : storedData + blocks * (255 - k));
			buffer_t result(blocks * k);
			std::array<unsigned, 510> exp{};
			std::array<unsigned, 256> log{};
			unsigned value = 1, polynomial = k == 239 ? 0x169 : 0x11d;
			for (unsigned i = 0; i < 255; ++i) {
				exp[i] = value;
				log[value] = i;
				value <<= 1;
				if (value & 256)
					value ^= polynomial;
			}
			for (unsigned i = 255; i < 510; ++i)
				exp[i] = exp[i - 255];
			for (size_t block = 0; block < blocks; ++block) {
				std::array<unsigned, 255> code{};
				bool complete = true;
				for (unsigned j = 0; j < 255; ++j) {
					size_t at;
					if (interleaved)
						at = j * blocks + block;
					else if (j >= k)
						at = storedData + block * (255 - k) + j - k;
					else {
						at = block * k + j;
						if (at >= storedData)
							at += blocks * (255 - k);
					}
					if (at < input.size())
						code[j] = input[at];
					else
						complete = false;
				}
				// Non-interleaved pages may omit the end of random RS padding. Their
				// payload is still verified by both the page CRC64 and data checksum.
				if (complete)
					for (unsigned root = 1; root <= 255 - k; ++root) {
						unsigned syndrome{};
						for (size_t j = 255; j-- > 0;)
							syndrome = (syndrome ? exp[log[syndrome] + root] : 0) ^ code[j];
						if (syndrome)
							throw xParseError("Reed-Solomon parity mismatch (repair is not supported)");
					}
				for (unsigned j = 0; j < k; ++j)
					result[block * k + j] = static_cast<std::uint8_t>(code[j]);
			}
			return result;
		}
		std::uint64_t Seed(std::uint64_t seed, size_t size, bool second = false) {
			seed = (seed + size) * 0x343fd + 0x269ec3;
			if (second)
				seed = seed * (0x100000000ULL + 0x343fd) + (size + 0x269ec3);
			else
				seed |= (seed * 0x343fd + 0x269ec3) << 32;
			return ~seed;
		}
		std::uint64_t DecodeSeed(std::uint64_t encoded) {
			std::uint64_t seed{};
			for (unsigned i = 0; i < 10; ++i)
				seed |= ((encoded >> (5 + 6 * i)) & 1) << (9 - i);
			return seed;
		}
		std::uint64_t CRC64(bytes_t data, std::uint64_t seed, bool mirrored) {
			auto crc = seed;
			auto byte = [&](unsigned b) {
				if (mirrored) {
					crc ^= b;
					for (int i = 0; i < 8; ++i)
						crc = (crc >> 1) ^ ((crc & 1) ? 0x95ac9329ac4bc9b5ULL : 0);
				} else {
					crc ^= std::uint64_t{b} << 56;
					for (int i = 0; i < 8; ++i)
						crc = (crc << 1) ^ ((crc >> 63) ? 0x42f0e1eba9ea3693ULL : 0);
				}
			};
			size_t at{};
			while (data.size() - at >= 8) {
				for (unsigned j : {6u, 7u, 4u, 5u, 2u, 3u, 0u, 1u})
					byte(data[at + j]);
				at += 8;
			}
			auto left = data.size() - at;
			if (left >= 4) {
				byte(data[at + 2]);
				byte(data[at + 3]);
				byte(data[at]);
				byte(data[at + 1]);
				at += 4;
			}
			for (; at < data.size(); ++at)
				byte(data[at]);
			return mirrored ? crc : ~crc;
		}
		std::uint32_t Checksum2007(bytes_t data, std::uint64_t seed) {
			seed = (seed + data.size()) * 0x343fd + 0x269ec3;
			std::uint32_t a = seed & 65535, b = (seed >> 16) & 65535;
			auto byte = [&](unsigned v) {
				a += v;
				b += a;
			};
			for (size_t at = 0; at < data.size();) {
				auto end = std::min(data.size(), at + 0x15b0);
				while (end - at >= 8) {
					for (unsigned j : {6u, 7u, 4u, 5u, 2u, 3u, 0u, 1u})
						byte(data[at + j]);
					at += 8;
				}
				if (end - at >= 4) {
					byte(data[at + 2]);
					byte(data[at + 3]);
					byte(data[at]);
					byte(data[at + 1]);
					at += 4;
				}
				while (at < end)
					byte(data[at++]);
				a %= 65521;
				b %= 65521;
			}
			return a | (b << 16);
		}
		buffer_t System2007(bytes_t file, std::uint64_t offset, std::uint64_t packed, std::uint64_t unpacked,
							std::uint64_t factor, std::uint64_t seed, std::uint64_t packedCRC,
							std::uint64_t unpackedCRC, Budget &budget) {
			seed = DecodeSeed(seed);
			if (!packed || packed > unpacked || !factor || packed > budget.options.maxDecodedBytes ||
				factor > budget.options.maxDecodedBytes / ((packed + 7) & ~7ULL))
				throw xParseError("invalid R2007 system page size");
			auto repeated = ((packed + 7) & ~7ULL) * factor;
			auto blocks = (repeated + 238) / 239;
			budget.Bytes(blocks * 239);
			auto rs = ReedSolomon(Slice(file, offset, blocks * 255), static_cast<size_t>(blocks), 239, true);
			auto data = Slice(rs, 0, packed);
			if (CRC64(data, Seed(seed, data.size()), true) != packedCRC)
				throw xParseError("R2007 system compressed CRC mismatch");
			budget.Bytes(unpacked);
			auto result = packed == unpacked ? buffer_t(data.begin(), data.end())
											 : Decompress2007(data, static_cast<size_t>(unpacked));
			if (CRC64(result, Seed(seed, result.size()), true) != unpackedCRC)
				throw xParseError("R2007 system uncompressed CRC mismatch");
			return result;
		}
		sContainer Container2007(bytes_t file, Budget &budget) {
			auto rs = ReedSolomon(Slice(file, 0x80, 765), 3, 239, true);
			auto length = std::bit_cast<std::int64_t>(LE(rs, 24, 8));
			if (length == INT64_MIN)
				throw xParseError("invalid R2007 header length");
			auto packed = Slice(rs, 32, static_cast<std::uint64_t>(length < 0 ? -length : length));
			auto header = length < 0 ? buffer_t(packed.begin(), packed.end()) : Decompress2007(packed, 0x110);
			if (header.size() != 0x110)
				throw xParseError("invalid R2007 header size");
			auto crc = LE(header, 0x108, 8);
			std::fill(header.begin() + 0x108, header.end(), 0);
			if (CRC64(header, Seed(0, header.size(), true), false) != crc)
				throw xParseError("R2007 file header CRC mismatch");
			auto h = [&](size_t at) { return LE(header, at, 8); };
			if (h(0x38) > file.size())
				throw xParseError("invalid R2007 page map address");
			auto map = System2007(file, h(0x38) + 0x480, h(0x50), h(0x58), h(0x18), h(0x20), h(0x10), h(0x80), budget);
			Cursor cursor{map};
			std::map<std::uint64_t, Page> pages;
			std::uint64_t address = 0x480;
			while (cursor.at < map.size()) {
				auto size = cursor.Read(8);
				auto signedId = std::bit_cast<std::int64_t>(cursor.Read(8));
				budget.Pages(1);
				if (signedId == INT64_MIN || !signedId)
					throw xParseError("invalid R2007 page id");
				auto id = static_cast<std::uint64_t>(signedId < 0 ? -signedId : signedId);
				Slice(file, address, size);
				if (!pages.emplace(id, Page{address, size}).second)
					throw xParseError("duplicate R2007 page id");
				address += size;
			}
			auto found = pages.find(h(0xc0));
			if (found == pages.end())
				throw xParseError("missing R2007 section map");
			auto descriptions =
				System2007(file, found->second.offset, h(0xb0), h(0xc8), h(0xd8), h(0xe0), h(0xd0), h(0xa8), budget);
			Cursor sections{descriptions};
			sContainer result;
			while (sections.at < descriptions.size()) {
				auto size = sections.Read(8), maxPage = sections.Read(8), encryption = sections.Read(8),
					 id = sections.Read(8), nameLength = sections.Read(8);
				sections.Read(8);
				auto encoding = sections.Read(8), n = sections.Read(8);
				if (nameLength > 1024)
					throw xParseError("invalid R2007 section name");
				std::string name;
				if (nameLength % 2)
					throw xParseError("odd UTF-16 section name length");
				for (std::uint64_t i = 0; i < nameLength / 2; ++i) {
					auto c = sections.Read(2);
					if (c > 127)
						throw xParseError("non-ASCII DWG section name");
					if (c)
						name += static_cast<char>(c);
				}
				try {
					if (encryption == 1)
						throw xParseError("password encrypted section is not supported");
					if (encoding != 1 && encoding != 4)
						throw xParseError("unsupported R2007 page encoding");
					if (n > (descriptions.size() - sections.at) / 56 || maxPage > budget.options.maxDecodedBytes)
						throw xParseError("invalid R2007 page count or size");
					sContainerSection section{id, buffer_t(budget.Bytes(size)), static_cast<size_t>(n)};
					std::uint64_t previousEnd{};
					for (std::uint64_t j = 0; j < n; ++j) {
						auto start = sections.Read(8), pageSize = sections.Read(8), pageId = sections.Read(8),
							 unpacked = sections.Read(8), compressed = sections.Read(8), checksum = sections.Read(8),
							 crc = sections.Read(8);
						auto page = pages.find(pageId);
						if (page == pages.end())
							throw xParseError("missing R2007 data page");
						if (!compressed || compressed > unpacked || unpacked > pageSize || start < previousEnd ||
							start > size || unpacked > size - start)
							throw xParseError("invalid R2007 data page bounds");
						auto blocks = (((compressed + 7) & ~7ULL) + 250) / 251;
						if (encoding == 4 && blocks * 255 > page->second.size)
							throw xParseError("short R2007 encoded page");
						auto decoded =
							ReedSolomon(Slice(file, page->second.offset, std::min(blocks * 255, page->second.size)),
										static_cast<size_t>(blocks), 251, encoding == 4,
										static_cast<size_t>((compressed + 7) & ~7ULL));
						auto data = Slice(decoded, 0, compressed);
						if (CRC64(data, Seed(h(0xf0), data.size()), true) != crc)
							throw xParseError("R2007 data page CRC mismatch");
						auto raw = compressed == unpacked ? buffer_t(data.begin(), data.end())
														  : Decompress2007(data, static_cast<size_t>(unpacked));
						if (Checksum2007(raw, h(0xf0)) != checksum)
							throw xParseError("R2007 data page checksum mismatch");
						std::copy(raw.begin(), raw.end(), section.data.begin() + static_cast<size_t>(start));
						previousEnd = start + unpacked;
					}
					if (!result.sections.emplace(name, std::move(section)).second)
						throw xParseError("duplicate R2007 section name");
				} catch (xParseError const &e) {
					throw xParseError(name + ": " + e.what());
				}
			}
			return result;
		}
		buffer_t System2004(bytes_t file, std::uint64_t offset, std::uint32_t type, Budget &budget) {
			auto header = Slice(file, offset, 20);
			auto size = LE(header, 4), packed = LE(header, 8);
			if (LE(header, 0) != type || LE(header, 12) != 2)
				throw xParseError("invalid system page header");
			auto data = Slice(file, offset + 20, packed);
			buffer_t check(header.begin(), header.end());
			std::fill(check.begin() + 16, check.end(), 0);
			if (Checksum(data, Checksum(check)) != LE(header, 16))
				throw xParseError("system page checksum mismatch");
			return Decompress2004(data, budget.Bytes(size));
		}
		sContainer Container2004(bytes_t file, Budget &budget) {
			auto encrypted = Slice(file, 0x80, 0x6c);
			buffer_t header(encrypted.begin(), encrypted.end());
			std::uint32_t random = 1;
			for (auto &b : header) {
				random = random * 0x343fd + 0x269ec3;
				b ^= static_cast<std::uint8_t>(random >> 16);
			}
			if (std::string_view(reinterpret_cast<char const *>(header.data()), 12) !=
				std::string_view{"AcFssFcAJMB\0", 12})
				throw xParseError("invalid decoded file header");
			auto crc = LE(header, 0x68);
			std::fill(header.begin() + 0x68, header.end(), 0);
			if (CRC32(header) != crc)
				throw xParseError("file header CRC32 mismatch");
			auto mapAddress = LE(header, 0x54, 8);
			if (mapAddress > file.size())
				throw xParseError("invalid page map address");
			auto map = System2004(file, mapAddress + 0x100, 0x41630e3b, budget);
			Cursor cursor{map};
			std::map<std::uint64_t, Page> pages;
			std::uint64_t address = 0x100;
			while (cursor.at < map.size()) {
				auto id = static_cast<std::int32_t>(cursor.Read());
				auto size = cursor.Read();
				budget.Pages(1);
				// The last system page's reserved allocation can extend beyond EOF.
				Slice(file, address, 0);
				if (id < 0)
					cursor.Take(16);
				else if (!id || !pages.emplace(id, Page{address, size}).second)
					throw xParseError("duplicate or invalid page id");
				address += size;
			}
			auto sectionPage = pages.find(LE(header, 0x5c));
			if (sectionPage == pages.end())
				throw xParseError("missing section map page");
			auto descriptions = System2004(file, sectionPage->second.offset, 0x4163003b, budget);
			Cursor sections{descriptions};
			auto count = sections.Read();
			sections.Take(16);
			if (count > (descriptions.size() - 20) / 96)
				throw xParseError("invalid section count");
			sContainer result;
			for (std::uint64_t i = 0; i < count; ++i) {
				auto size = sections.Read(8), n = sections.Read(), maxPage = sections.Read();
				sections.Read();
				auto compression = sections.Read(), id = sections.Read(), encryption = sections.Read();
				auto nameBytes = sections.Take(64);
				std::string name;
				for (auto c : nameBytes) {
					if (!c)
						break;
					name += static_cast<char>(c);
				}
				try {
					if (encryption == 1)
						throw xParseError("password encrypted section is not supported");
					if (compression != 1 && compression != 2)
						throw xParseError("unknown section compression");
					if (n > (descriptions.size() - sections.at) / 16 || maxPage > budget.options.maxDecodedBytes)
						throw xParseError("invalid section page count or size");
					sContainerSection section{id, buffer_t(budget.Bytes(size)), static_cast<size_t>(n)};
					std::uint64_t previousEnd{};
					for (std::uint64_t j = 0; j < n; ++j) {
						auto pageId = sections.Read(), packed = sections.Read(), start = sections.Read(8);
						auto found = pages.find(pageId);
						if (found == pages.end())
							throw xParseError("missing data page");
						auto page = found->second;
						auto encoded = Slice(file, page.offset, 32);
						buffer_t ph(encoded.begin(), encoded.end());
						std::uint32_t mask = 0x4164536b ^ static_cast<std::uint32_t>(page.offset);
						for (size_t k = 0; k < 32; ++k)
							ph[k] ^= static_cast<std::uint8_t>(mask >> (8 * (k % 4)));
						if (LE(ph, 0) != 0x4163043b || LE(ph, 4) != id || LE(ph, 8) != packed ||
							LE(ph, 16, 8) != start || LE(ph, 12) != page.size)
							throw xParseError("data page disagrees with section map");
						auto unpacked = maxPage;
						if (page.size < 32 || packed > page.size - 32 || unpacked > maxPage || start < previousEnd ||
							start > size)
							throw xParseError("invalid data page bounds: allocation=" + std::to_string(page.size) +
											  " packed=" + std::to_string(packed) + " unpacked=" +
											  std::to_string(unpacked) + " max=" + std::to_string(maxPage));
						auto data = Slice(file, page.offset + 32, packed);
						auto checksum = LE(ph, 28);
						if (Checksum(data) != checksum)
							throw xParseError("data page checksum mismatch");
						auto headerChecksum = LE(ph, 24);
						std::fill(ph.begin() + 24, ph.begin() + 28, 0);
						if (Checksum(ph, static_cast<std::uint32_t>(checksum)) != headerChecksum)
							throw xParseError("data page header checksum mismatch");
						auto decoded = compression == 2 ? Decompress2004(data, static_cast<size_t>(unpacked))
														: buffer_t(data.begin(), data.end());
						if (decoded.size() < std::min(unpacked, size - start))
							throw xParseError("short uncompressed page");
						auto copy = static_cast<size_t>(std::min(unpacked, size - start));
						std::copy_n(decoded.begin(), copy, section.data.begin() + static_cast<size_t>(start));
						previousEnd = start + copy;
					}
					if (!result.sections.emplace(name, std::move(section)).second)
						throw xParseError("duplicate section name");
				} catch (xParseError const &e) {
					throw xParseError(name + ": " + e.what());
				}
			}
			if (sections.at != descriptions.size())
				throw xParseError("trailing section map data");
			return result;
		}
	} // namespace

	std::vector<std::uint8_t> Decompress2004(std::span<std::uint8_t const> input, size_t size) {
		Cursor c{input};
		buffer_t out;
		out.reserve(size);
		auto extension = [&]() {
			size_t length{};
			unsigned b;
			do {
				b = c.Byte();
				if (length > size || (b ? b : 255) > size - length)
					throw xParseError("compression length overflow");
				length += b ? b : 255;
			} while (!b);
			return length;
		};
		auto literals = [&](size_t count) {
			if (count > size - out.size())
				throw xParseError("literal exceeds output");
			auto b = c.Take(count);
			out.insert(out.end(), b.begin(), b.end());
		};
		auto literalLength = [&]() -> size_t {
			if (c.at == input.size())
				throw xParseError("missing compression terminator");
			if (input[c.at] & 0xf0)
				return 0;
			auto n = c.Byte();
			return n ? n + 3 : 18 + extension();
		};
		literals(literalLength());
		for (;;) {
			auto op = c.Byte();
			if (op == 0x11)
				break;
			size_t count{}, distance{}, literal{};
			if (op >= 0x40) {
				count = (op >> 4) - 1;
				distance = (c.Byte() << 2) | ((op >> 2) & 3);
				++distance;
				literal = op & 3;
			} else if (op >= 0x10) {
				count = op == 0x10	 ? extension() + 9
						: op == 0x20 ? extension() + 33
						: op < 0x20	 ? (op & 15) + 2
									 : op - 30;
				auto first = c.Byte();
				distance = (first >> 2) | (c.Byte() << 6);
				distance += op < 0x20 ? 0x4000 : 1;
				literal = first & 3;
			} else
				throw xParseError("invalid compression opcode");
			Copy(out, distance, count, size);
			if (!literal)
				literal = literalLength();
			literals(literal);
		}
		if (out.size() != size)
			throw xParseError("decompressed size mismatch");
		// Writers commonly append two zero bytes after the terminator.
		if (input.size() - c.at > 2 || std::ranges::any_of(input.subspan(c.at), [](auto b) { return b != 0; }))
			throw xParseError("trailing compressed data");
		return out;
	}

	std::vector<std::uint8_t> Decompress2007(std::span<std::uint8_t const> input, size_t size) {
		Cursor c{input};
		buffer_t out;
		out.reserve(size);
		if (!size) {
			if (!input.empty())
				throw xParseError("nonempty compression of empty page");
			return out;
		}
		unsigned op = c.Byte();
		size_t literal{};
		if ((op >> 4) == 2) {
			c.Take(2);
			literal = c.Byte() & 7;
		}
		auto copyLiteral = [&](size_t length) {
			if (length > size - out.size())
				throw xParseError("R2007 literal exceeds output");
			auto source = c.Take(length);
			size_t at{};
			auto block = [&](size_t start, size_t n) {
				auto b = source.subspan(at + start, n);
				if (n == 16) {
					out.insert(out.end(), b.begin() + 8, b.end());
					out.insert(out.end(), b.begin(), b.begin() + 8);
				} else if (n == 2 || n == 3)
					out.insert(out.end(), b.rbegin(), b.rend());
				else
					out.insert(out.end(), b.begin(), b.end());
			};
			while (length >= 32) {
				block(16, 16);
				block(0, 16);
				at += 32;
				length -= 32;
			}
			// Literal tails use a byte permutation; each pair is (source offset, count).
			static std::vector<std::pair<unsigned, unsigned>> const tails[]{
				{},
				{{0, 1}},
				{{1, 1}, {0, 1}},
				{{2, 1}, {1, 1}, {0, 1}},
				{{0, 4}},
				{{4, 1}, {0, 4}},
				{{5, 1}, {1, 4}, {0, 1}},
				{{5, 2}, {1, 4}, {0, 1}},
				{{0, 8}},
				{{8, 1}, {0, 8}},
				{{9, 1}, {1, 8}, {0, 1}},
				{{9, 2}, {1, 8}, {0, 1}},
				{{8, 4}, {0, 8}},
				{{12, 1}, {8, 4}, {0, 8}},
				{{13, 1}, {9, 4}, {1, 8}, {0, 1}},
				{{13, 2}, {9, 4}, {1, 8}, {0, 1}},
				{{8, 8}, {0, 8}},
				{{9, 8}, {8, 1}, {0, 8}},
				{{17, 1}, {1, 16}, {0, 1}},
				{{16, 3}, {0, 16}},
				{{16, 4}, {0, 16}},
				{{20, 1}, {16, 4}, {0, 16}},
				{{20, 2}, {16, 4}, {0, 16}},
				{{20, 3}, {16, 4}, {0, 16}},
				{{16, 8}, {0, 16}},
				{{17, 8}, {16, 1}, {0, 16}},
				{{25, 1}, {17, 8}, {16, 1}, {0, 16}},
				{{25, 2}, {17, 8}, {16, 1}, {0, 16}},
				{{24, 4}, {16, 8}, {0, 16}},
				{{28, 1}, {24, 4}, {16, 8}, {0, 16}},
				{{28, 2}, {24, 4}, {16, 8}, {0, 16}},
				{{30, 1}, {26, 4}, {18, 8}, {2, 16}, {0, 2}}};
			for (auto [start, n] : tails[length])
				block(start, n);
		};
		auto instruction = [&]() {
			size_t length{}, distance{};
			unsigned first;
			switch (op >> 4) {
			case 0:
				length = (op & 15) + 19;
				first = c.Byte();
				op = c.Byte();
				length += (op >> 3) & 16;
				distance = ((op & 0x78) << 5) + 1 + first;
				break;
			case 1:
				length = (op & 15) + 3;
				first = c.Byte();
				op = c.Byte();
				distance = ((op & 0xf8) << 5) + 1 + first;
				break;
			case 2:
				distance = static_cast<size_t>(c.Read(2));
				length = op & 7;
				if (!(op & 8)) {
					op = c.Byte();
					length += op & 0xf8;
				} else {
					++distance;
					length += c.Byte() << 3;
					op = c.Byte();
					length += ((op & 0xf8) << 8) + 256;
				}
				break;
			default:
				length = op >> 4;
				distance = op & 15;
				op = c.Byte();
				distance += ((op & 0xf8) << 1) + 1;
				break;
			}
			Copy(out, distance, length, size);
		};
		for (;;) {
			if (!literal) {
				literal = op + 8;
				if (literal == 23) {
					auto extra = c.Byte();
					literal += extra;
					if (extra == 255) {
						std::uint64_t more;
						do {
							more = c.Read(2);
							if (literal > size || more > size - literal)
								throw xParseError("R2007 literal length overflow");
							literal += static_cast<size_t>(more);
						} while (more == 65535);
					}
				}
			}
			copyLiteral(literal);
			if (c.at == input.size())
				break;
			op = c.Byte();
			instruction();
			for (;;) {
				literal = op & 7;
				if (literal || c.at == input.size())
					break;
				op = c.Byte();
				if (!(op >> 4))
					break;
				if ((op >> 4) == 15)
					op &= 15;
				instruction();
			}
			if (c.at == input.size()) {
				if (literal)
					throw xParseError("missing R2007 literal tail");
				break;
			}
		}
		if (out.size() != size)
			throw xParseError("R2007 decompressed size mismatch");
		return out;
	}

	sContainer DecodeContainer(std::span<std::uint8_t const> input, sContainerOptions const &options) {
		if (input.size() > options.maxFileBytes)
			throw std::length_error("DWG file byte limit exceeded");
		auto version = DetectVersion(input);
		if (version < eVERSION::r2004)
			throw xParseError("container version not implemented");
		if (LE(input, 0x18) & 3)
			throw xParseError("password encrypted DWG is not supported");
		Budget budget{options};
		auto result = version == eVERSION::r2007 ? Container2007(input, budget) : Container2004(input, budget);
		result.version = version;
		result.codepage = static_cast<std::uint16_t>(LE(input, 0x13, 2));
		return result;
	}
} // namespace gtl::dwg::detail

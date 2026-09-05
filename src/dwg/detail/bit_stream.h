#pragma once

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>

namespace gtl::dwg::detail {

	class xParseError : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	// DWG consumes bits most-significant first, but raw multi-byte values are little-endian.
	class xBitStream {
		std::span<std::uint8_t const> m_data;
		size_t m_position{}, m_limit{};
	public:
		explicit xBitStream(std::span<std::uint8_t const> data) : m_data(data) {
			if (data.size() > std::numeric_limits<size_t>::max() / 8) throw xParseError("stream too large");
			m_limit = data.size() * 8;
		}
		size_t Position() const { return m_position; }
		size_t Remaining() const { return m_limit - m_position; }
		void Seek(size_t position) {
			if (position > m_limit) throw xParseError("bit offset outside stream");
			m_position = position;
		}
		void Limit(size_t end) {
			if (end < m_position || end > m_limit) throw xParseError("invalid object data boundary");
			m_limit = end;
		}
		void SkipBytes(size_t count) {
			if (count > Remaining() / 8) throw xParseError("truncated byte sequence");
			m_position += count * 8;
		}
		std::uint64_t Bits(unsigned count) {
			if (count > 64 || count > Remaining()) throw xParseError("truncated bit sequence");
			std::uint64_t value{};
			for (unsigned i{}; i < count; ++i, ++m_position)
				value = (value << 1) | ((m_data[m_position / 8] >> (7 - m_position % 8)) & 1);
			return value;
		}
		bool B() { return Bits(1) != 0; }
		std::uint8_t RC() { return static_cast<std::uint8_t>(Bits(8)); }
		std::uint64_t Raw(unsigned bytes) {
			if (bytes > 8) throw xParseError("invalid raw integer size");
			std::uint64_t value{};
			for (unsigned i{}; i < bytes; ++i) value |= std::uint64_t{RC()} << (8 * i);
			return value;
		}
		std::uint16_t RS() { return static_cast<std::uint16_t>(Raw(2)); }
		std::uint32_t RL() { return static_cast<std::uint32_t>(Raw(4)); }
		double RD() { return Finite(std::bit_cast<double>(Raw(8))); }
		std::int16_t BS() {
			switch (Bits(2)) {
			case 0: return std::bit_cast<std::int16_t>(RS());
			case 1: return RC();
			case 2: return 0;
			default: return 256;
			}
		}
		std::int32_t BL() {
			switch (Bits(2)) {
			case 0: return std::bit_cast<std::int32_t>(RL());
			case 1: return RC();
			case 2: return 0;
			default: throw xParseError("reserved bitlong code");
			}
		}
		double BD() {
			switch (Bits(2)) {
			case 0: return RD();
			case 1: return 1.;
			case 2: return 0.;
			default: throw xParseError("reserved bitdouble code");
			}
		}
		double DD(double defaultValue) {
			auto code = Bits(2);
			if (!code) return defaultValue;
			if (code == 3) return RD();
			auto raw = std::bit_cast<std::uint64_t>(defaultValue);
			auto patch = [&](unsigned index) {
				raw = (raw & ~(std::uint64_t{255} << (index * 8))) | (std::uint64_t{RC()} << (index * 8));
			};
			if (code == 2) { patch(4); patch(5); }
			for (unsigned i{}; i < 4; ++i) patch(i);
			return Finite(std::bit_cast<double>(raw));
		}
		std::uint64_t UMC() {
			std::uint64_t value{};
			for (unsigned shift{}; shift < 64; shift += 7) {
				auto b = RC();
				if (shift == 63 && (b & 0x7e)) throw xParseError("modular integer overflow");
				value |= std::uint64_t(b & 0x7f) << shift;
				if (!(b & 0x80)) return value;
			}
			throw xParseError("unterminated modular integer");
		}
		std::int64_t MC() {
			std::uint64_t value{};
			for (unsigned shift{}; shift < 63; shift += 7) {
				auto b = RC();
				bool last = !(b & 0x80);
				value |= std::uint64_t(b & (last ? 0x3f : 0x7f)) << shift;
				if (last) return (b & 0x40) ? -static_cast<std::int64_t>(value) : static_cast<std::int64_t>(value);
			}
			throw xParseError("modular signed integer overflow");
		}
		std::uint32_t MS() {
			auto low = RS();
			if (!(low & 0x8000)) return low;
			auto high = RS();
			if (high & 0x8000) throw xParseError("modular short overflow");
			return (low & 0x7fff) | (std::uint32_t{high} << 15);
		}
		std::uint64_t Handle(std::uint64_t reference = 0) {
			auto header = RC();
			unsigned code = header >> 4, size = header & 15;
			if (size > 8) throw xParseError("handle exceeds 64 bits");
			std::uint64_t value{};
			for (unsigned i{}; i < size; ++i) value = (value << 8) | RC();
			switch (code) {
			case 0: case 2: case 3: case 4: case 5: return value;
			case 6: case 8:
				if (size) throw xParseError("invalid implicit handle length");
				value = 1;
				break;
			case 10: case 12: break;
			default: throw xParseError("invalid handle code");
			}
			if (code == 6 || code == 10) {
				if (value > UINT64_MAX - reference) throw xParseError("handle overflow");
				return reference + value;
			}
			if (value > reference) throw xParseError("handle underflow");
			return reference - value;
		}
		std::string Text() {
			auto size = BS();
			if (size < 0 || static_cast<size_t>(size) > Remaining() / 8) throw xParseError("invalid text length");
			std::string result;
			result.reserve(size);
			for (int i{}; i < size; ++i) result += static_cast<char>(RC());
			// Some writers include a terminating NUL in the stored byte count.
			while (!result.empty() && result.back() == '\0') result.pop_back();
			return result;
		}
		static double Finite(double value) {
			if (!std::isfinite(value)) throw xParseError("non-finite coordinate");
			return value;
		}
	};

	inline std::uint16_t CRC16(std::span<std::uint8_t const> bytes, std::uint16_t seed = 0xc0c1) {
		for (auto byte : bytes) {
			seed ^= byte;
			for (int bit{}; bit < 8; ++bit) seed = static_cast<std::uint16_t>((seed >> 1) ^ ((seed & 1) ? 0xa001 : 0));
		}
		return seed;
	}

}

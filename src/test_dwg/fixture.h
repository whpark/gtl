#pragma once

#include <algorithm>
#include <bit>
#include <array>
#include <functional>
#include <map>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string_view>
#include <utility>
#include <vector>

namespace fixture {
	// Small, independently encoded R2000 parser fixture. Not a general DWG writer.
	struct Bits {
		std::vector<std::uint8_t> data;
		size_t position{};
		void Put(std::uint64_t value, unsigned count) {
			for (unsigned i{}; i < count; ++i, ++position) {
				if (position / 8 == data.size()) data.push_back(0);
				if ((value >> (count - 1 - i)) & 1) data[position / 8] |= 1 << (7 - position % 8);
			}
		}
		void Raw(std::uint64_t value, unsigned bytes) { for (unsigned i{}; i < bytes; ++i) Put((value >> (8 * i)) & 255, 8); }
		void Short(int value) { Put(0, 2); Raw(static_cast<std::uint16_t>(value), 2); }
		void Double(double value) { Put(0, 2); Raw(std::bit_cast<std::uint64_t>(value), 8); }
		void Point(std::array<double,3> p) { for (auto v : p) Double(v); }
		void Text(std::string_view s) { Short(static_cast<int>(s.size())); for (unsigned char c : s) Raw(c,1); }
		void Handle(unsigned value, unsigned code = 0) { Put((code << 4) | 1, 8); Put(value, 8); }
		void PatchRaw(size_t at, std::uint32_t value) {
			for (unsigned i{}; i < 32; ++i) {
				auto bit = at + i;
				unsigned mask = 1 << (7 - bit % 8);
				data[bit / 8] &= ~mask;
				if ((value >> ((i / 8) * 8 + 7 - i % 8)) & 1) data[bit / 8] |= mask;
			}
		}
	};
	inline std::uint16_t Crc(std::vector<std::uint8_t> const& data, unsigned seed = 0xc0c1) {
		for (unsigned byte : data) {
			seed ^= byte;
			for (unsigned i{}; i < 8; ++i) seed = (seed >> 1) ^ ((seed & 1) ? 0xa001 : 0);
		}
		return static_cast<std::uint16_t>(seed);
	}
	inline void Word(std::vector<std::uint8_t>& data, unsigned value, bool big = false) {
		data.push_back(static_cast<std::uint8_t>(big ? value >> 8 : value));
		data.push_back(static_cast<std::uint8_t>(big ? value : value >> 8));
	}
	inline void LongAt(std::vector<std::uint8_t>& data, size_t at, std::uint32_t value) {
		for (unsigned i{}; i < 4; ++i) data.at(at + i) = static_cast<std::uint8_t>(value >> (8 * i));
	}
	inline void Modular(std::vector<std::uint8_t>& data, unsigned value) {
		while (value >= 64) { data.push_back(static_cast<std::uint8_t>((value & 127) | 128)); value >>= 7; }
		data.push_back(static_cast<std::uint8_t>(value));
	}
	inline std::vector<std::uint8_t> Object(unsigned type, unsigned handle, bool layer, bool circle) {
		Bits bits;
		bits.Short(type);
		auto sizeAt = bits.position;
		bits.Raw(0, 4);
		bits.Handle(handle);
		bits.Short(0); // no EED
		if (!layer) { bits.Put(0, 1); bits.Put(2, 2); } // no graphic, model space
		bits.Put(2, 2); // zero reactors (BL)
		if (layer) {
			std::string_view name = "\xb0\xa1"; // Korean 'ga', CP949
			bits.Short(static_cast<int>(name.size()));
			for (unsigned char c : name) bits.Raw(c, 1);
			bits.Put(0, 1); bits.Short(0); bits.Put(0, 1);
			bits.Short(16 | (31 << 5)); // plotting, default lineweight
			bits.Short(3);
		}
		else {
			bits.Put(1, 1); // no previous/next links
			bits.Short(circle ? 256 : 1);
			bits.Double(1.);
			bits.Put(0, 4); // linetype and plotstyle bylayer
			bits.Short(0); bits.Raw(29, 1);
			if (circle) {
				bits.Double(10.); bits.Double(20.); bits.Double(0.); bits.Double(5.);
			}
			else {
				bits.Put(0, 1); // nonzero Z
				for (auto pair : {std::pair{1., 4.}, std::pair{2., 5.}, std::pair{3., 6.}}) {
					bits.Raw(std::bit_cast<std::uint64_t>(pair.first), 8);
					bits.Put(3, 2); bits.Raw(std::bit_cast<std::uint64_t>(pair.second), 8);
				}
			}
			bits.Put(1, 1); bits.Put(1, 1); // zero thickness, default extrusion
		}
		bits.PatchRaw(sizeAt, static_cast<std::uint32_t>(bits.position));
		if (layer) {
			bits.Handle(0, 4); bits.Handle(0, 3); bits.Handle(0, 5); bits.Handle(0, 5); bits.Handle(0, 5);
		}
		else { bits.Handle(0, 3); bits.Handle(1, 5); }
		std::vector<std::uint8_t> result;
		Word(result, static_cast<unsigned>(bits.data.size()));
		result.insert(result.end(), bits.data.begin(), bits.data.end());
		Word(result, Crc(result));
		return result;
	}
	inline std::vector<std::uint8_t> Container(std::map<unsigned, std::vector<std::uint8_t>> const& objects, bool r2000 = true) {
		std::vector<std::uint8_t> file(52);
		std::string_view signature = r2000 ? "AC1015" : "AC1014";
		std::copy(signature.begin(), signature.end(), file.begin());
		file[0x13] = 40; // ANSI_949
		LongAt(file, 21, 3);
		file[25] = 0; file[34] = 1; file[43] = 2;
		file.resize(70); // CRC + sentinel
		std::vector<unsigned> offsets;
		for (auto const& [handle, object] : objects) {
			offsets.push_back(static_cast<unsigned>(file.size()));
			file.insert(file.end(), object.begin(), object.end());
		}
		std::vector<std::uint8_t> map(2);
		unsigned previous{};
		unsigned previousHandle{}, index{};
		for (auto const& [handle, object] : objects) {
			auto offset = offsets[index++];
			Modular(map, handle-previousHandle); Modular(map, offset-previous);
			previous = offset; previousHandle = handle;
		}
		map[0] = static_cast<std::uint8_t>(map.size() >> 8); map[1] = static_cast<std::uint8_t>(map.size());
		Word(map, Crc(map), true);
		std::vector<std::uint8_t> terminator{0, 2};
		Word(terminator, Crc(terminator), true);
		map.insert(map.end(), terminator.begin(), terminator.end());
		LongAt(file, 44, static_cast<unsigned>(file.size()));
		LongAt(file, 48, static_cast<unsigned>(map.size()));
		file.insert(file.end(), map.begin(), map.end());
		auto crc = Crc({file.begin(), file.begin() + 52}, 0) ^ 0xa598;
		file[52] = static_cast<std::uint8_t>(crc); file[53] = static_cast<std::uint8_t>(crc >> 8);
		std::uint8_t const sentinel[]{0x95,0xa0,0x4e,0x28,0x99,0x82,0x1a,0xe5,0x5e,0x41,0xe0,0x5f,0x9d,0x3a,0x4d,0};
		std::copy(std::begin(sentinel), std::end(sentinel), file.begin() + 54);
		return file;
	}
	inline std::vector<std::uint8_t> Drawing() {
		return Container({{1,Object(0x33,1,true,false)}, {2,Object(0x13,2,false,false)}, {3,Object(0x12,3,false,true)}});
	}
	inline std::uint32_t PageChecksum(std::vector<std::uint8_t> const& b,std::uint32_t seed=0) {
		std::uint32_t a=seed&65535,s=seed>>16;
		for(auto c:b){a=(a+c)%65521;s=(s+a)%65521;}return a|(s<<16);
	}
	inline std::uint32_t HeaderCRC(std::vector<std::uint8_t> const& b) {
		std::uint32_t crc=~0u;
		for(auto c:b){crc^=c;for(int i=0;i<8;++i)crc=(crc>>1)^((crc&1)?0xedb88320u:0);}return ~crc;
	}
	inline std::vector<std::uint8_t> Literal2004(std::vector<std::uint8_t> const& data) {
		std::vector<std::uint8_t> b;
		if(data.size()<=18)b.push_back(static_cast<std::uint8_t>(data.size()-3));
		else {b.push_back(0);auto left=data.size()-18;while(left>255){b.push_back(0);left-=255;}b.push_back(static_cast<std::uint8_t>(left));}
		b.insert(b.end(),data.begin(),data.end());b.push_back(0x11);return b;
	}
	inline std::vector<std::uint8_t> CompressedContainer(std::string_view signature="AC1018") {
		// One raw data page and two independently encoded literal-only system pages.
		std::vector<std::uint8_t> file(736),header(108),page(32),data(64);
		std::copy(signature.begin(),signature.end(),file.begin());
		std::string_view magic{"AcFssFcAJMB\0",12};std::copy(magic.begin(),magic.end(),header.begin());
		LongAt(header,0x50,3);LongAt(header,0x54,608-256);LongAt(header,0x5c,2);LongAt(header,0x68,HeaderCRC(header));
		std::uint32_t random=1;
		for(size_t i=0;i<header.size();++i){random=random*0x343fd+0x269ec3;file[128+i]=header[i]^static_cast<std::uint8_t>(random>>16);}
		for(size_t i=0;i<12;++i)data[i]=static_cast<std::uint8_t>('a'+i);
		LongAt(page,0,0x4163043b);LongAt(page,4,1);LongAt(page,8,64);LongAt(page,12,96);
		LongAt(page,28,PageChecksum(data));LongAt(page,24,PageChecksum(page,PageChecksum(data)));
		for(size_t i=0;i<32;++i)file[256+i]=page[i]^static_cast<std::uint8_t>((0x4164536b^256)>>(8*(i%4)));
		std::copy(data.begin(),data.end(),file.begin()+288);
		auto system=[&](size_t at,unsigned type,std::vector<std::uint8_t> const& raw) {
			auto packed=Literal2004(raw);std::vector<std::uint8_t> h(20);
			LongAt(h,0,type);LongAt(h,4,static_cast<unsigned>(raw.size()));LongAt(h,8,static_cast<unsigned>(packed.size()));LongAt(h,12,2);
			LongAt(h,16,PageChecksum(packed,PageChecksum(h)));std::copy(h.begin(),h.end(),file.begin()+at);std::copy(packed.begin(),packed.end(),file.begin()+at+20);
		};
		std::vector<std::uint8_t> sections(132);LongAt(sections,0,1);LongAt(sections,20,12);LongAt(sections,28,1);LongAt(sections,32,64);
		LongAt(sections,40,1);LongAt(sections,44,1);std::string_view name="AcDb:Test";std::copy(name.begin(),name.end(),sections.begin()+52);
		LongAt(sections,116,1);LongAt(sections,120,64);system(352,0x4163003b,sections);
		std::vector<std::uint8_t> pages(24);LongAt(pages,0,1);LongAt(pages,4,96);LongAt(pages,8,2);LongAt(pages,12,256);LongAt(pages,16,3);LongAt(pages,20,128);
		system(608,0x41630e3b,pages);return file;
	}

	// Independent, deliberately uncompressed records for graph/transform tests.
	struct Graph {
		bool r2000{true};
		std::map<unsigned,std::vector<std::uint8_t>> objects;
		void Finish(unsigned id, Bits& b, size_t sizeAt, std::function<void(Bits&)> handles) {
			b.PatchRaw(sizeAt, static_cast<std::uint32_t>(b.position)); handles(b);
			std::vector<std::uint8_t> result; Word(result, static_cast<unsigned>(b.data.size()));
			result.insert(result.end(),b.data.begin(),b.data.end()); Word(result,Crc(result));
			objects[id] = std::move(result);
		}
		void Layer(unsigned id, std::string_view name, int color = 3) {
			Bits b; b.Short(0x33); size_t sizeAt = b.position;
			if (r2000) b.Raw(0,4);
			b.Handle(id); b.Short(0);
			if (!r2000) { sizeAt=b.position; b.Raw(0,4); }
			b.Put(2,2); b.Text(name); b.Put(0,1); b.Short(0); b.Put(0,1);
			if (r2000) b.Short(16 | (31<<5)); else b.Put(0,4);
			b.Short(color);
			Finish(id,b,sizeAt,[&](Bits& h) { for (int i=0;i<(r2000?5:4);++i) h.Handle(0); });
		}
		void Entity(unsigned type, unsigned id, unsigned owner, std::function<void(Bits&)> geometry,
			std::vector<unsigned> refs = {}, unsigned next = 0, unsigned layer = 1, int color = 0) {
			Bits b; b.Short(type); size_t sizeAt=b.position;
			if (r2000) b.Raw(0,4);
			b.Handle(id); b.Short(0); b.Put(0,1);
			if (!r2000) { sizeAt=b.position; b.Raw(0,4); }
			b.Put(owner?0:2,2); b.Put(2,2);
			if (!r2000) b.Put(1,1);
			b.Put(0,1); b.Short(color); b.Double(1.);
			if (r2000) b.Put(0,4);
			b.Short(0); if (r2000) b.Raw(30,1);
			geometry(b);
			Finish(id,b,sizeAt,[&](Bits& h) {
				if (owner) h.Handle(owner);
				h.Handle(0);
				if (!r2000) h.Handle(layer);
				h.Handle(0); h.Handle(next);
				if (r2000) h.Handle(layer);
				for (auto ref : refs) h.Handle(ref);
			});
		}
		void Block(unsigned id, unsigned first, unsigned last, std::array<double,3> base = {}) {
			Bits b; b.Short(0x31); size_t sizeAt=b.position;
			if (r2000) b.Raw(0,4);
			b.Handle(id); b.Short(0);
			if (!r2000) { sizeAt=b.position; b.Raw(0,4); }
			b.Put(2,2); b.Text("BLOCK"); b.Put(0,1); b.Short(0); b.Put(0,5);
			if (r2000) b.Put(0,1);
			b.Point(base); b.Text("");
			if (r2000) { b.Raw(0,1); b.Text(""); b.Put(2,2); }
			Finish(id,b,sizeAt,[&](Bits& h) {
				h.Handle(0); h.Handle(0); h.Handle(0); h.Handle(id+1);
				h.Handle(first); h.Handle(last); h.Handle(id+2); if(r2000) h.Handle(0);
			});
			Entity(4,id+1,id,[](Bits& b){b.Text("BLOCK");});
			Entity(5,id+2,id,[](Bits&){});
		}
		void Insert(unsigned id, unsigned owner, unsigned block, std::array<double,3> position = {},
			std::array<double,3> scale = {1,1,1}, double angle = 0., unsigned columns = 1, unsigned rows = 1,
			unsigned layer = 1, std::array<double,3> normal = {0,0,1}, int color = 1) {
			Entity(columns==1 && rows==1?7:8,id,owner,[&](Bits& b) {
				b.Point(position);
				if (!r2000) b.Point(scale);
				else { b.Put(0,2); b.Raw(std::bit_cast<std::uint64_t>(scale[0]),8);
					for(int i=1;i<3;++i) {b.Put(3,2); b.Raw(std::bit_cast<std::uint64_t>(scale[i]),8);} }
				b.Double(angle); b.Point(normal); b.Put(0,1);
				if(columns!=1 || rows!=1) {b.Short(columns);b.Short(rows);b.Double(10.);b.Double(20.);}
			},{block},0,layer,color);
		}
		void Circle(unsigned id, unsigned owner, unsigned next = 0) {
			Entity(0x12,id,owner,[&](Bits& b) {b.Point({1,2,0});b.Double(2.);
				if(r2000) b.Put(3,2);else {b.Double(0.);b.Point({0,0,1});}}, {},next);
		}
		void Polyline(bool three = false, unsigned vertexOwner = 2, unsigned next = 7) {
			Entity(three?0x10:0x0f,2,0,[&](Bits& b) {
				if (three) {b.Raw(0,1);b.Raw(1,1);}
				else {b.Short(1);b.Short(0);b.Double(0.);b.Double(0.);
					if(r2000)b.Put(1,1);else b.Double(0.);
					b.Double(9.);if(r2000)b.Put(1,1);else b.Point({0,0,1});}
			},{4,7,8});
			for (unsigned id : {4u,7u}) Entity(three?0x0b:0x0a,id,vertexOwner,[&](Bits& b) {
				b.Raw(0,1);b.Point({double(id),double(id+1),double(id+2)});
				if(!three) {b.Double(-2.);b.Double(id==4?.5:0.);b.Double(0.);}
			},{},id==4?next:8);
			Entity(6,8,2,[](Bits&){});
		}
		std::vector<std::uint8_t> Bytes() const {return Container(objects,r2000);}
	};
}

#pragma once

#include <algorithm>
#include <bit>
#include <array>
#include <functional>
#include <map>
#include <optional>
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
		void Long(std::uint32_t value) { Put(0,2); Raw(value,4); }
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
	inline std::vector<std::uint8_t> PackSections(std::map<std::string,std::vector<std::uint8_t>> const& sections, std::string_view signature="AC1018") {
		std::vector<std::uint8_t> file(256),descriptions(20),pageMap;
		LongAt(descriptions,0,static_cast<unsigned>(sections.size()));
		unsigned id=0;
		for(auto const& [name,raw]:sections) {
			++id;auto at=static_cast<unsigned>(file.size());auto padded=static_cast<unsigned>((raw.size()+31)&~size_t{31});
			std::vector<std::uint8_t> data=raw;data.resize(padded);std::vector<std::uint8_t> header(32);
			LongAt(header,0,0x4163043b);LongAt(header,4,id);LongAt(header,8,padded);LongAt(header,12,padded+32);
			LongAt(header,28,PageChecksum(data));LongAt(header,24,PageChecksum(header,PageChecksum(data)));
			for(size_t i=0;i<32;++i)header[i]^=static_cast<std::uint8_t>((0x4164536b^at)>>(8*(i%4)));
			file.insert(file.end(),header.begin(),header.end());file.insert(file.end(),data.begin(),data.end());
			auto p=pageMap.size();pageMap.resize(p+8);LongAt(pageMap,p,id);LongAt(pageMap,p+4,padded+32);
			auto d=descriptions.size();descriptions.resize(d+112);
			LongAt(descriptions,d,static_cast<unsigned>(raw.size()));LongAt(descriptions,d+8,1);LongAt(descriptions,d+12,padded);
			LongAt(descriptions,d+20,1);LongAt(descriptions,d+24,id);std::copy(name.begin(),name.end(),descriptions.begin()+d+32);
			LongAt(descriptions,d+96,id);LongAt(descriptions,d+100,padded);
		}
		auto system=[&](unsigned type,std::vector<std::uint8_t> const& raw) {
			auto packed=Literal2004(raw);std::vector<std::uint8_t> header(20);
			LongAt(header,0,type);LongAt(header,4,static_cast<unsigned>(raw.size()));LongAt(header,8,static_cast<unsigned>(packed.size()));LongAt(header,12,2);
			LongAt(header,16,PageChecksum(packed,PageChecksum(header)));
			file.insert(file.end(),header.begin(),header.end());file.insert(file.end(),packed.begin(),packed.end());file.resize((file.size()+31)&~size_t{31});
		};
		auto descriptionsAt=static_cast<unsigned>(file.size());system(0x4163003b,descriptions);
		auto mapAt=static_cast<unsigned>(file.size());auto p=pageMap.size();pageMap.resize(p+16);
		LongAt(pageMap,p,id+1);LongAt(pageMap,p+4,mapAt-descriptionsAt);LongAt(pageMap,p+8,id+2);
		auto mapAllocation=static_cast<unsigned>((20+Literal2004(pageMap).size()+31)&~size_t{31});LongAt(pageMap,p+12,mapAllocation);
		system(0x41630e3b,pageMap);
		std::copy(signature.begin(),signature.end(),file.begin());
		std::vector<std::uint8_t> header(108);std::string_view magic{"AcFssFcAJMB\0",12};std::copy(magic.begin(),magic.end(),header.begin());
		LongAt(header,0x50,id+2);LongAt(header,0x54,mapAt-256);LongAt(header,0x5c,id+1);LongAt(header,0x68,HeaderCRC(header));
		std::uint32_t random=1;for(size_t i=0;i<108;++i){random=random*0x343fd+0x269ec3;file[128+i]=header[i]^static_cast<std::uint8_t>(random>>16);}
		return file;
	}

	// Independent, deliberately uncompressed records for graph/transform tests.
	struct Graph {
		bool r2000{true};
		std::map<unsigned,std::vector<std::uint8_t>> objects;
		bool modern{};
		unsigned space{2};
		void Finish(unsigned id, Bits& b, size_t sizeAt, std::function<void(Bits&)> handles) {
			b.PatchRaw(sizeAt, static_cast<std::uint32_t>(b.position)); handles(b);
			std::vector<std::uint8_t> result; Word(result, static_cast<unsigned>(b.data.size()));
			result.insert(result.end(),b.data.begin(),b.data.end()); Word(result,Crc(result));
			objects[id] = std::move(result);
		}
		void Layer(unsigned id, std::string_view name, int color = 3, std::optional<std::uint32_t> rgb = {},unsigned lineType=0) {
			Bits b; b.Short(0x33); size_t sizeAt = b.position;
			if (r2000) b.Raw(0,4);
			b.Handle(id); b.Short(0);
			if (!r2000) { sizeAt=b.position; b.Raw(0,4); }
			b.Put(2,2); if(modern)b.Put(1,1); b.Text(name); b.Put(0,1); b.Short(0); b.Put(0,1);
			if (r2000) b.Short(16 | (31<<5)); else b.Put(0,4);
			b.Short(modern?0:color);
			if(modern){b.Long(rgb?0xc2000000|*rgb:0xc3000000|static_cast<unsigned>(color));b.Raw(0,1);}
			Finish(id,b,sizeAt,[&](Bits& h) { auto n=(r2000?5:4)-(modern?1:0);for (int i=0;i<n;++i) h.Handle(i+1==n?lineType:0); });
		}
		void Entity(unsigned type, unsigned id, unsigned owner, std::function<void(Bits&)> geometry,
			std::vector<unsigned> refs = {}, unsigned next = 0, unsigned layer = 1, int color = 0,
			std::optional<std::uint32_t> rgb = {}, std::optional<std::uint32_t> transparency = {}) {
			Bits b; b.Short(type); size_t sizeAt=b.position;
			if (r2000) b.Raw(0,4);
			b.Handle(id); b.Short(0); b.Put(0,1);
			if (!r2000) { sizeAt=b.position; b.Raw(0,4); }
			b.Put(owner?0:space,2); b.Put(2,2);
			if(modern)b.Put(1,1);
			if (!r2000) b.Put(1,1);
			if(!modern)b.Put(0,1);
			b.Short(color|(rgb?0x8000:0)|(transparency?0x2000:0));
			if(rgb)b.Long(*rgb);if(transparency)b.Long(*transparency);b.Double(1.);
			if (r2000) b.Put(0,4);
			b.Short(0); if (r2000) b.Raw(30,1);
			geometry(b);
			Finish(id,b,sizeAt,[&](Bits& h) {
				if (owner) h.Handle(owner);
				if(!modern)h.Handle(0);
				if (!r2000) h.Handle(layer);
				if(!modern){h.Handle(0); h.Handle(next);}
				if (r2000) h.Handle(layer);
				for (auto ref : refs) h.Handle(ref);
			});
		}
		void Block(unsigned id, unsigned first, unsigned last, std::array<double,3> base = {}) {
			Bits b; b.Short(0x31); size_t sizeAt=b.position;
			if (r2000) b.Raw(0,4);
			b.Handle(id); b.Short(0);
			if (!r2000) { sizeAt=b.position; b.Raw(0,4); }
			b.Put(2,2); if(modern)b.Put(1,1); b.Text("BLOCK"); b.Put(0,1); b.Short(0); b.Put(0,5);
			if (r2000) b.Put(0,1);
			if(modern)b.Long(first?(first==last?1:2):0);
			b.Point(base); b.Text("");
			if (r2000) { b.Raw(0,1); b.Text(""); b.Put(2,2); }
			Finish(id,b,sizeAt,[&](Bits& h) {
				h.Handle(0); if(!modern)h.Handle(0); h.Handle(0); h.Handle(id+1);
				if(!modern){h.Handle(first);h.Handle(last);}else if(first){h.Handle(first);if(first!=last)h.Handle(last);}
				h.Handle(id+2); if(r2000) h.Handle(0);
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
				if(modern)b.Long(2);
			},{4,7,8});
			for (unsigned id : {4u,7u}) Entity(three?0x0b:0x0a,id,vertexOwner,[&](Bits& b) {
				b.Raw(0,1);b.Point({double(id),double(id+1),double(id+2)});
				if(!three) {b.Double(-2.);b.Double(id==4?.5:0.);b.Double(0.);}
			},{},id==4?next:8);
			Entity(6,8,2,[](Bits&){});
		}
		std::vector<std::uint8_t> Bytes() const {return Container(objects,r2000);}
		std::map<std::string,std::vector<std::uint8_t>> Sections(bool customClass=false) const {
			std::map<std::string,std::vector<std::uint8_t>> sections;
			auto& data=sections["AcDb:AcDbObjects"];data={0xca,0x0d,0,0};
			std::vector<std::uint8_t> map(2);unsigned previousHandle{},previousOffset{};
			for(auto const& [id,record]:objects) {
				auto offset=static_cast<unsigned>(data.size());Modular(map,id-previousHandle);Modular(map,offset-previousOffset);
				data.insert(data.end(),record.begin(),record.end());previousHandle=id;previousOffset=offset;
			}
			map[0]=static_cast<std::uint8_t>(map.size()>>8);map[1]=static_cast<std::uint8_t>(map.size());Word(map,Crc(map),true);
			std::vector<std::uint8_t> end{0,2};Word(end,Crc(end),true);map.insert(map.end(),end.begin(),end.end());sections["AcDb:Handles"]=map;
			std::vector<std::uint8_t> classes{0x8d,0xa1,0xc4,0xb8,0xc4,0xa9,0xf8,0xc5,0xc0,0xdc,0xf4,0x5f,0xe7,0xcf,0xb6,0x8a};
			Bits body;body.Short(customClass?500:499);body.Raw(0,2);body.Put(1,1);
			if(customClass){body.Short(500);body.Short(0);body.Text("GTL");body.Text("TestClass");body.Text("TEST_CLASS");body.Put(0,1);body.Short(0x1f3);body.Long(0);body.Long(0x10000);body.Long(0x20000);body.Long(0);body.Long(0);}
			classes.resize(20);LongAt(classes,16,static_cast<unsigned>(body.data.size()));
			classes.insert(classes.end(),body.data.begin(),body.data.end());Word(classes,Crc({classes.begin()+16,classes.end()}));
			for(size_t i=0;i<16;++i)classes.push_back(static_cast<std::uint8_t>(~classes[i]));
			classes.resize(classes.size()+8);sections["AcDb:Classes"]=classes;
			return sections;
		}
	};
	// Independently encoded split data/string/handle streams for R2007 and later.
	inline auto UnicodeSections(unsigned year, std::u16string_view name=u"\uac00\U0001f642", bool badStringSize=false,
		unsigned entityType=0x12, std::function<void(Bits&)> geometry={},std::function<void(Bits&)> entityStrings={},std::vector<unsigned> entityHandles={}) {
		Graph graph;
		for (unsigned id : {1u,2u}) {
			bool layer=id==1;
			Bits data,strings;
			if(year>=2010){data.Put(0,2);data.Raw(layer?0x33:entityType,1);}
			else data.Short(layer?0x33:entityType);
			auto endAt=data.position;
			if(year==2007)data.Raw(0,4);
			data.Handle(id);data.Short(0);
			if(!layer){data.Put(0,1);data.Put(2,2);}
			data.Long(0);data.Put(1,1);
			if(year>=2013)data.Put(0,1);
			if(layer) {
				strings.Short(static_cast<int>(name.size()));for(auto c:name)strings.Raw(c,2);
				data.Short(0);data.Short(16|(31<<5));data.Short(0);data.Long(0xc2123456);data.Raw(0,1);
			} else {
				data.Short(256);data.Double(1);data.Put(0,6);data.Raw(0,1);
				if(year>=2010)data.Put(0,3);
				data.Short(0);data.Raw(29,1);
				if(geometry)geometry(data);
				else {data.Point({10,20,0});data.Double(5);data.Put(3,2);}
				if(entityStrings)entityStrings(strings);
			}
			for(size_t bit=0;bit<strings.position;++bit)data.Put((strings.data[bit/8]>>(7-bit%8))&1,1);
			if(strings.position)data.Raw(badStringSize?0x7fff:strings.position,2);
			data.Put(strings.position!=0,1);
			auto end=data.position;
			if(year==2007)data.PatchRaw(endAt,static_cast<unsigned>(end));
			if(layer)for(unsigned i=0;i<5;++i)data.Handle(0);
			else {data.Handle(1);for(auto handle:entityHandles)data.Handle(handle);}
			std::vector<std::uint8_t> record;Word(record,static_cast<unsigned>(data.data.size()));
			if(year>=2010)Modular(record,static_cast<unsigned>(data.data.size()*8-end));
			record.insert(record.end(),data.data.begin(),data.data.end());Word(record,Crc(record));graph.objects[id]=record;
		}
		auto sections=graph.Sections();
		auto& classes=sections["AcDb:Classes"];
		classes.resize(year>=2010?24:20);
		Bits body;body.Raw(0,4);body.Short(499);body.Raw(0,2);body.Put(1,1);body.Put(0,1);
		body.PatchRaw(0,static_cast<unsigned>(body.position));LongAt(classes,16,static_cast<unsigned>(body.data.size()));
		if(year>=2010)LongAt(classes,20,0);
		classes.insert(classes.end(),body.data.begin(),body.data.end());Word(classes,Crc({classes.begin()+16,classes.end()}));
		for(size_t i=0;i<16;++i)classes.push_back(static_cast<std::uint8_t>(~classes[i]));classes.resize(classes.size()+8);
		return sections;
	}
}

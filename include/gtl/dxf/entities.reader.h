#pragma once

#include "gtl/dxf/aliases.h"

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::dxf::entities {

	//=============================================================================================================================
	template < typename TField, bool bCheckMarker >
	bool ReadFieldSingleMember(TField& field, group_iter_t& iter, size_t& index) {
		// direct access to entity members
		constexpr static size_t nMemberSize = gtl::CountStructMember<TField>();
		bool bFound = ForEachIntSeq<nMemberSize>(
			[&]<int I>{
			auto& v = gtl::GetStructMember<I>(field);
			using member_t = gtl::struct_member_t<I, TField>;
			if constexpr (requires (member_t m) { m.IsGroupCodeMatch; }) {			// member with group code
				if (!member_t::IsGroupCodeMatch(iter->eCode))
					return false;
				index -= std::min(index, v.CountStoredVars());
				if (index > 0) {
					index--;
					return false;
				}
				if constexpr (member_t::IsFirstGroupCodeMatch(eGROUP_CODE::subclass)) {
					if (v.Compare(*iter) == 0) {
						iter++;
						return true;
					}
					if (!v.value.empty())
						return false;
				}
				if (!v.SetFromGroup(field, iter))
					return false;
				return true;
			}
			else if constexpr (requires (member_t m) { m.marker; m.marker.str; }) {
				//static sGroup const groupMarker{eGROUP_CODE::subclass, v.marker.str};
				//if (*iter == groupMarker)
				return ReadFieldMembers<member_t, bCheckMarker>(v, iter);
			}
			return false;
		});
		return bFound;
	}
	//-----------------------------------------------------------------------------------------------------------------------------

	template < eGROUP_CODE enclosure >
	bool ReadEnclosedData(std::string& name, groups_t& data, group_iter_t& iter) {
		if (iter->eCode != enclosure)
			return false;
		name = iter->GetValue<string_t>().value_or("");
		if (name.starts_with('{'))
			name = name.substr(1);
		else
			return false;
		for (iter++; iter; iter++) {
			auto const& r = *iter;
			if (r.eCode == enclosure) {
				iter++;
				return true;
			}
			data.push_back(*iter);
		}
		return false;
	}

	template < typename TField, bool bCheckMarker >
	bool ReadFieldMembers(TField& field, group_iter_t& iter) {
		// Read Marker (or first value)
		const auto iter0 = iter;
		if constexpr (requires (TField field) { field.marker; field.marker.str; }) {
			static sGroup const groupMarker{eGROUP_CODE::subclass, field.marker.str};
			if (*iter == groupMarker) {
				iter++;
			}
			else {
				if constexpr (bCheckMarker and field.marker.str[0])
					return false;
				else {
					//if (iter->eCode == eGROUP_CODE::subclass and (field.marker.str != ""sv))
					//	iter++;
				}
			}
		}
		if constexpr (gtl::CountStructMember<TField>() > 0) {
			std::map<eGROUP_CODE, int> mapGroupCodeToIndex; // for duplicated group number
			for (; iter; ) {
				// custom reading (after)
				if constexpr ( requires(TField field) { field.PreRead; }) {
					if (field.PreRead(iter))
						continue;
				}

				// Main
				size_t index = mapGroupCodeToIndex[iter->eCode]++;
				if (ReadFieldSingleMember<TField, bCheckMarker>(field, iter, index))
					continue;

				// control data
				if constexpr (requires (TField field) { field.controls; }) {
					if (iter->eCode == eGROUP_CODE::control) {
						field.controls.emplace_back();
						auto& item = field.controls.back();
						if (!ReadEnclosedData<eGROUP_CODE::control>(item.name, item.data, iter))
							return false;
						continue;
					}
				}

				// xdata
				if constexpr (requires (TField field) { field.xdata; }) {
					if (iter->eCode == eGROUP_CODE::xdata) {
						std::string dummy;
						if (!ReadEnclosedData<eGROUP_CODE::xdata>(field.xdata.name, field.xdata.data, iter))
							return false;
						continue;
					}
				}

				// custom reading (after)
				if constexpr ( requires(TField field) { field.PostRead; }) {
					if (field.PostRead(iter))
						continue;
				}

				// final
				if constexpr (requires(TField field) { field.unknowns_; }) {
					// Check if control codes
					if (auto eCode = iter->eCode;
						eCode == eGROUP_CODE::entity
						or eCode == eGROUP_CODE::table_entry
						or eCode == eGROUP_CODE::subclass)
					{
						break;
					}
					field.unknowns_.push_back(*iter);
					iter++;
					continue;
				}
				if (iter->eCode >= eGROUP_CODE::extended) {
					iter++;
					continue;
				}
				// still embedded
				if (iter->eCode == 101 or iter->eCode == 102) {	// skip embedded objects
					for (iter++; iter; iter++) {
						if (IsValueAnyOf(iter->eCode, 100, 102, 0, 2))
							break;
					}
				}
				break;
			}
		}
		if (iter == iter0)
			return false;
		return true;
	}

}	// namespace gtl::dxf::entities

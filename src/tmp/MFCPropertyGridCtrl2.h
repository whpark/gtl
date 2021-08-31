//=================================================================================================================================
// From Mocha.
// PWH. 2015.08.30.
//

#pragma once

#define AFX_EXT_CLASS_MISC

#include <cstdint>
#include <memory>
#include <vector>
#include <format>
#include "gtl/gtl.h"

#include <AFXPropertyGridCtrl.h>


//=================================================================================================================================
// CPropertyGridCtrlEx

class AFX_EXT_CLASS_MISC CMFCPropertyGridCtrl2 : public CMFCPropertyGridCtrl {
	DECLARE_DYNAMIC(CMFCPropertyGridCtrl2)

public:
	CMFCPropertyGridCtrl2();
	virtual ~CMFCPropertyGridCtrl2();

public:
	void SetPropertyColumnWidth(int nWidth) { m_nLeftColumnWidth = nWidth; }	// Get 함수는 Parent 에 있음.
	CMFCPropertyGridProperty* AddGroupedProperty(LPCTSTR pszGroup, LPCTSTR pszName, const COleVariant& value, LPCTSTR pszDescription = NULL, BOOL bAllowEdit = TRUE);
	CMFCPropertyGridProperty* FindItemByName(LPCTSTR pszGroup, LPCTSTR pszName);
	CMFCPropertyGridProperty* FindItemByName(LPCTSTR pszName, CMFCPropertyGridProperty* pPropParent = NULL, BOOL bSearchInChild = FALSE);

protected:
	DECLARE_MESSAGE_MAP()
};


//=================================================================================================================================

//---------------------------------------------------------------------------------------------------------------------------------
// Folder Picker (GridProperty)
class CMFCPropertyGridFolderProperty : public CMFCPropertyGridFileProperty {
public:
	CMFCPropertyGridFolderProperty(const CString& strName, const CString& strFolderName, DWORD_PTR dwData = 0, LPCTSTR lpszDescr = NULL) :
		CMFCPropertyGridFileProperty(strName, strFolderName, dwData, lpszDescr)
	{
	}
	virtual ~CMFCPropertyGridFolderProperty() {
	}

	// Overrides
public:
	virtual void OnClickButton(CPoint point) {
		TRACE("point : %d, %d\n", point.x, point.y);
		CFolderPickerDialog dlg((_bstr_t)variant_t(m_varValue), OFN_NOCHANGEDIR | /*OFN_EXPLORER | */OFN_ENABLESIZING, nullptr, 0, false);
		if (dlg.DoModal() != IDOK)
			return;
		SetValue(variant_t(dlg.GetPathName()));
	}

};

//---------------------------------------------------------------------------------------------------------------------------------
// Simple Min/Max
template < typename T >
class TMFCPropertyGridPropertyMinMax: public CMFCPropertyGridProperty {
public:
	using base_t = CMFCPropertyGridProperty;
public:
	T m_min{0}, m_max{0};

	TMFCPropertyGridPropertyMinMax(const CString& strName, T& value, T min, T max, LPCTSTR lpszDescr = nullptr, DWORD_PTR dwData = 0,
								   LPCTSTR lpszEditMask = nullptr, LPCTSTR lpszEditTemplate = nullptr, LPCTSTR lpszValidChars = nullptr)
		: CMFCPropertyGridProperty(strName, _variant_t(value), lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars),
		m_min(min), m_max(max)
	{
	}
	virtual ~TMFCPropertyGridPropertyMinMax() {
	}

public:
	virtual void SetMinMax(T min, T max) { m_min = min; m_max = max; }

	// Overriding
public:
	virtual BOOL OnUpdateValue() {
		if (m_min >= m_max)
			return base_t::OnUpdateValue();

		_variant_t v0 = GetValue();
		base_t::OnUpdateValue();
		_variant_t v1 = GetValue();
		T value = _variant_t(v1);
		if (value < m_min) { SetValue(_variant_t(m_min)); }
		if (value > m_max) { SetValue(_variant_t(m_max)); }
		return true;
	}
};


//---------------------------------------------------------------------------------------------------------------------------------
// Formatted Property. (double, int,... --> CString)

namespace misc_internal {
	template < bool bInteger, bool bSigned, int nSize >
	class TNumberVariableFormat {
	public:
		//LPCTSTR GetFormat() const;
		template < typename T > T GetValue(LPCTSTR psz);
	};
	//template <> inline LPCTSTR TNumberVariableFormat<true,   true,  2>::GetFormat() const { return _T("%hd");	};
	//template <> inline LPCTSTR TNumberVariableFormat<true,  false,  2>::GetFormat() const { return _T("%hu");	};
	//template <> inline LPCTSTR TNumberVariableFormat<true,   true,  4>::GetFormat() const { return _T("%d");	};
	//template <> inline LPCTSTR TNumberVariableFormat<true,  false,  4>::GetFormat() const { return _T("%u");	};
	//template <> inline LPCTSTR TNumberVariableFormat<true,   true,  8>::GetFormat() const { return _T("%I64d");	};
	//template <> inline LPCTSTR TNumberVariableFormat<true,  false,  8>::GetFormat() const { return _T("%I64u");	};
	//template <> inline LPCTSTR TNumberVariableFormat<false,  true,  4>::GetFormat() const { return _T("%g");	};
	//template <> inline LPCTSTR TNumberVariableFormat<false,  true,  8>::GetFormat() const { return _T("%g");	};

	template <> template < typename T > T TNumberVariableFormat<true,   true,  2>::GetValue(LPCTSTR psz) { return _tcstol(psz, nullptr, 0); }
	template <> template < typename T > T TNumberVariableFormat<true,  false,  2>::GetValue(LPCTSTR psz) { return _tcstoul(psz, nullptr, 0); }
	template <> template < typename T > T TNumberVariableFormat<true,   true,  4>::GetValue(LPCTSTR psz) { return _tcstol(psz, nullptr, 0); }
	template <> template < typename T > T TNumberVariableFormat<true,  false,  4>::GetValue(LPCTSTR psz) { return _tcstoul(psz, nullptr, 0); }
	template <> template < typename T > T TNumberVariableFormat<true,   true,  8>::GetValue(LPCTSTR psz) { return _tcstoi64(psz, nullptr, 0); }
	template <> template < typename T > T TNumberVariableFormat<true,  false,  8>::GetValue(LPCTSTR psz) { return _tcstoui64(psz, nullptr, 0); }
	template <> template < typename T > T TNumberVariableFormat<false,  true,  4>::GetValue(LPCTSTR psz) { return _tcstod(psz, nullptr); }
	template <> template < typename T > T TNumberVariableFormat<false,  true,  8>::GetValue(LPCTSTR psz) { return _tcstod(psz, nullptr); }
};

template < typename T, bool t_bInteger = ( T(0.1) == 0), bool t_bSigned = ( T(-1) < 0 ), int t_cSize = (int)sizeof(T) >
class TMFCPropertyGridPropertyFormattedValue : public CMFCPropertyGridProperty {
public:
	static const bool m_bInteger = t_bInteger;
	static const bool m_bSigned  = t_bSigned;
	static const int m_cSize    = t_cSize;
	using base_t = CMFCPropertyGridProperty;
public:
	T m_min{0}, m_max{0};
	CString m_strFormat;
	using CNumberFormat = misc_internal::TNumberVariableFormat<t_bInteger, t_bSigned, t_cSize>;

	TMFCPropertyGridPropertyFormattedValue(const CString& strName, T& valueDefault, LPCTSTR pszFormat = nullptr, LPCTSTR lpszDescr = nullptr, DWORD_PTR dwData = 0,
										LPCTSTR lpszEditMask = nullptr, LPCTSTR lpszEditTemplate = nullptr, LPCTSTR lpszValidChars = nullptr)
		: CMFCPropertyGridProperty(strName, _variant_t(CString(_T(""))), lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars), m_strFormat(pszFormat)
	{
		//if (m_strValidChars.IsEmpty())
		//	m_strValidChars = t_bInteger ? _T("0123456789+- um") : _T("0123456789+-.e um");
		//if (m_strFormat.IsEmpty())
		//	m_strFormat = CNumberFormat().GetFormat();
		auto str = CString(std::format(_T("{}"), valueDefault).c_str());
		//CString str;
		//str.Format(m_strFormat, valueDefault);
		SetOriginalValue(str);
		SetValue(str);
	}
	virtual ~TMFCPropertyGridPropertyFormattedValue() {
	}

public:
	virtual void SetMinMax(T min, T max) { m_min = min; m_max = max; }
	virtual T GetValueT() {
		CNumberFormat fmt;
		return fmt.GetValue<T>(CString(m_varValue));
	}
	virtual void SetValueT(const T& v) {
		base_t::SetValue(CString(std::format(_T("{}"), v).c_str())/*Format(m_strFormat, v)*/);
	}

	// Overriding
public:
	virtual BOOL OnUpdateValue() {
		base_t::OnUpdateValue();
		CString str(GetValue());
		T value = CNumberFormat().GetValue<T>(str);
		if (m_min < m_max) {
			if (value < m_min) { value = m_min; }
			else if (value > m_max) { value = m_max; }
		}
		SetValueT(value);

		return true;
	}
};

//=================================================================================================================================
// Property Helper... EXPORT 하지 않고, header 그대로 가져가서 사용할 수 있게...

//---------------------------------------------------------------------------------------------------------------------------------

class IPropItemBase {
public:
	CMFCPropertyGridProperty* m_pProperty = nullptr;

	IPropItemBase(CMFCPropertyGridProperty* pProperty = nullptr) : m_pProperty(pProperty) { }

	virtual bool Sync(bool bStoreToVar) = 0;
};

template < typename T >
class TPropItemSimple : public IPropItemBase {
public:
	T& m_value;

	TPropItemSimple(CMFCPropertyGridProperty* pProperty, T& value) : IPropItemBase(pProperty), m_value(value) {}

	virtual bool Sync(bool bStoreToVar) {
		if (!m_pProperty)
			return false;
		if (bStoreToVar) {
			m_value = _variant_t(m_pProperty->GetValue());
		} else {
			const auto& v = m_pProperty->GetValue();
			variant_t var(m_value);
			if (var.vt == v.vt) {
				m_pProperty->SetValue(var);
			} else {
				_variant_t varDest;
				varDest.ChangeType(v.vt, &var);
				m_pProperty->SetValue(varDest);
			}
		}
		return true;
	}
};

using CPropItemString = TPropItemSimple<CString>;
using CPropItemLong = TPropItemSimple<long>;
using CPropItemInt = TPropItemSimple<intptr_t>;
using CPropItemUInt = TPropItemSimple<uintptr_t>;
using CPropItemInt64 = TPropItemSimple<int64_t>;
using CPropItemUInt64 = TPropItemSimple<uint64_t>;
using CPropItemFloat = TPropItemSimple<float>;
using CPropItemDouble = TPropItemSimple<double>;
using CPropItemBool = TPropItemSimple<bool>;

template < typename T >
class TPropItemFormatted : public IPropItemBase {
public:
	T& m_value;
	TMFCPropertyGridPropertyFormattedValue<T>* m_pProp = nullptr;

	TPropItemFormatted(TMFCPropertyGridPropertyFormattedValue<T>* pProperty, T& value) : IPropItemBase(pProperty), m_value(value), m_pProp(pProperty) {}

	virtual bool Sync(bool bStoreToVar) {
		if (!m_pProp)
			return false;
		if (bStoreToVar) {
			m_value = m_pProp->GetValueT();
		} else {
			m_pProp->SetValueT(m_value);
		}
		return true;
	}
};

using CPropItemFormattedLong	= TPropItemFormatted<long>;
using CPropItemFormattedInt		= TPropItemFormatted<intptr_t>;
using CPropItemFormattedUInt	= TPropItemFormatted<uintptr_t>;
using CPropItemFormattedInt64	= TPropItemFormatted<int64_t>;
using CPropItemFormattedUInt64	= TPropItemFormatted<uint64_t>;
using CPropItemFormattedFloat	= TPropItemFormatted<float>;
using CPropItemFormattedDouble	= TPropItemFormatted<double>;

//-----------------------------------------------------------------------------
class CPropItemGroup : public IPropItemBase {
public:
	std::vector<std::unique_ptr<IPropItemBase>> m_subItems;

	using IPropItemBase::IPropItemBase;

	virtual bool Sync(bool bStoreToVar) {
		if (!m_pProperty)
			return false;
		int n = m_pProperty->GetSubItemsCount();
		for (int i = 0; i < n; i++) {
			auto* pSubItem = m_pProperty->GetSubItem(i);
			if (!pSubItem)
				continue;
		#if ((_MSC_VER >= _MSC_VER_VS2017) && (_MSC_VER < _MSC_VER_VS2019))	// VS2017 bug... (_Get_unwrapped)
			//auto pos = std::find_if(m_subItems.begin(), m_subItems.end(), [pSubItem](const auto& iter) ->bool { return iter->m_pProperty == pSubItem; });
			//auto test = _Get_unwrapped(m_subItems.begin());
			auto pos = m_subItems.begin();
			auto end = m_subItems.end();
			for (; pos != end; pos++) {
				if ((*pos).m_pProperty == pSubItem)
					break;
			}
		#else
			auto pos = std::find_if(m_subItems.begin(), m_subItems.end(), [pSubItem](auto const& A) ->bool { return A->m_pProperty == pSubItem; });
		#endif
			//auto pos = std::find_if(m_subItems.begin(), m_subItems.end(), [&](const IPropItemBase& A) ->bool { return _tcscmp(A.m_pProperty->GetName(), pSubItem->GetName()) == 0; });
			if (pos == m_subItems.end())
				continue;
			(*pos)->Sync(bStoreToVar);
		}
		return true;
	}

	//-------------------------------------------------------------------------
	IPropItemBase* AddSubItem(IPropItemBase* pSubItem) {
		if (!m_pProperty || !pSubItem || !pSubItem->m_pProperty)
			return nullptr;
		m_pProperty->AddSubItem(pSubItem->m_pProperty);
		return m_subItems.emplace_back(pSubItem) ? pSubItem : nullptr;
	}
	CPropItemGroup* AddSubGroup(LPCTSTR pszGroupName, bool bIsValueList = false) {
		if (!m_pProperty)
			return nullptr;
		auto* pPropGroup = new CMFCPropertyGridProperty(pszGroupName, 0, bIsValueList);
		auto* pGroup = new CPropItemGroup(pPropGroup);
		return AddSubItem(pGroup) ? pGroup : nullptr;
	}
	template < typename T >
	IPropItemBase* AddSubItemValue(LPCTSTR pszName, T& value, LPCTSTR pszDescription = nullptr, LPCTSTR lpszEditMask = nullptr, LPCTSTR lpszEditTemplate = nullptr, LPCTSTR lpszValidChars = nullptr, LPCTSTR pszOptions = nullptr, bool bEditable = true) {
		CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(pszName, variant_t(value), pszDescription, 0, lpszEditMask, lpszEditTemplate, lpszValidChars);
		AddOption(pProp, pszOptions);
		if (pProp->GetValue().vt != VT_BOOL)
			pProp->AllowEdit(bEditable);
		IPropItemBase* pChild = new TPropItemSimple<T>(pProp, value);
		return AddSubItem(pChild);
	}
	template < typename T >
	IPropItemBase* AddSubItemValueMinMax(LPCTSTR pszName, T& value, T min, T max, LPCTSTR pszDescription = nullptr, LPCTSTR lpszEditMask = nullptr, LPCTSTR lpszEditTemplate = nullptr, LPCTSTR lpszValidChars = nullptr, LPCTSTR pszOptions = nullptr, bool bEditable = true) {
		auto* pProp = new CMFCPropertyGridProperty(pszName, _variant_t(value), pszDescription, 0, lpszEditMask, lpszEditTemplate, lpszValidChars);
		AddOption(pProp, pszOptions);
		if (pProp->GetValue().vt != VT_BOOL)
			pProp->AllowEdit(bEditable);

		if (TMFCPropertyGridPropertyFormattedValue<T>::m_bInteger) {
			pProp->EnableSpinControl(true, min, max);
		}

		IPropItemBase* pChild = new TPropItemSimple<T>(pProp, value);
		return AddSubItem(pChild);
	}
	template < typename T >
	IPropItemBase* AddSubItemValueFormatted(LPCTSTR pszName, T& value, LPCTSTR pszFormat = nullptr, T min = 0, T max = 0, LPCTSTR pszDescription = nullptr, LPCTSTR lpszEditMask = nullptr, LPCTSTR lpszEditTemplate = nullptr, LPCTSTR lpszValidChars = nullptr, LPCTSTR pszOptions = nullptr, bool bEditable = true) {
		auto* pProp = new TMFCPropertyGridPropertyFormattedValue<T>(pszName, value, pszFormat, pszDescription, 0, lpszEditMask, lpszEditTemplate, lpszValidChars);
		AddOption(pProp, pszOptions);
		if (pProp->GetValue().vt != VT_BOOL)
			pProp->AllowEdit(bEditable);
		pProp->SetMinMax(min, max);
		IPropItemBase* pChild = new TPropItemFormatted<T>(pProp, value);
		return AddSubItem(pChild);
	}
	IPropItemBase* AddSubItemFilePath(LPCTSTR pszName, CString& strFilePath, bool bOpenFile, LPCTSTR pszFilter = _T("All Files|*.*||"), LPCTSTR pszDescription = nullptr, LPCTSTR pszOptions = nullptr, bool bEditable = true) {
		CMFCPropertyGridProperty* pProp = new CMFCPropertyGridFileProperty(pszName, bOpenFile, strFilePath, pszDescription, 0, pszFilter, pszDescription, 0);
		AddOption(pProp, pszOptions);
		pProp->AllowEdit(bEditable);
		IPropItemBase* pChild = new CPropItemString(pProp, strFilePath);
		return AddSubItem(pChild);
	}
	IPropItemBase* AddSubItemFolder(LPCTSTR pszName, CString& strFolder, LPCTSTR pszDescription = nullptr, LPCTSTR pszOptions = nullptr, bool bEditable = true) {
		auto* pProp = new CMFCPropertyGridFolderProperty(pszName, strFolder, 0, pszDescription);
		AddOption(pProp, pszOptions);
		pProp->AllowEdit(bEditable);
		IPropItemBase* pChild = new CPropItemString(pProp, strFolder);
		return AddSubItem(pChild);
	}

protected:
	void AddOption(CMFCPropertyGridProperty* pProperty, LPCTSTR pszOptions) {
		if (!pszOptions)
			return;
		auto strs = gtl::Split(std::wstring_view(pszOptions), _T(';'));
		for (const auto& str : strs)
			pProperty->AddOption(str.c_str(), false);
	}
};

//-----------------------------------------------------------------------------
class CPropGroupSet : public std::vector<std::unique_ptr<CPropItemGroup>> {
protected:
	CMFCPropertyGridCtrl* m_pCtrl = nullptr;

public:
	void SetGridCtrl(CMFCPropertyGridCtrl* pCtrl) { m_pCtrl = pCtrl; }

	bool Sync(bool bStoreToVar) {
		if (!m_pCtrl || !*m_pCtrl)
			return false;
		auto& ctrl = *m_pCtrl;

		if (bStoreToVar)
			ctrl.EndEditItem();

		int nItem = ctrl.GetPropertyCount();

		for (int i = 0; i < nItem; i++) {
			auto* pProp = ctrl.GetProperty(i);

		#if ((_MSC_VER >= _MSC_VER_VS2017) && (_MSC_VER < _MSC_VER_VS2019))	// VS2017 bug... (_Get_unwrapped)
			auto pos = begin();
			auto e = end();
			for (; pos != e; pos++) {
				if ((*pos).m_pProperty == pProp)
					break;
			}
		#else
			auto pos = std::find_if(begin(), end(), [pProp](auto const& rself) -> bool { return (rself->m_pProperty && pProp) ? (rself->m_pProperty == pProp) : false; } );
		#endif
			//auto pos = std::find_if(begin(), end(), [&](CPropItemGroup& A) -> bool { return (A.m_pProperty && pProp) ? (_tcscmp(A.m_pProperty->GetName(), pProp->GetName()) == 0) : false; } );
			if (pos == end())
				continue;

			(*pos)->Sync(bStoreToVar);
		}

		return true;
	}

	CPropItemGroup* AddPropItemGroup(LPCTSTR pszName, bool bIsValueList = false) {
		if (!m_pCtrl || !*m_pCtrl)
			return nullptr;
		auto& ctrl = *m_pCtrl;

		auto* pPropGroup = new CMFCPropertyGridProperty(pszName, 0, bIsValueList);
		auto* pItemGroup = new CPropItemGroup(pPropGroup);

		if (!this->emplace_back(pItemGroup)) {
			delete pItemGroup;
			delete pPropGroup;
			return nullptr;
		}
		ctrl.AddProperty(pPropGroup);
		return pItemGroup;
	}

};


//=================================================================================================================================
// CPropertyGridCtrlEx

class CMFCPropertyGridVarCtrl : public CMFCPropertyGridCtrl2, protected CPropGroupSet {

public:
	CMFCPropertyGridVarCtrl() {
		SetGridCtrl(this);
	}
	virtual ~CMFCPropertyGridVarCtrl() {
	}

public:
	using CPropGroupSet::Sync;
	using CPropGroupSet::AddPropItemGroup;

protected:
	using CMFCPropertyGridCtrl2::AddProperty;		// Hiding
	using CMFCPropertyGridCtrl2::DeleteProperty;	// Hiding

public:
	void RemoveAll() {
		CPropGroupSet::clear();
		CMFCPropertyGridCtrl2::RemoveAll();
	}

#if 0	// Add / Delete Property. NOT Tested
	using CMFCPropertyGridCtrl2::AddProperty;
	int AddProperty(CMFCPropertyGridProperty* pProp, BOOL bRedraw = TRUE, BOOL bAdjustLayout = TRUE);
	BOOL DeleteProperty(CMFCPropertyGridProperty*& pProp, BOOL bRedraw = TRUE, BOOL bAdjustLayout = TRUE) {
		BOOL bResult = false;
		DeletePropItem(pProp, nullptr, bRedraw, bAdjustLayout, bResult);
		return bResult;
	}

	IPropItemBase* FindPropItem(CMFCPropertyGridProperty* pProp) {
		return FindPropItem(pProp, nullptr);
	}

protected:
	bool DeletePropItem(CMFCPropertyGridProperty*& pProp, CPropItemGroup* pPropItemGroup, BOOL bRedraw, BOOL bAdjustLayout, BOOL& bResult) {
		qtl::TUniquePtrVector<IPropItemBase>& items = pPropItemGroup ? pPropItemGroup->m_subItems : (qtl::TUniquePtrVector<IPropItemBase>&)*this;

		for (int i = 0; i < items.size(); i++) {
			auto& item = items[i];
			if (item.m_pProperty == pProp) {
				pPropItemGroup->m_subItems.Delete(i);
				i--;
				bResult = CMFCPropertyGridCtrl2::DeleteProperty(pProp, bRedraw, bAdjustLayout);
				return true;
			}
			CPropItemGroup* pGroup = dynamic_cast<CPropItemGroup*>(&item);
			if (pGroup && DeletePropItem(pProp, pGroup, bRedraw, bAdjustLayout, bResult))
				return true;
		}
		return false;
	}

	IPropItemBase* FindPropItem(CMFCPropertyGridProperty* pProp, CPropItemGroup* pPropItemGroup) {
		qtl::TUniquePtrVector<IPropItemBase>& items = pPropItemGroup ? pPropItemGroup->m_subItems : (qtl::TUniquePtrVector<IPropItemBase>&)*this;

		for (int i = 0; i < items.size(); i++) {
			auto& item = items[i];
			if (item.m_pProperty == pProp)
				return &item;

			CPropItemGroup* pGroup = dynamic_cast<CPropItemGroup*>(&item);
			if (!pGroup) continue;
			IPropItemBase* pResult = FindPropItem(pProp, pGroup);
			if (pResult)
				return pResult;
		}
		return nullptr;
	}


#endif	// Add / Delete Property

};

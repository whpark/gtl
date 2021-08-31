// CMFCPropertyGridCtrl2.cpp : implementation file
//

#include "pch.h"
#include "MFCPropertyGridCtrl2.h"

// CMFCPropertyGridCtrl2

IMPLEMENT_DYNAMIC(CMFCPropertyGridCtrl2, CMFCPropertyGridCtrl)

CMFCPropertyGridCtrl2::CMFCPropertyGridCtrl2() {
}

CMFCPropertyGridCtrl2::~CMFCPropertyGridCtrl2() {
}

BEGIN_MESSAGE_MAP(CMFCPropertyGridCtrl2, CMFCPropertyGridCtrl)
END_MESSAGE_MAP()

// CMFCPropertyGridCtrl2 message handlers

CMFCPropertyGridProperty* CMFCPropertyGridCtrl2::AddGroupedProperty(LPCTSTR pszGroup, LPCTSTR pszName, const COleVariant& value, LPCTSTR pszDescription, BOOL bAllowEdit) {
	CMFCPropertyGridProperty* pProp = NULL;
	CMFCPropertyGridProperty* pPropParent = FindItemByName(pszGroup, NULL, FALSE);
	if (pPropParent) {
		pProp = FindItemByName(pszName, pPropParent, FALSE);
		if (pProp)
			return pProp;
	} else {
		pPropParent = new CMFCPropertyGridProperty(pszGroup, 0, FALSE);
		AddProperty(pPropParent);
	}

	pProp = new CMFCPropertyGridProperty(pszName, value, pszDescription);

	pProp->AllowEdit(bAllowEdit);
	pPropParent->AddSubItem(pProp);

	return pProp;
}

CMFCPropertyGridProperty* CMFCPropertyGridCtrl2::FindItemByName(LPCTSTR pszGroup, LPCTSTR pszName) {
	CMFCPropertyGridProperty* pProp = NULL;
	CMFCPropertyGridProperty* pPropParent = FindItemByName(pszGroup, NULL, FALSE);
	if (!pPropParent)
		return NULL;

	return FindItemByName(pszName, pPropParent, FALSE);
}
CMFCPropertyGridProperty* CMFCPropertyGridCtrl2::FindItemByName(LPCTSTR pszName, CMFCPropertyGridProperty* pPropParent, BOOL bSearchInChild) {
	if (!m_hWnd)
		return NULL;

	// Search Siblings First.

	if (pPropParent) {
		// Child Items
		int nProp = pPropParent->GetSubItemsCount();
		for (int iProp = 0; iProp < nProp; iProp++) {
			CMFCPropertyGridProperty* pProp = pPropParent->GetSubItem(iProp);
			if (!pProp)
				continue;
			if (_tcscmp(pProp->GetName(), pszName) == 0)
				return pProp;
		}
		if (!bSearchInChild)
			return NULL;
		for (int iProp = 0; iProp < nProp; iProp++) {
			CMFCPropertyGridProperty* pProp = pPropParent->GetSubItem(iProp);
			if (!pProp)
				continue;
			CMFCPropertyGridProperty* pPropChild = FindItemByName(pszName, pProp, bSearchInChild);
			if (pPropChild)
				return pPropChild;
		}
	} else {
		// Root Items
		int nProp = GetPropertyCount();
		for (int iProp = 0; iProp < nProp; iProp++) {
			CMFCPropertyGridProperty* pProp = GetProperty(iProp);
			if (!pProp)
				continue;
			if (_tcscmp(pProp->GetName(), pszName) == 0)
				return pProp;
		}
		if (!bSearchInChild)
			return NULL;
		for (int iProp = 0; iProp < nProp; iProp++) {
			CMFCPropertyGridProperty* pProp = GetProperty(iProp);
			if (!pProp)
				continue;
			CMFCPropertyGridProperty* pPropChild = FindItemByName(pszName, pProp, bSearchInChild);
			if (pPropChild)
				return pPropChild;
		}
	}
	return NULL;
}


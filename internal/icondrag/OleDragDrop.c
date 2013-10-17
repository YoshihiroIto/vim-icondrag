/**************************************************************************

	OleDragDrop.c

	(C) Copyright 1996-2002 By Tomoaki Nakashima. All right reserved.	
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

/**************************************************************************
	Include Files
**************************************************************************/

#define _INC_OLE
#include <windows.h>
#undef	_INC_OLE

#include <oleidl.h>
#include <objidl.h>

#include "OleDragDrop.h"


/* Clipboard format ���� Type of storage medium ���擾 */
static DWORD FormatToTymed(const UINT cfFormat)
{
	switch(cfFormat)
	{
	case CF_BITMAP:
		return TYMED_GDI;

	case CF_METAFILEPICT:
		return TYMED_MFPICT;

	case CF_ENHMETAFILE:
		return TYMED_ENHMF;
	}
	return TYMED_HGLOBAL;
}


/******************************************************************************

	IDropTarget

******************************************************************************/

static HRESULT STDMETHODCALLTYPE OLE_IDropTarget_QueryInterface(LPDROPTARGET pThis, REFIID riid, LPVOID *ppvObject);
static ULONG STDMETHODCALLTYPE OLE_IDropTarget_AddRef(LPDROPTARGET pThis);
static ULONG STDMETHODCALLTYPE OLE_IDropTarget_Release(LPDROPTARGET pThis);
static HRESULT STDMETHODCALLTYPE OLE_IDropTarget_DragEnter(LPDROPTARGET pThis, LPDATAOBJECT pdo, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
static HRESULT STDMETHODCALLTYPE OLE_IDropTarget_DragOver(LPDROPTARGET pThis, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
static HRESULT STDMETHODCALLTYPE OLE_IDropTarget_DragLeave(LPDROPTARGET pThis);
static HRESULT STDMETHODCALLTYPE OLE_IDropTarget_Drop(LPDROPTARGET pThis, LPDATAOBJECT pdo, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
static HRESULT APIPRIVATE OLE_IDropTarget_Internal_SendMessage(LPDROPTARGET pThis, UINT uNotify, LPDATAOBJECT pdo, DWORD grfKeyState, POINTL *ppt, LPDWORD pdwEffect);
static HRESULT APIPRIVATE DropTarget_GetData(LPDATAOBJECT pdo, UINT cfFormat, LPSTGMEDIUM psm);
static HRESULT APIPRIVATE DropTarget_QueryGetData(LPDATAOBJECT pdo, UINT cfFormat);

/* IDropTarget Virtual Table */
static IDropTargetVtbl dtv = {
	OLE_IDropTarget_QueryInterface,
	OLE_IDropTarget_AddRef,
	OLE_IDropTarget_Release,
	OLE_IDropTarget_DragEnter,
	OLE_IDropTarget_DragOver,
	OLE_IDropTarget_DragLeave,
	OLE_IDropTarget_Drop
};

typedef struct _IDROPTARGET_INTERNAL{
	LPVOID lpVtbl;
	ULONG m_refCnt;
	HWND hWnd;
	UINT uCallbackMessage;
	UINT *cFormat;
	int cfcnt;
	IDROPTARGET_NOTIFY dtn;
}IDROPTARGET_INTERNAL , *LPIDROPTARGET_INTERNAL;

/* OLE�̃h���b�v�^�[�Q�b�g�Ƃ��ēo�^���� */
BOOL APIPRIVATE OLE_IDropTarget_RegisterDragDrop(HWND hWnd, UINT uCallbackMessage, UINT *cFormat, int cfcnt)
{
	static IDROPTARGET_INTERNAL *pdti;

	pdti = GlobalAlloc(GPTR, sizeof(IDROPTARGET_INTERNAL));
	if(pdti == NULL){
		return FALSE;
	}
	pdti->lpVtbl = (LPVOID)&dtv;
	pdti->m_refCnt = 0;
	pdti->hWnd = hWnd;													/* ���b�Z�[�W���󂯎��E�B���h�E */
	pdti->uCallbackMessage = uCallbackMessage;							/* ���b�Z�[�W */
	pdti->cFormat = (UINT *)GlobalAlloc(GPTR, sizeof(UINT) * cfcnt);		/* �Ή����Ă���N���b�v�{�[�h�t�H�[�}�b�g */
	if(pdti->cFormat == NULL){
		GlobalFree(pdti);
		return FALSE;
	}
	CopyMemory(pdti->cFormat, cFormat, sizeof(UINT) * cfcnt);
	pdti->cfcnt = cfcnt;												/* �N���b�v�{�[�h�t�H�[�}�b�g�̐� */
	return (S_OK == RegisterDragDrop(hWnd, (LPDROPTARGET)pdti));
}

/* OLE�̃h���b�v�^�[�Q�b�g���������� */
void APIPRIVATE OLE_IDropTarget_RevokeDragDrop(HWND hWnd)
{
	RevokeDragDrop(hWnd);
}

static HRESULT STDMETHODCALLTYPE OLE_IDropTarget_QueryInterface(LPDROPTARGET pThis, REFIID riid, PVOID *ppvObject)
{
	//�v�����ꂽIID�Ɠ����ꍇ�̓I�u�W�F�N�g��Ԃ�
	if(IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IDropTarget)){
		*ppvObject = (LPVOID)pThis;
		((LPUNKNOWN)*ppvObject)->lpVtbl->AddRef((LPUNKNOWN)*ppvObject);
		return S_OK;
	}
	*ppvObject = NULL;
	return ResultFromScode(E_NOINTERFACE);
}

static ULONG STDMETHODCALLTYPE OLE_IDropTarget_AddRef(LPDROPTARGET pThis)
{
	CONST LPIDROPTARGET_INTERNAL pdti = (LPIDROPTARGET_INTERNAL)pThis;

	/* reference count���C���N�������g���� */
	pdti->m_refCnt++;
	return pdti->m_refCnt;
}

static ULONG STDMETHODCALLTYPE OLE_IDropTarget_Release(LPDROPTARGET pThis)
{
	CONST LPIDROPTARGET_INTERNAL pdti = (LPIDROPTARGET_INTERNAL)pThis;

	/* reference count���f�N�������g���� */
	pdti->m_refCnt--;

	/* reference count�� 0 �ɂȂ����ꍇ�̓I�u�W�F�N�g�̉�����s�� */
	if(pdti->m_refCnt == 0L){
		if(pdti->cFormat != NULL){
			GlobalFree(pdti->cFormat);
		}
		GlobalFree(pdti);
		return 0L;
	}
	return pdti->m_refCnt;
}

static HRESULT STDMETHODCALLTYPE OLE_IDropTarget_DragEnter(LPDROPTARGET pThis, LPDATAOBJECT pdo, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
	return OLE_IDropTarget_Internal_SendMessage(pThis, IDROPTARGET_NOTIFY_DRAGENTER, pdo, grfKeyState, &pt, pdwEffect);
}

static HRESULT STDMETHODCALLTYPE OLE_IDropTarget_DragOver(LPDROPTARGET pThis, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
	return OLE_IDropTarget_Internal_SendMessage(pThis, IDROPTARGET_NOTIFY_DRAGOVER, NULL, grfKeyState, &pt, pdwEffect);
}

static HRESULT STDMETHODCALLTYPE OLE_IDropTarget_DragLeave(LPDROPTARGET pThis)
{
	return OLE_IDropTarget_Internal_SendMessage(pThis, IDROPTARGET_NOTIFY_DRAGLEAVE, NULL, 0, NULL, NULL);
}

static HRESULT STDMETHODCALLTYPE OLE_IDropTarget_Drop(LPDROPTARGET pThis, LPDATAOBJECT pdo, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
	return OLE_IDropTarget_Internal_SendMessage(pThis, IDROPTARGET_NOTIFY_DROP, pdo, grfKeyState, &pt, pdwEffect);
}

static HRESULT APIPRIVATE OLE_IDropTarget_Internal_SendMessage(LPDROPTARGET pThis, UINT uNotify, LPDATAOBJECT pdo, DWORD grfKeyState, POINTL *ppt, LPDWORD pdwEffect)
{
	CONST LPIDROPTARGET_INTERNAL pdti = (LPIDROPTARGET_INTERNAL)pThis;
	CONST LPIDROPTARGET_NOTIFY pdtn = &(pdti->dtn);
	STGMEDIUM sm;
	UINT cfFormat = 0;
	int i;

	if(pdo){
		/* �Ή����Ă���N���b�v�{�[�h�t�H�[�}�b�g�����邩���ׂ� */
		for(i = 0;i < pdti->cfcnt;i++){
			if(DropTarget_QueryGetData(pdo, pdti->cFormat[i]) == S_OK){
				cfFormat = pdti->cFormat[i];
				break;
			}
		}
		/* �N���b�v�{�[�h�t�H�[�}�b�g����f�[�^���擾���� */
		if(cfFormat != 0){
			if (DropTarget_GetData(pdo, cfFormat, &sm) != S_OK){
				cfFormat = 0;
			}
		}
	}
	pdtn->ppt = ppt;					/* �}�E�X�|�C���^�̈ʒu */
	pdtn->grfKeyState = grfKeyState;	/* �L�[�A�}�E�X�{�^���̏�� */
	pdtn->cfFormat = cfFormat;			/* �N���b�v�{�[�h�t�H�[�}�b�g */
	pdtn->hMem = sm.hGlobal;			/* ���f�[�^ */
	pdtn->pdo = pdo;					/* IDataObject */

	/* �E�B���h�E�ɃC�x���g��ʒm���� */
	SendMessage(pdti->hWnd, pdti->uCallbackMessage, (WPARAM)uNotify, (LPARAM)pdtn);

	/* �N���b�v�{�[�h�`���̃f�[�^�̉�� */
	if(cfFormat){
		ReleaseStgMedium(&sm);
	}

	/* ���ʂ̐ݒ� */
	if(pdwEffect){
		*pdwEffect &= pdtn->dwEffect;

		if((*pdwEffect & DROPEFFECT_MOVE) && (*pdwEffect & DROPEFFECT_COPY)){
			*pdwEffect ^= DROPEFFECT_COPY;
			pdtn->dwEffect ^= DROPEFFECT_COPY;
		}
	}
	return S_OK;
}

static HRESULT APIPRIVATE DropTarget_GetData(LPDATAOBJECT pdo, UINT cfFormat, LPSTGMEDIUM psm)
{
	FORMATETC fmt;

	/* IDataObject�ɃN���b�v�{�[�h�`���̃f�[�^��v������ */
	fmt.cfFormat = cfFormat;
	fmt.ptd = NULL;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.tymed = FormatToTymed(cfFormat);
	return pdo->lpVtbl->GetData(pdo, &fmt, psm);
}

static HRESULT APIPRIVATE DropTarget_QueryGetData(LPDATAOBJECT pdo, UINT cfFormat)
{
	FORMATETC fmt;

	/* IDataObject�Ɏw��̃N���b�v�{�[�h�t�H�[�}�b�g�����݂��邩�₢���킹�� */
	fmt.cfFormat = cfFormat;
	fmt.ptd = NULL;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.tymed = FormatToTymed(cfFormat);
	return pdo->lpVtbl->QueryGetData(pdo, &fmt);
}


/******************************************************************************

	IEnumFORMATETC

******************************************************************************/

static HRESULT STDMETHODCALLTYPE OLE_IEnumFORMATETC_QueryInterface(LPENUMFORMATETC lpThis, REFIID riid, LPVOID FAR* lplpvObj);
static ULONG STDMETHODCALLTYPE OLE_IEnumFORMATETC_AddRef(LPENUMFORMATETC lpThis);
static ULONG STDMETHODCALLTYPE OLE_IEnumFORMATETC_Release(LPENUMFORMATETC lpThis);
static HRESULT STDMETHODCALLTYPE OLE_IEnumFORMATETC_Next(LPENUMFORMATETC lpThis, ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched);
static HRESULT STDMETHODCALLTYPE OLE_IEnumFORMATETC_Skip(LPENUMFORMATETC lpThis, ULONG celt);
static HRESULT STDMETHODCALLTYPE OLE_IEnumFORMATETC_Reset(LPENUMFORMATETC lpThis);
static HRESULT STDMETHODCALLTYPE OLE_IEnumFORMATETC_Clone(LPENUMFORMATETC lpThis, IEnumFORMATETC **ppenum);

/* IEnumFORMATETC Virtual Table */
static IEnumFORMATETCVtbl efv = {
	OLE_IEnumFORMATETC_QueryInterface,
	OLE_IEnumFORMATETC_AddRef,
	OLE_IEnumFORMATETC_Release,
	OLE_IEnumFORMATETC_Next,
	OLE_IEnumFORMATETC_Skip,
	OLE_IEnumFORMATETC_Reset,
	OLE_IEnumFORMATETC_Clone
};

typedef struct _IENUMFORMATETC_INTERNAL{
	LPVOID lpVtbl;
	ULONG m_refCnt;
	LPUNKNOWN m_pUnknownObj;
	ULONG m_currElement;
	ULONG m_numFormats;
	LPFORMATETC m_formatList;
}IENUMFORMATETC_INTERNAL , *LPIENUMFORMATETC_INTERNAL;

static HRESULT STDMETHODCALLTYPE OLE_IEnumFORMATETC_QueryInterface(LPENUMFORMATETC lpThis, REFIID riid, LPVOID FAR* lplpvObj)
{
	//�v�����ꂽIID�Ɠ����ꍇ�̓I�u�W�F�N�g��Ԃ�
	if(IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IEnumFORMATETC)){
		*lplpvObj = (LPVOID) lpThis;
		 ((LPUNKNOWN)*lplpvObj)->lpVtbl->AddRef(((LPUNKNOWN)*lplpvObj));
		return S_OK;
	}
	*lplpvObj = NULL;
	return ResultFromScode(E_NOINTERFACE);
}

static ULONG STDMETHODCALLTYPE OLE_IEnumFORMATETC_AddRef(LPENUMFORMATETC lpThis)
{
	CONST LPIENUMFORMATETC_INTERNAL pefi = (LPIENUMFORMATETC_INTERNAL)lpThis;

	/* reference count���C���N�������g���� */
	pefi->m_refCnt++;
	/* �e�I�u�W�F�N�g��reference count�������� */
	pefi->m_pUnknownObj->lpVtbl->AddRef(pefi->m_pUnknownObj);
	return pefi->m_refCnt;
}

static ULONG STDMETHODCALLTYPE OLE_IEnumFORMATETC_Release(LPENUMFORMATETC lpThis)
{
	CONST LPIENUMFORMATETC_INTERNAL pefi = (LPIENUMFORMATETC_INTERNAL)lpThis;

	/* reference count���f�N�������g���� */
	pefi->m_refCnt--;
	/* �e�I�u�W�F�N�g��reference count�����炷 */
	pefi->m_pUnknownObj->lpVtbl->Release(pefi->m_pUnknownObj);

	/* reference count�� 0 �ɂȂ����ꍇ�̓I�u�W�F�N�g�̉�����s�� */
	if(pefi->m_refCnt == 0L){
		if(pefi->m_formatList != NULL){
			GlobalFree(pefi->m_formatList);
		}
		GlobalFree(pefi);
		return 0L;
	}
	return pefi->m_refCnt;
}

static HRESULT STDMETHODCALLTYPE OLE_IEnumFORMATETC_Next(LPENUMFORMATETC lpThis, ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched)
{
	ULONG i;
	ULONG cReturn=0L;
	LPIENUMFORMATETC_INTERNAL lpefi = ((LPIENUMFORMATETC_INTERNAL)lpThis);

	if(pceltFetched){
		*pceltFetched = 0L;
	}
	if(lpefi->m_formatList == NULL){
		return ResultFromScode(S_FALSE);
	}

	if(rgelt == NULL){
		if(celt == 1){
			return ResultFromScode(S_FALSE);
		}else{
			return ResultFromScode(E_POINTER);
		}
	}

	if(lpefi->m_currElement >= lpefi->m_numFormats){
		return ResultFromScode(S_FALSE);
	}

	for(i = 0;i < celt && lpefi->m_currElement < lpefi->m_numFormats;i++, lpefi->m_currElement++){
		*rgelt = lpefi->m_formatList[lpefi->m_currElement];
		rgelt++;
	}

	if(pceltFetched != NULL){
		*pceltFetched = i;
	}
	return S_OK;

}

static HRESULT STDMETHODCALLTYPE OLE_IEnumFORMATETC_Skip(LPENUMFORMATETC lpThis, ULONG celt)
{
	LPIENUMFORMATETC_INTERNAL lpefi = ((LPIENUMFORMATETC_INTERNAL)lpThis);

	lpefi->m_currElement += celt;

	if(lpefi->m_currElement > lpefi->m_numFormats){
		lpefi->m_currElement = lpefi->m_numFormats;
		return ResultFromScode(S_FALSE);
	}
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE OLE_IEnumFORMATETC_Reset(LPENUMFORMATETC lpThis)
{
	LPIENUMFORMATETC_INTERNAL lpefi = ((LPIENUMFORMATETC_INTERNAL)lpThis);

	lpefi->m_currElement = 0L;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE OLE_IEnumFORMATETC_Clone(LPENUMFORMATETC lpThis, IEnumFORMATETC **ppenum)
{
	LPIENUMFORMATETC_INTERNAL pNew;
	LPIENUMFORMATETC_INTERNAL lpefi = ((LPIENUMFORMATETC_INTERNAL)lpThis);
	UINT i;

	/* IEnumFORMATETC���쐬���� */
	pNew = GlobalAlloc(GPTR, sizeof(IENUMFORMATETC_INTERNAL));
	if(pNew == NULL){
		return ResultFromScode(E_OUTOFMEMORY);
	}
	pNew->lpVtbl = (LPVOID)&efv;
	pNew->m_refCnt = 0;
	pNew->m_currElement = 0;

	pNew->m_pUnknownObj = lpefi->m_pUnknownObj;
	pNew->m_numFormats = lpefi->m_numFormats;

	/* �N���b�v�{�[�h�t�H�[�}�b�g�̃��X�g���R�s�[���� */
	pNew->m_formatList = GlobalAlloc(GPTR, sizeof(FORMATETC) * pNew->m_numFormats);
	if(pNew->m_formatList != NULL){
		for(i = 0;i < pNew->m_numFormats;i++){
			  pNew->m_formatList[i] = lpefi->m_formatList[i];
		}
	}

	*ppenum = (struct IEnumFORMATETC *)pNew;
	if(pNew == NULL){
		return ResultFromScode(E_OUTOFMEMORY);
	}
	((LPENUMFORMATETC)pNew)->lpVtbl->AddRef(((LPENUMFORMATETC)pNew));
	pNew->m_currElement = lpefi->m_currElement;
	return S_OK;
}


/******************************************************************************

	IDataObject

******************************************************************************/

static HRESULT STDMETHODCALLTYPE OLE_IDataObject_QueryInterface(LPDATAOBJECT lpThis, REFIID riid, LPVOID FAR *lplpvObj);
static ULONG STDMETHODCALLTYPE OLE_IDataObject_AddRef(LPDATAOBJECT lpThis);
static ULONG STDMETHODCALLTYPE OLE_IDataObject_Release(LPDATAOBJECT lpThis);
static HRESULT STDMETHODCALLTYPE OLE_IDataObject_GetData(LPDATAOBJECT lpThis, FORMATETC *pFormatetc, STGMEDIUM *pmedium);
static HRESULT STDMETHODCALLTYPE OLE_IDataObject_GetDataHere(LPDATAOBJECT lpThis, FORMATETC *pFormatetc, STGMEDIUM *pmedium);
static HRESULT STDMETHODCALLTYPE OLE_IDataObject_QueryGetData(LPDATAOBJECT lpThis, FORMATETC *pFormatetc);
static HRESULT STDMETHODCALLTYPE OLE_IDataObject_GetCanonicalFormatEtc(LPDATAOBJECT lpThis, FORMATETC *pFormatetcIn, FORMATETC *pFormatetcOut);
static HRESULT STDMETHODCALLTYPE OLE_IDataObject_SetData(LPDATAOBJECT lpThis, FORMATETC *pFormatetc, STGMEDIUM *pmedium, BOOL fRelease);
static HRESULT STDMETHODCALLTYPE OLE_IDataObject_EnumFormatEtc(LPDATAOBJECT lpThis, DWORD dwDirection, IEnumFORMATETC **ppenumFormatetc);
static HRESULT STDMETHODCALLTYPE OLE_IDataObject_DAdvise(LPDATAOBJECT lpThis, FORMATETC *pFormatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
static HRESULT STDMETHODCALLTYPE OLE_IDataObject_DUnadvise(LPDATAOBJECT lpThis, DWORD dwConnection);
static HRESULT STDMETHODCALLTYPE OLE_IDataObject_EnumDAdvise(LPDATAOBJECT lpThis, IEnumSTATDATA **ppenumAdvise);

/* IDataObject Virtual Table */
static IDataObjectVtbl dov = {
	OLE_IDataObject_QueryInterface,
	OLE_IDataObject_AddRef,
	OLE_IDataObject_Release,
	OLE_IDataObject_GetData,
	OLE_IDataObject_GetDataHere,
	OLE_IDataObject_QueryGetData,
	OLE_IDataObject_GetCanonicalFormatEtc,
	OLE_IDataObject_SetData,
	OLE_IDataObject_EnumFormatEtc,
	OLE_IDataObject_DAdvise,
	OLE_IDataObject_DUnadvise,
	OLE_IDataObject_EnumDAdvise
};

typedef struct _IDATAOBJECT_INTERNAL{
	LPVOID lpVtbl;
	ULONG m_refCnt;
	UINT m_numTypes;
	UINT m_maxTypes;
	FORMATETC *m_typeList;
	HWND hWnd;
	UINT uCallbackMessage;
}IDATAOBJECT_INTERNAL , *LPIDATAOBJECT_INTERNAL;


static HRESULT STDMETHODCALLTYPE OLE_IDataObject_QueryInterface(LPDATAOBJECT lpThis, REFIID riid, LPVOID FAR *lplpvObj)
{
	//�v�����ꂽIID�Ɠ����ꍇ�̓I�u�W�F�N�g��Ԃ�
	if(IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IDataObject)){
		*lplpvObj = lpThis;
		 ((LPUNKNOWN)*lplpvObj)->lpVtbl->AddRef(((LPUNKNOWN)*lplpvObj));
		return S_OK;
	}
	*lplpvObj = NULL;
	return ResultFromScode(E_NOINTERFACE);
}

static ULONG STDMETHODCALLTYPE OLE_IDataObject_AddRef(LPDATAOBJECT lpThis)
{
	CONST LPIDATAOBJECT_INTERNAL pdoi = (LPIDATAOBJECT_INTERNAL)lpThis;

	/* reference count���C���N�������g���� */
	pdoi->m_refCnt++;
	return pdoi->m_refCnt;
}

static ULONG STDMETHODCALLTYPE OLE_IDataObject_Release(LPDATAOBJECT lpThis)
{
	CONST LPIDATAOBJECT_INTERNAL pdoi = (LPIDATAOBJECT_INTERNAL)lpThis;

	/* reference count���f�N�������g���� */
	pdoi->m_refCnt--;

	/* reference count�� 0 �ɂȂ����ꍇ�̓I�u�W�F�N�g�̉�����s�� */
	if(pdoi->m_refCnt == 0L){
		if(pdoi->m_typeList != NULL){
			GlobalFree(pdoi->m_typeList);
		}
		GlobalFree(pdoi);
		return 0L;
	}
	return pdoi->m_refCnt;
}

static HRESULT STDMETHODCALLTYPE OLE_IDataObject_GetData(LPDATAOBJECT lpThis, FORMATETC *pFormatetc, STGMEDIUM *pmedium)
{
	CONST LPIDATAOBJECT_INTERNAL pdoi = (LPIDATAOBJECT_INTERNAL)lpThis;
	HGLOBAL hMem;
	UINT i;

	/* �v�����ꂽ�N���b�v�{�[�h�t�H�[�}�b�g�����݂��邩���ׂ� */
	for(i = 0;i < pdoi->m_numTypes;i++){
		if(pdoi->m_typeList[i].cfFormat == pFormatetc->cfFormat){
			break;
		}
	}
	if(i == pdoi->m_numTypes){
		/* �v�����ꂽ�N���b�v�{�[�h�t�H�[�}�b�g���T�|�[�g���ĂȂ��ꍇ */
		return ResultFromScode(DV_E_FORMATETC);
	}

	/* �E�B���h�E�Ƀf�[�^�̗v�����s�� */
	SendMessage(pdoi->hWnd, pdoi->uCallbackMessage, (WPARAM)pdoi->m_typeList[i].cfFormat, (LPARAM)&hMem);
	if(hMem == NULL){
		return ResultFromScode(STG_E_MEDIUMFULL);
	}
	pmedium->hGlobal = hMem;
	pmedium->tymed = FormatToTymed(pFormatetc->cfFormat);
	pmedium->pUnkForRelease = NULL;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE OLE_IDataObject_GetDataHere(LPDATAOBJECT lpThis, FORMATETC *pFormatetc, STGMEDIUM *pmedium)
{
	return ResultFromScode(E_NOTIMPL);
}

static HRESULT STDMETHODCALLTYPE OLE_IDataObject_QueryGetData(LPDATAOBJECT lpThis, FORMATETC *pFormatetc)
{
	CONST LPIDATAOBJECT_INTERNAL pdoi = (LPIDATAOBJECT_INTERNAL)lpThis;
	UINT i;

	/* �v�����ꂽ�N���b�v�{�[�h�t�H�[�}�b�g�����݂��邩���ׂ� */
	for(i = 0;i < pdoi->m_numTypes;i++){
		if(pdoi->m_typeList[i].cfFormat == pFormatetc->cfFormat){
			return S_OK;
		}
	}
	return ResultFromScode(DV_E_FORMATETC);
}

static HRESULT STDMETHODCALLTYPE OLE_IDataObject_GetCanonicalFormatEtc(LPDATAOBJECT lpThis, FORMATETC *pFormatetcIn, FORMATETC *pFormatetcOut)
{
	return ResultFromScode(E_NOTIMPL);
}

static HRESULT STDMETHODCALLTYPE OLE_IDataObject_SetData(LPDATAOBJECT lpThis, FORMATETC *pFormatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
	return ResultFromScode(E_NOTIMPL);
}

static HRESULT STDMETHODCALLTYPE OLE_IDataObject_EnumFormatEtc(LPDATAOBJECT lpThis, DWORD dwDirection, IEnumFORMATETC **ppenumFormatetc)
{
	CONST LPIDATAOBJECT_INTERNAL pdoi = (LPIDATAOBJECT_INTERNAL)lpThis;
	static IENUMFORMATETC_INTERNAL *pefi;
	UINT i;

	if(ppenumFormatetc == NULL){
		return ResultFromScode(E_INVALIDARG);
	}

	if(dwDirection != DATADIR_GET){
		*ppenumFormatetc = NULL;
		return ResultFromScode(E_NOTIMPL);
	}

	/* IEnumFORMATETC���쐬���� */
	pefi = GlobalAlloc(GPTR, sizeof(IENUMFORMATETC_INTERNAL));
	if(pefi == NULL){
		return E_OUTOFMEMORY;
	}
	pefi->lpVtbl = (LPVOID)&efv;
	pefi->m_refCnt = 0;
	pefi->m_currElement = 0;
	pefi->m_pUnknownObj = (struct IUnknown *)lpThis;
	pefi->m_numFormats = pdoi->m_numTypes;

	/* �N���b�v�{�[�h�t�H�[�}�b�g�̃��X�g���R�s�[���� */
	pefi->m_formatList = GlobalAlloc(GPTR, sizeof(FORMATETC) * pefi->m_numFormats);
	if(pefi->m_formatList != NULL){
		for(i = 0;i < pefi->m_numFormats;i++){
			  pefi->m_formatList[i] = pdoi->m_typeList[i];
		}
	}

	((LPENUMFORMATETC)pefi)->lpVtbl->AddRef(((LPENUMFORMATETC)pefi));

	*ppenumFormatetc = (struct IEnumFORMATETC *)pefi;
	if(*ppenumFormatetc == NULL){
		return E_OUTOFMEMORY;
	}
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE OLE_IDataObject_DAdvise(LPDATAOBJECT lpThis, FORMATETC *pFormatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
	return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

static HRESULT STDMETHODCALLTYPE OLE_IDataObject_DUnadvise(LPDATAOBJECT lpThis, DWORD dwConnection)
{
	return ResultFromScode(OLE_E_NOCONNECTION);
}

static HRESULT STDMETHODCALLTYPE OLE_IDataObject_EnumDAdvise(LPDATAOBJECT lpThis, IEnumSTATDATA **ppenumAdvise)
{
	return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}


/******************************************************************************

	IDropSource

******************************************************************************/

static HRESULT STDMETHODCALLTYPE OLE_IDropSource_QueryInterface(LPDROPSOURCE lpThis, REFIID riid, LPVOID FAR* lplpvObj);
static ULONG STDMETHODCALLTYPE OLE_IDropSource_AddRef(LPDROPSOURCE lpThis);
static ULONG STDMETHODCALLTYPE OLE_IDropSource_Release(LPDROPSOURCE lpThis);
static HRESULT STDMETHODCALLTYPE OLE_IDropSource_QueryContinueDrag(LPDROPSOURCE lpThis, BOOL fEscapePressed, DWORD grfKeyState);
static HRESULT STDMETHODCALLTYPE OLE_IDropSource_GiveFeedback(LPDROPSOURCE lpThis, DWORD dwEffect);

/* IDropSource Virtual Table */
static IDropSourceVtbl dsv = {
	OLE_IDropSource_QueryInterface,
	OLE_IDropSource_AddRef,
	OLE_IDropSource_Release,
	OLE_IDropSource_QueryContinueDrag,
	OLE_IDropSource_GiveFeedback,
};

typedef struct _IDROPSOURCE_INTERNAL{
	LPVOID lpVtbl;
	ULONG m_refCnt;
	DWORD m_keyState;
	DWORD m_button;
}IDROPSOURCE_INTERNAL , *LPIDROPSOURCE_INTERNAL;

static HRESULT STDMETHODCALLTYPE OLE_IDropSource_QueryInterface(LPDROPSOURCE lpThis, REFIID riid, LPVOID FAR *lplpvObj)
{
	//�v�����ꂽIID�Ɠ����ꍇ�̓I�u�W�F�N�g��Ԃ�
	if(IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IDropSource)){
		*lplpvObj = (LPVOID) lpThis;
		((LPUNKNOWN)*lplpvObj)->lpVtbl->AddRef(((LPUNKNOWN)*lplpvObj));
		return S_OK;
	}
	*lplpvObj = NULL;
	return ResultFromScode(E_NOINTERFACE);

}

static ULONG STDMETHODCALLTYPE OLE_IDropSource_AddRef(LPDROPSOURCE lpThis)
{
	CONST LPIDROPSOURCE_INTERNAL pdsi = (LPIDROPSOURCE_INTERNAL)lpThis;

	/* reference count���C���N�������g���� */
	pdsi->m_refCnt++;
	return pdsi->m_refCnt;
}

static ULONG STDMETHODCALLTYPE OLE_IDropSource_Release(LPDROPSOURCE lpThis)
{
	CONST LPIDROPSOURCE_INTERNAL pdsi = (LPIDROPSOURCE_INTERNAL)lpThis;

	/* reference count���f�N�������g���� */
	pdsi->m_refCnt--;

	/* reference count�� 0 �ɂȂ����ꍇ�̓I�u�W�F�N�g�̉�����s�� */
	if(pdsi->m_refCnt == 0L){
		GlobalFree(pdsi);
		return 0L;
	}
	return pdsi->m_refCnt;
}

static HRESULT STDMETHODCALLTYPE OLE_IDropSource_QueryContinueDrag(LPDROPSOURCE lpThis, BOOL fEscapePressed, DWORD grfKeyState)
{
	CONST LPIDROPSOURCE_INTERNAL pdsi = (LPIDROPSOURCE_INTERNAL)lpThis;

	if(fEscapePressed){
		/* �G�X�P�[�v�������ꂽ�ꍇ�̓L�����Z���ɂ��� */
		return ResultFromScode(DRAGDROP_S_CANCEL);
	}

	/* �w��̃L�[��}�E�X�������ꂽ�ꍇ�̓h���b�v�ɂ��� */
	if(pdsi->m_button == 0){
		if(grfKeyState != pdsi->m_keyState){
			return ResultFromScode(DRAGDROP_S_DROP);
		}
	}else{
		if(!(grfKeyState & pdsi->m_button)){
			return ResultFromScode(DRAGDROP_S_DROP);
		}
	}
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE OLE_IDropSource_GiveFeedback(LPDROPSOURCE lpThis, DWORD dwEffect)
{
	return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}

/* �h���b�O���h���b�v�̊J�n */
int APIPRIVATE OLE_IDropSource_Start(HWND hWnd, UINT uCallbackMessage, UINT *ClipFormtList, int cfcnt, int Effect)
{
	static IDATAOBJECT_INTERNAL *pdoi;
	static IDROPSOURCE_INTERNAL *pdsi;
	DWORD lpdwEffect;
	DWORD keyState;
	int i;
	int ret;

	/* IDataObject�̍쐬 */
	pdoi = GlobalAlloc(GPTR, sizeof(IDATAOBJECT_INTERNAL));
	if(pdoi == NULL){
		return -1;
	}
	pdoi->lpVtbl = (LPVOID)&dov;
	pdoi->m_refCnt = 0;
	pdoi->m_numTypes = cfcnt;
	pdoi->m_maxTypes = cfcnt;
	/* �L���ȃN���b�v�{�[�h�t�H�[�}�b�g��ݒ肷�� */
	pdoi->m_typeList = GlobalAlloc(GPTR, sizeof(FORMATETC) * cfcnt);
	if(pdoi->m_typeList == NULL){
		GlobalFree(pdoi);
		return -1;
	}
	for(i = 0;i < cfcnt;i++){
		pdoi->m_typeList[i].cfFormat = ClipFormtList[i];
		pdoi->m_typeList[i].ptd = NULL;
		pdoi->m_typeList[i].dwAspect = DVASPECT_CONTENT;
		pdoi->m_typeList[i].lindex = -1;
		pdoi->m_typeList[i].tymed = FormatToTymed(ClipFormtList[i]);
	}
	pdoi->hWnd = hWnd;
	pdoi->uCallbackMessage = uCallbackMessage;
	((LPDATAOBJECT)pdoi)->lpVtbl->AddRef((LPDATAOBJECT)pdoi);

	/* IDropSource�̍쐬 */
	pdsi = GlobalAlloc(GPTR, sizeof(IDROPSOURCE_INTERNAL));
	if(pdsi == NULL){
		/* IDataObject��������� */
		((LPDATAOBJECT)pdoi)->lpVtbl->Release((LPDATAOBJECT)pdoi);
		return -1;
	}
	pdsi->lpVtbl = (LPVOID)&dsv;
	pdsi->m_refCnt = 0;

	/* �L���ȃL�[�ƃ}�E�X�̏�� */
	if(GetKeyState(VK_RBUTTON) & 0x8000){
		pdsi->m_button = MK_RBUTTON;
	}else{
		pdsi->m_button = MK_LBUTTON;
	}

	/* ���݂̃L�[�ƃ}�E�X�̏�� */
	keyState = 0;
	if(GetKeyState(VK_SHIFT) & 0x8000){
		keyState |= MK_SHIFT;
	}
	if(GetKeyState(VK_CONTROL) & 0x8000){
		keyState |= MK_CONTROL;
	}
	if(GetKeyState(VK_MENU) & 0x8000){
		keyState |= MK_ALT;
	}
	if(GetKeyState(VK_LBUTTON) & 0x8000){
		keyState |= MK_LBUTTON;
	}
	if(GetKeyState(VK_MBUTTON) & 0x8000){
		keyState |= MK_MBUTTON;
	}
	if(GetKeyState(VK_RBUTTON) & 0x8000){
		keyState |= MK_RBUTTON;
	}
	pdsi->m_keyState = keyState;
	((LPDROPSOURCE)pdsi)->lpVtbl->AddRef((LPDROPSOURCE)pdsi);

	lpdwEffect = 0;

	/* �h���b�O&�h���b�v�̊J�n */
	ret = DoDragDrop((LPDATAOBJECT)pdoi, (LPDROPSOURCE)pdsi, Effect, &lpdwEffect);

	/* IDataObject��������� */
	((LPDATAOBJECT)pdoi)->lpVtbl->Release((LPDATAOBJECT)pdoi);
	/* IDropSource��������� */
	((LPDROPSOURCE)pdsi)->lpVtbl->Release((LPDROPSOURCE)pdsi);

	if(ret == DRAGDROP_S_DROP){
		/* �h���b�v��̃A�v���P�[�V�������ݒ肵�����ʂ�Ԃ� */
		return lpdwEffect;
	}
	return -1;
}
/* End of source */

/**************************************************************************

	OleDragDrop.h

	(C) Copyright 1996-2002 By Tomoaki Nakashima. All right reserved.	
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_OLEDRAGDROP
#define _INC_OLEDRAGDROP

#define IDROPTARGET_NOTIFY_DRAGENTER	0
#define IDROPTARGET_NOTIFY_DRAGOVER		1
#define IDROPTARGET_NOTIFY_DRAGLEAVE	2
#define IDROPTARGET_NOTIFY_DROP 		3

typedef struct _IDROPTARGET_NOTIFY{
	POINTL *ppt;					//�}�E�X�̈ʒu
	DWORD dwEffect;					//�h���b�O����ŁA�h���b�O�����Ώۂŋ���������
	DWORD grfKeyState;				//�L�[�̏��
	UINT cfFormat;					//�h���b�v�����f�[�^�̃N���b�v�{�[�h�t�H�[�}�b�g
	HANDLE hMem;					//�h���b�v�����f�[�^
	LPVOID pdo;						//IDataObject
}IDROPTARGET_NOTIFY , *LPIDROPTARGET_NOTIFY;




//DragTarget
BOOL APIPRIVATE OLE_IDropTarget_RegisterDragDrop(HWND hWnd, UINT uCallbackMessage, UINT *cFormat, int cfcnt);

//�h���b�O&�h���b�v�̃^�[�Q�b�g�Ƃ��ēo�^���܂��B

//[����]
//	�h���b�O&�h���b�v���삪�s��ꂽ�Ƃ��Ɏw��̃E�B���h�E�̎w��̃��b�Z�[�W�ɒʒm����܂��B
//	wParam �ɑ���̎��(IDROPTARGET_NOTIFY_)���ݒ肳��Ă��܂��B
//	lParam �� IDROPTARGET_NOTIFY �\���̂ւ̃|�C���^���ݒ肳��Ă��܂��B

//	cFormat �� �󂯎�邱�Ƃ��\�ȃN���b�v�{�[�h�t�H�[�}�b�g�̃��X�g���w�肵�܂��B
//	cfcnt �̓N���b�v�{�[�h�t�H�[�}�b�g�̔z��̗v�f�����w�肵�܂��B

void APIPRIVATE OLE_IDropTarget_RevokeDragDrop(HWND hWnd);

//�h���b�O���h���b�v�̃^�[�Q�b�g���������܂��B




//DropSource
int APIPRIVATE OLE_IDropSource_Start(HWND hWnd, UINT uCallbackMessage, UINT *ClipFormtList, int cfcnt, int Effect);

//�h���b�O���h���b�v���J�n����Ƃ��Ɏw�肵�܂��B
//�h���b�O���h���b�v����͎����I�ɍs���܂����A�f�[�^���K�v�Ȏ��́A�w��̃E�B���h�E���b�Z�[�W�Ńf�[�^�v�����s���܂��B

//[����]
//	hWnd �� uCallbackMessage �𑗂��ăf�[�^�̗v�����s���܂��B
//	���̎� wParam �ɗv������N���b�v�{�[�h�t�H�[�}�b�g�̒l�������Ă��܂��B
//	�v���O������ *(HANDLE *)lParam �Ƀf�[�^��ݒ肵�ĕԂ��܂��B(NULL�ł���)

//	ClipFormtList �̓T�|�[�g���Ă���N���b�v�{�[�h�t�H�[�}�b�g�̔z����w�肵�܂��B
//	cfcnt �̓N���b�v�{�[�h�t�H�[�}�b�g�̔z��̗v�f�����w�肵�܂��B

//	Effect �� �h���b�O����Ńh���b�O�����Ώۂŋ��������ʂ̑g�ݍ��킹���w�肵�܂��B

//[�߂�l]
//�h���b�v���s��ꂽ�ꍇ�́A�h���b�v��̃A�v���P�[�V�������ݒ肵�����ʂ�Ԃ��܂��B
//�L�����Z����G���[�̏ꍇ�� -1 ��Ԃ��܂��B


#endif

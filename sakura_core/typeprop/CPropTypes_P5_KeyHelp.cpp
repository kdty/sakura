/*!	@file
	�^�C�v�ʐݒ� - �L�[���[�h�w���v �y�[�W

	@author fon
	@date 2006/04/10 �V�K�쐬
*/
/*
	Copyright (C) 2006, fon, ryoji
	Copyright (C) 2007, ryoji

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "CPropTypes.h"
#include "env/CShareData.h"
#include "typeprop/CImpExpManager.h"	// 2010/4/23 Uchi
#include "dlg/CDlgOpenFile.h"
#include "charset/CharPointer.h"
#include "io/CTextStream.h"
#include "util/shell.h"
#include "util/module.h"
#include "sakura_rc.h"
#include "sakura.hh"

using namespace std;

static const DWORD p_helpids[] = {	// 2006.10.10 ryoji
	IDC_CHECK_KEYHELP,				HIDC_CHECK_KEYHELP,				//�L�[���[�h�w���v�@�\���g��
	IDC_LIST_KEYHELP,				HIDC_LIST_KEYHELP,				//SysListView32
	IDC_BUTTON_KEYHELP_UPD,			HIDC_BUTTON_KEYHELP_UPD,		//�X�V(&E)
	IDC_EDIT_KEYHELP,				HIDC_EDIT_KEYHELP,				//EDITTEXT
	IDC_BUTTON_KEYHELP_REF,			HIDC_BUTTON_KEYHELP_REF,		//�Q��(&O)...
	IDC_BUTTON_KEYHELP_TOP,			HIDC_BUTTON_KEYHELP_TOP,		//�擪(&T)
	IDC_BUTTON_KEYHELP_UP,			HIDC_BUTTON_KEYHELP_UP,			//���(&U)
	IDC_BUTTON_KEYHELP_DOWN,		HIDC_BUTTON_KEYHELP_DOWN,		//����(&G)
	IDC_BUTTON_KEYHELP_LAST,		HIDC_BUTTON_KEYHELP_LAST,		//�ŏI(&B)
	IDC_BUTTON_KEYHELP_INS,			HIDC_BUTTON_KEYHELP_INS,		//�}��(&S)
	IDC_BUTTON_KEYHELP_DEL,			HIDC_BUTTON_KEYHELP_DEL,		//�폜(&D)
	IDC_CHECK_KEYHELP_ALLSEARCH,	HIDC_CHECK_KEYHELP_ALLSEARCH,	//�S������������(&A)
	IDC_CHECK_KEYHELP_KEYDISP,		HIDC_CHECK_KEYHELP_KEYDISP,		//�L�[���[�h���\������(&W)
	IDC_CHECK_KEYHELP_PREFIX,		HIDC_CHECK_KEYHELP_PREFIX,		//�O����v����(&P)
	IDC_BUTTON_KEYHELP_IMPORT,		HIDC_BUTTON_KEYHELP_IMPORT,		//�C���|�[�g
	IDC_BUTTON_KEYHELP_EXPORT,		HIDC_BUTTON_KEYHELP_EXPORT,		//�G�N�X�|�[�g
	0, 0
};

static TCHAR* strcnv(TCHAR *str);
static TCHAR* GetFileName(const TCHAR *fullpath);

/*! �L�[���[�h�����t�@�C���ݒ� ���b�Z�[�W����

	@date 2006.04.10 fon �V�K�쐬
*/
INT_PTR CPropKeyHelp::DispatchEvent(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
)
{
	WORD	wNotifyCode;
	WORD	wID;
	HWND	hwndCtl, hwndList;
	int		idCtrl;
	NMHDR*	pNMHDR;
	int		nIndex, nIndex2;
	LV_ITEM	lvi;
	LV_COLUMN	col;
	RECT		rc;
	static int	nPrevIndex = -1;	//�X�V���ɂ��������Ȃ�o�O�C�� @@@ 2003.03.26 MIK

	BOOL	bUse;						/* ������ �g�p����/���Ȃ� */
	TCHAR	szAbout[DICT_ABOUT_LEN];	/* �����̐���(�����t�@�C����1�s�ڂ��琶��) */
	TCHAR	szPath[_MAX_PATH];			/* �t�@�C���p�X */
	DWORD	dwStyle;

	hwndList = GetDlgItem( hwndDlg, IDC_LIST_KEYHELP );

	switch( uMsg ){
	case WM_INITDIALOG:
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );
		/* �J�����ǉ� */
		::GetWindowRect( hwndList, &rc );
		/* ���X�g�Ƀ`�F�b�N�{�b�N�X��ǉ� */
		dwStyle = ListView_GetExtendedListViewStyle(hwndList);
		dwStyle |= LVS_EX_CHECKBOXES /*| LVS_EX_FULLROWSELECT*/ | LVS_EX_GRIDLINES;
		ListView_SetExtendedListViewStyle(hwndList, dwStyle);

		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 25 / 100;
		col.pszText  = _T("   �����t�@�C��");	/* �w�莫���t�@�C���̎g�p�� */
		col.iSubItem = 0;
		ListView_InsertColumn( hwndList, 0, &col );
		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 55 / 100;
		col.pszText  = _T("�����̐���");		/* �w�莫���̂P�s�ڂ��擾 */
		col.iSubItem = 1;
		ListView_InsertColumn( hwndList, 1, &col );
		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 18 / 100;
		col.pszText  = _T("�p�X");				/* �w�莫���t�@�C���p�X */
		col.iSubItem = 2;
		ListView_InsertColumn( hwndList, 2, &col );
		nPrevIndex = -1;	//@@@ 2003.05.12 MIK
		SetData( hwndDlg );	/* �_�C�A���O�f�[�^�̐ݒ� �����t�@�C���ꗗ */

		/* ���X�g������ΐ擪���t�H�[�J�X���� */
		if(ListView_GetItemCount(hwndList) > 0){
			ListView_SetItemState( hwndList, 0, LVIS_SELECTED /*| LVIS_FOCUSED*/, LVIS_SELECTED /*| LVIS_FOCUSED*/ );
		}
		/* ���X�g���Ȃ���Ώ����l�Ƃ��ėp�r��\�� */
		else{
			::DlgItem_SetText( hwndDlg, IDC_LABEL_KEYHELP_ABOUT, _T("�����t�@�C���̂P�s�ڂ̕�����") );
			::DlgItem_SetText( hwndDlg, IDC_EDIT_KEYHELP, _T("�L�[���[�h�����t�@�C�� �p�X") );
		}

		/* ������Ԃ�ݒ� */
		SendMessageCmd(hwndDlg, WM_COMMAND, (WPARAM)MAKELONG(IDC_CHECK_KEYHELP,BN_CLICKED), 0 );

		return TRUE;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID	= LOWORD(wParam);			/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl	= (HWND) lParam;		/* �R���g���[���̃n���h�� */

		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:

			switch( wID ){
			case IDC_CHECK_KEYHELP:	/* �L�[���[�h�w���v�@�\���g�� */
				if( !IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYHELP ) ){
					//EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_KEYHELP ), FALSE );			//�L�[���[�h�w���v�@�\���g��(&K)
					EnableWindow( GetDlgItem( hwndDlg, IDC_FRAME_KEYHELP ), FALSE );		  	//�����t�@�C���ꗗ(&L)
					EnableWindow( GetDlgItem( hwndDlg, IDC_LIST_KEYHELP ), FALSE );         	//SysListView32
					EnableWindow( GetDlgItem( hwndDlg, IDC_LABEL_KEYHELP_TITLE ), FALSE );  	//<�����̐���>
					EnableWindow( GetDlgItem( hwndDlg, IDC_LABEL_KEYHELP_ABOUT ), FALSE );  	//�����t�@�C���̊T�v
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_UPD ), FALSE );   	//�X�V(&E)
					EnableWindow( GetDlgItem( hwndDlg, IDC_LABEL_KEYHELP_KEYWORD ), FALSE );	//�����t�@�C��
					EnableWindow( GetDlgItem( hwndDlg, IDC_EDIT_KEYHELP ), FALSE );         	//EDITTEXT
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_REF ), FALSE );   	//�Q��(&O)...
					EnableWindow( GetDlgItem( hwndDlg, IDC_LABEL_KEYHELP_PRIOR ), FALSE );  	//���D��x(��)
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_TOP ), FALSE );   	//�擪(&T)
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_UP ), FALSE );    	//���(&U)
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_DOWN ), FALSE );  	//����(&G)
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_LAST ), FALSE );  	//�ŏI(&B)
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_INS ), FALSE );   	//�}��(&S)
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_DEL ), FALSE );   	//�폜(&D)
					EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_KEYHELP_ALLSEARCH ), FALSE );	//�S������������(&A)
					EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_KEYHELP_KEYDISP ), FALSE );	//�L�[���[�h���\������(&W)
					EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_KEYHELP_PREFIX ), FALSE );		//�O����v����(&P)
				}else{
					//EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_KEYHELP ), TRUE );			//�L�[���[�h�w���v�@�\���g��(&K)
					EnableWindow( GetDlgItem( hwndDlg, IDC_FRAME_KEYHELP ), TRUE );				//�����t�@�C���ꗗ(&L)
					EnableWindow( GetDlgItem( hwndDlg, IDC_LIST_KEYHELP ), TRUE );				//SysListView32
					EnableWindow( GetDlgItem( hwndDlg, IDC_LABEL_KEYHELP_TITLE ), TRUE );		//<�����̐���>
					EnableWindow( GetDlgItem( hwndDlg, IDC_LABEL_KEYHELP_ABOUT ), TRUE );  		//�����t�@�C���̊T�v
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_UPD ), TRUE );   		//�X�V(&E)
					EnableWindow( GetDlgItem( hwndDlg, IDC_LABEL_KEYHELP_KEYWORD ), TRUE );		//�����t�@�C��
					EnableWindow( GetDlgItem( hwndDlg, IDC_EDIT_KEYHELP ), TRUE );         		//EDITTEXT
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_REF ), TRUE );   		//�Q��(&O)...
					EnableWindow( GetDlgItem( hwndDlg, IDC_LABEL_KEYHELP_PRIOR ), TRUE );  		//���D��x(��)
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_TOP ), TRUE );   		//�擪(&T)
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_UP ), TRUE );    		//���(&U)
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_DOWN ), TRUE );  		//����(&G)
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_LAST ), TRUE );  		//�ŏI(&B)
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_INS ), TRUE );   		//�}��(&S)
					EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_KEYHELP_DEL ), TRUE );   		//�폜(&D)
					EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_KEYHELP_ALLSEARCH ), TRUE );	//�S������������(&A)
					EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_KEYHELP_KEYDISP ), TRUE );		//�L�[���[�h���\������(&W)
					EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_KEYHELP_PREFIX ), TRUE );		//�O����v����(&P)
				}
				m_Types.m_nKeyHelpNum = ListView_GetItemCount( hwndList );
				return TRUE;

			/* �}���E�X�V�C�x���g��Z�߂ď��� */
			case IDC_BUTTON_KEYHELP_INS:	/* �}�� */
			case IDC_BUTTON_KEYHELP_UPD:	/* �X�V */
				nIndex2 = ListView_GetItemCount(hwndList);
				/* �I�𒆂̃L�[��T���B */
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );

				if(wID == IDC_BUTTON_KEYHELP_INS){	/* �}�� */
					if( nIndex2 >= MAX_KEYHELP_FILE ){
						ErrorMessage( hwndDlg, _T("����ȏ�o�^�ł��܂���B"));
						return FALSE;
					}if( -1 == nIndex ){
						/* �I�𒆂łȂ���΍Ō�ɂ���B */
						nIndex = nIndex2;
						if(nIndex == 0)
							nPrevIndex = 0;
					}
				}else{								/* �X�V */
					if( -1 == nIndex ){
						ErrorMessage( hwndDlg, _T("�X�V���鎫�������X�g����I�����Ă��������B"));
						return FALSE;
					}
				}
				/* �X�V����L�[�����擾����B */
				auto_memset(szPath, 0, _countof(szPath));
				::DlgItem_GetText( hwndDlg, IDC_EDIT_KEYHELP, szPath, _countof(szPath) );
				if( szPath[0] == _T('\0') ) return FALSE;
				/* �d������ */
				nIndex2 = ListView_GetItemCount(hwndList);
				TCHAR szPath2[_MAX_PATH];
				int i;
				for(i = 0; i < nIndex2; i++){
					auto_memset(szPath2, 0, _countof(szPath2));
					ListView_GetItemText(hwndList, i, 2, szPath2, _countof(szPath2));
					if( _tcscmp(szPath, szPath2) == 0 ){
						if( (wID ==IDC_BUTTON_KEYHELP_UPD) && (i == nIndex) ){	/* �X�V���A�ς���Ă��Ȃ������牽�����Ȃ� */
						}else{
							ErrorMessage( hwndDlg, _T("���ɓo�^�ς݂̎����ł��B"));
							return FALSE;
						}
					}
				}

				/* �w�肵���p�X�Ɏ��������邩�`�F�b�N���� */
				{
					CTextInputStream_AbsIni in(szPath);
					if(!in){
						ErrorMessage( hwndDlg, _T("�t�@�C�����J���܂���ł����B\n\n%ts"), szPath );
						return FALSE;
					}
					// �J�����Ȃ�1�s�ڂ��擾���Ă������ -> szAbout
					std::wstring line=in.ReadLineW();
					_wcstotcs(szAbout,line.c_str(),_countof(szAbout));
					in.Close();
				}
				strcnv(szAbout);

				/* ���łɎ����̐������X�V */
				::DlgItem_SetText( hwndDlg, IDC_LABEL_KEYHELP_ABOUT, szAbout );	/* �����t�@�C���̊T�v */
				
				/* �X�V�̂Ƃ��͍s�폜����B */
				if(wID == IDC_BUTTON_KEYHELP_UPD){	/* �X�V */
					ListView_DeleteItem( hwndList, nIndex );
				}
				
				/* ON/OFF �t�@�C���� */
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 0;
				lvi.pszText = GetFileName(szPath);	/* �t�@�C������\�� */
				ListView_InsertItem( hwndList, &lvi );
				/* �����̐��� */
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 1;
				lvi.pszText  = szAbout;
				ListView_SetItem( hwndList, &lvi );
				/* �����t�@�C���p�X */
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 2;
				lvi.pszText  = szPath;
				ListView_SetItem( hwndList, &lvi );
				
				/* �f�t�H���g�Ń`�F�b�NON */
				ListView_SetCheckState(hwndList, nIndex, TRUE);

				/* �X�V�����L�[��I������B */
				ListView_SetItemState( hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData( hwndDlg );
				return TRUE;

			case IDC_BUTTON_KEYHELP_DEL:	/* �폜 */
				/* �I�𒆂̃L�[�ԍ���T���B */
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				/* �폜����B */
				ListView_DeleteItem( hwndList, nIndex );
				/* ���X�g���Ȃ��Ȃ����珉���l�Ƃ��ėp�r��\�� */
				if(ListView_GetItemCount(hwndList) == 0){
					::DlgItem_SetText( hwndDlg, IDC_LABEL_KEYHELP_ABOUT, _T("�����t�@�C���̂P�s�ڂ̕�����") );
					::DlgItem_SetText( hwndDlg, IDC_EDIT_KEYHELP, _T("�L�[���[�h�����t�@�C�� �p�X") );
				}/* ���X�g�̍Ō���폜�����ꍇ�́A�폜��̃��X�g�̍Ō��I������B */
				else if(nIndex > ListView_GetItemCount(hwndList)-1){
					ListView_SetItemState( hwndList, ListView_GetItemCount(hwndList)-1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				}/* �����ʒu�̃L�[��I����Ԃɂ���B */
				else{
					ListView_SetItemState( hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				}
				GetData( hwndDlg );
				return TRUE;

			case IDC_BUTTON_KEYHELP_TOP:	/* �擪 */
				/* �I�𒆂̃L�[��T���B */
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				if( 0 == nIndex ) return TRUE;	/* ���łɐ擪�ɂ���B */
				nIndex2 = 0;
				bUse = ListView_GetCheckState(hwndList, nIndex);
				ListView_GetItemText(hwndList, nIndex, 1, szAbout, _countof(szAbout));
				ListView_GetItemText(hwndList, nIndex, 2, szPath, _countof(szPath));
				ListView_DeleteItem(hwndList, nIndex);	/* �Â��L�[���폜 */
				/* ON-OFF */
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.pszText = GetFileName(szPath);	/* �t�@�C������\�� */
				ListView_InsertItem( hwndList, &lvi );
				/* �����̐��� */
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szAbout;
				ListView_SetItem( hwndList, &lvi );
				/* �����t�@�C���p�X */
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 2;
				lvi.pszText  = szPath;
				ListView_SetItem( hwndList, &lvi );
				ListView_SetCheckState(hwndList, nIndex2, bUse);
				/* �ړ������L�[��I����Ԃɂ���B */
				ListView_SetItemState( hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData( hwndDlg );
				return TRUE;

			case IDC_BUTTON_KEYHELP_LAST:	/* �ŏI */
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 - 1 == nIndex ) return TRUE;	/* ���łɍŏI�ɂ���B */
				bUse = ListView_GetCheckState(hwndList, nIndex);
				ListView_GetItemText(hwndList, nIndex, 1, szAbout, _countof(szAbout));
				ListView_GetItemText(hwndList, nIndex, 2, szPath, _countof(szPath));
				/* �L�[��ǉ�����B */
				/* ON-OFF */
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.pszText = GetFileName(szPath);	/* �t�@�C������\�� */
				ListView_InsertItem( hwndList, &lvi );
				/* �����̐��� */
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szAbout;
				ListView_SetItem( hwndList, &lvi );
				/* �����t�@�C���p�X */
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 2;
				lvi.pszText  = szPath;
				ListView_SetItem( hwndList, &lvi );
				ListView_SetCheckState(hwndList, nIndex2, bUse);
				/* �ړ������L�[��I����Ԃɂ���B */
				ListView_SetItemState( hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				ListView_DeleteItem(hwndList, nIndex);	/* �Â��L�[���폜 */
				GetData( hwndDlg );
				return TRUE;

			case IDC_BUTTON_KEYHELP_UP:	/* ��� */
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				if( 0 == nIndex ) return TRUE;	/* ���łɐ擪�ɂ���B */
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 <= 1 ) return TRUE;
				nIndex2 = nIndex - 1;
				bUse = ListView_GetCheckState(hwndList, nIndex);
				ListView_GetItemText(hwndList, nIndex, 1, szAbout, _countof(szAbout));
				ListView_GetItemText(hwndList, nIndex, 2, szPath, _countof(szPath));
				ListView_DeleteItem(hwndList, nIndex);	/* �Â��L�[���폜 */
				/* �L�[��ǉ�����B */
				/* ON-OFF */
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.pszText = GetFileName(szPath);	/* �t�@�C������\�� */
				ListView_InsertItem( hwndList, &lvi );
				/* �����̐��� */
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szAbout;
				ListView_SetItem( hwndList, &lvi );
				/* �����t�@�C���p�X */
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 2;
				lvi.pszText  = szPath;
				ListView_SetItem( hwndList, &lvi );
				ListView_SetCheckState(hwndList, nIndex2, bUse);
				/* �ړ������L�[��I����Ԃɂ���B */
				ListView_SetItemState( hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData( hwndDlg );
				return TRUE;

			case IDC_BUTTON_KEYHELP_DOWN:	/* ���� */
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 - 1 == nIndex ) return TRUE;	/* ���łɍŏI�ɂ���B */
				if( nIndex2 <= 1 ) return TRUE;
				nIndex2 = nIndex + 2;
				bUse = ListView_GetCheckState(hwndList, nIndex);
				ListView_GetItemText(hwndList, nIndex, 1, szAbout, _countof(szAbout));
				ListView_GetItemText(hwndList, nIndex, 2, szPath, _countof(szPath));
				/* �L�[��ǉ�����B */
				/* ON-OFF */
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.pszText = GetFileName(szPath);	/* �t�@�C������\�� */
				ListView_InsertItem( hwndList, &lvi );
				/* �����̐��� */
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szAbout;
				ListView_SetItem( hwndList, &lvi );
				/* �����t�@�C���p�X */
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 2;
				lvi.pszText  = szPath;
				ListView_SetItem( hwndList, &lvi );
				ListView_SetCheckState(hwndList, nIndex2, bUse);
				/* �ړ������L�[��I����Ԃɂ���B */
				ListView_SetItemState( hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				ListView_DeleteItem(hwndList, nIndex);	/* �Â��L�[���폜 */
				GetData( hwndDlg );
				return TRUE;

			case IDC_BUTTON_KEYHELP_REF:	/* �L�[���[�h�w���v �����t�@�C���́u�Q��...�v�{�^�� */
				{
					CDlgOpenFile	cDlgOpenFile;
					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
					// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
					TCHAR szWk[_MAX_PATH];
					::DlgItem_GetText( hwndDlg, IDC_EDIT_KEYHELP, szWk, _MAX_PATH );
					if( _IS_REL_PATH( szWk ) ){
						GetInidirOrExedir( szPath, szWk );
					}else{
						::lstrcpy( szPath, szWk );
					}
					cDlgOpenFile.Create( m_hInstance, hwndDlg, _T("*.khp"), szPath );
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						::DlgItem_SetText( hwndDlg, IDC_EDIT_KEYHELP, szPath );
					}
				}
				return TRUE;

			case IDC_BUTTON_KEYHELP_IMPORT:	/* �C���|�[�g */
				Import(hwndDlg);
				m_Types.m_nKeyHelpNum = ListView_GetItemCount( hwndList );
				return TRUE;

			case IDC_BUTTON_KEYHELP_EXPORT:	/* �G�N�X�|�[�g */
				Export(hwndDlg);
				return TRUE;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;

		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_KEYHELP );
			return TRUE;

		case PSN_KILLACTIVE:
			/* �_�C�A���O�f�[�^�̎擾 �����t�@�C�����X�g */
			GetData( hwndDlg );
			return TRUE;

		case PSN_SETACTIVE:
			m_nPageNum = 4;	//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			return TRUE;

		case LVN_ITEMCHANGED:	/*���X�g�̍��ڂ��ύX���ꂽ�ۂ̏���*/
			if( pNMHDR->hwndFrom == hwndList ){
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ){	//�폜�A�͈͊O�ŃN���b�N�����f����Ȃ��o�O�C��	//@@@ 2003.06.17 MIK
					nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_FOCUSED );
					return FALSE;
				}
				ListView_GetItemText(hwndList, nIndex, 1, szAbout, _countof(szAbout));
				ListView_GetItemText(hwndList, nIndex, 2, szPath, _countof(szPath));
				::DlgItem_SetText( hwndDlg, IDC_LABEL_KEYHELP_ABOUT, szAbout );	/* �����̐��� */
				::DlgItem_SetText( hwndDlg, IDC_EDIT_KEYHELP, szPath );			/* �t�@�C���p�X */
				nPrevIndex = nIndex;
			}
			break;
		}
		break;

	case WM_HELP:
		{	HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}return TRUE;

	case WM_CONTEXTMENU:	/* Context Menu */
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	}
	return FALSE;
}

void CheckDlgButtonBOOL(HWND hwnd, int id, BOOL bState ){
	CheckDlgButton( hwnd, id, (bState ? BST_CHECKED : BST_UNCHECKED) );
}

/*! �_�C�A���O�f�[�^�̐ݒ� �L�[���[�h�����t�@�C���ݒ�

	@date 2006.04.10 fon �V�K�쐬
*/
void CPropKeyHelp::SetData( HWND hwndDlg )
{
	HWND	hwndWork;
	int		i;
	LV_ITEM	lvi;
	DWORD	dwStyle;

	/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYHELP ), _countof2( m_Types.m_KeyHelpArr[0].m_szPath ) - 1 );

	// �g�p����E�g�p���Ȃ�
	CheckDlgButtonBOOL( hwndDlg, IDC_CHECK_KEYHELP, m_Types.m_bUseKeyWordHelp );
	CheckDlgButtonBOOL( hwndDlg, IDC_CHECK_KEYHELP_ALLSEARCH, m_Types.m_bUseKeyHelpAllSearch );
	CheckDlgButtonBOOL( hwndDlg, IDC_CHECK_KEYHELP_KEYDISP, m_Types.m_bUseKeyHelpKeyDisp );
	CheckDlgButtonBOOL( hwndDlg, IDC_CHECK_KEYHELP_PREFIX, m_Types.m_bUseKeyHelpPrefix );

	/* ���X�g */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_KEYHELP );
	ListView_DeleteAllItems(hwndWork);  /* ���X�g����ɂ��� */
	/* �s�I�� */
	dwStyle = ListView_GetExtendedListViewStyle( hwndWork );
	dwStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle( hwndWork, dwStyle );
	/* �f�[�^�\�� */
	for(i = 0; i < MAX_KEYHELP_FILE; i++){
		if( m_Types.m_KeyHelpArr[i].m_szPath[0] == _T('\0') ) break;
		/* ON-OFF */
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.pszText = GetFileName(m_Types.m_KeyHelpArr[i].m_szPath);
		ListView_InsertItem( hwndWork, &lvi );
		/* �����̐��� */
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = m_Types.m_KeyHelpArr[i].m_szAbout;
		ListView_SetItem( hwndWork, &lvi );
		/* �����t�@�C���p�X */
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 2;
		lvi.pszText  = m_Types.m_KeyHelpArr[i].m_szPath;
		ListView_SetItem( hwndWork, &lvi );
		/* ON/OFF���擾���ă`�F�b�N�{�b�N�X�ɃZ�b�g�i�Ƃ肠�������}���u�j */
		if(m_Types.m_KeyHelpArr[i].m_bUse){	// ON
			ListView_SetCheckState(hwndWork, i, TRUE);
		}
		else{
			ListView_SetCheckState(hwndWork, i, FALSE);
		}
	}
	ListView_SetItemState( hwndWork, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	return;
}

/*! �_�C�A���O�f�[�^�̎擾 �L�[���[�h�����t�@�C���ݒ�

	@date 2006.04.10 fon �V�K�쐬
*/
int CPropKeyHelp::GetData( HWND hwndDlg )
{
	HWND	hwndList;
	int	nIndex, i;
	TCHAR	szAbout[DICT_ABOUT_LEN];	/* �����̐���(�����t�@�C����1�s�ڂ��琶��) */
	TCHAR	szPath[_MAX_PATH];			/* �t�@�C���p�X */
//	m_nPageNum = 4;	//�����̃y�[�W�ԍ�

	/* �g�p����E�g�p���Ȃ� */
	m_Types.m_bUseKeyWordHelp      = ( BST_CHECKED == IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYHELP ) );
	m_Types.m_bUseKeyHelpAllSearch = ( BST_CHECKED == IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYHELP_ALLSEARCH ) );
	m_Types.m_bUseKeyHelpKeyDisp   = ( BST_CHECKED == IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYHELP_KEYDISP ) );
	m_Types.m_bUseKeyHelpPrefix    = ( BST_CHECKED == IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYHELP_PREFIX ) );

	/* ���X�g�ɓo�^����Ă������z��Ɏ�荞�� */
	hwndList = GetDlgItem( hwndDlg, IDC_LIST_KEYHELP );
	nIndex = ListView_GetItemCount( hwndList );
	for(i = 0; i < MAX_KEYHELP_FILE; i++){
		if( i < nIndex ){
			bool		bUse = false;						/* ����ON(1)/OFF(0) */
			szAbout[0]	= _T('\0');
			szPath[0]	= _T('\0');
			/* �`�F�b�N�{�b�N�X��Ԃ��擾����bUse�ɃZ�b�g */
			if(ListView_GetCheckState(hwndList, i))
				bUse = true;
			ListView_GetItemText( hwndList, i, 1, szAbout, _countof(szAbout) );
			ListView_GetItemText( hwndList, i, 2, szPath, _countof(szPath) );
			m_Types.m_KeyHelpArr[i].m_bUse = bUse;
			_tcscpy(m_Types.m_KeyHelpArr[i].m_szAbout, szAbout);
			_tcscpy(m_Types.m_KeyHelpArr[i].m_szPath, szPath);
		}else{	/* ���o�^�����̓N���A���� */
			m_Types.m_KeyHelpArr[i].m_szPath[0] = _T('\0');
		}
	}
	/* �����̍������擾 */
	m_Types.m_nKeyHelpNum = nIndex;
	return TRUE;
}

/*! �L�[���[�h�w���v�t�@�C�����X�g�̃C���|�[�g

	@date 2006.04.10 fon �V�K�쐬
*/
bool CPropKeyHelp::Import(HWND hwndDlg)
{
	// �C���|�[�g
	GetData( hwndDlg );

	CImpExpKeyHelp  cImpExpKeyHelp( m_Types );
	if (!cImpExpKeyHelp.ImportUI(m_hInstance, hwndDlg)) {
		// �C���|�[�g�����Ă��Ȃ�
		return false;
	}
	SetData( hwndDlg );

	return true;
}


/*! �L�[���[�h�w���v�t�@�C�����X�g�̃C���|�[�g�G�N�X�|�[�g

	@date 2006.04.10 fon �V�K�쐬
*/
bool CPropKeyHelp::Export(HWND hwndDlg)
{
	GetData(hwndDlg);
	CImpExpKeyHelp	cImpExpKeyHelp( m_Types );

	// �G�N�X�|�[�g
	return cImpExpKeyHelp.ExportUI(m_hInstance, hwndDlg);
}


/*! �����̐����̃t�H�[�}�b�g����

	@date 2006.04.10 fon �V�K�쐬
*/
static TCHAR* strcnv(TCHAR *str)
{
	TCHAR* p=str;
	/* ���s�R�[�h�̍폜 */
	if( NULL != (p=_tcschr(p,_T('\n'))) )
		*p=_T('\0');
	p=str;
	if( NULL != (p=_tcschr(p,_T('\r'))) )
		*p=_T('\0');
	/* �J���}�̒u�� */
	p=str;
	for(; (p=_tcschr(p,_T(','))) != NULL; ){
		*p=_T('.');
	}
	return str;
}

/*! �t���p�X����t�@�C������Ԃ�

	@date 2006.04.10 fon �V�K�쐬
	@date 2006.09.14 genta �f�B���N�g�����Ȃ��ꍇ�ɍŏ���1�������؂�Ȃ��悤��
*/
static TCHAR* GetFileName(const TCHAR* fullpath)
{
	const TCHAR* pszName = fullpath;
	CharPointerT p = fullpath;
	while( *p != _T('\0')  ){
		if( *p == _T('\\') ){
			pszName = p + 1;
			p++;
		}else{
			p++;
		}
	}
	return const_cast<TCHAR*>(pszName);
}


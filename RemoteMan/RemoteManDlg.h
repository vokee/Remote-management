
// RemoteManDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "sqlite3.h"
#include "SysSetDlg.h"
#include "AddGroupDlg.h"
#include "AddHostDlg.h"

struct CONFIG_STRUCT{
	char SysPassword[64];				//ϵͳ���룬ʹ��MD5����
	bool ParentShowHost;				//�������Ƿ���ʾ�ӷ��������
	char RadminPath[256];				//RADMIN·�������Ϊ�գ���ΪͬĿ¼�µ�radmin.exe
	char SSHPath[256];					//SSH·�������Ϊ�գ���ΪͬĿ¼�µ�SecureCRT.exe
	bool MstscConsole;					//Զ������ʹ��Console����
	bool MstscUseDrive;					//�Ƿ����ӱ��ط���
	char MstscLocalDrive[24];			//Զ������ӳ�䱾�ط�������ʽ:CDEF
	bool MstscRemoteAudio;				//Զ������ʹ��Զ����Ƶ
	int  MstscColor;					//Զ��������ɫ
	int  MstscWinpos;					//Զ������ֱ���
	bool MstscDeskImg;					//Զ������ʹ�����汳��
	bool MstscFontSmooth;				//Զ������ʹ������ƽ��
	bool MstscThemes;					//Զ�������Ӿ���ʽ
	int  RadminCtrlMode;				//RADMIN����ģʽ
	bool RadminFullScreen;				//RADMINʹ��ȫ������
	int  RadminColor;					//RADMIN��ɫ
	bool ReadFlag;						//�Ѷ�ȡ���
};


// CRemoteManDlg �Ի���
class CRemoteManDlg : public CDialogEx
{
//������ID��
enum {IDC_TOOLER_OPENRADMIN=10001,IDC_TOOLER_OPENMSTSC, IDC_TOOLER_OPENSSH, IDC_TOOLER_SET};
// ����
public:
	CRemoteManDlg(CWnd* pParent = NULL);	// ��׼���캯��
	virtual ~CRemoteManDlg();

// �Ի�������
	enum { IDD = IDD_REMOTEMAN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON			m_hIcon;
	CToolBar		m_ToolBar;
	CImageList		m_ImageList,m_ToolbarImageList;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	HTREEITEM hNowTreeItem;			//ö�����ؼ�ʱ��ǰ�ľ��
	CONFIG_STRUCT SysConfig;
	sqlite3	*m_pDB;
	afx_msg void OnBnClickedOk();
	void InitToolBar(void);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CListCtrl m_List;
	void OnToolbarClickedSysSet(void);
	bool OpenUserDb(void);
	CTreeCtrl m_Tree;
	void EnumTreeData(HTREEITEM hItem, int ParentNode);
	afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);
	void LoadHostList(int Node);
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	afx_msg LRESULT OnModifyPasswordMessage(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedCheckMstConsole();
	afx_msg void OnBnClickedCheckMstDrive();
	afx_msg void OnBnClickedCheckMstAudio();
	afx_msg void OnCbnSelchangeComboMstWinpos();
	afx_msg void OnBnClickedCheckRadminFullscreen();
	afx_msg void OnCbnSelchangeComboRadminCtrlmode();
	void OnMenuClickedAddGroup(void);
	void OnMenuClickedDelGroup(void);
	void OnMenuClickedAddHost(void);
	void OnMenuClickedEditHost(void);
	void OnMenuClickedDelHost(void);
	void OnToolbarClickedOpenMstsc(void);
	void OnToolbarClickedOpenRadmin(void);
	void OnToolbarClickedOpenSSH(void);
	void OnMenuClickedConnentHost(void);
	void EnumChildGroupId(HTREEITEM hItem,CArray<int ,int>&GroupArray);
protected:
	afx_msg LRESULT OnAddHostMessage(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnNMRClickTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);

	void MstscConnent(HOST_STRUCT const *pHost, CONFIG_STRUCT const *pConfig);
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	void OnMenuClickedRadminCtrl(UINT Id);
	void ConnentHost(int RadminCtrlMode);
};
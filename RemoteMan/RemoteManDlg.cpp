
// RemoteManDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "RemoteMan.h"
#include "RemoteManDlg.h"
#include "afxdialogex.h"

#pragma comment(lib, "Crypt32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

char const InitTabSqlStr[] = "\
BEGIN TRANSACTION;\r\n\
CREATE TABLE ConfigTab(\r\n\
id INTEGER primary key not null,\r\n\
Password char(64),\r\n\
ParentShowHost boolean,\r\n\
RadminPath char(256),\r\n\
SSHPath char(256),\r\n\
MstscConsole boolean,\r\n\
MstscUseDrive boolean,\r\n\
MstscLocalDrive char(24),\r\n\
MstscRemoteAudio boolean,\r\n\
MstscColor int,\r\n\
MstscWinpos int,\r\n\
MstscDeskImg boolean,\r\n\
MstscFontSmooth boolean,\r\n\
MstscThemes boolean,\r\n\
RadminCtrlMode int,\r\n\
RadminFullScreen boolean,\r\n\
RadminColor int\r\n\
);\r\n\
\r\n\
CREATE TABLE GroupTab(\r\n\
id INTEGER primary key AUTOINCREMENT,\r\n\
Name char(64) not null,\r\n\
ParentId int not null\r\n\
);\r\n\
\r\n\
CREATE TABLE HostTab(\r\n\
id INTEGER primary key AUTOINCREMENT,\r\n\
Name char(64) not null,\r\n\
ParentId int  not null,\r\n\
CtrlMode int  not null,\r\n\
HostAddrer char(64) not null,\r\n\
HostPort int not null,\r\n\
Account char(20) not null,\r\n\
Password char(24) not null,\r\n\
HostReadme char(256)\r\n\
);\r\n\
COMMIT;\r\n\
";

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

char *CryptRDPPassword(char const *Password, char *OutPassword)
{
	DATA_BLOB DataIn ;
	DATA_BLOB DataOut ;
	// mstsc.exe��ʹ�õ���unicode,���Ա��������ַ�ת��
	DWORD n=0;
	BYTE wPassword[64];
	while (*Password)
	{
		wPassword[n++]=*Password++;
		wPassword[n++]=0;
	}
	wPassword[n]=0;

	DataIn . pbData = wPassword ;
	DataIn . cbData = n ;
	if ( CryptProtectData ( &DataIn , L"psw" , // A description string
		//to be included with the
		// encrypted data.
		NULL , // Optional entropy not used.
		NULL , // Reserved.
		NULL , // Pass NULL for the
		// prompt structure.
		CRYPTPROTECT_UI_FORBIDDEN,
		&DataOut ) )
	{
		int len=0;
		for (n=0; n<DataOut.cbData; n++)
		{
			len+=sprintf_s(OutPassword+len,512-len,"%02X",DataOut.pbData[n]);
		}
		return OutPassword;
	}
	return "";
}

// CRemoteManDlg �Ի���
static int ReadTreeCallback(void* para, int n_column, char** column_value, char** column_name)
{
	CRemoteManDlg *pDlg=(CRemoteManDlg*)para;
	HTREEITEM hLast = pDlg->hNowTreeItem;
	if (column_value[1]!=NULL)
	{
		HTREEITEM hItem = pDlg->m_Tree.InsertItem(column_value[1],0,1,pDlg->hNowTreeItem);
		pDlg->m_Tree.SetItemData(hItem,atoi(column_value[0]));
		pDlg->EnumTreeData(hItem, atoi(column_value[0]));
		pDlg->hNowTreeItem = hLast;
	}
	return 0;
}

void CRemoteManDlg::EnumTreeData(HTREEITEM hItem, int ParentNode)
{
	char sqlstr[64];
	sprintf_s(sqlstr,sizeof(sqlstr),"select * from GroupTab where ParentId=%d;",ParentNode);
	TRACE("%s\r\n",sqlstr);
	hNowTreeItem = hItem;
	int rc = sqlite3_exec(m_pDB, sqlstr, ReadTreeCallback, this, NULL);
}

static int ReadConfigCallback(void* para, int n_column, char** column_value, char** column_name)
{
	CONFIG_STRUCT *pConfig = (CONFIG_STRUCT*)para;

	for (int i=0; i<n_column; i++)
	{
		if (column_value[i]==NULL) column_value[i]="";
		if (strcmp(column_name[i],"Password")==0)
			strcpy_s(pConfig->SysPassword,column_value[i]);
		else if (strcmp(column_name[i],"ParentShowHost")==0)
			pConfig->ParentShowHost=column_value[i][0]!='0' && column_value[i][0]!=0;
		else if (strcmp(column_name[i],"RadminPath")==0)
			strcpy_s(pConfig->RadminPath,column_value[i]);
		else if (strcmp(column_name[i],"SSHPath")==0)
			strcpy_s(pConfig->SSHPath,column_value[i]);
		else if (strcmp(column_name[i],"MstscConsole")==0)
			pConfig->MstscConsole=column_value[i][0]!='0' && column_value[i][0]!=0;
		else if (strcmp(column_name[i],"MstscUseDrive")==0)
			pConfig->MstscUseDrive=column_value[i][0]!='0' && column_value[i][0]!=0;
		else if (strcmp(column_name[i],"MstscLocalDrive")==0)
			strcpy_s(pConfig->MstscLocalDrive,column_value[i]);
		else if (strcmp(column_name[i],"MstscRemoteAudio")==0)
			pConfig->MstscRemoteAudio =column_value[i][0]!='0' && column_value[i][0]!=0;
		else if (strcmp(column_name[i],"MstscColor")==0)
			pConfig->MstscColor =atoi(column_value[i]);
		else if (strcmp(column_name[i],"MstscWinpos")==0)
			pConfig->MstscWinpos =atoi(column_value[i]);
		else if (strcmp(column_name[i],"MstscDeskImg")==0)
			pConfig->MstscDeskImg =column_value[i][0]!='0' && column_value[i][0]!=0;
		else if (strcmp(column_name[i],"MstscFontSmooth")==0)
			pConfig->MstscFontSmooth =column_value[i][0]!='0' && column_value[i][0]!=0;
		else if (strcmp(column_name[i],"MstscThemes")==0)
			pConfig->MstscThemes =column_value[i][0]!='0' && column_value[i][0]!=0;
		else if (strcmp(column_name[i],"RadminCtrlMode")==0)
			pConfig->RadminCtrlMode =atoi(column_value[i]);
		else if (strcmp(column_name[i],"RadminFullScreen")==0)
			pConfig->RadminFullScreen =column_value[i][0]!='0' && column_value[i][0]!=0;
		else if (strcmp(column_name[i],"RadminColor")==0)
			pConfig->RadminColor =atoi(column_value[i]);
	}
	pConfig->ReadFlag=true;

	return 0;
}

static int ReadIntCallback(void* para, int n_column, char** column_value, char** column_name)
{
	*(int*)para = atoi(column_value[0]);
	return 0;
}

bool CRemoteManDlg::OpenUserDb(void)
{
	int TabCnt=0;
	int rc = sqlite3_open("User.db",&m_pDB);
	if (rc) return false;
	//������ñ��Ƿ����
	char const *sqlstr = "select count(type) from sqlite_master where tbl_name='ConfigTab';";
	TRACE("%s\r\n",sqlstr);
	rc = sqlite3_exec(m_pDB, sqlstr, ReadIntCallback, &TabCnt, NULL);
	//��������ʱ��������
	if (TabCnt==0)
		rc=sqlite3_exec(m_pDB,InitTabSqlStr,NULL,NULL,NULL);
	return rc==0;
}

CRemoteManDlg::CRemoteManDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRemoteManDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
//	DeleteFile("User.db");
	if (!OpenUserDb())
	{
		char str[200];
		sprintf_s(str,sizeof(str),"�����ݿ�ʧ�ܣ�%s",sqlite3_errmsg(m_pDB));
		AfxMessageBox(str);
		exit(0);
	}
}

CRemoteManDlg::~CRemoteManDlg()
{
	char str[MAX_PATH];
	GetTempPath(sizeof(str),str);
	strcat_s(str,sizeof(str),"rdp_tmp");
	DeleteFile(str);
	sqlite3_close(m_pDB); 
}

void CRemoteManDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);
	DDX_Control(pDX, IDC_TREE1, m_Tree);
}

BEGIN_MESSAGE_MAP(CRemoteManDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDOK, &CRemoteManDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_TOOLER_SET, &CRemoteManDlg::OnToolbarClickedSysSet)
	ON_BN_CLICKED(ID_MENU_ADDGROUP, &CRemoteManDlg::OnMenuClickedAddGroup)
	ON_BN_CLICKED(ID_MENU_DELGROUP, &CRemoteManDlg::OnMenuClickedDelGroup)
	ON_BN_CLICKED(ID_MENU_ADDHOST, &CRemoteManDlg::OnMenuClickedAddHost)
	ON_BN_CLICKED(ID_MENU_EDITHOST, &CRemoteManDlg::OnMenuClickedEditHost)
	ON_BN_CLICKED(ID_MENU_DELHOST, &CRemoteManDlg::OnMenuClickedDelHost)
	ON_BN_CLICKED(ID_MENU_CONNENT, &CRemoteManDlg::OnMenuClickedConnentHost)
	ON_BN_CLICKED(IDC_TOOLER_OPENRADMIN, &CRemoteManDlg::OnToolbarClickedOpenRadmin)
	ON_BN_CLICKED(IDC_TOOLER_OPENMSTSC, &CRemoteManDlg::OnToolbarClickedOpenMstsc)
	ON_BN_CLICKED(IDC_TOOLER_OPENSSH, &CRemoteManDlg::OnToolbarClickedOpenSSH)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CRemoteManDlg::OnTvnSelchangedTree1)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CRemoteManDlg::OnLvnItemchangedList1)
	ON_MESSAGE(WM_MODIFY_PASSWORD_MESSAGE, &CRemoteManDlg::OnModifyPasswordMessage)
	ON_BN_CLICKED(IDC_CHECK_MST_CONSOLE, &CRemoteManDlg::OnBnClickedCheckMstConsole)
	ON_BN_CLICKED(IDC_CHECK_MST_DRIVE, &CRemoteManDlg::OnBnClickedCheckMstDrive)
	ON_BN_CLICKED(IDC_CHECK_MST_AUDIO, &CRemoteManDlg::OnBnClickedCheckMstAudio)
	ON_CBN_SELCHANGE(IDC_COMBO_MST_WINPOS, &CRemoteManDlg::OnCbnSelchangeComboMstWinpos)
	ON_BN_CLICKED(IDC_CHECK_RADMIN_FULLSCREEN, &CRemoteManDlg::OnBnClickedCheckRadminFullscreen)
	ON_CBN_SELCHANGE(IDC_COMBO_RADMIN_CTRLMODE, &CRemoteManDlg::OnCbnSelchangeComboRadminCtrlmode)
	ON_MESSAGE(WM_ADDHOST_MESSAGE, &CRemoteManDlg::OnAddHostMessage)
	ON_NOTIFY(NM_RCLICK, IDC_TREE1, &CRemoteManDlg::OnNMRClickTree1)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CRemoteManDlg::OnNMRClickList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CRemoteManDlg::OnNMDblclkList1)
	ON_COMMAND_RANGE(ID_MENU_FULLCTRL,ID_MENU_CLOSEHOST,&CRemoteManDlg::OnMenuClickedRadminCtrl)
END_MESSAGE_MAP()


// CRemoteManDlg ��Ϣ��������
void CRemoteManDlg::InitToolBar(void)
{
	m_ToolbarImageList.Create(32,32,ILC_COLOR24|ILC_MASK,1,1);
	m_ToolbarImageList.SetBkColor(RGB(255,255,255));
	m_ToolbarImageList.Add(AfxGetApp()->LoadIcon(IDI_ADDNODE));
	m_ToolbarImageList.Add(AfxGetApp()->LoadIcon(IDI_DELNODE));
	m_ToolbarImageList.Add(AfxGetApp()->LoadIcon(IDI_PC));
	m_ToolbarImageList.Add(AfxGetApp()->LoadIcon(IDI_EDIT));
	m_ToolbarImageList.Add(AfxGetApp()->LoadIcon(IDI_DEL));
	m_ToolbarImageList.Add(AfxGetApp()->LoadIcon(IDI_OPEN));
	m_ToolbarImageList.Add(AfxGetApp()->LoadIcon(IDR_MAINFRAME));
	m_ToolbarImageList.Add(AfxGetApp()->LoadIcon(IDI_RADMIN));
	m_ToolbarImageList.Add(AfxGetApp()->LoadIcon(IDI_SSH));
	m_ToolbarImageList.Add(AfxGetApp()->LoadIcon(IDI_SET));

	UINT array[13]={ID_MENU_ADDGROUP,ID_MENU_DELGROUP,ID_SEPARATOR,
					ID_MENU_ADDHOST,ID_MENU_EDITHOST,ID_MENU_DELHOST,ID_SEPARATOR,
					ID_MENU_CONNENT,IDC_TOOLER_OPENMSTSC,IDC_TOOLER_OPENRADMIN,IDC_TOOLER_OPENSSH,ID_SEPARATOR,
					IDC_TOOLER_SET};
	m_ToolBar.Create(this);
	m_ToolBar.SetButtons(array,13);
	m_ToolBar.SetButtonText(0,"���ӷ���");
	m_ToolBar.SetButtonText(1,"ɾ������");
	m_ToolBar.SetButtonText(3,"��������");
	m_ToolBar.SetButtonText(4,"�༭����");
	m_ToolBar.SetButtonText(5,"ɾ������");
	m_ToolBar.SetButtonText(7,"����");
	m_ToolBar.SetButtonText(8,"Զ������");
	m_ToolBar.SetButtonText(9,"Radmin");
	m_ToolBar.SetButtonText(10,"SecureCRT");
	m_ToolBar.SetButtonText(12,"����");

	m_ToolBar.GetToolBarCtrl().SetImageList(&m_ToolbarImageList);
	m_ToolBar.SetSizes(CSize(72,56),CSize(20,34));

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST,AFX_IDW_CONTROLBAR_LAST,0);
}

BOOL CRemoteManDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵������ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ����Ӷ���ĳ�ʼ������
	HICON hico=(HICON)LoadImage(AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDI_MSTSC),IMAGE_ICON,64,64,LR_DEFAULTCOLOR);
	((CStatic*)GetDlgItem(IDC_STATIC_PIC))->SetIcon(hico);

	InitToolBar();

	m_ImageList.Create(24,24,ILC_COLOR24|ILC_MASK,1,1);
	m_ImageList.SetBkColor(RGB(255,255,255));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_NODE_CLOSE));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_NODE_OPEN));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDR_MAINFRAME));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_RADMIN));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_SSH));
	m_Tree.SetImageList(&m_ImageList,LVSIL_NORMAL);

	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	m_List.SetImageList(&m_ImageList,LVSIL_SMALL);
	m_List.InsertColumn(0,"����",LVCFMT_LEFT,80);
	m_List.InsertColumn(1,"����������",LVCFMT_LEFT,168);
	m_List.InsertColumn(2,"����",LVCFMT_LEFT,145);
	m_List.InsertColumn(3,"�˿�",LVCFMT_LEFT,64);
	m_List.InsertColumn(4,"�˻�",LVCFMT_LEFT,120);

	//��ȡ����
	memset(&SysConfig,0,sizeof(CONFIG_STRUCT));
	while (1)
	{
		char const *sqlstr="select * from ConfigTab where id=0;";
		TRACE("%s\r\n",sqlstr);
		int rc = sqlite3_exec(m_pDB, sqlstr, ReadConfigCallback, &SysConfig, NULL);
		if (SysConfig.ReadFlag) break;
		sqlstr="insert into ConfigTab values(0, '',true,'','',true,true,'',false,0,0,false, false, true, 0, true, 1);";
		TRACE("%s\r\n",sqlstr);
		rc=sqlite3_exec(m_pDB, sqlstr, NULL, NULL, NULL);
	}
	((CButton*)GetDlgItem(IDC_CHECK_MST_CONSOLE))->SetCheck(SysConfig.MstscConsole);
	((CButton*)GetDlgItem(IDC_CHECK_MST_DRIVE))->SetCheck(SysConfig.MstscUseDrive);
	((CButton*)GetDlgItem(IDC_CHECK_MST_AUDIO))->SetCheck(SysConfig.MstscRemoteAudio);
	((CComboBox*)GetDlgItem(IDC_COMBO_MST_WINPOS))->SetCurSel(SysConfig.MstscWinpos);
	((CButton*)GetDlgItem(IDC_CHECK_RADMIN_FULLSCREEN))->SetCheck(SysConfig.RadminFullScreen);
	((CComboBox*)GetDlgItem(IDC_COMBO_RADMIN_CTRLMODE))->SetCurSel(SysConfig.RadminCtrlMode);

	//��ȡ����
	EnumTreeData(TVI_ROOT,0);
	if (m_Tree.GetCount()==0)
		SetDlgItemText(IDC_EDIT_README,"�������ӷ���.");
	else
	{
		HTREEITEM hTree=m_Tree.GetChildItem(0);
		m_Tree.SelectItem(hTree);
	}

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CRemoteManDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի���������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CRemoteManDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CRemoteManDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRemoteManDlg::OnBnClickedOk()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
//	CDialogEx::OnOK();
} 



HBRUSH CRemoteManDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ����� 
	return hbr;
}


void CRemoteManDlg::OnToolbarClickedSysSet(void)
{
	CSysSetDlg Dlg(SysConfig.ParentShowHost, SysConfig.MstscLocalDrive, SysConfig.MstscColor, SysConfig.MstscDeskImg,
		SysConfig.MstscFontSmooth, SysConfig.MstscThemes, SysConfig.RadminColor, SysConfig.RadminPath, SysConfig.SSHPath);
	if (Dlg.DoModal()==IDOK)
	{
		SysConfig.ParentShowHost=Dlg.m_ParentShowHost!=0;
		strcpy_s(SysConfig.RadminPath,sizeof(SysConfig.RadminPath),Dlg.m_RadminPath);
		strcpy_s(SysConfig.SSHPath,sizeof(SysConfig.SSHPath),Dlg.m_SshPath);
		strcpy_s(SysConfig.MstscLocalDrive,sizeof(SysConfig.MstscLocalDrive),Dlg.m_MstDriveStr);
		SysConfig.MstscColor=Dlg.m_MstColor;
		SysConfig.MstscDeskImg=Dlg.m_MstShowDeskImg!=0;
		SysConfig.MstscFontSmooth=Dlg.m_MstFontSmooth!=0;
		SysConfig.MstscThemes=Dlg.m_MstThemes!=0;
		SysConfig.RadminColor=Dlg.m_RadminColor;
		
		char sqlstr[1024];
		int n=sprintf_s(sqlstr,sizeof(sqlstr),"update ConfigTab set ParentShowHost=%s,RadminPath='%s',SSHPath='%s',MstscLocalDrive='%s',"
											  "MstscColor=%d,MstscDeskImg=%s,MstscFontSmooth=%s,MstscThemes=%s,RadminColor=%d where id=0;",
			SysConfig.ParentShowHost ? "true":"false",
			SysConfig.RadminPath,
			SysConfig.SSHPath,
			SysConfig.MstscLocalDrive,
			SysConfig.MstscColor,
			SysConfig.MstscDeskImg ? "true":"false",
			SysConfig.MstscFontSmooth ? "true":"false",
			SysConfig.MstscThemes ? "true":"false",
			SysConfig.RadminColor
			);
		TRACE("%s\r\n",sqlstr);
		int rc = sqlite3_exec(m_pDB, sqlstr, NULL, NULL, NULL);
	}
}

void CRemoteManDlg::OnMenuClickedAddGroup(void)
{
	CString GroupName;
	HTREEITEM hItem = m_Tree.GetSelectedItem();
	if (hItem!=NULL)
		GroupName=m_Tree.GetItemText(hItem);

	char sqlstr[128];
	CAddGroupDlg dlg(GroupName);
	if (dlg.DoModal()==IDOK)
	{
		int ParentId=0;
		if (dlg.m_AddRoot)
			hItem=TVI_ROOT;
		else
			ParentId=m_Tree.GetItemData(hItem);
		//���ж�������������Ƿ���������Ƶķ������
		int GroupCnt=0;
		sprintf_s(sqlstr,sizeof(sqlstr),"select count() from GroupTab where ParentId=%d and Name='%s';",ParentId,dlg.m_GroupName);
		TRACE("%s\r\n",sqlstr);		
		int rc = sqlite3_exec(m_pDB, sqlstr, ReadIntCallback, &GroupCnt, NULL);
		if (GroupCnt>0)
		{
			MessageBox("�÷������Ѿ�����","����",MB_ICONERROR);
			return;
		}
		//���ӵ����ݿ�
		sprintf_s(sqlstr,sizeof(sqlstr),"insert into GroupTab values(NULL,'%s',%d);",dlg.m_GroupName,ParentId);
		TRACE("%s\r\n",sqlstr);
		rc = sqlite3_exec(m_pDB, sqlstr, NULL, NULL, NULL);
		//��ȡ�������ݵ�ID
		sprintf_s(sqlstr,sizeof(sqlstr),"select id from GroupTab where ParentId=%d and Name='%s';",ParentId,dlg.m_GroupName);
		TRACE("%s\r\n",sqlstr);
		int Id=0;
		rc = sqlite3_exec(m_pDB, sqlstr, ReadIntCallback, &Id, NULL);
		hItem=m_Tree.InsertItem(dlg.m_GroupName,0,1,hItem);
		m_Tree.SetItemData(hItem,Id);
		m_Tree.SelectItem(hItem);
	}
}

void CRemoteManDlg::EnumChildGroupId(HTREEITEM hItem,CArray<int ,int>&GroupArray)
{
	HTREEITEM hChildItem;
	GroupArray.Add(m_Tree.GetItemData(hItem));
	//�����ӷ���
	hChildItem=m_Tree.GetChildItem(hItem);
	if (hChildItem==0) return;
	EnumChildGroupId(hChildItem,GroupArray);
	//�����ֵܷ���
	while (1)
	{
		hChildItem=m_Tree.GetNextSiblingItem(hChildItem);
		if (hChildItem==0) return;
		EnumChildGroupId(hChildItem,GroupArray);
	}
}

void CRemoteManDlg::OnMenuClickedDelGroup(void)
{
	HTREEITEM hItem=m_Tree.GetSelectedItem();
	if (hItem==0) return;
	if (MessageBox("ɾ�����齫ͬʱɾ���ӷ��鼰��Ӧ������ȷ��ɾ����","ע��",MB_OKCANCEL|MB_ICONWARNING)!=IDOK) return;
	CArray<int ,int>GroupArray;
	GroupArray.SetSize(0,20);
	//ö����ID
	EnumChildGroupId(hItem,GroupArray);
	//ɾ����
	int sqlstrlen=25+GroupArray.GetSize()*23;
	char *sqlstr=new char[sqlstrlen];
	int len=sprintf_s(sqlstr,sqlstrlen,"delete from GroupTab where id=%d",GroupArray[0]);
	for (int i=1; i<GroupArray.GetSize(); i++)
	{
		len+=sprintf_s(sqlstr+len,sqlstrlen-len," or id=%d",GroupArray[i]);
	}
	sqlstr[len]=';';
	sqlstr[len+1]=0;
	TRACE("%s\r\n",sqlstr);
	int rc = sqlite3_exec(m_pDB, sqlstr, NULL, NULL, NULL);
	//ɾ������
	len=sprintf_s(sqlstr,sqlstrlen,"delete from HostTab where ParentId=%d",GroupArray[0]);
	for (int i=1; i<GroupArray.GetSize(); i++)
	{
		len+=sprintf_s(sqlstr+len,sqlstrlen-len," or ParentId=%d",GroupArray[i]);
	}
	sqlstr[len]=';';
	sqlstr[len+1]=0;
	TRACE("%s\r\n",sqlstr);
	rc = sqlite3_exec(m_pDB, sqlstr, NULL, NULL, NULL);
	delete sqlstr;
	//ɾ�����ؼ��ڵ���б���
	SetDlgItemText(IDC_EDIT_README,"");
	m_List.DeleteAllItems();
	m_Tree.DeleteItem(hItem);
}


afx_msg LRESULT CRemoteManDlg::OnAddHostMessage(WPARAM wParam, LPARAM lParam)
{
	HOST_STRUCT *pHost = (HOST_STRUCT*)wParam;
	int ParentId=m_Tree.GetItemData(HTREEITEM(lParam));

	char sqlstr[512];
	//�鿴���������Ƿ����
	int HostCnt=0;
	sprintf_s(sqlstr,sizeof(sqlstr),"select count() from HostTab where ParentId=%d and Name='%s' and CtrlMode=%d;",
		ParentId, pHost->Name, pHost->CtrlMode);
	TRACE("%s\r\n",sqlstr);
	int rc = sqlite3_exec(m_pDB, sqlstr, ReadIntCallback, &HostCnt, NULL);
	if (HostCnt>0)
	{
		MessageBox("���������Ѵ���.","����",MB_ICONERROR);
		return 1;
	}
	//���ӵ����ݿ�
	sprintf_s(sqlstr,sizeof(sqlstr),"insert into HostTab values(NULL,'%s',%d,%d,'%s',%d,'%s','%s','%s');",
		pHost->Name, ParentId, pHost->CtrlMode, pHost->HostAddress, pHost->HostPort, pHost->Account, pHost->Password, pHost->ReadMe);
	TRACE("%s\r\n",sqlstr);
	rc = sqlite3_exec(m_pDB, sqlstr, NULL, NULL, NULL);
	//��ȡID
	int Id=0;
	sprintf_s(sqlstr,sizeof(sqlstr),"select id from HostTab where ParentId=%d and Name='%s';",ParentId,pHost->Name);
	TRACE("%s\r\n",sqlstr);
	rc = sqlite3_exec(m_pDB, sqlstr, ReadIntCallback, &Id, NULL);
	//���ӵ��б�
	int n=m_List.GetItemCount();
	m_List.InsertItem(n, CTRL_MODE[pHost->CtrlMode],2+pHost->CtrlMode);
	m_List.SetItemText(n, 1, pHost->Name);
	m_List.SetItemText(n, 2, pHost->HostAddress);
	sprintf_s(sqlstr,sizeof(sqlstr),"%d",pHost->HostPort);
	m_List.SetItemText(n, 3, sqlstr);
	m_List.SetItemText(n, 4, pHost->Account);
	m_List.SetItemData(n,Id);

	return 0;
}

void CRemoteManDlg::OnMenuClickedAddHost(void)
{
	HTREEITEM hItem=m_Tree.GetSelectedItem();
	if (hItem==NULL || m_Tree.GetCount()==0)
	{
		MessageBox("�������ӷ����ѡ�����.","����",MB_ICONERROR);
		return;
	}
	CAddHostDlg Dlg(NULL, hItem);
	Dlg.DoModal();
}

static int ReadHostCallback(void* para, int n_column, char** column_value, char** column_name)
{
	HOST_STRUCT *pHost = (HOST_STRUCT *)para;
	strcpy_s(pHost->Name,sizeof(pHost->Name),column_value[1]);
	pHost->CtrlMode=atoi(column_value[3]);
	strcpy_s(pHost->HostAddress,sizeof(pHost->HostAddress),column_value[4]);
	pHost->HostPort=atoi(column_value[5]);
	strcpy_s(pHost->Account,sizeof(pHost->Account),column_value[6]);
	if (column_value[7]!=NULL)
		strcpy_s(pHost->Password,sizeof(pHost->Password),column_value[7]);
	if (column_value[8]!=NULL)
		strcpy_s(pHost->ReadMe,sizeof(pHost->ReadMe),column_value[8]);

	return 0;
}

void CRemoteManDlg::OnMenuClickedEditHost(void)
{
	if (m_List.GetSelectedCount()!=1) return;
	int n=m_List.GetSelectionMark();
	int Id=m_List.GetItemData(n);
	HOST_STRUCT Host;
	memset(&Host,0,sizeof(Host));

	char sqlstr[512];
	sprintf_s(sqlstr,sizeof(sqlstr),"select * from HostTab where Id=%d;",Id);
	TRACE("%s\r\n",sqlstr);
	int rc = sqlite3_exec(m_pDB, sqlstr, ReadHostCallback, &Host, NULL);

	CAddHostDlg Dlg(&Host, 0);
	if (Dlg.DoModal()==IDOK)
	{
		//�������ݿ�
		sprintf_s(sqlstr,sizeof(sqlstr),"update HostTab set Name='%s',CtrlMode=%d,HostAddrer='%s',HostPort=%d,Account='%s',"
										"Password='%s',HostReadme='%s' where id=%d;",
			Dlg.m_Host.Name, Dlg.m_Host.CtrlMode, Dlg.m_Host.HostAddress, 
			Dlg.m_Host.HostPort, Dlg.m_Host.Account, Dlg.m_Host.Password, Dlg.m_Host.ReadMe,Id);
		TRACE("%s\r\n",sqlstr);
		int rc = sqlite3_exec(m_pDB, sqlstr, NULL, NULL, NULL);
		//�����б���
		m_List.SetItem(n,0,LVIF_IMAGE,NULL,2+Dlg.m_Host.CtrlMode,0,0,0);
		m_List.SetItemText(n,0,CTRL_MODE[Dlg.m_Host.CtrlMode]);
		m_List.SetItemText(n,1,Dlg.m_Host.Name);
		m_List.SetItemText(n,2,Dlg.m_Host.HostAddress);
		sprintf_s(sqlstr,sizeof(sqlstr),"%d",Dlg.m_Host.HostPort);
		m_List.SetItemText(n,3,sqlstr);
		m_List.SetItemText(n,4,Dlg.m_Host.Account);
		SetDlgItemText(IDC_EDIT_README,Dlg.m_Host.ReadMe);
	}
	m_List.SetFocus();
}


void CRemoteManDlg::OnMenuClickedDelHost(void)
{
	if (m_List.GetSelectedCount()==0) return;
	int Cnt=m_List.GetItemCount();
	bool *Flags=new bool[Cnt];
	int *Ids=new int[Cnt];
	//�г�ѡ������ID
	memset(Flags,0, sizeof(bool)*Cnt);
	POSITION  pos=m_List.GetFirstSelectedItemPosition();
	while (pos!=NULL)
	{
		int n=m_List.GetNextSelectedItem(pos);
		Flags[n]=true;
		Ids[n]=m_List.GetItemData(n);
	}
	//�Ӻ�ʼɾ���б�
	for (int i=Cnt-1; i>=0; i--)
	{
		if (Flags[i]) m_List.DeleteItem(i);
	}
	//ɾ�����ݿ�
	int sqlstrlen=30+Cnt*17;
	char *sqlstr=new char[sqlstrlen];
	int len=sprintf_s(sqlstr,sqlstrlen,"delete from HostTab where");
	bool bfirst=true;
	for (int i=0; i<Cnt; i++)
	{
		if (!Flags[i]) continue;
		len+=sprintf_s(sqlstr+len,sqlstrlen-len, bfirst?" id=%d":" or id=%d",Ids[i]);
		bfirst=false;
	}
	sqlstr[len]=';';
	sqlstr[len+1]=0;
	TRACE("%s\r\n",sqlstr);
	int rc = sqlite3_exec(m_pDB, sqlstr, NULL, NULL, NULL);
	//
	delete Flags;
	delete Ids;
	delete sqlstr;
}


void CRemoteManDlg::OnToolbarClickedOpenMstsc(void)
{
	char szBuffer[MAX_PATH];
	SHGetSpecialFolderPath(NULL, szBuffer, CSIDL_SYSTEM, FALSE);
	strcat_s(szBuffer,MAX_PATH,"\\Mstsc.exe");
	WinExec(szBuffer,WM_SHOWWINDOW);
}


void CRemoteManDlg::OnToolbarClickedOpenRadmin(void)
{
	char const *RadminPath="radmin.exe";			//��·��Ϊ��ʱʹ��ͬĿ¼�µ�radmin.exe
	if (SysConfig.RadminPath[0]!=0) RadminPath=SysConfig.RadminPath;
	//�鿴�ļ��Ƿ����
	CFileStatus fstatus;
	if (strstr(RadminPath,".exe")==NULL || !CFile::GetStatus(RadminPath,fstatus))
	{
//		AfxMessageBox("Radmin·�����ô���");
		return;
	}
	WinExec(RadminPath,WM_SHOWWINDOW);
}


void CRemoteManDlg::OnToolbarClickedOpenSSH(void)
{
	if (strstr(SysConfig.SSHPath,".exe"))
		WinExec(SysConfig.SSHPath,WM_SHOWWINDOW);
}


void CRemoteManDlg::MstscConnent(HOST_STRUCT const *pHost, CONFIG_STRUCT const *pConfig)
{
	char RdpStr[1536],str[512];
	//����ģʽ
	int len=sprintf_s(RdpStr,sizeof(RdpStr),"screen mode id:i:%d\r\n",pConfig->MstscWinpos==0?2:1);
	//����
	int Width=1024,Height=768;
	if (pConfig->MstscWinpos!=0)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_MST_WINPOS))->GetLBText(pConfig->MstscWinpos,str);
		char *p=strchr(str,' ');
		*p=0;
		p+=4;
		Width=atoi(str);
		Height=atoi(p);
	}
	len+=sprintf_s(RdpStr+len,sizeof(RdpStr)-len,"desktopwidth:i:%d\r\ndesktopheight:i:%d\r\n",Width,Height);
	//��ɫλ��
	len+=sprintf_s(RdpStr+len,sizeof(RdpStr)-len,"session bpp:i:%d\r\n",pConfig->MstscColor==0 ? 16:pConfig->MstscColor==1 ? 24:32);
	//Ĭ������
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"winposstr:s:0,1,0,0,1024,768\r\n");
	len+=sizeof("winposstr:s:0,1,0,0,1024,768\r\n")-1;
	//Զ�̷�������ַ
	len+=sprintf_s(RdpStr+len,sizeof(RdpStr)-len,"full address:s:%s:%d\r\n",pHost->HostAddress,pHost->HostPort);
	//�����ݴ��䵽�ͻ��˼����ʱ�Ƿ�����ݽ���ѹ��
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"compression:i:1\r\n");
	len+=sizeof("compression:i:1\r\n")-1;
	//ȷ����ʱ�� Windows �����Ӧ�õ��������ӵ�Զ�̻Ự
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"keyboardhook:i:2\r\n");
	len+=sizeof("keyboardhook:i:2\r\n")-1;
	//��������
	len+=sprintf_s(RdpStr+len,sizeof(RdpStr)-len,"audiomode:i:%d\r\n",pConfig->MstscRemoteAudio ? 0:2);
	//������ 
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"redirectclipboard:i:1\r\n");
	len+=sizeof("redirectclipboard:i:1\r\n")-1;
	//PnP���弴���豸
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"devicestoredirect:s:\r\n");
	len+=sizeof("devicestoredirect:s:\r\n")-1;
	//����������
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"drivestoredirect:s:");
	len+=sizeof("drivestoredirect:s:")-1;
	if (pConfig->MstscUseDrive)
	{
		char volume[]="C:\\";
		for (int i=0; pConfig->MstscLocalDrive[i]>='A' && pConfig->MstscLocalDrive[i]<='Z'; i++)
		{
			volume[0]=pConfig->MstscLocalDrive[i];
			GetVolumeInformation(volume,str,16,NULL,NULL,NULL,NULL,0);
			len+=sprintf_s(RdpStr+len,sizeof(RdpStr)-len,"%s (%c:);",str,pConfig->MstscLocalDrive[i]);
		}
	}
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"\r\n");
	len+=2;
	//�Ƿ��Զ����Ӵ�ӡ��
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"redirectprinters:i:0\r\n");
	len+=sizeof("redirectprinters:i:0\r\n")-1;
	//�Ƿ��Զ�����COM���п�
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"redirectcomports:i:0\r\n");
	len+=sizeof("redirectcomports:i:0\r\n")-1;
	//�Ƿ��Զ��������ܿ�
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"redirectsmartcards:i:0\r\n");
	len+=sizeof("redirectsmartcards:i:0\r\n")-1;
	//ȫ��ģʽʱ�Ƿ���ʾ������
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"displayconnectionbar:i:1\r\n");
	len+=sizeof("displayconnectionbar:i:1\r\n")-1;
	//�ڶϿ����Ӻ��Ƿ��Զ�������������
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"autoreconnection enabled:i:1\r\n");
	len+=sizeof("autoreconnection enabled:i:1\r\n")-1;
	//�û�������
	len+=sprintf_s(RdpStr+len,sizeof(RdpStr)-len,"username:s:%s\r\ndomain:s:\r\n",pHost->Account);
	//ָ��Ҫ��Զ�̻Ự����Ϊ shell����������Դ���������Զ������ĳ���
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"alternate shell:s:\r\n");
	len+=sizeof("alternate shell:s:\r\n")-1;
	////RDP��������ʱ�Զ�������Ӧ�ó������ڵ��ļ���λ�ó�
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"shell working directory:s:\r\n");
	len+=sizeof("shell working directory:s:\r\n")-1;
	//RDP�����������
	len+=sprintf_s(RdpStr+len,sizeof(RdpStr)-len,"password 51:b:%s\r\n",CryptRDPPassword(pHost->Password,str));
	//�Ƿ��ֹ��ʾ���汳��
	len+=sprintf_s(RdpStr+len,sizeof(RdpStr)-len,"disable wallpaper:i:%d\r\n",pConfig->MstscDeskImg?0:1);
	//�Ƿ��ֹ����
	len+=sprintf_s(RdpStr+len,sizeof(RdpStr)-len,"disable themes:i:%d\r\n",pConfig->MstscThemes?0:1);
	//�Ƿ���������ƽ��
	len+=sprintf_s(RdpStr+len,sizeof(RdpStr)-len,"allow font smoothing:i:%d\r\n",pConfig->MstscFontSmooth?1:0);
	//���ļ����ϵ���λ��ʱ�Ƿ���ʾ�ļ�������
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"disable full window drag:i:1\r\n");
	len+=sizeof("disable full window drag:i:1\r\n")-1;
	//���ò˵�����
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"disable menu anims:i:1\r\n");
	len+=sizeof("disable menu anims:i:1\r\n")-1;
	//���ù������
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"disable cursor setting:i:0\r\n");
	len+=sizeof("disable cursor setting:i:0\r\n")-1;
	//�Ƿ�λͼ�����ڱ��ؼ������
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"bitmapcachepersistenable:i:1\r\n");
	len+=sizeof("bitmapcachepersistenable:i:1\r\n")-1;
	//���������������֤��������
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"authentication level:i:0\r\n");
	len+=sizeof("authentication level:i:0\r\n")-1;
	//?
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"prompt for credentials:i:0\r\n");
	len+=sizeof("prompt for credentials:i:0\r\n")-1;
	//ȷ���Ƿ񱣴��û���ƾ�ݲ��������� RD ���غ�Զ�̼����
	strcpy_s(RdpStr+len,sizeof(RdpStr)-len,"promptcredentialonce:i:1\r\n");
	len+=sizeof("promptcredentialonce:i:1\r\n")-1;

	//�洢���ļ�
	GetTempPath(sizeof(str),str);
	strcat_s(str,sizeof(str),"rdp_tmp");
//	sprintf_s(str,sizeof(str),"c:\\%s_RDP.rdp",pHost->HostAddress);
	CFile file;
	if (!file.Open(str,CFile::modeCreate|CFile::modeWrite|CFile::typeBinary)) return;
	file.Write(RdpStr,len);
	file.Close();

	//����
	char szBuffer[MAX_PATH];
	SHGetSpecialFolderPath(NULL, szBuffer, CSIDL_SYSTEM, FALSE);
	sprintf_s(RdpStr,sizeof(RdpStr),"%s\\Mstsc.exe /%s %s", szBuffer,pConfig->MstscConsole?"console":"admin",str);		//����
	WinExec(RdpStr, WM_SHOWWINDOW);
	TRACE("Rdp�ļ�����=%d Rdp������:%s\r\n",len,RdpStr);
}

//CtrlMode=-1ʱ��ʹ�������еĿ���ģʽ
void RadminConnent(HOST_STRUCT const *pHost, CONFIG_STRUCT const *pConfig, int CtrlMode)
{
	static const char MODE[][10]={"","/noinput","/file","/shutdown"};
	static const char COLOUR[][8]={"/8bpp","/16bpp","/24bpp"};
	char const *RadminPath="radmin.exe";			//��·��Ϊ��ʱʹ��ͬĿ¼�µ�radmin.exe
	if (pConfig->RadminPath[0]!=0) RadminPath=pConfig->RadminPath;
	//�鿴�ļ��Ƿ����
	CFileStatus fstatus;
	if (strstr(RadminPath,".exe")==NULL || !CFile::GetStatus(RadminPath,fstatus))
	{
		AfxMessageBox("Radmin·�����ô���");
		return;
	}

	if (CtrlMode==-1) CtrlMode=pConfig->RadminCtrlMode;	
	char str1[100],str2[30];
	//����Radmin���ӷ�����
	sprintf_s(str1,sizeof(str1),"/connect:%s:%d %s %s",pHost->HostAddress,pHost->HostPort,MODE[CtrlMode],COLOUR[pConfig->RadminColor]);
	if (pConfig->RadminFullScreen)
		strcat_s(str1,sizeof(str1)," /fullscreen");
	TRACE("%s\r\n",str1);
	ShellExecute(NULL,"open",RadminPath,str1,NULL,SW_SHOW);
	//��������
	sprintf_s(str1,sizeof(str1),"Radmin ��ȫ�ԣ�%s",pHost->HostAddress);	//Radmin2.2�Ĵ�������
	sprintf_s(str2,sizeof(str2),"Radmin ��ȫ��: %s",pHost->HostAddress);	//Radmin3.4�Ĵ�������
	//����Radmin��������
	HWND hWnd;
	clock_t t2,t1=clock();
	for (t2=t1;t2-t1<8000;t2=clock())
	{
		Sleep(1);
		hWnd=FindWindow(NULL,str1);
		if (hWnd!=NULL)
			break;
		hWnd=FindWindow(NULL,str2);
		if (hWnd!=NULL)
			break;
	}
	if (hWnd==NULL)	return;
	//��д��Ϣ������
	HWND UserWnd=::GetDlgItem(hWnd,0x7ff);
	HWND PasswordWnd=::GetDlgItem(hWnd,0x800);
	if (UserWnd!=NULL)
		::SendMessage(UserWnd,WM_SETTEXT,0,(LPARAM)pHost->Account);
	if (PasswordWnd!=NULL)
		::SendMessage(PasswordWnd,WM_SETTEXT,0,(LPARAM)pHost->Password);
	::PostMessage(hWnd,WM_COMMAND,0x78,0);
}

void SSHConnent(HOST_STRUCT const *pHost, CONFIG_STRUCT const *pConfig)
{
	//�鿴�ļ��Ƿ����
	CFileStatus fstatus;
	if (strstr(pConfig->SSHPath,".exe")==NULL || !CFile::GetStatus(pConfig->SSHPath,fstatus))
	{
		AfxMessageBox("SSH·�����ô���");
		return;
	}
	//����
	char str[128];
	sprintf_s(str,sizeof(str),"%s /ssh2 %s@%s /P %d /PASSWORD %s",
		pConfig->SSHPath,pHost->Account,pHost->HostAddress,pHost->HostPort,pHost->Password);
	WinExec(str,WM_SHOWWINDOW);
}

void CRemoteManDlg::ConnentHost(int RadminCtrlMode)
{
	if (m_List.GetSelectedCount()!=1) return;
	int n=m_List.GetSelectionMark();
	//��ȡ������Ϣ
	char sqlstr[128];
	HOST_STRUCT Host;
	sprintf_s(sqlstr,sizeof(sqlstr),"select * from HostTab where Id=%d;",m_List.GetItemData(n));
	TRACE("%s\r\n",sqlstr);
	int rc = sqlite3_exec(m_pDB, sqlstr, ReadHostCallback, &Host, NULL);
	//
	if (strcmp(CTRL_MODE[Host.CtrlMode],CTRL_MODE_RDP_NAME)==0)
		MstscConnent(&Host,&SysConfig);
	else if (strcmp(CTRL_MODE[Host.CtrlMode],CTRL_MODE_RADMIN_NAME)==0)
		RadminConnent(&Host,&SysConfig,RadminCtrlMode);
	else if (strcmp(CTRL_MODE[Host.CtrlMode],CTRL_MODE_SSH_NAME)==0)
		SSHConnent(&Host,&SysConfig);
	else if (strcmp(CTRL_MODE[Host.CtrlMode],CTRL_MODE_VNC_NAME)==0)
	{

	}
}

void CRemoteManDlg::OnMenuClickedConnentHost(void)
{
	ConnentHost(-1);
}

void CRemoteManDlg::OnMenuClickedRadminCtrl(UINT Id)
{
	ConnentHost(Id-ID_MENU_FULLCTRL);
}

void CRemoteManDlg::OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	m_List.DeleteAllItems();
	SetDlgItemText(IDC_EDIT_README,"");
	LoadHostList(m_Tree.GetItemData(pNMTreeView->itemNew.hItem));
	*pResult = 0;
}

static int ReadShowHostCallback(void* para, int n_column, char** column_value, char** column_name)
{
	CRemoteManDlg *pDlg = (CRemoteManDlg*)para;
	int n=pDlg->m_List.GetItemCount();

	pDlg->m_List.InsertItem(n,CTRL_MODE[atoi(column_value[3])],2+atoi(column_value[3]));	//��������
	pDlg->m_List.SetItemText(n,1,column_value[1]);					//��������
	pDlg->m_List.SetItemText(n,2,column_value[4]);					//������ַ
	pDlg->m_List.SetItemText(n,3,column_value[5]);					//�˿�
	pDlg->m_List.SetItemText(n,4,column_value[6]);					//�ʺ�
	pDlg->m_List.SetItemData(n,atoi(column_value[0]));				//ID
	return 0;
}

static int ReadChildIdCallback(void* para, int n_column, char** column_value, char** column_name)
{
	CRemoteManDlg *pDlg = (CRemoteManDlg*)para;
	pDlg->LoadHostList(atoi(column_value[0]));
	return 0;
}

void CRemoteManDlg::LoadHostList(int Node)
{
	int rc;
	char sqlstr[64];
	//��������������
	sprintf_s(sqlstr,sizeof(sqlstr),"select * from HostTab where ParentId=%d;",Node);
	TRACE("%s\r\n",sqlstr);
	rc = sqlite3_exec(m_pDB, sqlstr, ReadShowHostCallback, this, NULL);
	//�����ӽڵ�������
	if (SysConfig.ParentShowHost)
	{
		sprintf_s(sqlstr,sizeof(sqlstr),"select id from GroupTab where ParentId=%d;",Node);
		TRACE("%s\r\n",sqlstr);
		rc = sqlite3_exec(m_pDB, sqlstr, ReadChildIdCallback, this, NULL);
	}
}

static int ReadStrCallback(void* para, int n_column, char** column_value, char** column_name)
{
	if (column_value[0]!=NULL)
		strcpy_s((char*)para,256,column_value[0]);
	return 0;
}

void CRemoteManDlg::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	if (pNMLV->uNewState==0) return;
	int n=m_List.GetSelectedCount();
	if (n!=1) 
	{
		SetDlgItemText(IDC_EDIT_README,"");
		return;
	}
	int Id=m_List.GetItemData(pNMLV->iItem);

	char sqlstr[64], Readme[256]={0};
	sprintf_s(sqlstr,sizeof(sqlstr),"select HostReadme from HostTab where id=%d;",Id);
	TRACE("%s\r\n",sqlstr);
	int rc = sqlite3_exec(m_pDB, sqlstr, ReadStrCallback, Readme, NULL);
	SetDlgItemText(IDC_EDIT_README,Readme);
	
	*pResult = 0;
}

afx_msg LRESULT CRemoteManDlg::OnModifyPasswordMessage(WPARAM wParam, LPARAM lParam)
{
	char const *Src=(char*)wParam;
	char const *New=(char*)lParam;

	if (strcmp(Src,SysConfig.SysPassword)!=0)
		return LRESULT("ԭ�������.");

	char sqlstr[128];
	strcpy_s(SysConfig.SysPassword,sizeof(SysConfig.SysPassword),New);
	int n=sprintf_s(sqlstr,sizeof(sqlstr),"update ConfigTab set Password='%s' where id=0;",SysConfig.SysPassword);
	TRACE("%s\r\n",sqlstr);
	int rc = sqlite3_exec(m_pDB, sqlstr, NULL, NULL, NULL);

	return LRESULT("�����޸ĳɹ�.");
}

void CRemoteManDlg::OnBnClickedCheckMstConsole()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	SysConfig.MstscConsole=((CButton*)GetDlgItem(IDC_CHECK_MST_CONSOLE))->GetCheck()!=0;

	char sqlstr[128];
	int n=sprintf_s(sqlstr,sizeof(sqlstr),"update ConfigTab set MstscConsole=%s where id=0;",SysConfig.MstscConsole?"true":"false");
	TRACE("%s\r\n",sqlstr);
	int rc = sqlite3_exec(m_pDB, sqlstr, NULL, NULL, NULL);
}

void CRemoteManDlg::OnBnClickedCheckMstDrive()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	SysConfig.MstscUseDrive=((CButton*)GetDlgItem(IDC_CHECK_MST_DRIVE))->GetCheck()!=0;

	char sqlstr[128];
	int n=sprintf_s(sqlstr,sizeof(sqlstr),"update ConfigTab set MstscUseDrive=%s where id=0;",SysConfig.MstscUseDrive?"true":"false");
	TRACE("%s\r\n",sqlstr);
	int rc = sqlite3_exec(m_pDB, sqlstr, NULL, NULL, NULL);
}

void CRemoteManDlg::OnBnClickedCheckMstAudio()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	SysConfig.MstscRemoteAudio=((CButton*)GetDlgItem(IDC_CHECK_MST_AUDIO))->GetCheck()!=0;

	char sqlstr[128];
	int n=sprintf_s(sqlstr,sizeof(sqlstr),"update ConfigTab set MstscRemoteAudio=%s where id=0;",SysConfig.MstscRemoteAudio?"true":"false");
	TRACE("%s\r\n",sqlstr);
	int rc = sqlite3_exec(m_pDB, sqlstr, NULL, NULL, NULL);
}

void CRemoteManDlg::OnCbnSelchangeComboMstWinpos()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	SysConfig.MstscWinpos=((CComboBox*)GetDlgItem(IDC_COMBO_MST_WINPOS))->GetCurSel();

	char sqlstr[128];
	int n=sprintf_s(sqlstr,sizeof(sqlstr),"update ConfigTab set MstscWinpos=%d where id=0;",SysConfig.MstscWinpos);
	TRACE("%s\r\n",sqlstr);
	int rc = sqlite3_exec(m_pDB, sqlstr, NULL, NULL, NULL);
}

void CRemoteManDlg::OnBnClickedCheckRadminFullscreen()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	SysConfig.RadminFullScreen=((CButton*)GetDlgItem(IDC_CHECK_RADMIN_FULLSCREEN))->GetCheck()!=0;

	char sqlstr[128];
	int n=sprintf_s(sqlstr,sizeof(sqlstr),"update ConfigTab set RadminFullScreen=%s where id=0;",SysConfig.RadminFullScreen?"true":"false");
	TRACE("%s\r\n",sqlstr);
	int rc = sqlite3_exec(m_pDB, sqlstr, NULL, NULL, NULL);
}


void CRemoteManDlg::OnCbnSelchangeComboRadminCtrlmode()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	SysConfig.RadminCtrlMode=((CComboBox*)GetDlgItem(IDC_COMBO_RADMIN_CTRLMODE))->GetCurSel();

	char sqlstr[128];
	int n=sprintf_s(sqlstr,sizeof(sqlstr),"update ConfigTab set RadminCtrlMode=%d where id=0;",SysConfig.RadminCtrlMode);
	TRACE("%s\r\n",sqlstr);
	int rc = sqlite3_exec(m_pDB, sqlstr, NULL, NULL, NULL);
}


void CRemoteManDlg::OnNMRClickTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	CMenu Menu;
	Menu.LoadMenu(IDR_MENU_RCLICK);
	CMenu *pSubMenu=Menu.GetSubMenu(m_Tree.GetSelectedItem()==NULL ? 0:1);
	CPoint point;
	GetCursorPos(&point);
	pSubMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL,point.x,point.y,this);

	*pResult = 0;
}


void CRemoteManDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	int Index=0;
	int SelCnt=m_List.GetSelectedCount();

	if (SelCnt==0)
		Index=2;
	else if (SelCnt==1)
	{
		char str[12];
		m_List.GetItemText(pNMItemActivate->iItem,0,str,10);
		Index=strcmp(str,CTRL_MODE_RADMIN_NAME)==0 ? 4:3;
	}
	else
		Index=5;

	CMenu Menu;
	Menu.LoadMenu(IDR_MENU_RCLICK);
	CMenu *pSubMenu=Menu.GetSubMenu(Index);
	CPoint point;
	GetCursorPos(&point);
	pSubMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL,point.x,point.y,this);

	*pResult = 0;
}

void CRemoteManDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	OnMenuClickedConnentHost();
	*pResult = 0;
}

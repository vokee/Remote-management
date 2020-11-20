// AddHostDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "RemoteMan.h"
#include "AddHostDlg.h"
#include "afxdialogex.h"


// CAddHostDlg �Ի���

IMPLEMENT_DYNAMIC(CAddHostDlg, CDialogEx)

CAddHostDlg::CAddHostDlg(HOST_STRUCT const *pHost,  HTREEITEM hItem, CWnd* pParent /*=NULL*/)
	: CDialogEx(CAddHostDlg::IDD, pParent)
	, m_hParentItem(hItem)
{
	if (pHost)
	{
		bAddHost=false;
		memcpy(&m_Host, pHost, sizeof(m_Host));
	}
	else
	{
		bAddHost=true;
		memset(&m_Host, 0, sizeof(m_Host));
	}
}

CAddHostDlg::~CAddHostDlg()
{
}

void CAddHostDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAddHostDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAddHostDlg::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO_CTRLMODE, &CAddHostDlg::OnCbnSelchangeComboCtrlmode)
END_MESSAGE_MAP()


// CAddHostDlg ��Ϣ��������


BOOL CAddHostDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ����Ӷ���ĳ�ʼ��
	CComboBox *pBox = (CComboBox*)GetDlgItem(IDC_COMBO_CTRLMODE);
	for(int i=0; i<sizeof(CTRL_MODE)/sizeof(CTRL_MODE[0]);i++)
	{
		pBox->AddString(CTRL_MODE[i]);
	}
	if (bAddHost)
	{
		pBox->SetCurSel(0);
		SetCtrlModeDefPort(0);
		SetDlgItemText(IDC_EDIT_USER,"Administrator");
	}
	else
	{
		pBox->SetCurSel(m_Host.CtrlMode);
		SetDlgItemInt(IDC_EDIT_HOSTPORT,m_Host.HostPort);
		SetDlgItemText(IDC_EDIT_HOSTADDR,m_Host.HostAddress);
		SetDlgItemText(IDC_EDIT_HOSTNAME,m_Host.Name);
		SetDlgItemText(IDC_EDIT_USER,m_Host.Account);
		SetDlgItemText(IDC_EDIT_PASSWORD,m_Host.Password);
		SetDlgItemText(IDC_EDIT_README,m_Host.ReadMe);
		SetWindowText("�༭����");
		SetDlgItemText(IDOK,"ȷ��");
		SetDlgItemText(IDCANCEL,"ȡ��");
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}


void CAddHostDlg::OnBnClickedOk()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	CString str;
	//����������
	GetDlgItemText(IDC_EDIT_HOSTNAME,str);
	str.Trim();
	if (str.GetLength()>=sizeof(m_Host.Name))
	{
		MessageBox("�������Ƴ���","����",MB_ICONERROR);
		return;
	}
	strcpy_s(m_Host.Name,str);
	//��������ַ
	GetDlgItemText(IDC_EDIT_HOSTADDR,str);
	str.Trim();
	if (str.GetLength()>=sizeof(m_Host.HostAddress) || str.GetLength()<2)
	{
		MessageBox("������ַ���ô���","����",MB_ICONERROR);
		return;
	}
	strcpy_s(m_Host.HostAddress,str);
	if (m_Host.Name[0]==0) strcpy_s(m_Host.Name,str);
	//�˿�
	m_Host.HostPort=GetDlgItemInt(IDC_EDIT_HOSTPORT);
	if (m_Host.HostPort==0)
	{
		MessageBox("�����˿����ô���","����",MB_ICONERROR);
		return;
	}
	//����ģʽ
	m_Host.CtrlMode=((CComboBox*)GetDlgItem(IDC_COMBO_CTRLMODE))->GetCurSel();
	//�û���
	GetDlgItemText(IDC_EDIT_USER,str);
	str.Trim();
	if (str.GetLength()>=sizeof(m_Host.Account) || str.GetLength()<2)
	{
		MessageBox("�û������ô���","����",MB_ICONERROR);
		return;
	}
	strcpy_s(m_Host.Account,str);
	//����
	GetDlgItemText(IDC_EDIT_PASSWORD,str);
	if (str.GetLength()>=sizeof(m_Host.Password))
	{
		MessageBox("�û��������ô���","����",MB_ICONERROR);
		return;
	}
	strcpy_s(m_Host.Password,str);
	//˵��
	GetDlgItemText(IDC_EDIT_README,str);
	if (str.GetLength()>=sizeof(m_Host.ReadMe))
	{
		MessageBox("����˵�����ô���","����",MB_ICONERROR);
		return;
	}
	strcpy_s(m_Host.ReadMe,str);

	//��������
	if (bAddHost)
		GetParent()->SendMessage(WM_ADDHOST_MESSAGE, WPARAM(&m_Host), (LPARAM)m_hParentItem);
	else
		CDialogEx::OnOK();
}


void CAddHostDlg::OnCbnSelchangeComboCtrlmode()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	SetCtrlModeDefPort(((CComboBox*)GetDlgItem(IDC_COMBO_CTRLMODE))->GetCurSel());
}


void CAddHostDlg::SetCtrlModeDefPort(int CtrlMode)
{
	if (strcmp(CTRL_MODE[CtrlMode],CTRL_MODE_RDP_NAME)==0)
		SetDlgItemInt(IDC_EDIT_HOSTPORT,3389);
	else if (strcmp(CTRL_MODE[CtrlMode],CTRL_MODE_RADMIN_NAME)==0)
		SetDlgItemInt(IDC_EDIT_HOSTPORT,4899);
	else if (strcmp(CTRL_MODE[CtrlMode],CTRL_MODE_SSH_NAME)==0)
		SetDlgItemInt(IDC_EDIT_HOSTPORT,22);
	else if (strcmp(CTRL_MODE[CtrlMode],CTRL_MODE_VNC_NAME)==0)
		SetDlgItemInt(IDC_EDIT_HOSTPORT,5900);
}
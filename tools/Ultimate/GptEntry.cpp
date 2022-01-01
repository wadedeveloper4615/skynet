#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "GptEntry.h"
#include "UltimageResize.h"

#include "Attribute.h"
#include "GPT_GUID_Type.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGptEntry dialog

IMPLEMENT_DYNCREATE(CGptEntry, CPropertyPage)

CGptEntry::CGptEntry() : CPropertyPage(CGptEntry::IDD) {
  //{{AFX_DATA_INIT(CGptEntry)
  m_guid_type = _T("");
  m_guid = _T("");
  m_start_lba = _T("");
  m_last_lba = _T("");
  m_attrib = _T("");
  m_name = _T("");
  //}}AFX_DATA_INIT
}

CGptEntry::~CGptEntry() {
}

void CGptEntry::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CGptEntry)
  DDX_Text(pDX, IDC_GPTE_GUID_TYPE, m_guid_type);
  DDV_MaxChars(pDX, m_guid_type, 48);
  DDX_Text(pDX, IDC_GPTE_GUID, m_guid);
  DDV_MaxChars(pDX, m_guid, 48);
  DDX_Text(pDX, IDC_GPTE_START_LBA, m_start_lba);
  DDV_MaxChars(pDX, m_start_lba, 32);
  DDX_Text(pDX, IDC_GPTE_LAST_LBA, m_last_lba);
  DDV_MaxChars(pDX, m_last_lba, 32);
  DDX_Text(pDX, IDC_GPTE_ATTRIB, m_attrib);
  DDV_MaxChars(pDX, m_attrib, 20);
  DDX_Text(pDX, IDC_GPTE_NAME, m_name);
  DDV_MaxChars(pDX, m_name, 71);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGptEntry, CPropertyPage)
  //{{AFX_MSG_MAP(CGptEntry)
  ON_BN_CLICKED(ID_GPT_APPLY, OnGptApply)
  ON_EN_CHANGE(IDC_GPTE_ATTRIB, OnTabItemChange)
  ON_EN_KILLFOCUS(IDC_GPTE_GUID, OnKillfocusGpteGuid)
  ON_EN_KILLFOCUS(IDC_GPTE_GUID_TYPE, OnKillfocusGpteGuidType)
  ON_BN_CLICKED(IDC_GUID_CREATE_TYPE, OnGuidSelectType)
  ON_BN_CLICKED(IDC_GUID_TYPE, OnGuidSelectGUID)
  ON_EN_CHANGE(IDC_GPTE_GUID, OnTabItemChange)
  ON_EN_CHANGE(IDC_GPTE_GUID_TYPE, OnTabItemChange)
  ON_EN_CHANGE(IDC_GPTE_LAST_LBA, OnTabItemChange)
  ON_EN_CHANGE(IDC_GPTE_NAME, OnTabItemChange)
  ON_EN_CHANGE(IDC_GPTE_START_LBA, OnTabItemChange)
  ON_BN_CLICKED(IDC_ATTRIBUTE, OnAttribute)
  ON_BN_CLICKED(IDC_GUID_PLUS_MINUS, OnPlusMinus)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGptEntry message handlers

void CGptEntry::OnGptApply() {
  // just update the members.
  // the Apply button on the main GPT dialog saves to disk
  UpdateData(TRUE); // get from dialog
  
  // Add the '*' to the tab title
  CPropertySheet *Sheet = (CPropertySheet *) GetParent();
  TC_ITEM ti;
  char szText[64];
  strcpy(szText, m_Title);
  ti.mask = TCIF_TEXT;
  ti.pszText = szText;
  Sheet->GetTabControl()->SetItem(m_index, &ti);
  m_dirty = FALSE;
}

void CGptEntry::OnTabItemChange() {
  if (!m_dirty) {
    CPropertySheet *Sheet = (CPropertySheet *) GetParent();
    TC_ITEM ti;
    char szText[64];
    
    ti.mask = TCIF_TEXT;
    ti.pszText = szText;
    ti.cchTextMax = 64;
    Sheet->GetTabControl()->GetItem(m_index, &ti);
    strcat(szText, "*");
    Sheet->GetTabControl()->SetItem(m_index, &ti);
    m_dirty = TRUE;
  }
}

void CGptEntry::OnKillfocusGpteGuid() {
  CString cs;
  GetDlgItemText(IDC_GPTE_GUID, cs);
  if (!GUID_CheckFormat(cs)) {
    AfxMessageBox("GUID has illegal format\r\n"
                  "Must be in XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX format\r\n"
                  "with only hexadecimal characters.");
    //GetDlgItem(IDC_GPTE_GUID)->SetFocus();
  }
}

void CGptEntry::OnKillfocusGpteGuidType() {
  CString cs;
  GetDlgItemText(IDC_GPTE_GUID_TYPE, cs);
  if (!GUID_CheckFormat(cs)) {
    AfxMessageBox("GUID has illegal format\r\n"
                  "Must be in XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX format\r\n"
                  "with only hexadecimal characters.");
    //GetDlgItem(IDC_GPTE_GUID_TYPE)->SetFocus();
  }
}

void CGptEntry::OnGuidSelectType() {
  CGUIDType list;
  list.m_title = "GUID Type";
  list.m_list_type = TRUE;

  if (list.DoModal() == IDOK)
    SetDlgItemText(IDC_GPTE_GUID_TYPE, list.m_type);
}

void CGptEntry::OnGuidSelectGUID() {
  CGUIDType list;
  list.m_title = "GUID";
  list.m_list_type = FALSE;

  if (list.DoModal() == IDOK)
    SetDlgItemText(IDC_GPTE_GUID, list.m_type);
}

// https://docs.microsoft.com/en-us/windows-server/administration/windows-commands/gpt
S_ATTRIBUTES gpt_entry_attrbs[] = {
                                //            |                               | <- max (col 67)
  {           ( 1<<0),           ( 1<<0),  0, "Required Partition"             , {-1, } },
  {           ( 1<<1),           ( 1<<1),  1, "Hidden Partition"               , {-1, } },
  {           ( 1<<2),           ( 1<<2),  2, "Legacy BIOS Bootable"           , {-1, } },
  { ((DWORD64) 1<<60), ((DWORD64) 1<<60),  3, "Read-only"                      , {-1, } },
  { ((DWORD64) 1<<61), ((DWORD64) 1<<61),  4, "Shaddow"                        , {-1, } },
  { ((DWORD64) 1<<62), ((DWORD64) 1<<62),  5, "Hidden"                         , {-1, } },
  { ((DWORD64) 1<<63), ((DWORD64) 1<<63),  6, "do not automount"               , {-1, } },
  {                 0,      (DWORD64) -1, -1, NULL                             , {-1, } }
};

void CGptEntry::OnAttribute() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_GPTE_ATTRIB, cs);
  dlg.m_title = "Entry Attribute";
  dlg.m_attrib = convert64(cs);
  dlg.m_attributes = gpt_entry_attrbs;
  if (dlg.DoModal() == IDOK) {
    cs.Format("0x%016I64X", dlg.m_attrib);
    SetDlgItemText(IDC_GPTE_ATTRIB, cs);
  }
}

// if adding sectors, insert 'count' sectors at last + 1
// if subtracting sectors, simply change last value.  Then ask if we should remove last-new_last
void CGptEntry::OnPlusMinus() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  CPropertySheet *Sheet = (CPropertySheet *) GetParent();
  CTabCtrl *TabCtrl = Sheet->GetTabControl();
  BYTE buffer[MAX_SECT_SIZE];
  
  const int tab_count = TabCtrl->GetItemCount();  // count of tabs
  const int cur_tab = TabCtrl->GetCurSel();       // zero based index of current tab
  long val, differ;
  
  // get the property page of the current tab
  CGptEntry *page = (CGptEntry *) Sheet->GetPage(cur_tab);
  
  // make sure the data has been saved to the members
  page->UpdateData(TRUE); // get from dialog
  
  DWORD64 start = _atoi64(page->m_start_lba);
  DWORD64 last  = _atoi64(page->m_last_lba);
  DWORD64 size = last - start + 1;
  
  // get an absolute value to resize to
  CUltimageResize resize;
  resize.m_cur_size.Format("%I64i", size);
  resize.m_new_size = "";
  if (resize.DoModal() != IDOK)
    return;
  differ = (long) ((long long) convert64(resize.m_new_size) - (long long) size);
  // TODO: we assume differ will be within range (i.e.: -differ is not greater than current size)
  
  // read in the GPT backup sector
  size_t backup = dlg->Gpt.GetDlgItemInt(IDC_GPT_BACKUP_LBA);
  dlg->ReadFromFile(buffer, backup, 1);
  
  if ((differ > 0) || (AfxMessageBox("Remove sectors from image?", MB_YESNO, 0) == IDYES)) {
    if (differ > 0)
      dlg->InsertSectors(last + 1, differ);
    else
      dlg->RemoveSectors(last + 1, -differ);
    
    // now scroll through the entries above us and add to their starting and ending values
    for (int i=cur_tab+1; i<tab_count; i++) {
      CGptEntry *p = (CGptEntry *) Sheet->GetPage(i);
      val = atol(p->m_start_lba);
      p->m_start_lba.Format("%i", val + differ);
      val = atol(p->m_last_lba);
      p->m_last_lba.Format("%i", val + differ);
    }

    // update the 'last usable' value on the GPT dialog
    val = atol(dlg->Gpt.m_last_lba);
    dlg->Gpt.SetDlgItemInt(IDC_GPT_LAST_LBA, val + differ);
    
    // update the location of the backup GPT
    backup += differ;
  }
  
  // update the 'last' value of this tab
  val = atol(page->m_last_lba);
  page->m_last_lba.Format("%i", val + differ);

  // write the backup to the new location
  dlg->Gpt.SetDlgItemInt(IDC_GPT_BACKUP_LBA, (UINT) backup);
  dlg->WriteToFile(buffer, backup, 1);

  // update the GPT's CRC's and Last Usable Sector Entry
  dlg->Gpt.OnEcrcButton();
  dlg->Gpt.OnGptApply();

  // reload the image?
  if (AfxMessageBox("Reload image?", MB_YESNO, 0) == IDYES)
    SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
  else
    AfxMessageBox("Save all data and reload the image to show corrected values");
}

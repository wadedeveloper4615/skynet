#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"

#include "Lean.h"
#include "LeanJournal.h"

#include "Attribute.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLeanJournal dialog
CLeanJournal::CLeanJournal(CWnd* pParent /*=NULL*/)
  : CDialog(CLeanJournal::IDD, pParent) {
  //{{AFX_DATA_INIT(CLeanJournal)
  m_entry_count = _T("");
  m_flags = _T("");
  m_crc = _T("");
  m_magic = _T("");
  m_new_inode = _T("");
  m_org_inode = _T("");
  m_padding = _T("");
  m_entry_reserved = _T("");
  m_reserved = _T("");
  //}}AFX_DATA_INIT
  m_buffer = NULL;
}

void CLeanJournal::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CLeanJournal)
  DDX_Text(pDX, IDC_ENTRY_COUNT, m_entry_count);
  DDX_Text(pDX, IDC_FLAGS, m_flags);
  DDX_Text(pDX, IDC_LEAN_CRC, m_crc);
  DDX_Text(pDX, IDC_LEAN_MAGIC, m_magic);
  DDX_Text(pDX, IDC_NEW_INODE, m_new_inode);
  DDX_Text(pDX, IDC_ORG_INODE, m_org_inode);
  DDX_Text(pDX, IDC_PADDING, m_padding);
  DDX_Text(pDX, IDC_ENTRY_RESERVED, m_entry_reserved);
  DDX_Text(pDX, IDC_RESERVED, m_reserved);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLeanJournal, CDialog)
  //{{AFX_MSG_MAP(CLeanJournal)
  ON_BN_CLICKED(ID_CLEAR, OnClear)
  ON_BN_CLICKED(ID_ENTRY_CLEAR, OnEntryClear)
  ON_BN_CLICKED(ID_NEXT, OnNext)
  ON_BN_CLICKED(ID_PREV, OnPrev)
  ON_BN_CLICKED(IDC_FLAGS_B, OnFlagsB)
  ON_BN_CLICKED(IDC_LEAN_CRC_UPDATE, OnLeanCrcUpdate)
  ON_BN_CLICKED(IDC_LEAN_MAGIC_UPDATE, OnLeanMagicUpdate)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLeanJournal message handlers
BOOL CLeanJournal::OnInitDialog() {
  S_LEAN_JOURNAL *hdr = (S_LEAN_JOURNAL *) m_buffer;
  
  m_crc.Format("0x%08X", hdr->checksum);
  m_magic.Format("0x%08X", hdr->magic);
  m_entry_count.Format("%i", hdr->entry_cnt);
  m_padding.Format("0x%08X", hdr->padding);
  
  BYTE *b = (BYTE *) hdr->reserved;
  m_reserved.Format("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", 
    b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
  SetDlgItemText(IDC_ENTRY_RESERVED, m_reserved);
  
  CDialog::OnInitDialog();
  
  m_current = -1;
  m_count = hdr->entry_cnt;
  if (m_count > 0)
    OnNext();
  else {
    GetDlgItem(IDC_NEW_INODE)->EnableWindow(FALSE);
    GetDlgItem(IDC_ORG_INODE)->EnableWindow(FALSE);
    GetDlgItem(IDC_FLAGS_B)->EnableWindow(FALSE);
    GetDlgItem(IDC_FLAGS)->EnableWindow(FALSE);
    GetDlgItem(ID_ENTRY_CLEAR)->EnableWindow(FALSE);
    GetDlgItem(ID_PREV)->EnableWindow(FALSE);
    GetDlgItem(ID_NEXT)->EnableWindow(FALSE);
  }
  
  return TRUE;
}

void CLeanJournal::OnClear() {
  SetDlgItemText(IDC_RESERVED, "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
}

void CLeanJournal::OnEntryClear() {
  SetDlgItemText(IDC_ENTRY_RESERVED, "00 00 00 00 00 00 00 00 00 00 00 00");
}

void CLeanJournal::EntrytoDialog() {
  S_LEAN_JOURNAL_ENTRY *entry = (S_LEAN_JOURNAL_ENTRY *) ((BYTE *) m_buffer + sizeof(S_LEAN_JOURNAL));
  CString cs;
  
  if (m_current >= 0) {
    cs.Format("%I64i", entry[m_current].new_inode);
    SetDlgItemText(IDC_NEW_INODE, cs);
    cs.Format("%I64i", entry[m_current].org_inode);
    SetDlgItemText(IDC_ORG_INODE, cs);
    cs.Format("0x%08X", entry[m_current].flags);
    SetDlgItemText(IDC_FLAGS, cs);
  
    BYTE *b = (BYTE *) entry[m_current].resv;
    cs.Format("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11]);
    SetDlgItemText(IDC_ENTRY_RESERVED, cs);
  }
}

void CLeanJournal::DialogtoEntry() {
  S_LEAN_JOURNAL_ENTRY *entry = (S_LEAN_JOURNAL_ENTRY *) ((BYTE *) m_buffer + sizeof(S_LEAN_JOURNAL));
  CString cs;
  
  if (m_current >= 0) {
    GetDlgItemText(IDC_NEW_INODE, cs);
    entry[m_current].new_inode = convert64(cs);
    GetDlgItemText(IDC_ORG_INODE, cs);
    entry[m_current].org_inode = convert64(cs);
    GetDlgItemText(IDC_FLAGS, cs);
    entry[m_current].flags = convert32(cs);
    GetDlgItemText(IDC_ENTRY_RESERVED, cs);
    ConvertDumpToBuffer(cs, entry[m_current].resv, 3 * sizeof(DWORD));
  }
}

void CLeanJournal::OnNext() {
  CString cs;
  
  DialogtoEntry();
  
  if ((m_current + 1) < m_count) {
    m_current++;
    EntrytoDialog();
  }
  
  GetDlgItem(ID_NEXT)->EnableWindow(((m_current + 1) < m_count));
  GetDlgItem(ID_PREV)->EnableWindow(m_current > 0);
  
  cs.Format("%i of %i", m_current + 1, m_count);
  SetDlgItemText(IDC_ENTRY_DISPLAY, cs);
}

void CLeanJournal::OnPrev() {
  CString cs;
  
  DialogtoEntry();
  
  if (m_current > 0) {
    m_current--;
    EntrytoDialog();
  }
  
  GetDlgItem(ID_NEXT)->EnableWindow(((m_current + 1) < m_count));
  GetDlgItem(ID_PREV)->EnableWindow(m_current > 0);
  
  cs.Format("%i of %i", m_current + 1, m_count);
  SetDlgItemText(IDC_ENTRY_DISPLAY, cs);
}

S_ATTRIBUTES journal_flags[] = {
                                 //            |                               | <- max (col 67)
  { (1<<0),             (1<<0),             0, "Valid"                          , {-1, } },
  { 0,                         (DWORD) -1, -1, NULL                             , {-1, } }
};

void CLeanJournal::OnFlagsB() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_FLAGS, cs);
  dlg.m_title = "Entry Flags";
  dlg.m_attrib = convert32(cs);
  dlg.m_attributes = journal_flags;
  if (dlg.DoModal() == IDOK) {
    cs.Format("0x%08X", dlg.m_attrib);
    SetDlgItemText(IDC_FLAGS, cs);
  }
}

void CLeanJournal::OnLeanCrcUpdate() {
  S_LEAN_JOURNAL *hdr = (S_LEAN_JOURNAL *) m_buffer;
  CString cs;
  
  UpdateData(TRUE);
  
  DWORD crc = m_parent->LeanCalcCRC(hdr, sizeof(S_LEAN_JOURNAL) + (hdr->entry_cnt * sizeof(S_LEAN_JOURNAL_ENTRY)));
  cs.Format("0x%08X", crc);
  SetDlgItemText(IDC_LEAN_CRC, cs);
}

void CLeanJournal::OnLeanMagicUpdate() {
  SetDlgItemText(IDC_LEAN_MAGIC, "0x4C4E524A");
}

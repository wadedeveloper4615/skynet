#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "SFSEntry.h"

#include "SFS.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSFSEntry dialog
CSFSEntry::CSFSEntry(CWnd* pParent /*=NULL*/)
  : CDialog(CSFSEntry::IDD, pParent) {
  //{{AFX_DATA_INIT(CSFSEntry)
  m_cont_count = _T("");
  m_crc = _T("");
  m_end_block = _T("");
  m_filelen = _T("");
  m_name = _T("");
  m_start_block = _T("");
  m_time_stamp = _T("");
  m_entry_type = _T("");
  m_entry_index = _T("");
  //}}AFX_DATA_INIT
}

void CSFSEntry::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CSFSEntry)
  DDX_Text(pDX, IDC_ENTRY_CONT_COUNT, m_cont_count);
  DDX_Text(pDX, IDC_ENTRY_CRC, m_crc);
  DDX_Text(pDX, IDC_ENTRY_END_BLOCK, m_end_block);
  DDX_Text(pDX, IDC_ENTRY_FILELEN, m_filelen);
  DDX_Text(pDX, IDC_ENTRY_NAME, m_name);
  DDX_Text(pDX, IDC_ENTRY_START, m_start_block);
  DDX_Text(pDX, IDC_ENTRY_TIME, m_time_stamp);
  DDX_Text(pDX, IDC_ENTRY_TYPE, m_entry_type);
  DDX_Text(pDX, IDC_ENTRY_INDEX, m_entry_index);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSFSEntry, CDialog)
  //{{AFX_MSG_MAP(CSFSEntry)
  ON_EN_CHANGE(IDC_ENTRY_TIME, OnChangeEntryTime)
  ON_BN_CLICKED(IDC_CRC_UPDATE, OnCrcUpdate)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSFSEntry message handlers

BOOL CSFSEntry::OnInitDialog() {
  m_entry_type.Format("0x%02X", m_type);
  m_crc.Format("0x%02X", m_entry[1]);
  m_entry_index.Format("%i", m_index);
  switch (m_type) {
    case SFS_ENTRY_VOL_ID: {    // 0x01  // volume ID
      struct S_SFS_VOL_ID *id = (struct S_SFS_VOL_ID *) m_entry;
      m_time_stamp.Format("%I64i", id->time_stamp);
      
      SetDlgItemText(IDC_ENTRY_CONT_COUNT_DISP, "Reserved:");
      m_cont_count.Format("0x%04X", id->resvd);
      m_name = id->name;
      
      GetDlgItem(IDC_ENTRY_START)->EnableWindow(FALSE);
      GetDlgItem(IDC_ENTRY_END_BLOCK)->EnableWindow(FALSE);
      GetDlgItem(IDC_ENTRY_FILELEN)->EnableWindow(FALSE);
      SetWindowText("SFS Entry: Volume ID");
    } break;
      
    case SFS_ENTRY_START: {    // 0x02  // start marker
      struct S_SFS_START *start = (struct S_SFS_START *) m_entry;
      
      GetDlgItem(IDC_ENTRY_CONT_COUNT)->EnableWindow(FALSE);
      GetDlgItem(IDC_ENTRY_TIME)->EnableWindow(FALSE);
      GetDlgItem(IDC_ENTRY_START)->EnableWindow(FALSE);
      GetDlgItem(IDC_ENTRY_END_BLOCK)->EnableWindow(FALSE);
      GetDlgItem(IDC_ENTRY_FILELEN)->EnableWindow(FALSE);
      GetDlgItem(IDC_ENTRY_NAME)->EnableWindow(FALSE);
      SetWindowText("SFS Entry: Start");
    } break;
      
    case SFS_ENTRY_DIR:        // 0x11  // directory entry
    case SFS_ENTRY_DIR_DEL: {  // 0x19  // deleted directory
      struct S_SFS_DIR *dir = (struct S_SFS_DIR *) m_entry;
      m_cont_count.Format("%i", dir->num_cont);
      m_time_stamp.Format("%I64i", dir->time_stamp);
      // TODO: what if continuation slots used? we need to get rest of name
      m_name = dir->name;
      
      GetDlgItem(IDC_ENTRY_START)->EnableWindow(FALSE);
      GetDlgItem(IDC_ENTRY_END_BLOCK)->EnableWindow(FALSE);
      GetDlgItem(IDC_ENTRY_FILELEN)->EnableWindow(FALSE);
      if (m_type == SFS_ENTRY_DIR)
        SetWindowText("SFS Entry: Directory");
      else
        SetWindowText("SFS Entry: Deleted Directory");
    } break;
      
    case SFS_ENTRY_FILE:       // 0x12  // file entry
    case SFS_ENTRY_FILE_DEL: { // 0x1A  // deleted file
      struct S_SFS_FILE *file = (struct S_SFS_FILE *) m_entry;
      m_cont_count.Format("%i", file->num_cont);
      m_time_stamp.Format("%I64i", file->time_stamp);
      m_start_block.Format("%I64i", file->start_block);
      m_end_block.Format("%I64i", file->end_block);
      m_filelen.Format("%I64i", file->file_len);
      // TODO: what if continuation slots used? we need to get rest of name
      m_name = file->name;
      
      if (m_type == SFS_ENTRY_FILE)
        SetWindowText("SFS Entry: File Entry");
      else
        SetWindowText("SFS Entry: Deleted File Entry");
    } break;
      
    case SFS_ENTRY_UNUSABLE: { // 0x18  // unusable entry (bad sector(s))
      
      SetWindowText("SFS Entry: Unusable Block Entry");
    } break;
      
    case SFS_ENTRY_UNUSED:     // 0x10  // unused
    default:
      AfxMessageBox("Unknown or Invalid Entry Found");
      GetDlgItem(IDC_ENTRY_CONT_COUNT)->EnableWindow(FALSE);
      GetDlgItem(IDC_ENTRY_TIME)->EnableWindow(FALSE);
      GetDlgItem(IDC_ENTRY_START)->EnableWindow(FALSE);
      GetDlgItem(IDC_ENTRY_END_BLOCK)->EnableWindow(FALSE);
      GetDlgItem(IDC_ENTRY_FILELEN)->EnableWindow(FALSE);
  }
  
  CDialog::OnInitDialog();
  
  OnChangeEntryTime();
  
  return TRUE;
}

void CSFSEntry::OnChangeEntryTime() {
  CString cs;
  
  // convert timestamp from 1/65536ths to milliseconds
  GetDlgItemText(IDC_ENTRY_TIME, cs);
  DWORD64 timestamp = ((INT64) convert64(cs) / 65536);
  if (timestamp > 0) {
    if (timestamp <= 0xFFFFFFFF) {  // VC++ 6.0 limited the value to 32-bits  (Crashes if larger)
      CTime cTime((time_t) timestamp);  // converts seconds since 1 Jan 1970 to CTime
      cs.Format("%04i/%02i/%02i  %02i:%02i:%02i", cTime.GetYear(), cTime.GetMonth(), cTime.GetDay(), cTime.GetHour(), cTime.GetMinute(), cTime.GetSecond());
    } else
      cs = "Error in Timestamp";
    SetDlgItemText(IDC_ENTRY_TIME_DISP, cs);
  } else
    SetDlgItemText(IDC_ENTRY_TIME_DISP, "");
}

void CSFSEntry::OnCrcUpdate() {
  // TODO: Check/Update the CRC for this entry (including continuation slots)
  AfxMessageBox("TODO");
}

void CSFSEntry::OnOK() {
  // TODO: Update the Entry from the dialog items
  // (name from continuation slots too)
  AfxMessageBox("TODO");
  CDialog::OnOK();
}

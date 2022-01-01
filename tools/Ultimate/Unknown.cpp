#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"
#include "FYSOSSig.h"

#include "Unknown.h"

#include "Attribute.h"

#include "Lean.h"
#include "Fat.h"
#include "ExFat.h"
#include "FYSFS.h"
#include "SFS.h"
#include "Ext2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnknown property page
IMPLEMENT_DYNCREATE(CUnknown, CPropertyPage)

CUnknown::CUnknown() : CPropertyPage(CUnknown::IDD) {
  //{{AFX_DATA_INIT(CUnknown)
  m_dump = _T("");
  //}}AFX_DATA_INIT
}

CUnknown::~CUnknown() {
}

void CUnknown::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CUnknown)
  DDX_Text(pDX, IDC_DUMP, m_dump);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUnknown, CPropertyPage)
  //{{AFX_MSG_MAP(CUnknown)
  ON_BN_CLICKED(ID_CLEAN, OnClean)
  ON_BN_CLICKED(ID_FORMAT, OnFormat)
  ON_BN_CLICKED(ID_INSERT, OnInsert)
  ON_BN_CLICKED(IDC_DUMP_PREV, OnDumpPrev)
  ON_BN_CLICKED(IDC_DUMP_NEXT, OnDumpNext)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnknown message handlers
BOOL CUnknown::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  // set the font of the DUMP window to a fixed font
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  GetDlgItem(IDC_DUMP)->SetFont(&dlg->m_DumpFont);
  
  // display the base/size string
  CString csBaseSize;
  csBaseSize.Format("Start: %I64i, Size: %I64i", m_lba, m_size);
  SetDlgItemText(IDC_BASE_SIZE_STR, csBaseSize);
  
  OnDumpPrev();
  
  return TRUE;
}

void CUnknown::Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  m_lba = lba;
  m_size = size;
  m_index = index;
  m_color = color;
  m_current = 0;
  
  // 
  m_psp.dwFlags |= PSP_USETITLE;
  m_psp.pszTitle = "Unknown";
  dlg->m_image_bar.UpdateTitle(dlg->Unknown[index].m_draw_index, "Unknown");
  
  // Add the page to the control
  if (IsNewTab)
    dlg->m_TabControl.AddPage(this);
  dlg->m_TabControl.SetActivePage(this);
}

// Simply zero out the "partition"
void CUnknown::OnClean() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  DWORD64 sect;
  
  CString cs;
  cs.Format("This will clear the partition.\r\n"
            "  Start LBA = %I64i\r\n"
            "    End LBA = %I64i\r\n"
            "  Proceed?\r\n", m_lba, m_lba + m_size - 1);
  if (AfxMessageBox(cs, MB_YESNO, 0) == IDYES) {
    CWaitCursor wait; // display a wait cursor
    memset(buffer, 0, dlg->m_sect_size);
    for (sect=m_lba; sect<m_lba + m_size - 1; sect++)
      dlg->WriteToFile(buffer, sect, 1);
    Start(m_lba, m_size, m_color, m_index, FALSE);
    OnDumpPrev();
  }
}

S_ATTRIBUTES format_attrbs[] = {
                //            |                               | <- max (col 67)
  { FS_LEAN,    FS_LEAN,   0, "Lean FS"                        , {-1, } },
  { FS_EXT2,    FS_EXT2,   1, "Linux Ext 2"                    , {-1, } },
  { FS_SFS,     FS_SFS,    2, "Simple FS"                      , {-1, } },
  { FS_EXFAT,   FS_EXFAT,  3, "exFAT"                          , {-1, } },
  { FS_FAT12,   FS_FAT12,  4, "FAT 12"                         , {-1, } },
  { FS_FAT16,   FS_FAT16,  5, "FAT 16"                         , {-1, } },
  { FS_FYSFS,   FS_FYSFS,  6, "FYS FS"                         , {-1, } },
  { FS_FAT32,   FS_FAT32,  7, "FAT 32"                         , {-1, } },
  { 0,        (DWORD) -1, -1, NULL                             , {-1, } }
};

void CUnknown::OnFormat() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  CAttribute adlg;
  
  adlg.m_title = "Choose File System:";
  adlg.m_attrib = 0;
  adlg.m_attributes = format_attrbs;
  adlg.m_single = TRUE;
  if (adlg.DoModal() == IDOK) {
    switch (adlg.m_attrib) {
      case FS_LEAN:
        FormatLean();
        break;
      case FS_EXT2:
        FormatExt2();
        break;
      case FS_SFS:
        FormatSFS();
        break;
      case FS_EXFAT:
        FormatExFat();
        break;
      case FS_FAT12:
        FormatFat(FS_FAT12);
        break;
      case FS_FAT16:
        FormatFat(FS_FAT16);
        break;
      case FS_FYSFS:
        FormatFYSFS();
        break;
      case FS_FAT32:
        FormatFat(FS_FAT32);
        break;
    }
    dlg->ReadFromFile(buffer, m_lba, 1);
    DumpIt(m_dump, buffer, 0, dlg->m_sect_size, FALSE);
    UpdateData(FALSE); // send to dialog
  }
}

void CUnknown::FormatLean(void) {
  CLean Lean;
  
  // build a "bogus" Lean partition
  Lean.m_lba = m_lba;
  Lean.m_size = m_size;
  Lean.m_hard_format = TRUE;
  
  // format it
  if (Lean.Format(TRUE))
    AfxMessageBox("Minimal format complete.  Close image file and re-open to parse correctly.");
  else
    AfxMessageBox("Format aborted.");
}

void CUnknown::FormatFat(int Type) {
  CFat Fat;
  
  // build a "bogus" Fat partition
  Fat.m_lba = m_lba;
  Fat.m_size = m_size;
  Fat.m_fat_size = Type;
  Fat.m_bpb_buffer = calloc(MAX_SECT_SIZE, 1);
  Fat.m_hard_format = TRUE;
  
  // format it
  if (Fat.FatFormat(TRUE))
    AfxMessageBox("Minimal format complete.  Close image file and re-open to parse correctly.");
  else
    AfxMessageBox("Format aborted.");
}

void CUnknown::FormatExFat(void) {
  CExFat ExFat;
  
  // build a "bogus" Fat partition
  ExFat.m_lba = m_lba;
  ExFat.m_size = m_size;
  ExFat.m_hard_format = TRUE;
  
  // format it
  if (ExFat.ExFatFormat(TRUE))
    AfxMessageBox("Minimal format complete.  Close image file and re-open to parse correctly.");
  else
    AfxMessageBox("Format aborted.");
}

void CUnknown::FormatFYSFS(void) {
  CFYSFS FYSFS;
  
  // build a "bogus" Fat partition
  FYSFS.m_lba = m_lba;
  FYSFS.m_size = m_size;
  FYSFS.m_hard_format = TRUE;
  
  // format it
  if (FYSFS.FYSFSFormat(TRUE, TRUE))
    AfxMessageBox("Minimal format complete.  Close image file and re-open to parse correctly.");
  else
    AfxMessageBox("Format aborted.");
}

void CUnknown::FormatSFS(void) {
  CSFS SFS;
  
  SFS.m_lba = m_lba;
  SFS.m_size = m_size;
  
  // format it
  SFS.m_hard_format = TRUE;

  if (SFS.Format(TRUE))
    AfxMessageBox("Minimal format complete.  Close image file and re-open to parse correctly.");
  else
    AfxMessageBox("Format aborted.");
}

void CUnknown::FormatExt2(void) {
  CExt2 Ext2;
  
  Ext2.m_lba = m_lba;
  Ext2.m_size = m_size;
  
  // format it
  Ext2.m_hard_format = TRUE;

  if (Ext2.Ext2Format(TRUE))
    AfxMessageBox("Minimal format complete.  Close image file and re-open to parse correctly.");
  else
    AfxMessageBox("Format aborted.");
}

void CUnknown::OnInsert() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  S_FYSOSSIG *s_sig;
  CString cs;
  CFile file;
  BOOL reload = FALSE;
  
  CFileDialog odlg (
    TRUE,             // Create an open file dialog
    _T(".img"),       // Default file extension
    NULL,             // Default Filename
    OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER, // flags
    _T(".img files (.img)|*.img|")    // Filter string
    _T(".bin files (.bin)|*.bin|")    // Filter string
    _T("All Files (*.*)|*.*|")        // Filter string
    _T("|")
  );
  if (odlg.DoModal() != IDOK)
    return;
  
  POSITION pos = odlg.GetStartPosition();
  cs = odlg.GetNextPathName(pos);
  
  if (file.Open(cs, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) == 0) {
    AfxMessageBox("Error Opening Image File...");
    return;
  }
  
  LARGE_INTEGER file_length = dlg->GetFileLength((HANDLE) file.m_hFile);
  DWORD64 TotalBlocks = (file_length.QuadPart + (dlg->m_sect_size - 1)) / dlg->m_sect_size;
  
  if (TotalBlocks > m_size) {
    int ret = AfxMessageBox("Size of image to insert is larger than space in this partition.  Truncate?", MB_TOPMOST | MB_DEFBUTTON3 | MB_ICONEXCLAMATION | MB_YESNOCANCEL, 0);
    if (ret == IDCANCEL) {
      file.Close();
      return;
    } else if (ret == IDYES)
      TotalBlocks = m_size;
  }
  
  cs.Format("This will overwrite the partition.\r\n"
            "  Start LBA = %I64i\r\n"
            "       Size = %I64i\r\n"
            "  Proceed?\r\n", m_lba, TotalBlocks);
  if (AfxMessageBox(cs, MB_YESNO, 0) == IDYES) {
    CWaitCursor wait; // display a wait cursor
    BYTE buffer[MAX_SECT_SIZE];
    DWORD64 sect;
    LONG lIdle = 0;
    for (sect=0; sect<TotalBlocks; sect++) {
      file.Read(buffer, dlg->m_sect_size);
      // if the first sector and we are FYSOS specific, update the base signature
      if ((sect == 0) && AfxGetApp()->GetProfileInt("Settings", "ForceFYSOS", TRUE)) {
        s_sig = (S_FYSOSSIG *) (buffer + S_FYSOSSIG_OFFSET);
        s_sig->base = m_lba;
        s_sig->sig = (rand() << 16) | rand();
        s_sig->boot_sig =0xAA55;
      }
      dlg->WriteToFile(buffer, m_lba + sect, 1);
      AfxGetApp()->OnIdle(lIdle++);  // avoid "Not Responding" notice
    }
    reload = TRUE;
  }
  
  file.Close();

  if (reload)
    dlg->SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
}

void CUnknown::OnDumpPrev() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  CString cs;
  
  if (m_current > 0)
    m_current--;
  
  GetDlgItem(IDC_DUMP_PREV)->EnableWindow(m_current > 0);
  GetDlgItem(IDC_DUMP_NEXT)->EnableWindow(m_current < (m_size - 1));
  cs.Format("LSN %I64i of %I64i", m_current, m_size - 1);
  SetDlgItemText(IDC_DUMP_STR, cs);
  
  dlg->ReadFromFile(buffer, m_lba + m_current, 1);
  DumpIt(m_dump, buffer, (DWORD) (m_current * dlg->m_sect_size), dlg->m_sect_size, FALSE);
  SetDlgItemText(IDC_DUMP, m_dump);
}

void CUnknown::OnDumpNext() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  CString cs;
  
  if (m_current < (m_size - 1))
    m_current++;
  
  GetDlgItem(IDC_DUMP_PREV)->EnableWindow(m_current > 0);
  GetDlgItem(IDC_DUMP_NEXT)->EnableWindow(m_current < (m_size - 1));
  cs.Format("LSN %I64i of %I64i", m_current, m_size - 1);
  SetDlgItemText(IDC_DUMP_STR, cs);
  
  dlg->ReadFromFile(buffer, m_lba + m_current, 1);
  DumpIt(m_dump, buffer, (DWORD) (m_current * dlg->m_sect_size), dlg->m_sect_size, FALSE);
  SetDlgItemText(IDC_DUMP, m_dump);
}

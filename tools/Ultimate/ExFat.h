#if !defined(AFX_EXFAT_H__990B37DC_4B4D_4698_AABF_40316E64814E__INCLUDED_)
#define AFX_EXFAT_H__990B37DC_4B4D_4698_AABF_40316E64814E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExFat.h : header file
//

#include "MyImageList.h"
#include "MyTreeCtrl.h"
#include "fat.h"

// Notes:
// - spec sheet: https://docs.microsoft.com/en-us/windows/win32/fileio/exfat-specification
// - To ensure interoperability of exFAT volumes in a broad set of usage scenarios, implementations should use 
//   partition type 07h for MBR partitioned storage and partition GUID {EBD0A0A2-B9E5-4433-87C0-68B6B72699C7} 
//   for GPT partitioned storage.

#pragma pack(push, 1)

#define EXFAT_FAT_BAD    0xFFFFFFF7
#define EXFAT_FAT_LAST   0xFFFFFFFF

#define EXFAT_ATTR_ARCHIVE   0x20
#define EXFAT_ATTR_SUB_DIR   0x10
#define EXFAT_ATTR_SYSTEM    0x04
#define EXFAT_ATTR_HIDDEN    0x02
#define EXFAT_ATTR_READ_ONLY 0x01
#define EXFAT_ATTR_ALL       0x37


#define EXFAT_DIR_EOD        0x00   // end of directory
#define EXFAT_DIR_LABEL_UN   0x03   // Volume Name Record. Master Entry (Unnamed Volume)
#define EXFAT_DIR_DELETED    0x05   // Deleted File Name Record
#define EXFAT_DIR_DELETED_S  0x40   // Deleted File Name Record, Stream Extension
#define EXFAT_DIR_DELETED_F  0x41   // Deleted File Name Record, Filename Extension
#define EXFAT_DIR_BITMAP     0x81   // Bitmap Logical Location and Size
#define EXFAT_DIR_UCASE      0x82   // Up-Case Table Logical Location and Size
#define EXFAT_DIR_LABEL      0x83   // Volume Name Record, Master Entry (Named Volume)
#define EXFAT_DIR_ENTRY      0x85   // Directory Entry Record
#define EXFAT_DIR_STRM_EXT   0xC0   // Directory Entry Record, Stream Extension
#define EXFAT_DIR_NAME_EXT   0xC1   // Directory Entry Record, Filename Extension
#define EXFAT_DIR_WINCE_ACC  0xE2   // 
#define EXFAT_DIR_GUID       0xA0   // 
#define EXFAT_DIR_TEXFAT     0xA1   // 

#define EXFAT_FLAGS_ALL_POS  (1<<0)
#define EXFAT_FLAGS_NO_FAT   (1<<1)

// this is the default size of the memory allocated for the root cache
//#define EXFAT_ROOT_DEF_SIZE  65536  // 65536 = 128 sectors of root data

typedef struct _S_EXFAT_VBR {
  BYTE    jmp[3];
  char    oem_name[8];
  BYTE    reserved0[53];      // should be zeros
  DWORD64 part_offset;
  DWORD64 total_sectors;
  DWORD   first_fat;
  DWORD   sect_per_fat;
  DWORD   data_region_lba;
  DWORD   data_region_size;   // in clusters
  DWORD   root_dir_cluster;
  DWORD   serial;
  WORD    fs_version;
  WORD    flags;
  BYTE    log_bytes_per_sect;
  BYTE    log_sects_per_clust;
  BYTE    num_fats;
  BYTE    drive_sel;
  BYTE    percent_heap;
  BYTE    reserved1[7];
}S_EXFAT_VBR;

// sector 9 has 10 of these 48-byte entries
//  (10 48-byte entries)
//  (28 bytes reserved)
//  (4 byte 0x0000AA55 (00 00 55 AA))
/*
struct S_EXFAT_PARAMS {
  struct S_GUID OemParameterType; // Value is OEM_FLASH_PARAMETER_GUID
  DWORD EraseBlockSize; // Erase block size in bytes
  DWORD PageSize;
  DWORD NumberOfSpareBlocks;
  DWORD tRandomAccess;  // Random Access Time in nanoseconds
  DWORD tProgram;       // Program time in nanoseconds
  DWORD tReadCycle;     // Serial read cycle time in nanoseconds
  DWORD tWriteCycle;    // Write Cycle time in nanoseconds
  BYTE  Reserved[4];
};
*/

#define EXFAT_MIN_SECONDARY_COUNT   2
#define EXFAT_MAX_SECONDARY_COUNT  18

#define EXFAT_MAX_NAME_PER_ENTRY   15  // max count of chars per entry
//#define EXFAT_NO_FAT_CHAIN_VALID   (1<<1)

typedef struct _S_EXFAT_ROOT {
  BYTE  entry_type;
  union {
    struct {
      BYTE    sec_count;
      WORD    crc;
      WORD    attributes;
      WORD    resv1;
      DWORD   created;
      DWORD   last_mod;
      DWORD   last_acc;
      BYTE    created_ms;
      BYTE    last_mod_ms;
      BYTE    created_tz_offset;
      BYTE    last_mod_tz_offset;
      BYTE    last_access_tz_offset;
      BYTE    resv2[7];
    } dir_entry;
    struct {
      BYTE    flags;
      BYTE    resv1;
      BYTE    name_len;
      WORD    name_hash;
      WORD    resv2;
      DWORD64 valid_data_len;
      DWORD   resv3;
      DWORD   first_cluster;
      DWORD64 data_len;
    } stream_ext;
    struct {
      BYTE    flags;
      WORD    name[15];
    } file_name_ext;
    struct {
      BYTE    len;
      WORD    name[11];
      BYTE    resv1[8];
    } label;
    struct {
      BYTE    flags;
      BYTE    resv1[18];
      DWORD   cluster_strt;
      DWORD64 data_len;
    } bitmap;
    struct {
      BYTE    resv1[3];
      DWORD   crc;
      BYTE    resv2[12];
      DWORD   cluster_strt;
      DWORD64 data_len;
    } up_case_table;
    struct {
      BYTE    sec_count;  // always 0
      WORD    crc;
      WORD    flags;
      S_GUID guid;
      BYTE    resv1[10];
    } guid;
  } type;
}S_EXFAT_ROOT;

// cannot be > 240 bytes
typedef struct _S_EXFAT_ITEMS {
  HTREEITEM hParent;    // parent directory so we can load it for this entry
  DWORD64 FileSize;     //
  DWORD Cluster;        //
  DWORD EntryOffset;    // byte offset from start of root to first entry of this file
  DWORD ErrorCode;
  BYTE  Flags;
  BOOL  CanCopy;        // the entry is not a deleted/invalid/other that we can copy out to the host
}S_EXFAT_ITEMS;

#pragma pack(pop)

/////////////////////////////////////////////////////////////////////////////
// CExFat dialog

class CExFat : public CPropertyPage {
  DECLARE_DYNCREATE(CExFat)
  
// Construction
public:
  CExFat();
  ~CExFat();
  
// Dialog Data
  //{{AFX_DATA(CExFat)
  enum { IDD = IDD_EXFAT };
  CMyTreeCtrl m_dir_tree;
  CString m_data_region_lba;
  CString m_data_region_size;
  CString m_drive_sel;
  CString m_fats;
  CString m_first_fat;
  CString m_flags;
  CString m_fs_version;
  CString m_jmp0;
  CString m_jmp1;
  CString m_jmp2;
  CString m_log_bytes_sect;
  CString m_log_bytes_cluster;
  CString m_oem_name;
  CString m_part_offset;
  CString m_percent_heap;
  CString m_reserved0;
  CString m_reserved1;
  CString m_root_cluster;
  CString m_sect_fat;
  CString m_serial_number;
  CString m_total_sectors;
  //}}AFX_DATA
  
  DWORD GetNewColor(int index);
  void Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab);

  void ParseDir(S_EXFAT_ROOT *root, DWORD64 root_size, HTREEITEM parent);
  void *ReadFile(DWORD cluster, DWORD64 *size, BYTE flags);
  void WriteFile(void *buffer, DWORD Cluster, DWORD64 size, BYTE flags);
  void ZeroCluster(DWORD Cluster);
  DWORD ExfatGetNextSect(DWORD cluster);
  WORD ExFatCheckDirCRC(BYTE *buffer, const int len);
  DWORD VBRChecksum(const void *buffer, const int len);
  int  ExFatFillClusterList(S_FAT_ENTRIES *EntryList, DWORD StartCluster, BYTE Flags);
  int  AllocateFAT(S_FAT_ENTRIES *EntryList, DWORD size, BYTE *flags);
  bool ExFatClearClusterChain(DWORD Cluster, DWORD64 datalen, BYTE Flags);
  void ExFATMarkFAT(DWORD Cluster, DWORD Value);
  void ExFATMarkBP(DWORD Cluster, bool Mark);
  void AllocateRoot(CString csName, S_EXFAT_ITEMS *RootItems, DWORD StartCluster, DWORD FileSize, BYTE flags, bool isFolder);
  WORD exFATNameHash(WORD *name, int length);
  
  void SendToDialog(S_EXFAT_VBR *vbr);
  void ReceiveFromDialog(S_EXFAT_VBR *vbr);
  void SaveItemInfo(HTREEITEM hItem, HTREEITEM hParent, DWORD Cluster, DWORD64 FileSize, BYTE flags, DWORD EntryOffset, DWORD ErrorCode, BOOL CanCopy);
  
  void CopyFile(HTREEITEM hItem, CString csName);
  void CopyFolder(HTREEITEM hItem, CString csPath, CString csName);
  BOOL InsertFile(S_EXFAT_ITEMS *RootItems, CString csName, CString csPath);
  void InsertFolder(S_EXFAT_ITEMS *RootItems, CString csName, CString csPath);
  void DeleteFile(HTREEITEM hItem);
  void DeleteFolder(HTREEITEM hItem);
  
  void *ExFatLoadFAT(void *fat_buffer);
  void ExFatWriteFAT(void *fat_buffer);
  void *ExFatLoadBP(void *bp_buffer);
  void ExFatWriteBP(void *bp_buffer);
  bool ExFatFormat(const BOOL AskForBoot);
  
  CMyImageList m_TreeImages;
  HTREEITEM m_hRoot;
  BOOL      m_too_many;
  
  void   *m_vbr_buffer;
  void   *m_fat_buffer;  // fat cluster buffer
  
  void   *m_bp_buffer;   // bitmap buffer
  DWORD   m_bp_cluster;  // cluster of bitmap
  DWORD64 m_bp_size;     // size in bytes of bitmap
  BYTE    m_bp_flags;    // flags of bitmap
  
  BOOL    m_isvalid;
  int     m_index; // index into dlg->ExFat[]
  DWORD64 m_lba;   // starting lba of this partition
  DWORD64 m_size;  // size of this partition in sectors
  DWORD   m_color; // color used in image bar
  int     m_draw_index;
  BOOL    m_hard_format;

  BOOL    m_show_del;
  BOOL    m_del_clear;
  
// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CExFat)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL
  
// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CExFat)
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
  afx_msg void OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnExFatApply();
  afx_msg void OnExFatClean();
  afx_msg void OnExFatFormat();
  afx_msg void OnExFatCheck();
  afx_msg void OnExFatCopy();
  afx_msg void OnExFatInsert();
  afx_msg void OnExFatEntry();
  afx_msg void OnFysosSig();
  afx_msg void OnChangeExFatFsVersion();
  afx_msg void OnSerialUpdate();
  afx_msg void OnExFatDelete();
  afx_msg void OnExpand();
  afx_msg void OnCollapse();
  afx_msg void OnChangeExfatLogBytesSect();
  afx_msg void OnChangeExfatLogSectCluster();
  afx_msg void OnFlags();
  afx_msg void OnShowDeleted();
  afx_msg void OnDelClear();
  afx_msg void OnExfatRestoreBackup();
  afx_msg void OnExfatUpdateBackup();
  afx_msg void OnExfatParamSect();
  afx_msg void OnUpdateCode();
  afx_msg void OnSearch();
  afx_msg void OnErase();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXFAT_H__990B37DC_4B4D_4698_AABF_40316E64814E__INCLUDED_)

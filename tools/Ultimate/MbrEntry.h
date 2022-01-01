#if !defined(AFX_MBRENTRY_H__CE4DA7DF_FB65_4EAE_974C_F70059D38CCB__INCLUDED_)
#define AFX_MBRENTRY_H__CE4DA7DF_FB65_4EAE_974C_F70059D38CCB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MbrEntry.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMbrEntry dialog

class CMbrEntry : public CPropertyPage {
  DECLARE_DYNCREATE(CMbrEntry)

// Construction
public:
  CMbrEntry();
  ~CMbrEntry();

// Dialog Data
  //{{AFX_DATA(CMbrEntry)
  enum { IDD = IDD_MBR_ENTRY };
  CString	m_begin_cyl;
  CString	m_begin_head;
  CString	m_begin_sect;
  CString	m_boot_id;
  CString	m_end_cyl;
  CString	m_end_head;
  CString	m_end_sect;
  CString m_size;
  CString m_start_lba;
  CString	m_sys_id;
  //}}AFX_DATA
  
  CString m_Title;
  int    m_index;
  BOOL   m_dirty;
  void  *m_parent;  // pointer to parent CMBR
  
// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CMbrEntry)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CMbrEntry)
  afx_msg void OnMbrBeginUpdate();
  afx_msg void OnMbrEndUpdate();
  afx_msg void OnMbrApply();
  afx_msg void OnChangeMbrSysId();
  virtual BOOL OnInitDialog();
  afx_msg void OnTabItemChange();
  afx_msg void OnTabItemChangeSize();
  afx_msg void OnSysId();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

  void LBAtoCHS(const DWORD lba, WORD *cyl, BYTE *head, BYTE *sect);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MBRENTRY_H__CE4DA7DF_FB65_4EAE_974C_F70059D38CCB__INCLUDED_)

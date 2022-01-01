#if !defined(AFX_ULTIMAGERESIZE_H__4B2FFF50_B1A8_45FE_BA06_6351A589D5B2__INCLUDED_)
#define AFX_ULTIMAGERESIZE_H__4B2FFF50_B1A8_45FE_BA06_6351A589D5B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UltimageResize.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUltimageResize dialog

class CUltimageResize : public CDialog
{
// Construction
public:
  CUltimageResize(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
  //{{AFX_DATA(CUltimageResize)
  enum { IDD = IDD_ULTIMATE_RESIZE };
  CString	m_cur_size;
  CString	m_new_size;
  CString m_title;
  //}}AFX_DATA


// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CUltimageResize)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(CUltimageResize)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
public:
  virtual BOOL OnInitDialog();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ULTIMAGERESIZE_H__4B2FFF50_B1A8_45FE_BA06_6351A589D5B2__INCLUDED_)

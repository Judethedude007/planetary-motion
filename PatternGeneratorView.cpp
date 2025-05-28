			// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface
// (the "Fluent UI") and is provided only as referential material to supplement the
// Microsoft Foundation Classes Reference and related electronic documentation
// included with the MFC C++ library software.
// License terms to copy, use or distribute the Fluent UI are available separately.
// To learn more about our Fluent UI licensing program, please visit
// https://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// PatternGeneratorView.cpp : implementation of the CPatternGeneratorView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "PatternGenerator.h"
#endif

#include "PatternGeneratorDoc.h"
#include "PatternGeneratorView.h"
#include <cmath>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPatternGeneratorView

IMPLEMENT_DYNCREATE(CPatternGeneratorView, CView)

BEGIN_MESSAGE_MAP(CPatternGeneratorView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CPatternGeneratorView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_BUTTON_START, &CPatternGeneratorView::OnButtonStart)
	ON_COMMAND(ID_BUTTON_STOP, &CPatternGeneratorView::OnButtonStop)
	ON_COMMAND(ID_BUTTON_RESUME, &CPatternGeneratorView::OnButtonResume)
END_MESSAGE_MAP()

// CPatternGeneratorView construction/destruction

CPatternGeneratorView::CPatternGeneratorView() noexcept
{
	// Initialize variables
	m_pCurrentThread = NULL;
	m_iCounter = 0;

	// Initialize planetary motion data
	m_EllipseCenter = CPoint(400, 300);
	m_EllipseMajor = 200;    // This will be the radius of the circular orbit
	m_EllipseMinor = 200;    // Make this equal to m_EllipseMajor for circular motion
	m_EllipseMajor = 200;    // This will be the radius of the circular orbit
	m_EllipseMinor = 200;    // Make this equal to m_EllipseMajor for circular motion
	m_EllipseMajor = 200;    // This will be the radius of the circular orbit
	m_EllipseMinor = 200;    // Make this equal to m_EllipseMajor for circular motion
	
	m_Planet1Angle = 0.0;
	m_Planet2Angle = 3.14159 / 2; // 90 degrees offset
	
	m_Planet1Speed = 0.05;
	m_Planet2Speed = 0.02;
	
	// Initial positions for planets
	m_Planet1 = CPoint(
		m_EllipseCenter.x + (int)(m_EllipseMajor * cos(m_Planet1Angle)),
		m_EllipseCenter.y + (int)(m_EllipseMinor * sin(m_Planet1Angle))
	);
	
	m_Planet2 = CPoint(
		m_EllipseCenter.x + (int)(m_EllipseMajor * 0.6 * cos(m_Planet2Angle)),
		m_EllipseCenter.y + (int)(m_EllipseMinor * 0.6 * sin(m_Planet2Angle))
	);
}

CPatternGeneratorView::~CPatternGeneratorView()
{
	// Make sure to terminate the thread safely if it exists
	if (m_pCurrentThread != NULL)
	{
		DWORD exitCode;
		if (GetExitCodeThread(m_pCurrentThread->m_hThread, &exitCode) && 
			exitCode == STILL_ACTIVE)
		{
			::TerminateThread(m_pCurrentThread->m_hThread, 0);
		}
		m_pCurrentThread = NULL;
	}
}

BOOL CPatternGeneratorView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

// CPatternGeneratorView drawing
void CPatternGeneratorView::OnDraw(CDC* pDC)
{
	CPatternGeneratorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// Create pens for drawing
	CPen redPen, bluePen, greenPen, blackPen;
	redPen.CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	bluePen.CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
	greenPen.CreatePen(PS_SOLID, 3, RGB(0, 255, 0));
	blackPen.CreatePen(PS_SOLID, 2, RGB(0, 0, 0));

	CPen* pOldPen = pDC->SelectObject(&blackPen);
		
	// Draw the circular orbit for the red planet
	CRect ellipseRect(
		m_EllipseCenter.x - m_EllipseMajor,
		m_EllipseCenter.y - m_EllipseMajor,
		m_EllipseCenter.x + m_EllipseMajor,
		m_EllipseCenter.y + m_EllipseMajor
	);
	pDC->Ellipse(ellipseRect);

	// Draw circular orbit for green planet (smaller, 0.6 ratio)
	CRect smallEllipseRect(
		m_EllipseCenter.x - (int)(m_EllipseMajor * 0.6),
		m_EllipseCenter.y - (int)(m_EllipseMajor * 0.6),
		m_EllipseCenter.x + (int)(m_EllipseMajor * 0.6),
		m_EllipseCenter.y + (int)(m_EllipseMajor * 0.6)
	);
	pDC->Ellipse(smallEllipseRect);

	// Draw the sun at the center
	CBrush yellowBrush(RGB(255, 255, 0));
	CBrush* pOldBrush = pDC->SelectObject(&yellowBrush);
	pDC->Ellipse(m_EllipseCenter.x - 20, m_EllipseCenter.y - 20,
				m_EllipseCenter.x + 20, m_EllipseCenter.y + 20);

	// Draw the planets
	// Planet 1 - Red planet
	pDC->SelectObject(&redPen);
	CBrush redBrush(RGB(255, 0, 0));
	pDC->SelectObject(&redBrush);
	pDC->Ellipse(m_Planet1.x - 10, m_Planet1.y - 10,
				m_Planet1.x + 10, m_Planet1.y + 10);

	// Planet 2 - Green planet
	pDC->SelectObject(&greenPen);
	CBrush greenBrush(RGB(0, 200, 0));
	pDC->SelectObject(&greenBrush);
	pDC->Ellipse(m_Planet2.x - 15, m_Planet2.y - 15,
				m_Planet2.x + 15, m_Planet2.y + 15);

	// Display counter info
	CString strCounter;
	strCounter.Format(L"Animation Frame: %d", m_iCounter);
	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->SetBkMode(TRANSPARENT);
	pDC->TextOut(50, 50, strCounter);

	// Restore original GDI objects
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}


// CPatternGeneratorView printing

void CPatternGeneratorView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CPatternGeneratorView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CPatternGeneratorView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CPatternGeneratorView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CPatternGeneratorView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CPatternGeneratorView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CPatternGeneratorView diagnostics

#ifdef _DEBUG
void CPatternGeneratorView::AssertValid() const
{
	CView::AssertValid();
}

void CPatternGeneratorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CPatternGeneratorDoc* CPatternGeneratorView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPatternGeneratorDoc)));
	return (CPatternGeneratorDoc*)m_pDocument;
}
#endif //_DEBUG


// CPatternGeneratorView message handlers
void CPatternGeneratorView::OnButtonStart()
{
	// If there's already a thread running, stop it
	if (m_pCurrentThread != NULL)
	{
		DWORD exitCode;
		if (GetExitCodeThread(m_pCurrentThread->m_hThread, &exitCode) && 
			exitCode == STILL_ACTIVE)
		{
			::TerminateThread(m_pCurrentThread->m_hThread, 0);
		}
		m_pCurrentThread = NULL;
	}
	
	// Reset counter and angles
	m_iCounter = 0;
	m_Planet1Angle = 0.0;
	m_Planet2Angle = 3.14159 / 2;
	
	// Reset planet positions for circular motion
	int radius1 = m_EllipseMajor;
	int radius2 = (int)(m_EllipseMajor * 0.6);
	
	m_Planet1 = CPoint(
		m_EllipseCenter.x + (int)(radius1 * cos(m_Planet1Angle)),
		m_EllipseCenter.y + (int)(radius1 * sin(m_Planet1Angle))
	);
	
	m_Planet2 = CPoint(
		m_EllipseCenter.x + (int)(radius2 * cos(m_Planet2Angle)),
		m_EllipseCenter.y + (int)(radius2 * sin(m_Planet2Angle))
	);
	
	// Start the animation thread
	m_pCurrentThread = AfxBeginThread(CPatternGeneratorView::StartThread, this);
}

void CPatternGeneratorView::OnButtonStop()
{
	// Safely suspend the thread
	if (m_pCurrentThread != NULL)
	{
		m_pCurrentThread->SuspendThread();
	}
}

void CPatternGeneratorView::OnButtonResume()
{
	// Safely resume the thread
	if (m_pCurrentThread != NULL)
	{
		m_pCurrentThread->ResumeThread();
	}
}

UINT CPatternGeneratorView::StartThread(LPVOID Param)
{
	CPatternGeneratorView* pView = (CPatternGeneratorView*)Param;
	if (pView == NULL)
		return 1;  // Exit if the view is null

	while (true)
	{
		// Update planetary positions with uniform circular motion
		pView->m_Planet1Angle += pView->m_Planet1Speed;
		if (pView->m_Planet1Angle >= 6.28318) // 2*pi
			pView->m_Planet1Angle -= 6.28318;
			
		pView->m_Planet2Angle += pView->m_Planet2Speed;
		if (pView->m_Planet2Angle >= 6.28318)
			pView->m_Planet2Angle -= 6.28318;
			
		// Calculate new positions with uniform circular radius
		int radius1 = pView->m_EllipseMajor; // Use major axis as radius for circular motion
		int radius2 = (int)(pView->m_EllipseMajor * 0.6); // Use 0.6 of major axis as radius for second planet
		
		pView->m_Planet1.x = pView->m_EllipseCenter.x + (int)(radius1 * cos(pView->m_Planet1Angle));
		pView->m_Planet1.y = pView->m_EllipseCenter.y + (int)(radius1 * sin(pView->m_Planet1Angle));
		
		pView->m_Planet2.x = pView->m_EllipseCenter.x + (int)(radius2 * cos(pView->m_Planet2Angle));
		pView->m_Planet2.y = pView->m_EllipseCenter.y + (int)(radius2 * sin(pView->m_Planet2Angle));
		
		// Increment counter
		pView->m_iCounter++;
		
		// Trigger redraw
		pView->Invalidate();

		// Sleep to control animation speed
		Sleep(50);
	}

	return 0;
}

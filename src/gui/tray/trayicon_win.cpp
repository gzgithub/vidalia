/*
 * trayicon_win.cpp - Windows trayicon, adapted from Qt example
 * Copyright (C) 2003  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "trayicon.h"

#include <QWidget>
#include <QApplication>
#include <QImage>
#include <QPixmap>
#include <QBitmap>
#include <QCursor>
#include <QLibrary>
#include <QEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QSysInfo>

#include <qt_windows.h>

static uint WM_TASKBARCREATED = 0;
#define WM_NOTIFYICON	(WM_APP+101)

typedef BOOL (WINAPI *PtrShell_NotifyIcon)(DWORD,PNOTIFYICONDATA);
static PtrShell_NotifyIcon ptrShell_NotifyIcon = 0;

typedef HBITMAP (WINAPI *PtrCreateBitmap)(int,int,UINT,UINT,CONST VOID*);
static PtrCreateBitmap ptrCreateBitmap = 0;

typedef BOOL (WINAPI *PtrDeleteObject)(HGDIOBJ);
static PtrDeleteObject ptrDeleteObject = 0;

static void 
resolveLibs()
{
  QLibrary lib_shell32("shell32");
  QLibrary lib_gdi32("gdi32");

  //lib.setAutoUnload(FALSE);
  lib_shell32.load();
  lib_gdi32.load();

  static bool triedResolve = FALSE;
  if ((!ptrShell_NotifyIcon || !ptrCreateBitmap || !ptrDeleteObject) && !triedResolve) {
    triedResolve = TRUE;
    ptrShell_NotifyIcon = (PtrShell_NotifyIcon)lib_shell32.resolve("Shell_NotifyIconW");
    ptrCreateBitmap = (PtrCreateBitmap)lib_gdi32.resolve("CreateBitmap");
    ptrDeleteObject = (PtrDeleteObject)lib_gdi32.resolve("DeleteObject");
  }
}

class TrayIcon::TrayIconPrivate : public QWidget
{
public:
  HICON		 hIcon;
  HBITMAP  hMask;
  TrayIcon *iconObject;

  TrayIconPrivate(TrayIcon *object)
      : QWidget(0), hIcon(0), hMask(0), iconObject(object)
  {
    if (!WM_TASKBARCREATED)
      WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));
    resolveLibs();
  }

  ~TrayIconPrivate()
  {
    if (hMask) {
      ptrDeleteObject( hMask );
      hMask = 0; // michalj
    }
    if (hIcon) {
      DestroyIcon(hIcon);
      hIcon = 0; // michalj
    }
  }

  // the unavoidable A/W versions. Don't forget to keep them in sync!
  bool trayMessageA(DWORD msg)
  {
    NOTIFYICONDATAA tnd;
    ZeroMemory(&tnd, sizeof(NOTIFYICONDATAA));
    tnd.cbSize  = sizeof(NOTIFYICONDATAA);
    tnd.hWnd    = winId();
    tnd.uID = 1; // michalj

    if (msg != NIM_DELETE) {
      tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
      tnd.uCallbackMessage = WM_NOTIFYICON;
      tnd.hIcon = hIcon;
      if (!iconObject->toolTip().isNull()) {
        // Tip is limited to 63 + NULL; lstrcpyn appends a NULL terminator.
        QString tip = iconObject->toolTip().left(63) + QChar();
        lstrcpynA(tnd.szTip, (const char*)tip.toLocal8Bit(), qMin(tip.length()+1, 64));
      }
    }
    return Shell_NotifyIconA(msg, &tnd);
  }

#ifdef UNICODE
    bool trayMessageW(DWORD msg)
    {           
      if (!ptrShell_NotifyIcon)
        return trayMessageA(msg);

      NOTIFYICONDATAW tnd;
      ZeroMemory(&tnd, sizeof(NOTIFYICONDATAW));
      tnd.cbSize = sizeof(NOTIFYICONDATAW);
      tnd.hWnd = winId();
      tnd.uID = 1; // michalj

      if (msg != NIM_DELETE) {
        tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        tnd.uCallbackMessage = WM_NOTIFYICON;
        tnd.hIcon = hIcon;
      
        if (!iconObject->toolTip().isNull()) {
          // Tip is limited to 63 + NULL; lstrcpyn appends a NULL terminator.
          QString tip = iconObject->toolTip().left(63) + QChar();
          lstrcpynW(tnd.szTip, (TCHAR*)tip.unicode(), qMin(tip.length()+1, 64));
        }
      }
      return ptrShell_NotifyIcon(msg, &tnd);
    }
#endif

    bool 
    trayMessage(DWORD msg)
    {
      QT_WA(return trayMessageW(msg);,
            return trayMessageA(msg);)
    }

    bool 
    iconDrawItem(LPDRAWITEMSTRUCT lpdi)
    {
      if (!hIcon)
        return FALSE;

      DrawIconEx(lpdi->hDC, lpdi->rcItem.left, lpdi->rcItem.top, hIcon, 0, 0, 0, NULL, DI_NORMAL);
      return TRUE;
    }

    bool 
    winEvent(MSG *m, long *result)
    {
      switch(m->message) {
        case WM_DRAWITEM:
          return iconDrawItem( (LPDRAWITEMSTRUCT)m->lParam );
        case WM_NOTIFYICON:
        {
          QMouseEvent *e = 0;
          QPoint gpos = QCursor::pos();
          switch (m->lParam) {
            case WM_MOUSEMOVE: 
              e = new QMouseEvent(QEvent::MouseMove, mapFromGlobal(gpos), gpos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);	
              break;
            case WM_LBUTTONDOWN:  
              e = new QMouseEvent(QEvent::MouseButtonPress, mapFromGlobal(gpos), gpos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
              break;
            case WM_LBUTTONUP: 
              e = new QMouseEvent(QEvent::MouseButtonRelease, mapFromGlobal(gpos), gpos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier); 
              break;
            case WM_LBUTTONDBLCLK:
              e = new QMouseEvent(QEvent::MouseButtonDblClick, mapFromGlobal(gpos), gpos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
              break;
            case WM_RBUTTONDOWN: 
              e = new QMouseEvent(QEvent::MouseButtonPress, mapFromGlobal(gpos), gpos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
              break;
            case WM_RBUTTONUP: 
              e = new QMouseEvent(QEvent::MouseButtonRelease, mapFromGlobal(gpos), gpos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
              break;
           case WM_RBUTTONDBLCLK: 
             e = new QMouseEvent(QEvent::MouseButtonDblClick, mapFromGlobal(gpos), gpos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
             break;
           case WM_MBUTTONDOWN:
             e = new QMouseEvent(QEvent::MouseButtonPress, mapFromGlobal(gpos), gpos, Qt::MidButton, Qt::MidButton, Qt::NoModifier);
             break;
           case WM_MBUTTONUP: 
             e = new QMouseEvent(QEvent::MouseButtonRelease, mapFromGlobal(gpos), gpos, Qt::MidButton, Qt::MidButton, Qt::NoModifier);
             break;
           case WM_MBUTTONDBLCLK: 
             e = new QMouseEvent(QEvent::MouseButtonDblClick, mapFromGlobal(gpos), gpos, Qt::MidButton, Qt::MidButton, Qt::NoModifier);
             break;
           case WM_CONTEXTMENU: 
             e = new QMouseEvent(QEvent::MouseButtonRelease, mapFromGlobal(gpos), gpos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
             break;
        }
        if (e) {
          bool res = QApplication::sendEvent(iconObject, e);
          delete e;
          return res;
        }
      }
      break;
      default:
        if (m->message == WM_TASKBARCREATED) 
          trayMessage(NIM_ADD);
    }
    return QWidget::winEvent(m, result);
  }
};

static HBITMAP 
createIconMask(const QPixmap &qp)
{
  QImage bm = qp.toImage();
  int w = bm.width();
  int h = bm.height();
  int bpl = ((w+15)/16)*2; /* bpl, 16 bit alignment */
  
  uchar *bits = new uchar[bpl*h];
  bm.invertPixels();
  for (int y = 0; y < h; y++) {
    memcpy(bits+y*bpl, bm.scanLine(y), bpl);
  }
  HBITMAP hbm = ptrCreateBitmap(w, h, 1, 1, bits);
  delete [] bits;
  
  return hbm;
}

static HICON 
createIcon(const QPixmap &pm, HBITMAP &hbm)
{
  QPixmap maskpm(pm.size());
  QPixmap mask(pm.size());
  if (!pm.mask().isNull()) {
    QPainter p(&mask);
    p.drawPixmap(0, 0, pm.mask());
    p.end();
  } else {
    mask.fill(Qt::color0);
    maskpm.fill(Qt::color1);
  }
  
  QPainter q(&maskpm);
  q.drawPixmap(0, 0, pm);
  q.end();

  ICONINFO iconInfo;
  iconInfo.fIcon    = TRUE;
  iconInfo.hbmMask  = hbm = createIconMask(mask);
  iconInfo.hbmColor = maskpm.toWinHBITMAP();

  HICON icon = CreateIconIndirect(&iconInfo);
  ptrDeleteObject(iconInfo.hbmMask);
  iconInfo.hbmMask = hbm = 0; // michalj
  return icon;
}

void 
TrayIcon::sysInstall()
{
  if (!d) {
    d = new TrayIconPrivate(this);
    d->hIcon = createIcon(pm, d->hMask);
    d->trayMessage(NIM_ADD);
  }
}

void 
TrayIcon::sysRemove()
{
  if (d) {
    d->trayMessage(NIM_DELETE);

    delete d;
    d = 0;
  }
}

void 
TrayIcon::sysUpdateIcon()
{
  if ( d ) {
    if ( d->hMask )
      if ( d->hMask ) {
        ptrDeleteObject( d->hMask );
        if ( d->hIcon ) {
          d->hMask = 0; // michalj
        }
      }
    if ( d->hIcon ) {
      DestroyIcon( d->hIcon );
      d->hIcon = 0; // michalj
    }
    
    d->hMask = createIconMask( pm );
 		d->hIcon = createIcon( pm, d->hMask );
 		d->trayMessage( NIM_MODIFY );
  }
}

void 
TrayIcon::sysUpdateToolTip()
{
  if (d)
    d->trayMessage(NIM_MODIFY);
}

/****************************************************************
 *  $Id$
 * 
 *  Vidalia is distributed under the following license:
 *
 *  Copyright (C) 2006,  Matt Edman, Justin Hipple
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#include "../mainwindow.h"
#include "messagelog.h"

MessageLog::MessageLog(QWidget *parent, Qt::WFlags flags)
: QMainWindow(parent, flags)
{
  ui.setupUi(this);
 
  _settings = new VidaliaSettings();
  _clock = new QDateTime(QDateTime::currentDateTime());
 
  _messagesShown = 0;
  _maxCount = _settings->getMaxMsgCount();

  _createActions();
}

/*
 Binds events to actions 
*/
void
MessageLog::_createActions()
{
  connect(ui.actionHistory_Size, SIGNAL(triggered()), 
      this, SLOT(setMaxCount()));
  
  connect(ui.actionMessage_Filters, SIGNAL(triggered()),
      this, SLOT(setMsgFilter()));
}

/*
 Creates a dialog that allows changing of the maxiumum
 number of message to display in the message history log.
*/
void
MessageLog::setMaxCount()
{
  bool ok_clicked = false;
  int value = 0;
  
  value = QInputDialog::getInteger(this, tr("Enter Max History Size"),
      tr("(1 - 99,999 items)"), _maxCount, 1, 99999, 1, &ok_clicked);
  
  if (ok_clicked) {
    /* if new max is < number of shown messages then remove some */
    while (value < _messagesShown) {
      if (!ui.lstMessages->isItemHidden(ui.lstMessages->topLevelItem(0))) {
        _messagesShown--;
      }
      ui.lstMessages->takeTopLevelItem(0);
    }
    _settings->setMaxMsgCount(value);
    _maxCount = value;
  }
}

/*
 Creates a dialog that allows changing of the message
 filter, that is, which messages the log displays
*/
void
MessageLog::setMsgFilter()
{
  bool ok_clicked = false;
  
  FilterDialog* filterDialog = new FilterDialog(_settings, this);
  ok_clicked = filterDialog->exec();
  
  /* If filters changed, refilter the log message list */
  if (ok_clicked) {
    _filterLog(); 
  }
}

/*
 Cycles through the list, hiding and showing appropriate messages.
 Removes messages if newly shown messages put us over _maxCount.
*/
void
MessageLog::_filterLog()
{
  QTreeWidgetItem* current = new QTreeWidgetItem();
  int currentIndex = ui.lstMessages->topLevelItemCount() - 1;
  bool showCurrent;
  _messagesShown = 0;
  
  while (currentIndex > -1) {
    current = ui.lstMessages->topLevelItem(currentIndex);
    
    /* Keep ALL messages until SHOWING maximum possible */
    if (_messagesShown < _maxCount) {
      
      /* Show or hide message accordingly */
      showCurrent = _settings->getShowMsg(current->text(1));
      ui.lstMessages->setItemHidden(current, !showCurrent);
      if (showCurrent) {
        _messagesShown++;
      }
    /* If we are showing the maximum, then get rid of the rest */
    } else {
      ui.lstMessages->takeTopLevelItem(currentIndex);
    }
    currentIndex--;
  }
}

/*
 Writes a message to the Message History and tags it with
 the proper date, time and type.
*/
void 
MessageLog::write(const char* type, const char* message)
{
  QTreeWidgetItem *newMessage = new QTreeWidgetItem();
 
  /* Remove top message if message log is at maximum setting */
  if (ui.lstMessages->topLevelItemCount() == _maxCount) {
    ui.lstMessages->takeTopLevelItem(0);
  }
  
  /* Change row color and text for serious warnings and errors */
  if (!qstrcmp(type, MSG_VIDERR) or !qstrcmp(type, MSG_TORERR)) {
    /* Critical messages are red with white text */
    for (int i=0; i < ui.lstMessages->columnCount(); i++) {
      newMessage->setBackgroundColor(i, Qt::red);
      newMessage->setTextColor(i, Qt::white);
    }
  } else if (!qstrcmp(type, MSG_TORWARN)) {
    /* Warning messages are yellow with black text */
    for (int i=0; i < ui.lstMessages->columnCount(); i++) {
      newMessage->setBackgroundColor(i, Qt::yellow);
    }
  }
    
  /* Set Time */
  newMessage->setText(0, _clock->currentDateTime().toString(Qt::TextDate));

  /* Set Type */
  newMessage->setTextAlignment(1, Qt::AlignCenter);
  newMessage->setText(1, tr(type));

  /* Set Message Body */
  newMessage->setText(2, tr(message));

  /* Add the message to the bottom of the list */
  ui.lstMessages->addTopLevelItem(newMessage);

  /* Hide the message if necessary */
  if (_settings->getShowMsg(type)) {
    _messagesShown++;
  } else {
    ui.lstMessages->setItemHidden(newMessage, true);
  }
}

/** Overloads the default close() slot, so we can force the parent to become
 * visible. This only matters on Mac, so we can ensure the correct menubar is
 * displayed. */
void
MessageLog::close()
{
  MainWindow *p = (MainWindow *)parent();
  if (p) {
    p->show();
  }
  QMainWindow::close();
}

/** Serves the same purpose as MessageLog::close(), but this time responds to
 * when the user clicks on the X in the titlebar */
void
MessageLog::closeEvent(QCloseEvent *e)
{
  MainWindow *p = (MainWindow *)parent();
  if (p) {
    p->show();
  }
  e->accept();
}


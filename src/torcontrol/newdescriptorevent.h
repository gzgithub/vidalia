/****************************************************************
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

/** 
 * \file newdescriptorevent.h
 * \version $Id$
 * \brief Event dispatched upon receiving a new descriptor from Tor
 */

#ifndef _NEWDESCRIPTOREVENT_H
#define _NEWDESCRIPTOREVENT_H

#include <QStringList>
#include <QEvent>


class NewDescriptorEvent : public QEvent
{
public:
  /** Default constructor */
  NewDescriptorEvent(QStringList idList)
  : QEvent((QEvent::Type)CustomEventType::NewDescriptorEvent)
  { _idList = idList; }
  
  /** Returns a list of new server IDs. */
  QStringList descriptorIDs() const { return _idList; }
  
private:
  /** A list of new descriptors available. */
  QStringList _idList;
};

#endif

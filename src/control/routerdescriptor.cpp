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
 * \file routerdescriptor.cpp
 * \version $Id$
 * \brief Parses a blob of router descriptor text from Tor
 */

#include <QtGlobal>

#include "routerdescriptor.h"


/** Constructor. Just assigns the ID and determines whether the router is
 * responsive or not based on the presence of a "!" at the start of the ID.
 * See tor-spec.txt for details. */
RouterDescriptor::RouterDescriptor(QStringList descriptor)
{
  _status = Online;
  parseDescriptor(descriptor);
}

/** Parses this router's descriptor for relevant information. */
void
RouterDescriptor::parseDescriptor(QStringList descriptor)
{
  foreach (QString line, descriptor) {
    if (line.startsWith("router ")) {
      QStringList parts = line.remove(0,qstrlen("router ")).split(" ");
      _name    = parts.at(0);
      _ip      = QHostAddress(parts.at(1));
      _orPort  = (quint16)parts.at(2).toUInt();
      _dirPort = (quint16)parts.at(4).toUInt();
    } else if (line.startsWith("platform ")) {
      _platform = line.remove(0,qstrlen("platform "));
    } else if (line.startsWith("published ")) {
      _published = QDateTime::fromString(
                               line.remove(0,qstrlen("published ")),
                               "yyyy-MM-dd HH:mm:ss");
    } else if (line.startsWith("opt fingerprint ")) {
      _fingerprint = line.remove(0,qstrlen("opt fingerprint "));
      _id = _fingerprint.remove(" ");
    } else if (line.startsWith("fingerprint ")) {
      _fingerprint = line.remove(0,qstrlen("fingerprint "));
      _id = _fingerprint.remove(" ");
    } else if (line.startsWith("uptime ")) {
      _uptime = (quint64)line.remove(0,qstrlen("uptime ")).toULongLong();
    } else if (line.startsWith("bandwidth ")) {
      QStringList bw = line.remove(0,qstrlen("bandwidth ")).split(" ");
      _avgBandwidth      = (quint64)bw.at(0).toULongLong();
      _burstBandwidth    = (quint64)bw.at(1).toULongLong();
      _observedBandwidth = (quint64)bw.at(2).toULongLong();
    } else if (line.startsWith("contact ")) {
      _contact = line.remove(0,qstrlen("contact "));
    } else if (line.startsWith("hibernating ")) {
      if (line.remove(0,qstrlen("hibernating ")).trimmed() == "1") {
        _status = Hibernating;
      }
    }
  }
}

/** Returns a string representation of the status of this router. */
QString
RouterDescriptor::status()
{
  if (_status == Online) {
    return tr("Online");
  } else if (_status == Hibernating) {
    return tr("Hibernating");
  }
  return tr("Offline");
}

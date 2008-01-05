/****************************************************************
 *  Vidalia is distributed under the following license:
 *
 *  Copyright (C) 2006-2007,  Matt Edman, Justin Hipple
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
 * \file circuit.h
 * \version $Id$
 * \brief Object representing a Tor circuit
 */

#ifndef _CIRCUIT_H
#define _CIRCUIT_H

#include <QCoreApplication>
#include <QStringList>
#include <QString>


class Circuit
{
  Q_DECLARE_TR_FUNCTIONS(Circuit)

public:
  /** Circuit status events */
  enum Status {
    Unknown,   /**< Unknown circuit status */
    Launched,  /**< Circuit ID assigned to new circuit */
    Built,     /**< All hops finished */
    Extended,  /**< Circuit extended by one hop */
    Failed,    /**< Circuit closed (was not built) */
    Closed     /**< Circuit closed (was built) */
  };

  /** Default constructor. */
  Circuit();
  /** Constructor */
  Circuit(quint64 circId, Status status, QString path);
  /** Constructor */
  Circuit(quint64 circId, Status status, QStringList hops);

  /** Parses the given string (in Tor's control protocol format) */
  static Circuit fromString(QString circuit);
  /** Converts a string description of a circuit's status to an enum value */
  static Status toStatus(QString strStatus);
  
  /** Returns true if all fields in this Circuit are empty. */
  bool isEmpty();
  
  /** Returns the ID for this circuit */
  quint64 id() { return _circId; }
  /** Returns the status of this circuit */
  Status status() { return _status; }
  /** Returns a string representation of the status of this circuit. */
  QString statusString();
  /** Returns the path chosen for this circuit */
  QString path() { return _path; }
  /** Returns the length of the circuit's path. */
  uint length() { return hops().size(); }
  /** Returns a list of hops on the path. */
  QStringList hops() { return _path.isEmpty() ? QStringList() 
                                              : _path.split(","); }

private:
  quint64 _circId; /**< Circuit ID. */
  Status _status;  /**< Circuit status. */
  QString _path;   /**< Circuit path. */
};

#endif

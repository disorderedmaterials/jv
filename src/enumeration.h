/*
	*** Enumeration
	*** src/enumeration.h
	Copyright T. Youngs 2012-2016

	This file is part of JournalViewer.

	JournalViewer is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	JournalViewer is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with JournalViewer.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef JOURNALVIEWER_ENUMERATION_H
#define JOURNALVIEWER_ENUMERATION_H

#include "list.h"
#include <QString>

// Forward Declarations
/* none */

// Enumerated value, storing name and integer index
class EnumeratedValue : public ListItem<EnumeratedValue>
{
	public:
	// Constructor
	EnumeratedValue(QString name, int value);

	private:
	// Text name of enumerated value
	QString name_;
	// Integer value
	int value_;

	public:
	// Return name of enumerated value
		const QString& name() const;
	// Return value
	int value() const;
};

class Enumeration : public ListItem<Enumeration>
{
	public:
	// Constructor
	Enumeration(QString name);

	private:
	// Name associated to enumeration
	QString name_;
	// List of enumerated values
	List<EnumeratedValue> values_;

	public:
	// Return name of enumeration
	QString name();
	// Find/create enumerated value
	EnumeratedValue* value(QString value);
};

#endif

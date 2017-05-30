/*
	*** Enumeration Functions
	*** src/enumeration.cpp
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

#include "enumeration.h"


/*
 * Enumerated Value
 */

// Constructor
EnumeratedValue::EnumeratedValue(QString name, int value) : ListItem<EnumeratedValue>()
{
	name_ = name;
	value_ = value;
}

// Return name of enumerated value
const QString& EnumeratedValue::name() const
{
	return name_;
}

// Return value
int EnumeratedValue::value() const
{
	return value_;
}

/*
 * Enumeration
 */

// Constructor
Enumeration::Enumeration(QString name) : ListItem<Enumeration>()
{
	name_ = name;
}

// Return name of enumeration
QString Enumeration::name()
{
	return name_;
}

// Find/create enumerated value
EnumeratedValue* Enumeration::value(QString value)
{
	// Does existing value exist in list?
	for (EnumeratedValue* ev = values_.first(); ev != NULL; ev = ev->next) if (ev->name() == value) return ev;

	// Nope - create new one
	EnumeratedValue* newValue = new EnumeratedValue(value, values_.nItems());
	values_.own(newValue);
	return newValue;
}

/*
	*** TTreeWidgetItem - QTreeWidgetItem with data awareness for sorting
	*** src/ttablewidgetitem.h
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

#ifndef JV_TTABLEWIDGETITEM_H
#define JV_TTABLEWIDGETITEM_H

#include <QTreeWidgetItem>

// Foward Declarations
class InstrumentBlockValue;

class TTreeWidgetItem : public QTreeWidgetItem
{
	public:
	// Constructors
	TTreeWidgetItem(QTreeWidgetItem* parent, InstrumentBlockValue* block);
	TTreeWidgetItem(QTreeWidget* parent, QString name);

	private:
	// Source block value
	InstrumentBlockValue* block_;

	public:
	// Return source block value
	InstrumentBlockValue* block();
};

#endif

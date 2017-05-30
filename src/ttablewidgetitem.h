/*
	*** TTableWidgetItem - QTableWidgetItem with data awareness for sorting
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

#include <QTableWidgetItem>

// Foward Declarations
class RunData;

class TTableWidgetItem : public QTableWidgetItem
{
	public:
	// Sorting type (determines sorting method)
	enum SortAsType { IntegerSort, DoubleSort, DurationSort, DateSort, StringSort };
	// Constructor
	TTableWidgetItem(RunData* source, const QString& data, TTableWidgetItem::SortAsType sortType);


	private:
	// Source RunData
	RunData* source_;
	// Sorting Type
	SortAsType sortType_;

	public:
	// Return source RunData
	RunData* source();

	
	/*
	// Virtuals
	*/
	public:
	// Less than operator (<)
	bool operator<(const QTableWidgetItem& other) const;
};

#endif

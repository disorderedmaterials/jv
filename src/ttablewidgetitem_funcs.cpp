/*
	*** TTableWidgetItem Functions
	*** src/ttablewidgetitem_funcs.cpp
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

#include "ttablewidgetitem.h"
#include "rundata.h"

// Constructor
TTableWidgetItem::TTableWidgetItem(RunData* source, const QString& data, TTableWidgetItem::SortAsType sortType) : QTableWidgetItem()
{
	setText(data);
	sortType_ = sortType;
	source_ = source;
}

// Return source RunData
RunData* TTableWidgetItem::source()
{
	return source_;
}

/*
// Virtuals
*/
bool TTableWidgetItem::operator<(const QTableWidgetItem& other) const
{
	const TTableWidgetItem* tOther = (TTableWidgetItem*) &other;
	switch (sortType_)
	{
		case (TTableWidgetItem::IntegerSort):
			return atoi(qPrintable(text())) < atoi(qPrintable(other.text()));
			break;
		case (TTableWidgetItem::StringSort):
			return text() < other.text();
			break;
		case (TTableWidgetItem::DoubleSort):
			return atof(qPrintable(text())) < atof(qPrintable(other.text()));
			break;
		case (TTableWidgetItem::DateSort):
			return QDateTime::fromString(text()) < QDateTime::fromString(other.text());
			break;
		case (TTableWidgetItem::DurationSort):
			return source_->duration() < tOther->source_->duration();
			break;
	}
	return false;
}

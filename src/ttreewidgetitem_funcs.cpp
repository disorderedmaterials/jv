/*
	*** TTreeWidgetItem Functions
	*** src/ttreewidgetitem_funcs.cpp
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

#include "ttreewidgetitem.h"
#include "instrument.h"

// Constructors
TTreeWidgetItem::TTreeWidgetItem(QTreeWidgetItem* parent, InstrumentBlockValue* block) : QTreeWidgetItem(parent)
{
	block_ = block;
	setCheckState(0, block_->selected() ? Qt::Checked : Qt::Unchecked);
	setText(1, block_->name());
	if (block_->value().isEmpty()) setText(2, block_->value());
	else setText(2, block_->value() + QString(" ") + block_->units());
}
TTreeWidgetItem::TTreeWidgetItem(QTreeWidget* parent, QString name) : QTreeWidgetItem(parent)
{
	block_ = NULL;
	setText(0, name);
}

// Return associated block value
InstrumentBlockValue* TTreeWidgetItem::block()
{
	return block_;
}


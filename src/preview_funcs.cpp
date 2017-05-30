/*
	*** Preview Functions
	*** src/preview_funcs.cpp
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

#include "preview.hui"

// Constructors
Preview::Preview(QWidget* parent) : QWidget(parent)
{
}

// Destructor
Preview::~Preview()
{
}

// Return Document
// Document& Preview::document()
// {
// 	return document_;
// }

// General repaint callback
void Preview::paintEvent(QPaintEvent* event)
{
// 	if (source_ == NULL) return;

}

/*
	*** Log Window
	*** src/logwindow.h
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

#ifndef JOURNALVIEWER_LOGWINDOW_H
#define JOURNALVIEWER_LOGWINDOW_H

#include "ui_logwindow.h"

// Forward Declarations
/* none */

class LogWindow : public QDialog
{
	// All Qt declarations must include this macro
	Q_OBJECT


	/*
	// Window Functions
	*/
	public:
	// Constructor / Destructor
	LogWindow(QWidget* parent);
	~LogWindow();
	// Main form declaration
	Ui::LogWindow ui;


	/*
	// Widget Slots
	*/
	private slots:
	// Close button
	void on_CloseButton_clicked(bool checked);
};

#endif

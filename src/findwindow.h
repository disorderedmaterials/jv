/*
	*** Find Window
	*** src/findwindow.h
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

#ifndef JOURNALVIEWER_FINDWINDOW_H
#define JOURNALVIEWER_FINDWINDOW_H

#include "ui_findwindow.h"

// Forward Declarations
class JournalViewer;

class FindWindow : public QDialog
{
	// All Qt declarations must include this macro
	Q_OBJECT


	/*
	// Window Functions
	*/
	public:
	// Constructor / Destructor
	FindWindow(JournalViewer& parent);
	~FindWindow();
	// Main form declaration
	Ui::FindWindow ui;
	// Parent JournalViewer window
	JournalViewer& jvParent_;

	private:
	// Whether window is currently refreshing
	bool refreshing_;

	public:
	// Refresh window
	bool refresh();


	/*
	// Widget Slots
	*/
	private slots:
	// Search text changed
	void on_TextEdit_textChanged(QString text);
	// Text edit return pressed
	void on_TextEdit_returnPressed();
	// Search style changed
	void on_SearchStyleCombo_currentIndexChanged(int index);
	// Case sensitivity changed
	void on_CaseSensitiveCheck_clicked(bool checked);
	// Search clear button pressed
	void on_ClearButton_clicked(bool checked);
	// Next button clicked
	void on_NextButton_clicked(bool checked);
	// Previous button clicked
	void on_PreviousButton_clicked(bool checked);
};

#endif

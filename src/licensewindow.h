/*
	*** License Window
	*** src/licensewindow.h
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

#ifndef JOURNALVIEWER_LICENSEWINDOW_H
#define JOURNALVIEWER_LICENSEWINDOW_H

#include "ui_licensewindow.h"

// Forward Declarations
/* none */

class LicenseWindow : public QDialog
{
	// All Qt declarations must include this macro
	Q_OBJECT


	/*
	// Window Functions
	*/
	public:
	// Constructor / Destructor
	LicenseWindow(QWidget* parent);
	~LicenseWindow();
	// Main form declaration
	Ui::LicenseWindow ui;


	/*
	// Widget Slots
	*/
	private slots:
	void on_RejectButton_clicked(bool checked);
	void on_AcceptButton_clicked(bool checked);
};

#endif

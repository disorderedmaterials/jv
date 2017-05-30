/*
	*** License Window Functinos
	*** src/licensewindow_funcs.cpp
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

#include "licensewindow.h"
#include <QTextStream>
#include <QSettings>

// Constructor
LicenseWindow::LicenseWindow(QWidget* parent) : QDialog(parent)
{
	// Call the main creation function
	ui.setupUi(this);
}

// Destructor
LicenseWindow::~LicenseWindow()
{
}

/*
// Widget Slots
*/

void LicenseWindow::on_RejectButton_clicked(bool checked)
{
	QSettings settings;
	settings.setValue("LicenseAccepted", false);
	reject();
}

void LicenseWindow::on_AcceptButton_clicked(bool checked)
{
	QSettings settings;
	settings.setValue("LicenseAccepted", true);
	accept();
}

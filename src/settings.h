/*
	*** Settings
	*** src/settings.h
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

#ifndef JOURNALVIEWER_SETTINGS_H
#define JOURNALVIEWER_SETTINGS_H

#include "ui_settings.h"
#include "rundata.h"
#include "list.h"
#include <QDialog>

// Forward Declarations
class JournalViewer;
class Document;

class Settings : public QDialog
{
	// All Qt declarations must include this macro
	Q_OBJECT


	/*
	 * Window Functions
	 */
	public:
	// Constructor / Destructor
	Settings(JournalViewer* parent);
	~Settings();
	// Main form declaration
	Ui::Settings ui;

	private:
	// MainWindow pointer
	JournalViewer* parent_;


	/*
	 * Widget Slots
	 */
	private slots:
	// -- Access Tab
	// Select local journal directory
	void on_JournalDirectorySelectButton_clicked(bool checked);
	// Select data directory
	void on_DataDirectorySelectButton_clicked(bool checked);
	// Select user data directory
	void on_UserDataDirectorySelectButton_clicked(bool checked);
	
	// -- Style Tab
	// Simple List Style selected
	void on_StyleSimpleListRadio_clicked(bool checked);
	// Indented list style selected
	void on_StyleIndentedListRadio_clicked(bool checked);
	// Header Text colour button clicked
	void on_StyleHeaderTextColourButton_clicked(bool checked);
	// Header Cell colour button clicked
	void on_StyleHeaderBGColourButton_clicked(bool checked);
	// Highlight Text colour button clicked
	void on_StyleHighlightTextColourButton_clicked(bool checked);
	// Highlight Cell colour button clicked
	void on_StyleHighlightBGColourButton_clicked(bool checked);
	// Select Document Font button pressed
	void on_SelectDocumentFontButton_clicked(bool checked);
	// Select Plot Font button pressed
	void on_SelectPlotFontButton_clicked(bool checked);
	// Select Report Plot Font button pressed
	void on_SelectDocumentPlotFontButton_clicked(bool checked);
	// Close Window
	void on_CloseButton_clicked(bool checked);


	/*
	 * Settings
	 */
	private:
	// Get settings from JournalViewer
	void getSettings();
	// Save settings back into JournalViewer
	void setSettings();
};

#endif

/*
	*** Find Window Functinos
	*** src/findwindow_funcs.cpp
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

#include "findwindow.h"
#include "jv.h"

// Constructor
FindWindow::FindWindow(JournalViewer& parent) : QDialog(&parent), jvParent_(parent)
{
	// Call the main creation function
	ui.setupUi(this);
}

// Destructor
FindWindow::~FindWindow()
{
}

// Refresh window
bool FindWindow::refresh()
{
	refreshing_ = true;

	// Update number of matches
	ui.CountLabel->setText("Matched " + QString::number(jvParent_.nFindMatches()));

	refreshing_ = false;
}

// Search text changed
void FindWindow::on_TextEdit_textChanged(QString text)
{
	int n = jvParent_.find(text, (JournalViewer::SearchStyle) ui.SearchStyleCombo->currentIndex(), ui.CaseSensitiveCheck->isChecked());

	jvParent_.findNext();

	refresh();
}

// Text edit return pressed
void FindWindow::on_TextEdit_returnPressed()
{
	accept();
}

// Search style changed
void FindWindow::on_SearchStyleCombo_currentIndexChanged(int index)
{
	int n = jvParent_.find(ui.TextEdit->text(), (JournalViewer::SearchStyle) ui.SearchStyleCombo->currentIndex(), ui.CaseSensitiveCheck->isChecked());

	jvParent_.findNext();

	refresh();
}

// Case sensitivity changed
void FindWindow::on_CaseSensitiveCheck_clicked(bool checked)
{
	int n = jvParent_.find(ui.TextEdit->text(), (JournalViewer::SearchStyle) ui.SearchStyleCombo->currentIndex(), ui.CaseSensitiveCheck->isChecked());

	jvParent_.findNext();

	refresh();
}

// Search clear button pressed
void FindWindow::on_ClearButton_clicked(bool checked)
{
	ui.TextEdit->setText("");

	refresh();
}

// Next button clicked
void FindWindow::on_NextButton_clicked(bool checked)
{
	jvParent_.findNext();
}

// Previous button clicked
void FindWindow::on_PreviousButton_clicked(bool checked)
{
	jvParent_.findPrevious();
}

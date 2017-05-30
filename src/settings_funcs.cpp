/*
	*** Settings Functinos
	*** src/settings_funcs.cpp
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

#include "settings.h"
#include "jv.h"
#include <QFileDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QStringList>

// Constructor
Settings::Settings(JournalViewer* parent) : QDialog()
{
	// Call the main creation function
	ui.setupUi(this);

	parent_ = parent;
	
	// Add Instrument combo items
	for (int n=0; n<ISIS::nInstruments; ++n) ui.AccessDefaultInstrumentCombo->addItem(ISIS::capitalisedName((ISIS::ISISInstrument) n));
	ui.AccessDefaultInstrumentCombo->addItem("<None>");

	ui.StyleHeaderTextLabel->setAutoFillBackground(true);
	ui.StyleHighlightTextLabel->setAutoFillBackground(true);

	getSettings();
}

// Destructor
Settings::~Settings()
{
}

/*
 * Widget Slots
 */

// Close Button Pressed
void Settings::on_CloseButton_clicked(bool checked)
{
	setSettings();
	accept();
}

/*
 * Access Tab
 */

// Select local journal directory
void Settings::on_JournalDirectorySelectButton_clicked(bool checked)
{
	QDir newDir(ui.JournalDirectoryEdit->text());
	QString fileName = QFileDialog::getExistingDirectory(this, "Select local journal directory", newDir.path());
	if (!fileName.isEmpty()) ui.JournalDirectoryEdit->setText(fileName);
}

// Select data directory
void Settings::on_DataDirectorySelectButton_clicked(bool checked)
{
	QDir newDir(ui.DataDirectoryEdit->text());
	QString fileName = QFileDialog::getExistingDirectory(this, "Select data (raw/logfile) directory", newDir.path());
	if (!fileName.isEmpty()) ui.DataDirectoryEdit->setText(fileName);
}

// Select user data directory
void Settings::on_UserDataDirectorySelectButton_clicked(bool checked)
{
	QDir newDir(ui.UserDataDirectoryEdit->text());
	QString fileName = QFileDialog::getExistingDirectory(this, "Select user data (raw/logfile) directory", newDir.path());
	if (!fileName.isEmpty()) ui.UserDataDirectoryEdit->setText(fileName);
}

/*
 * Style Tab
 */

// Simple List Style selected
void Settings::on_StyleSimpleListRadio_clicked(bool checked)
{
}

// Indented list style selected
void Settings::on_StyleIndentedListRadio_clicked(bool checked)
{
}

// Header Text colour button clicked
void Settings::on_StyleHeaderTextColourButton_clicked(bool checked)
{
	// Raise a colour dialog
	QColor newColor = QColorDialog::getColor(ui.StyleHeaderTextLabel->palette().text().color(), this, "Select Header Text Colour");
	QPalette newPalette(ui.StyleHeaderTextLabel->palette());
	newPalette.setColor(QPalette::Text, newColor);
	newPalette.setColor(QPalette::WindowText, newColor);
	ui.StyleHeaderTextLabel->setPalette(newPalette);
	ui.StyleHeaderTextLabel->update();
}

// Header Cell colour button clicked
void Settings::on_StyleHeaderBGColourButton_clicked(bool checked)
{
	// Raise a colour dialog
	QColor newColor = QColorDialog::getColor(ui.StyleHeaderTextLabel->palette().window().color(), this, "Select Header Cell Colour");
	QPalette newPalette(ui.StyleHeaderTextLabel->palette());
	newPalette.setColor(QPalette::Window, newColor);
	newPalette.setColor(QPalette::Button, newColor);
	ui.StyleHeaderTextLabel->setPalette(newPalette);
	ui.StyleHeaderTextLabel->update();
}

// Highlight Text colour button clicked
void Settings::on_StyleHighlightTextColourButton_clicked(bool checked)
{
	// Raise a colour dialog
	QColor newColor = QColorDialog::getColor(ui.StyleHighlightTextLabel->palette().text().color(), this, "Select Highlight Text Colour");
	QPalette newPalette(ui.StyleHighlightTextLabel->palette());
	newPalette.setColor(QPalette::Text, newColor);
	newPalette.setColor(QPalette::WindowText, newColor);
	ui.StyleHighlightTextLabel->setPalette(newPalette);
	ui.StyleHighlightTextLabel->update();
}

// Highlight Cell colour button clicked
void Settings::on_StyleHighlightBGColourButton_clicked(bool checked)
{
	// Raise a colour dialog
	QColor newColor = QColorDialog::getColor(ui.StyleHighlightTextLabel->palette().window().color(), this, "Select Highlight Cell Colour");
	QPalette newPalette(ui.StyleHighlightTextLabel->palette());
	newPalette.setColor(QPalette::Window, newColor);
	newPalette.setColor(QPalette::Button, newColor);
	ui.StyleHighlightTextLabel->setPalette(newPalette);
	ui.StyleHighlightTextLabel->update();
}

// Select Table Font button pressed
void Settings::on_SelectDocumentFontButton_clicked(bool checked)
{
	// Raise a font dialog
	bool ok;
	QFont newFont = QFontDialog::getFont(&ok, ui.DocumentFontLabel->font(), this);
	ui.DocumentFontLabel->setFont(newFont);
}

// Select Plot Font button pressed
void Settings::on_SelectPlotFontButton_clicked(bool checked)
{
	// Raise a font dialog
	bool ok;
	QFont newFont = QFontDialog::getFont(&ok, ui.PlotFontLabel->font(), this);
	ui.PlotFontLabel->setFont(newFont);
}

// Select Report Plot Font button pressed
void Settings::on_SelectDocumentPlotFontButton_clicked(bool checked)
{
	// Raise a font dialog
	bool ok;
	QFont newFont = QFontDialog::getFont(&ok, ui.ReportPlotFontLabel->font(), this);
	ui.ReportPlotFontLabel->setFont(newFont);
}

/*
 * Settings
 */

// Get settings from JournalViewer
void Settings::getSettings()
{
	// -- Access
	if (parent_->journalAccessType() == 0) ui.AccessDiskOnlyRadio->setChecked(true);
	else if (parent_->journalAccessType() == 1) ui.AccessNetOnlyRadio->setChecked(true);
	else if (parent_->journalAccessType() == 2) ui.AccessDiskAndNetRadio->setChecked(true);
	ui.JournalDirectoryEdit->setText(parent_->journalDirectory().path());
	ui.JournalURLEdit->setText(parent_->journalUrl().toString());
	ui.DataDirectoryEdit->setText(parent_->dataDirectory().path());
	ui.ForceISOEncoding->setChecked(parent_->forceISOEncoding());
	ui.UserDataDirectoryEdit->setText(parent_->userDataDirectory().path());
	ui.AccessDefaultInstrumentCombo->setCurrentIndex(parent_->defaultInstrument());
	ui.AutoReloadCheck->setChecked(parent_->autoReload());
	ui.AutoReloadFrequencySpin->setValue(parent_->autoReloadFrequency());
	ui.AutoReloadFrequencySpin->setEnabled(ui.AutoReloadCheck->isChecked());
	ui.AutoReloadFrequencyLabel->setEnabled(ui.AutoReloadCheck->isChecked());
	if (parent_->blockDataSource() == RunData::LogBeforeNexusSource) ui.BlockLogBeforeNexusRadio->setChecked(true);
	else if (parent_->blockDataSource() == RunData::NexusBeforeLogSource) ui.BlockNexusBeforeLogRadio->setChecked(true);
	else if (parent_->blockDataSource() == RunData::LogOnlySource) ui.BlockLogOnlyRadio->setChecked(true);
	else if (parent_->blockDataSource() == RunData::NexusOnlySource) ui.BlockNexusOnlyRadio->setChecked(true);

	// -- Export
	QPalette headerPalette(QApplication::palette());
	headerPalette.setColor(QPalette::Text, parent_->headerTextColour());
	headerPalette.setColor(QPalette::WindowText, parent_->headerTextColour());
	headerPalette.setColor(QPalette::Window, parent_->headerBGColour());
	headerPalette.setColor(QPalette::Button, parent_->headerBGColour());
	ui.StyleHeaderTextLabel->setPalette(headerPalette);
	ui.StyleHeaderTextLabel->update();
	QPalette highlightPalette(QApplication::palette());
	highlightPalette.setColor(QPalette::Text, parent_->highlightTextColour());
	highlightPalette.setColor(QPalette::WindowText, parent_->highlightTextColour());
	highlightPalette.setColor(QPalette::Window, parent_->highlightBGColour());
	highlightPalette.setColor(QPalette::Button, parent_->highlightBGColour());
	ui.StyleHighlightTextLabel->setPalette(highlightPalette);
	ui.StyleHighlightTextLabel->update();
	ui.DocumentFontLabel->setFont( parent_->documentFont() );
	ui.PlotFontLabel->setFont( parent_->plotFont() );
	ui.ReportPlotFontLabel->setFont( parent_->reportPlotFont() );
	if (parent_->documentLayout() == JournalViewer::ListLayout) ui.StyleSimpleListRadio->setChecked(true);
	else if (parent_->documentLayout() == JournalViewer::IndentedLayout) ui.StyleIndentedListRadio->setChecked(true);

	// -- Margins
	ui.GeneralMarginLeftSpin->setValue(parent_->documentMargins()[0]);
	ui.GeneralMarginTopSpin->setValue(parent_->documentMargins()[1]);
	ui.GeneralMarginRightSpin->setValue(parent_->documentMargins()[2]);
	ui.GeneralMarginBottomSpin->setValue(parent_->documentMargins()[3]);
	ui.ReportMarginLeftSpin->setValue(parent_->reportMargins()[0]);
	ui.ReportMarginTopSpin->setValue(parent_->reportMargins()[1]);
	ui.ReportMarginRightSpin->setValue(parent_->reportMargins()[2]);
	ui.ReportMarginBottomSpin->setValue(parent_->reportMargins()[3]);
}

// Save settings back into JournalViewer
void Settings::setSettings()
{
	// -- Access
	if (ui.AccessDiskOnlyRadio->isChecked()) parent_->setJournalAccessType(JournalViewer::DiskOnlyAccess);
	else if (ui.AccessNetOnlyRadio->isChecked()) parent_->setJournalAccessType(JournalViewer::NetOnlyAccess);
	else if (ui.AccessDiskAndNetRadio->isChecked()) parent_->setJournalAccessType(JournalViewer::DiskAndNetAccess);
	parent_->setJournalDirectory(ui.JournalDirectoryEdit->text());
	parent_->setJournalUrl(ui.JournalURLEdit->text());
	parent_->setDataDirectory(ui.DataDirectoryEdit->text());
	parent_->setForceISOEncoding(ui.ForceISOEncoding->isChecked());
	parent_->setUserDataDirectory(ui.UserDataDirectoryEdit->text());
	parent_->setDefaultInstrument((ISIS::ISISInstrument) ui.AccessDefaultInstrumentCombo->currentIndex());
	parent_->setAutoReload(ui.AutoReloadCheck->isChecked());
	parent_->setAutoReloadFrequency(ui.AutoReloadFrequencySpin->value());
	if (ui.BlockLogBeforeNexusRadio->isChecked()) parent_->setBlockDataSource(RunData::LogBeforeNexusSource) ;
	else if (ui.BlockNexusBeforeLogRadio->isChecked()) parent_->setBlockDataSource(RunData::NexusBeforeLogSource);
	else if (ui.BlockLogOnlyRadio->isChecked()) parent_->setBlockDataSource(RunData::LogOnlySource);
	else if (ui.BlockNexusOnlyRadio->isChecked()) parent_->setBlockDataSource(RunData::NexusOnlySource);

	// -- Style
	parent_->setHeaderTextColour(ui.StyleHeaderTextLabel->palette().color(QPalette::Text));
	parent_->setHeaderBGColour(ui.StyleHeaderTextLabel->palette().color(QPalette::Window));
	parent_->setHighlightTextColour(ui.StyleHighlightTextLabel->palette().color(QPalette::Text));
	parent_->setHighlightBGColour(ui.StyleHighlightTextLabel->palette().color(QPalette::Window));
	parent_->setDocumentFont(ui.DocumentFontLabel->font());
	parent_->setPlotFont(ui.PlotFontLabel->font());
	parent_->setReportPlotFont(ui.ReportPlotFontLabel->font());
	if (ui.StyleSimpleListRadio->isChecked()) parent_->setDocumentLayout(JournalViewer::ListLayout);
	else if (ui.StyleIndentedListRadio->isChecked()) parent_->setDocumentLayout(JournalViewer::IndentedLayout);

	// -- Margins
	parent_->setDocumentMargins(ui.GeneralMarginLeftSpin->value(), ui.GeneralMarginTopSpin->value(), ui.GeneralMarginRightSpin->value(), ui.GeneralMarginBottomSpin->value());
	parent_->setReportMargins(ui.ReportMarginLeftSpin->value(), ui.ReportMarginTopSpin->value(), ui.ReportMarginRightSpin->value(), ui.ReportMarginBottomSpin->value());
}

/*
	*** JournalViewer Window Functions
	*** src/jv_funcs.cpp
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

#include "jv.h"
#include "report.h"
#include "settings.h"
#include "version.h"
#include "datainterface.h"
#include "document.h"
#include "ttablewidgetitem.h"
#include "isis.h"
#include "reflist.h"
#include "messenger.hui"
#include "samplereport.h"
#include "quickreport.h"
#include "licensewindow.h"
#include "findwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>
#include <QCloseEvent>
#include <QtPrintSupport/QPrintDialog>
#include <QClipboard>
#include <QSettings>

// Constructor
JournalViewer::JournalViewer(QMainWindow *parent) : QMainWindow(parent)
{
	// Initialise the icon resource
	Q_INIT_RESOURCE(icons);
	
	// Call the main creation function
	ui.setupUi(this);
	
	// Set pointer in ISISParser
	ISIS::setParent(this);

	// Tweak any widgets we need to
	QString title;
#ifdef LITE
	title.sprintf("JournalViewer Lite (%s)", JVVERSION);
#else
	title.sprintf("JournalViewer (%s)", JVVERSION);
#endif
	setWindowTitle(title);

	// Connect Data Table's header 
	connect((QObject*)ui.DataTable->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(dataTable_headerClicked(int)));
	ui.DataTable->horizontalHeader()->setSectionsMovable(false);
	
	// Connect contextMenuEvent in DataTable
	connect(ui.DataTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(dataTable_contextMenuEvent(QPoint)));

	// Create Dialogs and set pointer in Messenger
	logWindow_ = new LogWindow(this);
	msg.setTextBrowser(logWindow_->ui.LogBrowser);
	findWindow_ = new FindWindow(*this);

	// Set default settings, then attempt to load in stored settings
	setDefaultSettings();
	retrieveSettings();

	// Create View menu items
	for (int n=0; n<RunProperty::nProperties; ++n)
	{
		RunProperty::Property prop = (RunProperty::Property) n;
		if (RunProperty::propertyIsHidden(prop)) continue;
		viewPropertyAction[n] = ui.menuView->addAction(RunProperty::property(prop));
		viewPropertyAction[n]->setCheckable(true);
		viewPropertyAction[n]->setChecked(false);
		
		// Is it in the starting visible columns list?
		for (RunProperty* rp = visibleProperties_.first(); rp != NULL; rp = rp->next) if (rp->type() == prop) viewPropertyAction[n]->setChecked(true);
		QObject::connect(viewPropertyAction[n], SIGNAL(triggered(bool)), this, SLOT(viewMenuActionTriggered(bool)));
	}
	
	// Add permanent widgets to statusbar
	statusHttpProgressLabel_ = new QLabel;
	statusHttpProgressLabel_->setVisible(false);
	ui.statusbar->addPermanentWidget(statusHttpProgressLabel_);
	statusHttpProgress_ = new QProgressBar;
	statusHttpProgress_->setMaximumSize(100, 100);
	statusHttpProgress_->setVisible(false);
	ui.statusbar->addPermanentWidget(statusHttpProgress_);
	statusSourceLabel_ = new QLabel;
	ui.statusbar->addPermanentWidget(statusSourceLabel_);
	statusSearchLabel_ = new QLabel;
	ui.statusbar->addPermanentWidget(statusSearchLabel_);
	statusGroupLabel_ = new QLabel;
	statusGroupLabel_->setText("Grouping");
	ui.statusbar->addPermanentWidget(statusGroupLabel_);

	// Setup DataInterface for journal data acquisition
	dataInterface_ = new DataInterface(statusHttpProgress_, statusHttpProgressLabel_);
	connect(ui.actionCancelDownload, SIGNAL(triggered(bool)), dataInterface_, SLOT(cancel()));

	// Set initial variable values
	currentJournal_ = NULL;
	currentInstrument_ = NULL;
	nRunDataVisible_ = 0;
	viewByGroup_ = false;
	refreshing_ = false;

	// Update status bar
	updateStatusBarPermanentWidgets();

	// Setup QTimer for autorefresh
	connect(&autoReloadTimer_, SIGNAL(timeout()), this, SLOT(updateJournalData()));
	autoReloadTimer_.setSingleShot(false);
	setAutoReloadFrequency(autoReloadFrequency_);

	// Setup QTimer for hiding statusbar progress indicator
	connect(&hideProgressTimer_, SIGNAL(timeout()), this, SLOT(hideProgressBar()));
	hideProgressTimer_.setSingleShot(true);
	hideProgressTimer_.setInterval(3000);

	/* JV Lite */
#ifdef LITE
	// Change 'Cycle' label to 'Experiment'
	ui.DisplayCycleLabel->setText("Experiment");
#endif
}

// Destructor
JournalViewer::~JournalViewer()
{
}

// Clear all loaded data
void JournalViewer::clear()
{
	refreshing_ = true;
	ui.InstrumentCombo->clear();
	ui.JournalCombo->clear();
	setJournal(NULL);
	setInstrument(NULL);
	instruments_.clear();
	runData_.clear();
	refreshing_ = false;

	updateDataTable();
}

// Initialise JournalViewer
bool JournalViewer::initialise(bool startTimers)
{
	// Clear existing Instrument / Journal data
	clear();

	// Check local journal storage / directory
#ifndef LITE
	if (journalAccessType_ != JournalViewer::NetOnlyAccess)
	{
		msg.print("Looking for local directory " + journalDirectory_.path());
		if (!journalDirectory_.exists())
		{
			QMessageBox::StandardButton button = QMessageBox::question(this, "Directory Not Found", QString("The specified local journal directory '") + journalDirectory_.path() + "' does not exist.\nCreate it now?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
			if (button == QMessageBox::Yes)
			{
				if (!journalDirectory_.mkpath(journalDirectory_.path()))
				{
					QMessageBox::critical(this, "Error", QString("Failed to create the directory '") + journalDirectory_.path() + "' .\nCheck the path and your permissions to write to it.");
					msg.print("Failed to create local journal directory " + journalDirectory_.path());
					ui.statusbar->showMessage("Failed to create local journal directory.", 3000);
					return false;
				}
			}
			else if (button == QMessageBox::Cancel) return false;
		}
	}
#endif

	refreshing_ = true;

	// Add instruments...
	for (int n=0; n<ISIS::nInstruments; ++n)
	{
		ISIS::ISISInstrument inst = (ISIS::ISISInstrument) n;
		if (!addInstrument(inst))
		{
			QMessageBox::StandardButton button = QMessageBox::critical(this, "Error", QString("Failed to load instrument '") + ISIS::ndxName(inst) + "'.\nCheck the local and http path specifications.", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
			msg.print("Failed to load instrument " + ISIS::capitalisedName(inst));
			ui.statusbar->showMessage(QString("Failed to load instrument ") + ISIS::capitalisedName(inst), 3000);
			if (button == QMessageBox::Cancel) return false;
		}
	}

	// Populate InstrumentCombo, and determine instrument to display
	Instrument* defInst = NULL;
#ifdef LITE
	ui.InstrumentCombo->addItem("Local Experiments");
	defInst = instruments_.first();
#else
	for (Instrument* inst = instruments_.first(); inst != NULL; inst = inst->next)
	{
		ui.InstrumentCombo->addItem(inst->capitalisedName());
		if (inst->instrument() == defaultInstrument_)
		{
			ui.InstrumentCombo->setCurrentIndex(ui.InstrumentCombo->count()-1);
			defInst = inst;
		}
	}
#endif
	if (defInst == NULL) defInst = instruments_.first();

	refreshing_ = false;

	// Set default instrument (starting journal combo will be set automatically)
	setInstrument(defInst);

	// Start timers?
	if (startTimers)
	{
		if (autoReload_) autoReloadTimer_.start();
	}

	// Good to go - show some data
	filterRunData();
	updateDataTable();

	return true;
}

// Show license if necessary, returning whether it was accepted
bool JournalViewer::showLicense(bool force)
{
	/* Show License if not yet accepted */
	QSettings settings;
	if (settings.value("LicenseAccepted").toBool() && (!force)) return true;

	// Not (yet) accepted, so show it
	LicenseWindow licenseWindow(NULL);
	if (licenseWindow.exec() == QDialog::Rejected) return false;
	return true;
}

// Window close event
void JournalViewer::closeEvent(QCloseEvent *event)
{
	storeSettings();
	event->accept();
}

/*
// Widget Slots
*/

/*
 * Instrument / Journal Selection
 */

// Current instrument changed
void JournalViewer::on_InstrumentCombo_currentIndexChanged(int index)
{
	if (refreshing_) return;

	Instrument* inst = index == -1 ? NULL : instruments_[index];
	if (inst == NULL)
	{
		printf("Error - InstrumentCombo managed to have an index that doesn't correspond to an Instrument list entry.\n");
		return;
	}

	setInstrument(inst);
	filterRunData();
	updateDataTable();

	msg.print(("Instrument changed to ") + inst->capitalisedName());
}

// Current journal changed
void JournalViewer::on_JournalCombo_currentIndexChanged(int index)
{
	if (refreshing_) return;

	// Check for valid instrument pointer
	if (currentInstrument_ == NULL)
	{
		printf("Current instrument pointer is NULL.\n");
		return;
	}

	// Get new Journal pointer
	Journal* jrnl = index == -1 ? NULL : currentInstrument_->journal(index);
	if (jrnl == NULL)
	{
		printf("Error - JournalCombo index does not correspond to a valid journal for this Instrument.\n");
		return;
	}

	// Set new Journal (table will be updated)
	setJournal(jrnl);
}

// Reload journal button clicked
void JournalViewer::on_ReloadJournalButton_clicked(bool checked)
{
	updateJournalData();
}

/* 
 * Search Panel
 */

// Search text modified
void JournalViewer::on_SearchEdit_returnPressed()
{
	filterRunData();
	updateDataTable();
}

// Search style changed
void JournalViewer::on_SearchStyleCombo_currentIndexChanged(int index)
{
	filterRunData();
	updateDataTable();
}

// Search box clear button pressed
void JournalViewer::on_SearchClearButton_clicked(bool checked)
{
	ui.SearchEdit->clear();
	filterRunData();
	updateDataTable();
}

// Case Sensitivity checkbox clicked
void JournalViewer::on_SearchCaseSensitiveCheck_clicked(bool checked)
{
	filterRunData();
	updateDataTable();
}

/*
 * Filter Group
 */

// Filter user combo changed
void JournalViewer::on_FilterUserCombo_currentIndexChanged(int index)
{
	if (refreshing_) return;
	filterRunData();
	updateDataTable();
}

// User search box clear button pressed
void JournalViewer::on_UserSearchClearButton_clicked(bool checked)
{
	ui.FilterUserCombo->setCurrentIndex(0);
	filterRunData();
	updateDataTable();
}

// Filter RB combo changed
void JournalViewer::on_FilterRBCombo_currentIndexChanged(int index)
{
	if (refreshing_) return;
	filterRunData();
	updateDataTable();
}

// RB search box clear button pressed
void JournalViewer::on_RBSearchClearButton_clicked(bool checked)
{
	ui.FilterRBCombo->setCurrentIndex(0);
	filterRunData();
	updateDataTable();
}

// Filter date type changed
void JournalViewer::on_FilterDateTypeCombo_currentIndexChanged(int index)
{
	if (refreshing_) return;
	filterRunData();
	updateDataTable();
}

// From date filter changed
void JournalViewer::on_FilterFromDateTimeEdit_dateTimeChanged(const QDateTime& datetime)
{
	if (refreshing_) return;
	filterRunData();
	updateDataTable();
}

// To date filter changed
void JournalViewer::on_FilterToDateTimeEdit_dateTimeChanged(const QDateTime& datetime)
{
	if (refreshing_) return;
	filterRunData();
	updateDataTable();
}

// From run filter changed
void JournalViewer::on_FilterFromRunSpin_valueChanged(int value)
{
	if (refreshing_) return;
	filterRunData();
	updateDataTable();
}

// To run filter changed
void JournalViewer::on_FilterToRunSpin_valueChanged(int value)
{
	if (refreshing_) return;
	filterRunData();
	updateDataTable();
}

// From date filter lock status changed
void JournalViewer::on_FilterFromDateTimeLockButton_clicked(bool checked)
{
}

// To date filter lock status changed
void JournalViewer::on_FilterToDateTimeLockButton_clicked(bool checked)
{
}

// From run filter lock status changed
void JournalViewer::on_FilterFromRunLockButton_clicked(bool checked)
{
}

// To run filter lock status changed
void JournalViewer::on_FilterToRunLockButton_clicked(bool checked)
{
}

/*
 * Data Table
 */

// Header of data table clicked
void JournalViewer::dataTable_headerClicked(int section)
{
	// Turn off grouping, since this is now not relevant
	if (viewByGroup_)
	{
		viewByGroup_ = false;
		ui.DataTable->sortByColumn(section);
	}

	updateDataTableHighlighting();

	updateStatusBarPermanentWidgets();
}

// Table cell single-clicked
void JournalViewer::on_DataTable_itemClicked(QTableWidgetItem* item)
{
}

// Table cell double-clicked
void JournalViewer::on_DataTable_itemDoubleClicked(QTableWidgetItem* item)
{
	plotSelectedRunData(RunData::LogBeforeNexusSource);
}

// Context menu event
void JournalViewer::dataTable_contextMenuEvent(const QPoint &pos)
{
	// Get QTableWidteItem under clicked point
	TTableWidgetItem* sourceItem = (TTableWidgetItem*) ui.DataTable->itemAt(pos);
	if (!sourceItem) return;

	// Create and execute context menu
	QMenu menu(this);
	QAction *selectSimilarAction = menu.addAction(tr("Select Similar"));
	QAction *sampleReportAction = menu.addAction(tr("Sample Report"));
	QAction *copyForGudrun = menu.addAction(tr("Copy as Gudrun File List"));
	QAction *selectedAction = menu.exec(ui.DataTable->viewport()->mapToGlobal(pos));

	// Act on action!
	if (selectedAction == selectSimilarAction) 
	{
		// Loop over DataTable items and set new selection
		ui.DataTable->clearSelection();
		ui.DataTable->setSelectionMode(QAbstractItemView::MultiSelection);
		TTableWidgetItem* item;
		for (int row = 0; row < ui.DataTable->rowCount(); ++row)
		{
			item = (TTableWidgetItem*) ui.DataTable->item(row, 0);
			if (!item) continue;
			if (item->source()->title() == sourceItem->source()->title()) ui.DataTable->selectRow(row);
		}
		ui.DataTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
	}
	else if (selectedAction == sampleReportAction) 
	{
		// Get current selection and open new SampleReport window containing this data
		RefList<RunData,int> selectedData = getTableContents(true);
		if (selectedData.nItems() == 0) return;

		SampleReport* sampleReportWin = new SampleReport(this);
		Instrument* inst = selectedData.first()->item->instrument();
		sampleReportWin->setWindowTitle(inst->capitalisedName() + " Sample Report");
		sampleReportWin->addRunData(selectedData, inst->location() == ISIS::Muon);
		sampleReportWin->finaliseAndShow();
	}
	else if (selectedAction == copyForGudrun)
	{
		// Construct list of RunData, in display order, to save
		RefList<RunData,int> selectedData = getTableContents(true);

		// Create text for selected items and copy to clipboard
		QString text;
		text.sprintf("%i  %i          Number of files and period number\n", selectedData.nItems(), 1);
		for (RefListItem<RunData,int>* ri = selectedData.first(); ri != NULL; ri = ri->next)
		{
			text += ri->item->name();
			text += ".raw          SAMPLE Copied from JournalViewer\n";
		}
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(text);
	}
}
    
/*
 * File Menu
 */

// File->SaveAsText selected
void JournalViewer::on_actionFileSaveAsText_triggered(bool checked)
{
	// Count current visible items
	int nSelected = ui.DataTable->selectedItems().count();

	if (nRunDataVisible_ == 0)
	{
		QMessageBox::warning(this, "Warning", "No data currently visible - nothing to save!");
		return;
	}
	
	// Save selection or all items?
	bool saveSelectionOnly = false;
	if (nSelected > 0)
	{
		QMessageBox::StandardButton button = QMessageBox::question(this, "Save", "There are selected items - would you like to export just the selected items?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
		if (button == QMessageBox::Yes) saveSelectionOnly = true;
	}

	// Get a file name to save under
	static QDir currentDirectory;
	QString fileName = QFileDialog::getSaveFileName(this, "Save data as text file", currentDirectory.path());
	if (!fileName.isEmpty())
	{
		// Save dir for later
		currentDirectory.setPath(fileName);

		// Construct list of RunData, in display order, to save
		RefList<RunData,int> data = getTableContents(saveSelectionOnly);

		QFile file(fileName);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

		QTextStream out(&file);
		QString text;
		createText(text, data);
		out << text;
		
		file.close();
	}
}

// File->SaveAsPDF selected
void JournalViewer::on_actionFileSaveAsPDF_triggered(bool checked)
{
	// Count current visible items
	int nSelected = ui.DataTable->selectedItems().count();

	if (nRunDataVisible_ == 0)
	{
		QMessageBox::warning(this, "Warning", "No data currently visible - nothing to save!");
		return;
	}
	
	// Save selection or all items?
	bool saveSelectionOnly = false;
	if (nSelected > 0)
	{
		QMessageBox::StandardButton button = QMessageBox::question(this, "Save", "There are selected items - would you like to export just the selected items?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
		if (button == QMessageBox::Yes) saveSelectionOnly = true;
	}

	// Get a file name to save under
	static QDir currentDirectory;
	QString fileName = QFileDialog::getSaveFileName(this, "Save data as PDF file", currentDirectory.path());
	if (!fileName.isEmpty())
	{
		// Save dir for later
		currentDirectory.setPath(fileName);

		// Construct list of RunData, in display order, to save
		RefList<RunData,int> data = getTableContents(saveSelectionOnly);

		// Create a QPrinter, directing to file
		QPrinter printer;
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setOutputFileName(fileName);
		printer.setPageMargins(documentMargins_[0], documentMargins_[1], documentMargins_[2], documentMargins_[3], QPrinter::Millimeter);

		// Create the QPainter
		QPainter painter;
		painter.begin(&printer);
		painter.setFont(documentFont_);
		Document doc(painter, &printer);
		createDocument(doc, data);
		painter.end();
	}
}

// File->Print selected
void JournalViewer::on_actionFilePrint_triggered(bool checked)
{
	// Set page margins 
	printer_.setPageMargins(documentMargins_[0], documentMargins_[1], documentMargins_[2], documentMargins_[3], QPrinter::Millimeter);

	QPrintDialog printDialog(&printer_, this);
	if (printDialog.exec() == QDialog::Accepted)
	{
		// Construct list of RunData, in display order, to save
		RefList<RunData,int> data = getTableContents(false);

		// Create the QPainter
		QPainter painter;
		painter.setFont(documentFont_);
		painter.begin(&printer_);
		Document doc(painter, &printer_);
		createDocument(doc, data);
		painter.end();
	}
}

// File->Print selected
void JournalViewer::on_actionFilePrintSelection_triggered(bool checked)
{
	// Set page margins 
	printer_.setPageMargins(documentMargins_[0], documentMargins_[1], documentMargins_[2], documentMargins_[3], QPrinter::Millimeter);

	QPrintDialog printDialog(&printer_, this);
	if (printDialog.exec() == QDialog::Accepted)
	{
		// Construct list of RunData, in display order, to save
		RefList<RunData,int> data = getTableContents(true);

		// Create the QPainter
		QPainter painter;
		painter.begin(&printer_);
		painter.setFont(documentFont_);
		Document doc(painter, &printer_);
		createDocument(doc, data);
		painter.end();
	}
}

// File->Quit selected
void JournalViewer::on_actionFileQuit_triggered(bool checked)
{
	storeSettings();
	QApplication::exit(0);
}

/*
 * View Menu
 */

// Item selected in View menu
void JournalViewer::viewMenuActionTriggered(bool checked)
{
	// Cast sender and convert menu item to Property enum
	QAction* action = qobject_cast<QAction*> (sender());
	RunProperty::Property property = RunProperty::nProperties;
	for (int n=0; n<RunProperty::nProperties; ++n) if (viewPropertyAction[n] == action) property = (RunProperty::Property) n;
	if (property == RunProperty::nProperties) return;

	// Search list for visible RunProperty
	RunProperty* rp = NULL;
	for (rp = visibleProperties_.first(); rp != NULL; rp = rp->next) if (rp->type() == property) break;

	if (checked)
	{
		// Add the item to the list, if it not already there
		if (rp) msg.print("Property is already visible : " + action->text());
		else
		{
			visibleProperties_.add()->setType(property);
			updateDataTable();
		}
	}
	else
	{
		// Remove the item to the list, unless it isn't there
		if (rp)
		{
			visibleProperties_.remove(rp);
			updateDataTable();
		}
		else msg.print("Property is not visible : " + action->text());
	}
}

/*
 * Selection Menu
 */

// Clear current selection
void JournalViewer::on_actionSelectionClear_triggered(bool checked)
{
	ui.DataTable->clearSelection();
}

// Plot data selected
void JournalViewer::on_actionSelectionPlotData_triggered(bool checked)
{
	plotSelectedRunData(blockDataSource_);
}

// Plot logfile data selected
void JournalViewer::on_actionSelectionPlotLogfileData_triggered(bool checked)
{
	plotSelectedRunData(RunData::LogOnlySource, true);
}

// Plot Nexus file data selected
void JournalViewer::on_actionSelectionPlotNexusData_triggered(bool checked)
{
	plotSelectedRunData(RunData::NexusOnlySource, true);
}

// Copy selection to clipboard
void JournalViewer::on_actionSelectionCopy_triggered(bool checked)
{
	// Construct list of RunData, in display order, to save
	RefList<RunData,int> data = getTableContents(true);

	// Create text for selected items and copy to clipboard
	QString text;
	createText(text, data);
	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setText(text);
}

/*
 * Tools Menu
 */

// Tools->Find... selected
void JournalViewer::on_actionToolsFind_triggered(bool checked)
{
	findWindow_->show();
}

// Tools->FindNext selected
void JournalViewer::on_actionToolsFindNext_triggered(bool checked)
{
	findNext();
}

// Tools->FindPrevious selected
void JournalViewer::on_actionToolsFindPrevious_triggered(bool checked)
{
	findPrevious();
}

// Tools->Reload Data selected
void JournalViewer::on_actionToolsReloadData_triggered(bool checked)
{
	initialise();
}

// Tools->GroupData selected
void JournalViewer::on_actionToolsGroupData_triggered(bool checked)
{
	viewByGroup_ = checked;
	
	if (viewByGroup_)
	{
		// Update table
		updateDataTable();
		
		// Sort table by hidden Group column
		ui.DataTable->sortByColumn(visibleProperties_.nItems(), Qt::AscendingOrder);
	}
	else ui.DataTable->sortByColumn(0, Qt::AscendingOrder);
	updateDataTableHighlighting();
	updateStatusBarPermanentWidgets();
}

// Tools->Reset Filters selected
void JournalViewer::on_actionToolsResetFilters_triggered(bool checked)
{
	// Clear search text
	ui.SearchEdit->clear();

	// Reset filters
	resetFilters();

	// Update
	filterRunData();
	updateDataTable();
}

// File->CreateExperimentReport selected
void JournalViewer::on_actionToolsCreateExperimentReport_triggered(bool checked)
{
	ReportGenerator reporter(this, availableRB_, availableRBUsers_, runData_);
	int result = reporter.exec();
	if (result != QDialog::Accepted) return;

	if (reporter.reportResult() == ReportGenerator::SaveResult)
	{
		// Get a file name to save under
		static QDir currentDirectory;
		QString fileName = QFileDialog::getSaveFileName(this, "Save report PDF", currentDirectory.path());
		if (!fileName.isEmpty())
		{
			// Create a QPrinter, directing to file
			QPrinter printer;
			printer.setOutputFormat(QPrinter::PdfFormat);
			printer.setOutputFileName(fileName);
			printer.setPageMargins(reportMargins_[0], reportMargins_[1], reportMargins_[2], reportMargins_[3], QPrinter::Millimeter);

			// Create the QPainter
			QPainter painter;
			painter.begin(&printer);
			Document doc(painter, &printer);
			reporter.createReport(doc);
			painter.end();
		}
	}
	else if (reporter.reportResult() == ReportGenerator::PrintResult)
	{
		// Set printer margins 
		printer_.setPageMargins(reportMargins_[0], reportMargins_[1], reportMargins_[2], reportMargins_[3], QPrinter::Millimeter);

		QPrintDialog printDialog(&printer_, this);
		if (printDialog.exec() == QDialog::Accepted)
		{
			// Create the QPainter
			QPainter painter;
			painter.setFont(documentFont_);
			painter.begin(&printer_);
			Document doc(painter, &printer_);
			reporter.createReport(doc);
			painter.end();
		}
	}
}

// Tools->CreateQuickReport selected
void JournalViewer::on_actionToolsCreateQuickReport_triggered(bool checked)
{
	QuickReport qr(this, availableRB_, availableRBUsers_, runData_);
	int result = qr.exec();
	if (result != QDialog::Accepted) return;

	if (qr.reportResult() == QuickReport::SaveResult)
	{
		// Get a file name to save under
		static QDir currentDirectory;
		QString fileName = QFileDialog::getSaveFileName(this, "Save quick report PDF", currentDirectory.path());
		if (!fileName.isEmpty())
		{
			// Create a QPrinter, directing to file
			QPrinter printer;
			printer.setOutputFormat(QPrinter::PdfFormat);
			printer.setOutputFileName(fileName);
			printer.setPageMargins(reportMargins_[0], reportMargins_[1], reportMargins_[2], reportMargins_[3], QPrinter::Millimeter);

			// Create the QPainter
			QPainter painter;
			painter.begin(&printer);
			Document doc(painter, &printer);
			qr.createReport(doc);
			painter.end();
		}
	}
	else if (qr.reportResult() == QuickReport::PrintResult)
	{
		// Set printer margins 
		printer_.setPageMargins(reportMargins_[0], reportMargins_[1], reportMargins_[2], reportMargins_[3], QPrinter::Millimeter);

		QPrintDialog printDialog(&printer_, this);
		if (printDialog.exec() == QDialog::Accepted)
		{
			// Create the QPainter
			QPainter painter;
			painter.setFont(documentFont_);
			painter.begin(&printer_);
			Document doc(painter, &printer_);
			qr.createReport(doc);
			painter.end();
		}
	}
}

// Tools->Settings selected
void JournalViewer::on_actionToolsSettings_triggered(bool checked)
{
	// Store a couple of current values - if they change, we'll need to reload data
	JournalAccess accessType = journalAccessType_;
	QDir localDir = journalDirectory_;
	Settings settingsDialog(this);

	/* Lite Version - Disable some controls */
#ifdef LITE
	settingsDialog.ui.JournalSourceGroup->setEnabled(false);
	settingsDialog.ui.DefaultInstrumentGroup->setEnabled(false);
	settingsDialog.ui.DataDirectorySelectButton->setEnabled(false);
	settingsDialog.ui.DataDirectoryEdit->setEnabled(false);
	settingsDialog.ui.DataDirectoryLabel->setEnabled(false);
#endif

	if (settingsDialog.exec() == QDialog::Accepted)
	{
		// Reload data?
		bool reload = false;
		if (accessType != journalAccessType_) reload = true;
		if (localDir != journalDirectory_) reload = true;
		if (reload) initialise();
	}
}

// Tools->Log Window selected
void JournalViewer::on_actionToolsLogWindow_triggered(bool checked)
{
	logWindow_->show();
}

// Tools->Regenerate Local Journals selected
void JournalViewer::on_actionToolsRegenerateLocalJournals_triggered(bool checked)
{
	updateLocalJournals();
	if (currentInstrument_->shortName() == "LOCAL") initialise();
}

// Tools->Show License selected
void JournalViewer::on_actionToolsShowLicense_triggered(bool checked)
{
	if (!showLicense(true)) QApplication::quit();
}

/*
 * Hidden Actions
 */
void JournalViewer::on_actionCancelDownload_triggered(bool checked)
{
}

/*
 * Windows / Widgets
 */

// Set enabled state of instrument / journal combo boxes
void JournalViewer::setJournalControlsEnabled(bool enabled)
{
#ifdef LITE
	ui.InstrumentCombo->setEnabled(false);
#else
	ui.InstrumentCombo->setEnabled(enabled);
#endif
	ui.JournalCombo->setEnabled(enabled);
	ui.ReloadJournalButton->setEnabled(enabled);
}

// Update status bar permanent widgets
void JournalViewer::updateStatusBarPermanentWidgets()
{
	QString s;
	// -- Source
	if (journalAccessType_ == JournalViewer::DiskOnlyAccess) s = "DISK";
	else if (journalAccessType_ == JournalViewer::NetOnlyAccess) s = "NET";
	else if (journalAccessType_ == JournalViewer::DiskAndNetAccess) s = "DISK/NET";
	statusSourceLabel_->setText(s);
	
	// -- Search / Filter
	s = "";
	if (nRunDataVisible_ == runData_.nItems()) s = "All Runs Displayed";
	else QTextStream(&s) << "Search/Filter Active - " << nRunDataVisible_ << " of " << runData_.nItems() << " shown";
	statusSearchLabel_->setText(s);

	// -- Grouping Status
	statusGroupLabel_->setEnabled(viewByGroup_);
	statusGroupLabel_->setText(viewByGroup_ ? "Grouping is On" : "Grouping is Off");
}

// Hide progress bar
void JournalViewer::hideProgressBar()
{
	statusHttpProgress_->setVisible(false);
	statusHttpProgressLabel_->setVisible(false);
}

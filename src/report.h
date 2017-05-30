/*
	*** Report Generator
	*** src/report.h
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

#ifndef JOURNALVIEWER_REPORT_H
#define JOURNALVIEWER_REPORT_H

#include "ui_report.h"
#include "list.h"
#include "instrument.h"
#include "rundata.h"
#include <QDialog>
#include <QTextDocument>

// Forward Declarations
class JournalViewer;
class Document;
class PlotWidget;
class Journal;

class ReportGenerator : public QDialog
{
	// All Qt declarations must include this macro
	Q_OBJECT


	/*
	// Window Functions
	*/
	public:
	// Constructor / Destructor
	ReportGenerator(JournalViewer* parent, const QList<int>& availableRB, QStringList availableRBUsers, RefList<RunData,Journal*>& runData);
	~ReportGenerator();
	// Result Enum
	enum Result { CancelResult, SaveResult, PrintResult };
	// Main form declaration
	Ui::ReportGenerator ui;

	private:
	// MainWindow pointer
	JournalViewer* parent_;
	// Target Instrument
	Instrument* instrument_;
	// Whether window is currently refreshing
	bool refreshing_ ;
	// Selected RB number
	int rbNumber_;
	// List of available RB numbers
	QList<int> availableRB_;
	// Reference to master RunData list
	RefList<RunData,Journal*>& runData_;
	// Array of available properties to show
	bool availableProperties_[RunProperty::nProperties];
	// List of displayed properties, in order
	List<RunProperty> visibleProperties_;
	// Result to return from dialog
	ReportGenerator::Result result_;

	private:
	// Store settings
	void storeSettings();
	// Retrieve settings
	void retrieveSettings();

	public:
	// Return dialog report result
	ReportGenerator::Result reportResult();

	/*
	// Experiment Details
	*/
	private:
	// Number of runs for experiment
	int nRuns_;
	// DateTime of beginning of first run
	QDateTime beginRunDateTime_;
	// DateTime of end of last run
	QDateTime endRunDateTime_;
	// Total duration of runs (in seconds)
	int totalRunTime_;
	// Total current accumulated by all runs
	double totalCurrent_;

	private:
	// Update experiment details
	void updateDetails(int rbNo);
	// Retrieve new Instrument Information
	void retrieveInstrumentInformation();
	// Update Instrument Information tree
	void updateInstrumentInformation();
	// Update available / visible properties
	void updatePropertyLists();

	public:
	// Create experiment report
	bool createReport(Document& report);


	/*
	// Widget Slots
	*/
	private slots:
	// -- Experiment Tab
	// RBNumber changed
	void on_RBCombo_currentIndexChanged(int index);
	// Experiment Part check clicked
	void on_PartCheck_clicked(bool checked);
	// Experiment NParts changed
	void on_NPartsSpin_valueChanged(int value);
	// -- Instrument Information Tab
	// RunInformation radio clicked
	void on_RunInformationRadio_clicked(bool checked);
	// RunInformation combo changed
	void on_RunInformationCombo_currentIndexChanged(int index);
	// CurrentSECI values radio clicked
	void on_CurrentSECIRadio_clicked(bool checked);
	// Reload info button pressed
	void on_ReloadButton_clicked(bool checked);
	// Block tree item clicked
	void on_BlockTree_itemClicked(QTreeWidgetItem* item, int column);
	// Block Tree context menu - Set Group
	void blockTreeContextMenu_setGroup(bool checked);
	// Block Tree context menu - Remove Group
	void blockTreeContextMenu_removeGroup(bool checked);
	// -- Run Data Tab
	// Add property to visible list
	void on_AddPropertyButton_clicked(bool checked);
	// Remove property from visible list
	void on_RemovePropertyButton_clicked(bool checked);
	// Move property up
	void on_MovePropertyUpButton_clicked(bool checked);
	// Move property down
	void on_MovePropertyDownButton_clicked(bool checked);
	// -- Dialog
	// Set as Default button pressed
	void on_SetAsDefaultButton_clicked(bool checked);
	// Cancel button pressed
	void on_CancelButton_clicked(bool checked);
	// SaveAsPDF Button pressed
	void on_SaveAsPDFButton_clicked(bool checked);
	// Print Button pressed
	void on_PrintButton_clicked(bool checked);


	/*
	// Graph Generation
	*/
	private:
	// Create beam current graph
	void setupBeamCurrentGraph(PlotWidget& target, ISIS::Location targetStation);
	// Create generic property graph
	void setupGraph(PlotWidget& target, QString blockName, double minY, QColor minColor, double maxY, QColor maxColor, QString altBlockName = QString());
};

#endif

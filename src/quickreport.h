/*
	*** Quick Report Generator
	*** src/quickreport.h
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

#ifndef JOURNALVIEWER_QUICKREPORT_H
#define JOURNALVIEWER_QUICKREPORT_H

#include "ui_quickreport.h"
#include "list.h"
#include "instrument.h"
#include "rundata.h"
#include "rbdata.h"
#include <QDialog>
#include <QTextDocument>

// Forward Declarations
class JournalViewer;
class Document;
class Journal;

class QuickReport : public QDialog
{
	// All Qt declarations must include this macro
	Q_OBJECT


	/*
	// Window Functions
	*/
	public:
	// Constructor / Destructor
	QuickReport(JournalViewer* parent, const QList<int>& availableRB, QStringList availableRBUsers, RefList<RunData, Journal*>& runData);
	~QuickReport();
	// Result Enum
	enum Result { CancelResult, SaveResult, PrintResult };
	// Main form declaration
	Ui::QuickReport ui;

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
	// Result to return from dialog
	QuickReport::Result result_;

	public:
	// Return dialog report result
	QuickReport::Result reportResult();


	/*
	// Experiment Details
	*/
	private:
	// List of data for RB numbers
	List<RBData> rbData_;

	private:
	// Generate report data
	void generateData();

	public:
	// Create experiment report
	bool createReport(Document& report);


	/*
	// Widget Slots
	*/
	private:
	// Update report tree
	void updateReportTree();

	private slots:
	// Close button pressed
	void on_CloseButton_clicked(bool checked);
	// SaveAsPDF Button pressed
	void on_SaveAsPDFButton_clicked(bool checked);
	// Print Button pressed
	void on_PrintButton_clicked(bool checked);
};

#endif

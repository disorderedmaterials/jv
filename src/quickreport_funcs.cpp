/*
	*** QuickReport Functinos
	*** src/quickreport_funcs.cpp
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

#include "quickreport.h"
#include "jv.h"
#include "datainterface.h"
#include "document.h"
#include "messenger.hui"
#include <QString>
#include <QSettings>
#include <QInputDialog>
#include <QProgressDialog>

// Constructor
QuickReport::QuickReport(JournalViewer* parent, const QList<int>& availableRB, QStringList availableRBUsers, RefList<RunData, Journal*>& runData) : QDialog(), runData_(runData)
{
	// Call the main creation function
	ui.setupUi(this);

	parent_ = parent;
	instrument_ = parent_->currentInstrument();
	availableRB_ = availableRB;
	rbNumber_ = -1;

	// Generate data
	generateData();
	
	refreshing_ = true;
	updateReportTree();
	refreshing_ = false;
}

// Destructor
QuickReport::~QuickReport()
{
}

// Return dialog report result
QuickReport::Result QuickReport::reportResult()
{
	return result_;
}

/*
// Report
*/

// Create experiment report
bool QuickReport::createReport(Document& report)
{
	const char* dateFormat = "dddd d MMMM yyyy";
	
	// Setup colours
	QColor headerBG(16, 40, 120);  // ISIS Blue??
	QColor headerText(255, 255, 255);
	
	QString s, valueString;
	QString uA = QString(QChar(0x03BC)) + "A";

	// Variables
	int n, lastGroup = -1;
	RunData* rd = NULL;

	// Clear old document
	report.clearCommands();
	report.setBackgroundColour(QColor(255,255,255));
	report.setForegroundColour(QColor(0,0,0));
	report.setFont(parent_->documentFont());

	/* First part of report contains instrument name and date range */
	report.clearColumns();
	report.addColumn(false);
	report.setColours(headerText, headerBG);
	report.addBoldText("Quick Report - " + parent_->currentInstrument()->capitalisedName(), 99, Qt::AlignHCenter);
	report.endRow();
	report.addBoldText(runData_.first()->item->startDateTimeString() + " to " + runData_.last()->item->endDateTimeString(), 99, Qt::AlignHCenter);
	report.endRow();

	// Leave a blank row
	report.setColours(QColor(0,0,0), QColor(255,255,255));
	report.endRow();
	
	// RB Data section
	report.clearColumns();
	for (int n=0; n<7; ++n) report.addColumn();

	// Write column headers
	report.setColours(QColor(0,0,0), QColor(255,255,255));
	report.addBoldText("RB Number");
	report.addBoldText("Proton Time");
	report.addBoldText(uA);
	report.addBoldText("NRuns");
	report.addBoldText("First Run");
	report.addBoldText("Last Run");
	report.addBoldText("Literal Time");
	report.endRow();

	// Loop over rbData
	for (RBData* data = rbData_.first(); data != NULL; data = data->next)
	{
		// Create new root treenode and add total data...
		report.addBoldText(QString::number(data->rbNumber()) + " (" + QString::number(data->nParts()) + " parts)");
		report.addText(RunData::durationAsString(data->totalRunTime()));
		report.addText(QString::number(data->totalCurrent(), 'f', 1));
		report.addText(QString::number(data->totalNRuns()));
		report.addText(data->totalBeginRunDateTime().toString("hh:mm:ss dd/MM/yy"));
		report.addText(data->totalEndRunDateTime().toString("hh:mm:ss dd/MM/yy"));
		report.addText(RunData::durationAsString(data->totalLiteralTime()));
		report.endRow();

		// Add parts data
		int count = 1;
		for (RBDataPart* part = data->parts(); part != NULL; part = part->next)
		{
			report.addText("Part " + QString::number(count), 1, Qt::AlignHCenter);
			report.addText(RunData::durationAsString(part->runTime()));
			report.addText(QString::number(part->current(), 'f', 1));
			report.addText(QString::number(part->nRuns()));
			report.addText(part->beginRunDateTime().toString("hh:mm:ss dd/MM/yy"));
			report.addText(part->endRunDateTime().toString("hh:mm:ss dd/MM/yy"));
			report.addText(RunData::durationAsString(part->literalTime()));
			report.endRow();
			++count;
		}
	}
	report.end();

	return result_;
}

/*
// Widget Slots
*/

// Update report tree
void QuickReport::updateReportTree()
{
	QString uA = QString(QChar(0x03BC)) + "A";

	// Clear the current tree
	ui.ReportTree->clear();
	ui.ReportTree->setColumnCount(7);
	ui.ReportTree->setHeaderLabels(QStringList() << "RB Number" << "Proton Time" << uA << "NRuns" << "First Run" << "Last Run" << "Literal Time");

	// Loop over rbData
	for (RBData* data = rbData_.first(); data != NULL; data = data->next)
	{
		// Create new root treenode and add total data...
		QTreeWidgetItem* parentItem = new QTreeWidgetItem(ui.ReportTree);
		parentItem->setText(0, QString::number(data->rbNumber()) + " (" + QString::number(data->nParts()) + " parts)");
		parentItem->setText(1, RunData::durationAsString(data->totalRunTime()));
		parentItem->setText(2, QString::number(data->totalCurrent(), 'f', 1));
		parentItem->setText(3, QString::number(data->totalNRuns()));
		parentItem->setText(4, data->totalBeginRunDateTime().toString("hh:mm:ss dd/MM/yy"));
		parentItem->setText(5, data->totalEndRunDateTime().toString("hh:mm:ss dd/MM/yy"));
		parentItem->setText(6, RunData::durationAsString(data->totalLiteralTime()));

		// Add parts data
		int count = 1;
		for (RBDataPart* part = data->parts(); part != NULL; part = part->next)
		{
			QTreeWidgetItem* partItem = new QTreeWidgetItem(parentItem);
			partItem->setText(0, "Part " + QString::number(count));
			partItem->setText(1, RunData::durationAsString(part->runTime()));
			partItem->setText(2, QString::number(part->current(), 'f', 1));
			partItem->setText(3, QString::number(part->nRuns()));
			partItem->setText(4, part->beginRunDateTime().toString("hh:mm:ss dd/MM/yy"));
			partItem->setText(5, part->endRunDateTime().toString("hh:mm:ss dd/MM/yy"));
			partItem->setText(6, RunData::durationAsString(part->literalTime()));
			++count;
		}
	}

	for (int n=0; n<5; ++n) ui.ReportTree->resizeColumnToContents(n);
}

// Generate data
void QuickReport::generateData()
{
	// Generate some data!
	// Clear old data first
	rbData_.clear();

	// First, we will create entries in the rbData_ list for the relevant (one, or all) RB numbers, in numerical order
	if (rbNumber_ == -1)
	{
		for (int n=0; n<availableRB_.count(); ++n)
		{
			// Loop over current rbData_ and find the first one with an RB number higher than the current one
			RBData* data = NULL, *newData;
			for (data = rbData_.first(); data != NULL; data = data->next) if (data->rbNumber() > availableRB_.at(n)) break;
			// Insert new RBdata before 'data' in the list....
			newData  = rbData_.insertBefore(data);
			newData->setRBNumber(availableRB_.at(n));
		}
	}
	else
	{
		// Single RB number selected
		RBData* data = rbData_.add();
		data->setRBNumber(rbNumber_);
	}

	// Loop over all available run data, adding their data to the correct rbData_
	RBData* lastData = NULL;
	for (RefListItem<RunData,Journal*>* ri = runData_.first(); ri != NULL; ri = ri->next)
	{
		RunData* rd = ri->item;
// 		if (rd->rbNumber() != rbNumber_) continue;

		// Check lastData first, since many data are likely to in one consecutive block for a given RB
		// If not that one, loop over defined rbData_, seeing if any accept this runData
		if (lastData && lastData->update(rd)) continue;
		else for (lastData = rbData_.first(); lastData != NULL; lastData = lastData->next) if (lastData->update(rd)) break;
	}
}

// Close Button Pressed
void QuickReport::on_CloseButton_clicked(bool checked)
{
	result_ = QuickReport::CancelResult;
	reject();
}

// SaveAsPDF Button Pressed
void QuickReport::on_SaveAsPDFButton_clicked(bool checked)
{
	result_ = QuickReport::SaveResult;
	accept();
}

// Print Button pressed
void QuickReport::on_PrintButton_clicked(bool checked)
{
	result_ = QuickReport::PrintResult;
	accept();
}

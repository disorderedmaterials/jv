/*
	*** Sample Report
	*** src/samplereport.h
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

#ifndef JOURNALVIEWER_SAMPLEREPORT_H
#define JOURNALVIEWER_SAMPLEREPORT_H

#include "ui_samplereport.h"
#include "reflist.h"

// Forward Declarations
class RunData;

// SampleReportGroup, storing information about a group of like run data
class SampleReportGroup
{
	public:
	// Constructor / Destructor
	SampleReportGroup();
	~SampleReportGroup();
	// List pointers
	SampleReportGroup* prev, *next;

	private:
	// Common title of run numbers in this group
	QString title_;
	// List of associated run data
	RefList<RunData,int> runData_;
	// Flag indicating that run time variable is MeV (rather than uAh)
	bool useMeV_;
	// Sum of 'run time' variable
	double runTimeSum_;

	public:
	// Add supplied RunData, if it is similar
	bool addIfSimilar(RunData* datum);
	// Add supplied RunData, setting title of group etc.
	void addFirst(RunData* datum, bool useMeV);
	// Return title of run numbers
	QString title();
	// Return first RunData in list
	RefListItem<RunData,int>* runData();
	// Return 'run time' sum variable
	double runTimeSum();
};

class SampleReport: public QDialog
{
	// All Qt declarations must include this macro
	Q_OBJECT


	/*
	// Window Functions
	*/
	public:
	// Constructor / Destructor
	SampleReport(QWidget* parent);
	~SampleReport();
	// Main form declaration
	Ui::SampleReportWindow ui;
	
	private:
	// Refreshing flag
	bool refreshing_;

	public:
	// Add RunData to GraphWidget
	void addRunData(RefList<RunData,int>& data, bool useMeV);
	// Finalise and show GraphWidget
	void finaliseAndShow();


	/*
	// Widget Slots
	*/
	private slots:
	// Close button
	void on_CloseButton_clicked(bool checked);
};

#endif

/*
	*** RB Data
	*** src/rbdata.h
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

#ifndef JOURNALVIEWER_RBDATA_H
#define JOURNALVIEWER_RBDATA_H

#include "list.h"
#include "instrument.h"
#include "rundata.h"

// Forward Declarations
//class Journal;

// Part information data
class RBDataPart : public ListItem<RBDataPart>
{
	public:
	RBDataPart();

	private:
	// Number of runs in this part
	int nRuns_;
	// DateTime of beginning of first run in this part
	QDateTime beginRunDateTime_;
	// DateTime of end of last run in this part
	QDateTime endRunDateTime_;
	// Total duration of runs (in seconds)
	int runTime_;
	// Total current accumulated by all runs
	double current_;
	// Index of last rundata added to this part
	int lastRunNumber_;

	public:
	// Update this part with supplied RunData (which we assume is the correct RB number), returning if we accepted it
	bool update(RunData* rd);
	// Return number of runs
	int nRuns();
	// Return run time in seconds
	int runTime();
	// Return proton current
	double current();
	// Return datetime of first run over all parts
	QDateTime beginRunDateTime();
	// Return datetime of last run over all parts
	QDateTime endRunDateTime();
	// Return literal total time in seconds
	int literalTime();
};

// Simple class for data / statistics of a given RB number
class RBData : public ListItem<RBData>
{
	public:
	RBData();
	
	private:
	// RB Number
	int rbNumber_;
	// Total number of runs
	int totalNRuns_;
	// DateTime of beginning of first run for whole RB
	QDateTime totalBeginRunDateTime_;
	// DateTime of end of last run for whole RB
	QDateTime totalEndRunDateTime_;
	// Total duration of all runs (in seconds)
	int totalRunTime_;
	// Total current accumulated by all runs
	double totalCurrent_;
	// List of parts for this RB number
	List<RBDataPart> parts_;

	public:
	// Set RB number for this data
	void setRBNumber(int rbno);
	// Return RB number for this data
	int rbNumber();
	// Update information with supplied RunData, returning whether it was accepted or not
	bool update(RunData* rd);
	// Return total number of parts
	int nParts();
	// Return first part in list
	RBDataPart* parts();
	// Return total number of runs
	int totalNRuns();
	// Return total run time in seconds
	int totalRunTime();
	// Return total proton current
	double totalCurrent();
	// Return datetime of first run over all parts
	QDateTime totalBeginRunDateTime();
	// Return datetime of last run over all parts
	QDateTime totalEndRunDateTime();
	// Return literal total time in seconds
	int totalLiteralTime();
};

#endif

/*
	*** RB Data
	*** src/rbdata.cpp
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

#include "rbdata.h"

/*
 * RBDataPart
 */

// Constructor
RBDataPart::RBDataPart() : ListItem<RBDataPart>()
{
	// Private variables
	nRuns_ = 0;
	current_ = 0.0;
	runTime_ = 0;
	lastRunNumber_ = -1;
// 	runTime_.c
}

// Update this part with supplied RunData (which we assume is the correct RB number), returning if we accepted it
bool RBDataPart::update(RunData* rd)
{
	// Check the index of the run - if it is not consecutive to the last one in our list (if there is one) then we reject it
	if ((lastRunNumber_ != -1) && ((lastRunNumber_+1) != rd->runNumber())) return false;

	// Update totals for this part
	if (nRuns_ == 0)
	{
		beginRunDateTime_ = rd->startDateTime();
		endRunDateTime_ = rd->endDateTime();
	}
	else
	{
		if (rd->startDateTime() < beginRunDateTime_) beginRunDateTime_ = rd->startDateTime();
		if (rd->endDateTime() > endRunDateTime_) endRunDateTime_ = rd->endDateTime();
	}
	current_ += rd->protonCharge();
	runTime_ += rd->duration();
	++nRuns_;
	lastRunNumber_ = rd->runNumber();

	return true;
}

// Return number of runs
int RBDataPart::nRuns()
{
	return nRuns_;
}

// Return run time in seconds
int RBDataPart::runTime()
{
	return runTime_;
}

// Return proton current
double RBDataPart::current()
{
	return current_;
}

// Return datetime of first run over all parts
QDateTime RBDataPart::beginRunDateTime()
{
	return beginRunDateTime_;
}

// Return datetime of last run over all parts
QDateTime RBDataPart::endRunDateTime()
{
	return endRunDateTime_;
}

// Return literal total time in seconds
int RBDataPart::literalTime()
{
	return beginRunDateTime_.secsTo(endRunDateTime_);
}

/*
 * RBData
 */

// Constructor
RBData::RBData() : ListItem<RBData>()
{
	// Private variables
	rbNumber_ = -1;
	totalNRuns_ = 0;
	totalCurrent_ = 0.0;
	totalRunTime_= 0;
}

// Set RB number for this data
void RBData::setRBNumber(int rbno)
{
	rbNumber_ = rbno;
}

// Return RB number for this data
int RBData::rbNumber()
{
	return rbNumber_;
}

// Update information with supplied RunData, returning whether it was accepted or not
bool RBData::update(RunData* rd)
{
	// First, check that a valid RunData was passed, and that it's RB number matches ours
	if ((!rd) || (rd->rbNumber() != rbNumber_)) return false;

	// Update user list???!?
// 	if (!userNames.contains(rd->user())) userNames << rd->user();

	// Find / create a suitable part to add this to
	if ((parts_.nItems() == 0) || (!parts_.last()->update(rd)))
	{
		RBDataPart* newPart = parts_.add();
		newPart->update(rd);
	}

	// Update totals
	if (totalNRuns_ == 0)
	{
		totalBeginRunDateTime_ = rd->startDateTime();
		totalEndRunDateTime_ = rd->endDateTime();
	}
	else
	{
		if (rd->startDateTime() < totalBeginRunDateTime_) totalBeginRunDateTime_ = rd->startDateTime();
		if (rd->endDateTime() > totalEndRunDateTime_) totalEndRunDateTime_ = rd->endDateTime();
	}
	totalCurrent_ += rd->protonCharge();
	totalRunTime_ += rd->duration();
	++totalNRuns_;

	return true;
}

// Return total number of parts
int RBData::nParts()
{
	return parts_.nItems();
}

// Return first part in list
RBDataPart* RBData::parts()
{
	return parts_.first();
}

// Return total number of runs
int RBData::totalNRuns()
{
	return totalNRuns_;
}

// Return total run time in seconds
int RBData::totalRunTime()
{
	return totalRunTime_;
}

// Return total proton current
double RBData::totalCurrent()
{
	return totalCurrent_;
}

// Return datetime of first run over all parts
QDateTime RBData::totalBeginRunDateTime()
{
	return totalBeginRunDateTime_;
}

// Return datetime of last run over all parts
QDateTime RBData::totalEndRunDateTime()
{
	return totalEndRunDateTime_;
}

// Return literal total time of experiment in seconds
int RBData::totalLiteralTime()
{
	return totalBeginRunDateTime_.secsTo(totalEndRunDateTime_);
}

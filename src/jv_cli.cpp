/*
	*** JournalViewer CLI Control Functions
	*** src/jv_cli.cpp
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
#include "instrument.h"
#include "datainterface.h"
#include "messenger.hui"
#include <QRegExp>

// Change current journal
bool JournalViewer::changeJournal(const char* journalName)
{
	if (currentInstrument_ == NULL) return false;

	// Construct full journal name (we expect to receive a string of format YY/C)
	QString jname;
	if (QString(journalName).toLower() == "all") jname = "All";
	else jname = QString("Cycle ") + journalName;

	// See if a journal with this name exists
	Journal* journal = currentInstrument_->journal(jname);
	if (journal == NULL)
	{
		msg.print("Error: No Journal with name '%s' exists for the current instrument.", journalName);
		return false;
	}

	// Clear all current visibility flags
	setAllRunDataVisible(true);

	setJournal(journal);
	msg.print("Changed to journal '%s'.", qPrintable(jname));
	return true;
}

// Change current instrument
bool JournalViewer::changeInstrument(const char* instrumentName)
{
	// Search for instrument provided...
	ISIS::ISISInstrument inst = ISIS::instrument(instrumentName);
	if (inst == ISIS::nInstruments)
	{
		msg.print("Failed to find instrument '%s'.", instrumentName);
		return false;
	}

	setInstrument(instruments_[inst]);
	msg.print("Changed to instrument '%s'.", instrumentName);

	// Clear all current visibility flags
	setAllRunDataVisible(true);

	return true;
}

// Show available journals for current instrument
void JournalViewer::showJournals()
{
	msg.print("Journals available for current instrument:");
	QString lastPrefix, cycleStrings;
	QRegularExpression re("([0-9][0-9])/[0-9]");
	for (Journal* journal = currentInstrument_->journals(); journal != NULL; journal = journal->next)
	{
		QRegularExpressionMatch match = re.match(journal->cycle());
		if (match.hasMatch())
		{
			// If the captured part differs from lastPrefix, print out any existing data in cycleStrings and start again
			if (match.captured(1) != lastPrefix)
			{
				if (!cycleStrings.isEmpty()) msg.print(cycleStrings);
				lastPrefix = match.captured(1);
				cycleStrings = journal->cycle() + " ";
			}
			else cycleStrings += journal->cycle() + "  ";
		}
		else if (journal->cycle() == "All")
		{
			if (!cycleStrings.isEmpty()) msg.print(cycleStrings);
			cycleStrings.clear();
			lastPrefix.clear();
			msg.print("All");
		}
		else msg.print("Error parsing cycle name '%s' in showJournals().", qPrintable(journal->cycle()));
	}
	if (!cycleStrings.isEmpty()) msg.print(cycleStrings);
}

// Search for and display runs matching supplied string / search style
bool JournalViewer::searchRuns(QString searchString, QRegExp::PatternSyntax searchType)
{
	// Construct a regexp from the supplied text
	QRegExp re(searchString);
	re.setPatternSyntax(searchType);
	if (!re.isValid())
	{
		msg.print("Error: Search string is invalid. No search performed.");
		return false;
	}

	// We will store matching runs in a reflist for now, also storing the character lengths of the visible properties so we can print it out nicely
	RefList<RunData, Journal*> matches;
	int columnWidths[visibleProperties_.nItems()];
	int n;
	for (n=0; n<visibleProperties_.nItems(); ++n) columnWidths[n] = -1;
	for (RefListItem<RunData,Journal*>* ri = runData_.first(); ri != NULL; ri = ri->next)
	{
		RunData* rd = ri->item;

		// Is this a match?
		if (re.indexIn(rd->title()) == -1) continue;

		// Add run to our reflist, and store character lengths
		matches.add(rd);
		n = 0;
		for (RunProperty* property = visibleProperties_.first(); property != NULL; property = property->next, ++n)
		{
			QString propertyText = rd->propertyAsString(property->type());
			if (propertyText.length() > columnWidths[n]) columnWidths[n] = propertyText.length();
		}
	}

	// Print out data in a nice format
	QString format;
	n = 0;
	for (RunProperty* property = visibleProperties_.first(); property != NULL; property = property->next, ++n) format += "%"+QString::number(columnWidths[n])+"s  ";
	for (RefListItem<RunData,Journal*>* ri = matches.first(); ri != NULL; ri = ri->next)
	{
		RunData* rd = ri->item;

		QString data;
		n = 0;
		for (RunProperty* property = visibleProperties_.first(); property != NULL; property = property->next, ++n) data += QString("%1  ").arg(rd->propertyAsString(property->type()), -columnWidths[n], QChar(' '));
		msg.print(data);
	}
	msg.print("");
	msg.print("%i matching run(s).", matches.nItems());

	return true;
}

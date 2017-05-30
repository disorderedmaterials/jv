/*
	*** JournalViewer Printing Functions
	*** src/jv_printing.cpp
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
#include "document.h"
#include "ttablewidgetitem.h"
#include "messenger.hui"

// Create Document ready for printing
void JournalViewer::createDocument(Document& document, const RefList<RunData,int>& data)
{
	// Variables
	bool highlight;
	int nProperties, n, lastGroup = -1;
	RunProperty::Property properties[RunProperty::nProperties];
	RunData* rd = NULL;

	// Clear old document
	document.clearCommands();
	document.setBackgroundColour(QColor(255,255,255));
	document.setForegroundColour(QColor(0,0,0));

	// Create copy of run properties to base columns on
	nProperties = 0;
	for (RunProperty* rp = visibleProperties_.first(); rp != NULL; rp = rp->next) properties[nProperties++] = rp->type();

	switch (documentLayout_)
	{
		/*
		 * Simple list, one line for each run
		 * The only expanding column will be the RunTitle (if it is to be written out)
		 */
		case (JournalViewer::ListLayout):
			// Create basic column layout
			document.clearColumns();
			for (n=0; n<nProperties; ++n) document.addColumn(properties[n] != RunProperty::Title);
			
			// Write header items
			document.setBackgroundColour(headerBGColour_);
			document.setForegroundColour(headerTextColour_);
			for (n=0; n<nProperties; ++n) document.addText(RunProperty::property(properties[n]));
			document.endRow();

			highlight = false;
			rd = NULL;
			for (RefListItem<RunData,int>* ri = data.first(); ri != NULL; ri = ri->next)
			{
				// Get RunData pointer
				rd = ri->item;

				// Determine highlight status
				if (highlightDocument_)
				{
					if (viewByGroup_) highlight = rd->group()%2 == 0;
					else highlight = !highlight;

					// Set colours based on highlighting
					document.setBackgroundColour(highlight ? highlightBGColour_ : QColor(255,255,255));
					document.setForegroundColour(highlight ? highlightTextColour_ : QColor(0,0,0));
				}

				// Loop over RunProperties
				for (n=0; n<nProperties; ++n) document.addText(rd->propertyAsString(properties[n]));
				document.endRow();
			}
			break;
		/*
		 * Indented Layout
		 * If viewing by group, the title will span the whole page, and run info will be written on the subsequent lines
		 * If not, the run number and title will appear on the first line, and run info will be written on the next line
		 */
		case (JournalViewer::IndentedLayout):
			// Construct column layout
			document.clearColumns();

			// Must remove run number and title from properties[]
			for (n=0; n<nProperties; ++n) if ((properties[n] == RunProperty::RunNumber) || (properties[n] == RunProperty::Title)) properties[n] = RunProperty::nProperties;

			// Add column for run number...
			document.addColumn();

			// Add on columns for run properties
			for (n=0; n<nProperties; ++n) if (properties[n] != RunProperty::nProperties) document.addColumn(false);

			// Loop over data to display
			highlight = false;
			rd = NULL;
			for (RefListItem<RunData,int>* ri = data.first(); ri != NULL; ri = ri->next)
			{
				// Get RunData pointer
				rd = ri->item;

				// If viewing by group, leave an empty row
				if (viewByGroup_ && (lastGroup != rd->group()) && (lastGroup != -1)) document.endRow();

				// Determine highlight status
				if (highlightDocument_)
				{
					if (viewByGroup_) highlight = (lastGroup != rd->group());
					else highlight = !highlight;

					// Set colours based on highlighting
					document.setBackgroundColour(highlight ? highlightBGColour_ : QColor(255,255,255));
					document.setForegroundColour(highlight ? highlightTextColour_ : QColor(0,0,0));
				}

				// Write run number (if not viewing by group) or title only (if we are)
				if (!viewByGroup_)
				{
					document.addText(rd->propertyAsString(RunProperty::RunNumber));
				document.addText(rd->title(), 99);
					document.endRow();
				}
				else if (lastGroup != rd->group())
				{
					// Write new title row, since group number has changed
					document.addText(rd->title(), 99);
					document.endRow();

					// Reset highlighting here, if it was on
					if (highlightDocument_)
					{
						document.setBackgroundColour(QColor(255,255,255));
						document.setForegroundColour(QColor(0,0,0));
					}
				}
				
				// Write run data
				// -- Write run number if viewing by group, otherwise write blank column
				if (viewByGroup_) document.addText(rd->propertyAsString(RunProperty::RunNumber));
				else document.addText("");

				// -- Loop over RunProperties
				for (n=0; n<nProperties; ++n) if (properties[n] != RunProperty::nProperties) document.addText(rd->propertyAsString(properties[n]));

				// End row
				document.endRow();

				// Set group number
				lastGroup = rd->group();
			}
			break;
		default:
			break;
	}

	document.end();
}

// Create text document
void JournalViewer::createText(QString& string, const RefList<RunData,int>& data, QString separator)
{
	// Loop over table items rather than RunData, so we preserve the sort order of the items within it...
	RunData* rd = NULL;
	TTableWidgetItem* item = NULL, *nextItem;
	int lastRunNumber = -1;

	// Loop over list of selected RunData
	for (RefListItem<RunData,int>* ri = data.first(); ri != NULL; ri = ri->next)
	{
		// Get RunData pointer
		rd = ri->item;

		for (RunProperty* rp = visibleProperties_.first(); rp != NULL; rp = rp->next)
		{
			if (RunProperty::propertyNeedsQuotes(rp->type())) string += QString("\"" + rd->propertyAsString(rp->type()) + "\"");
			else string += rd->propertyAsString(rp->type());
			
			string += separator;
		}
		string += "\n";
	}
}

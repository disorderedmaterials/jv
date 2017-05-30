/*
	*** Sample Report Functinos
	*** src/samplereport_funcs.cpp
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

#include "samplereport.h"
#include "rundata.h"

/*
 * SampleReportGroup
 */

// Constructor
SampleReportGroup::SampleReportGroup()
{
	// Public variables
	next = NULL;
	prev = NULL;

	// Private variables
	useMeV_ = false;
	runTimeSum_ = 0.0;
}

// Destructor
SampleReportGroup::~SampleReportGroup()
{
}

// Add supplied RunData, if it is similar
bool SampleReportGroup::addIfSimilar(RunData* datum)
{
	if (title_ != datum->title()) return false;
	runData_.add(datum);
	runTimeSum_ += (useMeV_ ? datum->totalMEvents() : datum->protonCharge());
	return true;
}

// Add supplied RunData, setting title of group etc.
void SampleReportGroup::addFirst(RunData* datum, bool useMeV)
{
	if (runData_.nItems() != 0)
	{
		printf("Error - tried to begin a SampleReportGroup which already contains RunData.\n");
		return;
	}
	title_ = datum->title();
	useMeV_ = useMeV;
	addIfSimilar(datum);
}

// Return title of run numbers
QString SampleReportGroup::title()
{
	return title_;
}

// Return first RunData in list
RefListItem<RunData,int>* SampleReportGroup::runData()
{
	return runData_.first();
}

// Return 'run time' sum variable
double SampleReportGroup::runTimeSum()
{
	return runTimeSum_;
}

/*
 * SampleReport
 */
// Constructor
SampleReport::SampleReport(QWidget* parent) : QDialog(parent)
{
	// Call the main creation function
	ui.setupUi(this);
	refreshing_= false;
}

// Destructor
SampleReport::~SampleReport()
{
}

// Add RunData to Tree
void SampleReport::addRunData(RefList<RunData,int>& data, bool useMeV)
{
	// Setup TreeView
	ui.SampleReportTree->clear();
	ui.SampleReportTree->setColumnCount(2);
	if (useMeV) ui.SampleReportTree->setHeaderLabels( QStringList() << "Title / Run Number" << "Total MeV" );
	else ui.SampleReportTree->setHeaderLabels( QStringList() << "Title / Run Number" << "Total uAh" );
	
	// First, need to group runs with same title together
	List<SampleReportGroup> groupedData;
	SampleReportGroup* group;
	RefListItem<RunData,int>* ri;

	// Loop over supplied RunData, creating groups as we go
	for (ri = data.first(); ri != NULL; ri = ri->next)
	{
		// Loop over existing groups, seeing if it matches
		for (group = groupedData.first(); group != NULL; group = group->next) if (group->addIfSimilar(ri->item)) break;
		if (group == NULL)
		{
			group = groupedData.add();
			group->addFirst(ri->item, useMeV);
		}
	}

	// Done - populate TreeView now...
	QTreeWidgetItem* parent, *item;
	RunData* rd;
	for (group = groupedData.first(); group != NULL; group = group->next)
	{
		// Create toplevel item
		parent = new QTreeWidgetItem(ui.SampleReportTree);
		parent->setText(0, group->title());
		parent->setText(1, QString::number(group->runTimeSum()));
		parent->setExpanded(false);
		ui.SampleReportTree->addTopLevelItem(parent);

		// Now add child items...
		for (ri = group->runData(); ri != NULL; ri = ri->next)
		{
			rd = ri->item;
			item = new QTreeWidgetItem(parent);
			item->setText(0, QString::number(rd->runNumber()));
			item->setText(1, QString::number(rd->protonCharge()));
		}
	}
}

// Finalise and show GraphWidget
void SampleReport::finaliseAndShow()
{
	refreshing_ = true;

	refreshing_ = false;

	// Go!
	show();
}

/*
// Widget Slots
*/

// Close button pressed
void SampleReport::on_CloseButton_clicked(bool checked)
{
	hide();
}

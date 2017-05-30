/*
	*** RunData Window Functinos
	*** src/rundatawindow_funcs.cpp
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

#include "rundatawindow.h"
#include "rundata.h"
#include "messenger.hui"
#include <QListWidgetItem>
#include <QClipboard>
#include <QMenu>
#include <QFileDialog>
#include <QTextStream>

// Constructor
RunDataWindow::RunDataWindow(QWidget* parent, QFont& font) : QDialog(parent)
{
	// Call the main creation function
	ui.setupUi(this);
	ui.PlotArea->setCoordinatesLabel(ui.CoordinatesLabel);
	ui.PlotArea->setFont(font);
	ui.AbsoluteTimeCheck->setChecked(ui.PlotArea->absoluteTime());
	ui.ShowBeginEndCheck->setChecked(ui.PlotArea->showBeginEnd());
	ui.ShowLegendCheck->setChecked(ui.PlotArea->showLegend());
	ui.OnChangeCheck->setChecked(ui.PlotArea->onChangeData());
	ui.SingleValueTree->setHeaderLabels(QStringList() << "Group" << "Block" << "Value");
	ui.AnalysisTree->setHeaderLabels(QStringList() << "Property/Run" << "NPoints" << "Min(Run)" << "Avg(Run)" << "Max(Run)" << "Min(All)" << "Avg(All)" << "Max(All)");

	// Connect contextMenuEvent in RunPropertyList
	connect(ui.RunPropertyList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(runPropertyList_contextMenuEvent(QPoint)));
}

// Destructor
RunDataWindow::~RunDataWindow()
{
}

// Update analysis tab
void RunDataWindow::updateAnalysisTree()
{
	// Create main tree nodes, named after available plotDataBlocks
	RefList<PlotDataBlock,QTreeWidgetItem*> treeBlocks;
	QTreeWidgetItem* parent, *item;
	for (PlotDataBlock* pdb = ui.PlotArea->dataSetBlocks().first(); pdb != NULL; pdb = pdb->next)
	{
		parent = new QTreeWidgetItem(ui.AnalysisTree);
		parent->setText(0, pdb->blockName());
		ui.AnalysisTree->addTopLevelItem(parent);
		treeBlocks.add(pdb, parent);
	}

	// Now, loop over datasets and add analysis to tree
	RefListItem<PlotDataBlock,QTreeWidgetItem*>* ri;
	for (PlotData* pd = ui.PlotArea->dataSets().first(); pd != NULL; pd = pd->next)
	{
		// Find parent item for this dataset
		for (ri = treeBlocks.first(); ri != NULL; ri = ri->next) if (pd->block() == ri->item) break;
		if (ri == NULL) continue;

		parent = ri->data;
		item = new QTreeWidgetItem(parent);
		item->setText(0, pd->name());

		Data2D& data = pd->data();
		bool noValue;
		double value;

		item->setText(1, QString::number(data.runNPoints()));

		// Run min, average, and max 
		value = data.runMinimum(noValue);
		item->setText(2, noValue ? "--" : QString::number(value));
		value = data.runAverage(noValue);
		item->setText(3, noValue ? "--" : QString::number(value));
		value = data.runMaximum(noValue);
		item->setText(4, noValue ? "--" : QString::number(value));

		// Full Min, average, and max
		value = data.minimum(noValue);
		item->setText(5, noValue ? "--" : QString::number(value));
		value = data.average(noValue);
		item->setText(6, noValue ? "--" : QString::number(value));
		value = data.maximum(noValue);
		item->setText(7, noValue ? "--" : QString::number(value));

	}
	for (int n=0; n<5; ++n) ui.AnalysisTree->resizeColumnToContents(n);
}

// Export run property data
void RunDataWindow::exportRunPropertyData(QString property)
{
	RefList<PlotDataBlock,int> properties;
	// If a 'property' was provided, make sure it exists...
	if (!property.isEmpty())
	{
		PlotDataBlock* prop = ui.PlotArea->dataSetBlock(property);
		if (prop) properties.add(prop);
		else
		{
			printf("Internal Error: Named block %s does not exist in the PlotArea.\n", qPrintable(property));
			return;
		}
	}
	else properties = ui.PlotArea->visibleDataSetBlocks();

	// Get number of run data in list, and number that are selected
	//int nRuns = ui.PlotArea->dataSetGroups().nItems();

	// For each selected property, ask for a filename and then save the data
	static QDir currentDirectory;
	for (RefListItem<PlotDataBlock,int>* ri = properties.first(); ri != NULL; ri = ri->next)
	{
		PlotDataBlock* pdb = ri->item;

		QString fileName = QFileDialog::getSaveFileName(this, "Export property '" + QString(pdb->blockName()) + "'", currentDirectory.path());
		if (fileName.isEmpty()) return;

		msg.print("Writing datafile '%s'...\n", qPrintable(fileName));

		// Construct a separate reflist of PlotData in numerical (name_) order, since they may have been added to the list in any order
		RefList<PlotData,int> orderedList;
		RefListItem<PlotData,int>* rk, *newItem;
		for (RefListItem<PlotData,int>* rj = pdb->plotData().first(); rj != NULL; rj = rj->next)
		{
			PlotData* pd = rj->item;

			// Get run number for current plot data
			int runNumber = pd->name().toInt();

			// Now step through orderedList until we find the first item with a higher runNumber
			rk = NULL;
			for (rk = orderedList.first(); rk != NULL; rk = rk->next) if (rk->data > runNumber) break;
			newItem = (rk ? orderedList.addBefore(rk, pd) : orderedList.add(pd));
			newItem->data = runNumber;
		}

		// Open file and textstream
		QFile file(fileName);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
		QTextStream out(&file);
		out << "# Property: '" + pdb->blockName() + "'\n";
		out << "# seconds(absolute)  y   time(relative)\n";

		// Loop over PlotData associated to property
		QDateTime originTime = ui.PlotArea->absoluteTimeOrigin();
		for (RefListItem<PlotData,int>* rj = orderedList.first(); rj != NULL; rj = rj->next)
		{
			PlotData* pd = rj->item;
			const Array<int>& x = pd->data().arrayX();
			const Array<Data2DValue>& y = pd->data().arrayY();
			QDateTime runStartTime = pd->runTimeStart();
			
			// Write three columns - absoluteX  Y  date/timeX
			QDateTime pointTime;
			pointTime = runStartTime.addSecs(x.value(0));
			out << originTime.secsTo(pointTime) << "     " << y.value(0).y() << "     " << pointTime.toString("dd/MM/yyThh:mm:ss") << "   # " << pd->name() << "\n";
			for (int n=1; n<pd->data().nPoints(); ++n)
			{
				pointTime = runStartTime.addSecs(x.value(n));
				out << originTime.secsTo(pointTime) << "     " << y.value(n).y() << "     " << pointTime.toString("dd/MM/yyThh:mm:ss") << "\n";
			}
		}
		file.close();
	}
}

// Add RunData to GraphWidget
void RunDataWindow::addRunData(RunData* rd)
{
	// Check for valid pointer
	if (rd == NULL) return;

	QString name = QString::number(rd->runNumber());

	// Add item to RunList
	QListWidgetItem* item = new QListWidgetItem(name);
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
	item->setCheckState(Qt::Checked);
	ui.RunList->addItem(item);

	// Create a new time period to associate with the PlotData we're about to create
	PlotDataGroup* group = ui.PlotArea->addPlotDataGroup(name, true, rd->startDateTime(), rd->endDateTime());

	// Loop over block data stored in RunData
	for (Data2D* blockData = rd->blockData(); blockData != NULL; blockData = blockData->next)
	{
		// Add dataset to PlotArea
		ui.PlotArea->addDataSet(group, (*blockData), name, rd->endDateTime(), blockData->name());
	}
	
	// Add single value data to tree
	if (rd->singleValues() != NULL)
	{
		// Create parent Tree node
		QTreeWidgetItem* parent = new QTreeWidgetItem(ui.SingleValueTree);
		parent->setText(0, name);
		
		QTreeWidgetItem* item;
		for (SingleValue* sv = rd->singleValues(); sv != NULL; sv = sv->next)
		{
			item = new QTreeWidgetItem(parent);
			item->setText(0, sv->groupName());
			item->setText(1, sv->blockName());
			item->setText(2, sv->value());
		}

		// Set group to be expanded
		parent->setExpanded(true);
	}
}

// Finalise and show GraphWidget
void RunDataWindow::finaliseAndShow()
{
	refreshing_ = true;

	// Set Group items in RunPropertyList
	for (PlotDataBlock* pdb = ui.PlotArea->dataSetBlocks().first(); pdb != NULL; pdb = pdb->next)
	{
		QListWidgetItem* item = new QListWidgetItem(pdb->blockName());
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Unchecked);
		ui.RunPropertyList->addItem(item);
	}

	// Update analysis tree
	updateAnalysisTree();

	refreshing_ = false;

	// Go!
	show();
}

/*
// Widget Slots
*/

/*
 * SECI/RunLog Widget
 */

// Run List item changed
void RunDataWindow::on_RunList_itemChanged(QListWidgetItem* item)
{
	if (item == NULL) return;
	ui.PlotArea->setGroupVisible( item->text(), item->checkState() == Qt::Checked );
}

// Run Property item changed
void RunDataWindow::on_RunPropertyList_itemChanged(QListWidgetItem* item)
{
	if (item == NULL) return;
	ui.PlotArea->setBlockVisible( item->text(), item->checkState() == Qt::Checked );
}

// Absolute time check clicked
void RunDataWindow::on_AbsoluteTimeCheck_clicked(bool checked)
{
	ui.PlotArea->setAbsoluteTime(checked);
}

// Begin/End markers check clicked
void RunDataWindow::on_ShowBeginEndCheck_clicked(bool checked)
{
	ui.PlotArea->setShowBeginEnd(checked);
}

// Show Legend check clicked
void RunDataWindow::on_ShowLegendCheck_clicked(bool checked)
{
	ui.PlotArea->setShowLegend(checked);
}

// -- On Change check clicked
void RunDataWindow::on_OnChangeCheck_clicked(bool checked)
{
	ui.PlotArea->setOnChangeData(checked);
}

// -- Export button clicked
void RunDataWindow::on_ExportButton_clicked(bool checked)
{
	exportRunPropertyData();
}

// Context menu event
void RunDataWindow::runPropertyList_contextMenuEvent(const QPoint &pos)
{
	// Get QTableWidteItem under clicked point
	QListWidgetItem* sourceItem = ui.RunPropertyList->itemAt(pos);
	if (!sourceItem) return;

	// Create and execute context menu
	QMenu menu(this);
	QAction *exportData = menu.addAction(tr("Export this property"));
	QAction *selectedAction = menu.exec(ui.RunPropertyList->viewport()->mapToGlobal(pos));

	// Act on action!
	if (selectedAction == exportData) 
	{
		// Export data for clicked value
		exportRunPropertyData(sourceItem->text());
	}
}

/*
 * Analysis Tab
 */

// Tree item collapsed
void RunDataWindow::on_AnalysisTree_itemCollapsed(QTreeWidgetItem* item)
{
	for (int n=0; n<5; ++n) ui.AnalysisTree->resizeColumnToContents(n);
}

// Tree item expanded
void RunDataWindow::on_AnalysisTree_itemExpanded(QTreeWidgetItem* item)
{
	for (int n=0; n<5; ++n) ui.AnalysisTree->resizeColumnToContents(n);
}

// -- Copy to clipboard button clicked
void RunDataWindow::on_AnalysisCopyToClipboardButton_clicked(bool checked)
{
	QList<QTreeWidgetItem*> selection = ui.AnalysisTree->selectedItems();
	
	QTreeWidgetItem* parentItem = NULL;
	QString text;
	foreach (QTreeWidgetItem* item, selection)
	{
		if (item->parent() == NULL) continue;

		// Check parent of item - if it is different from the last one, write a new header...
		if (item->parent() != parentItem) text += item->parent()->text(0) + "\nRun\tNPoints\tMin\tAvg\tMax\n";
		for (int n=0; n<5; ++n) text += item->text(n) + "\t";
		text += "\n";
		parentItem = item->parent();
	}

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setText(text);
}

/*
 * Dialog
 */

// Close button pressed
void RunDataWindow::on_CloseButton_clicked(bool checked)
{
	close();
}

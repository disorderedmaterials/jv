/*
	*** JournalViewer RunData Functions
	*** src/jv_rundata.cpp
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
#include "messenger.hui"
#include "ttablewidgetitem.h"
#include "rundatawindow.h"
#include "datainterface.h"
#include <QMessageBox>
#include <QProgressDialog>

/*
 * Run Data
 */

// Determine limits, including construction of unique lists
void JournalViewer::findFilterLimits()
{
	// Clear lists
	availableUsers_.clear();
	availableRB_.clear();
	availableRBUsers_.clear();

	// Set initial values
	RefListItem<RunData,Journal*>* ri = runData_.first();
	if (ri != NULL)
	{
		RunData* rd = ri->item;
		earliestRunStart_ = rd->startDateTime();
		latestRunStart_ = rd->endDateTime();
		firstRunNumber_ = rd->runNumber();
		lastRunNumber_ = rd->runNumber();
	}
	else
	{
		earliestRunStart_ = QDateTime::currentDateTime();
		latestRunStart_ = QDateTime::currentDateTime();
		firstRunNumber_ = 0;
		lastRunNumber_ = 0;
	}

	// Loop over items...
	for (ri = runData_.first(); ri != NULL; ri = ri->next)
	{
		RunData* rd = ri->item;
		if (!availableUsers_.contains(rd->user())) availableUsers_ << rd->user();
		if (!availableRB_.contains(rd->rbNumber()))
		{
			availableRB_ << rd->rbNumber();
			availableRBUsers_ << rd->user();
		}
		if (rd->runNumber() < firstRunNumber_) firstRunNumber_ = rd->runNumber();
		if (rd->runNumber() > lastRunNumber_) lastRunNumber_ = rd->runNumber();
		if (rd->startDateTime() < earliestRunStart_) earliestRunStart_ = rd->startDateTime();
		if (rd->endDateTime() > latestRunStart_) latestRunStart_ = rd->endDateTime();
	}
	
	// Recreate combo box items
	ui.FilterUserCombo->clear();
	ui.FilterUserCombo->addItem("<All>");
	foreach (QString user, availableUsers_) ui.FilterUserCombo->addItem(user);
	ui.FilterRBCombo->clear();
	ui.FilterRBCombo->addItem("<All>");
	QString tempString;
	foreach (int rbno, availableRB_) ui.FilterRBCombo->addItem(tempString.setNum(rbno));

	// Set date/time limits
	ui.FilterFromDateTimeEdit->setDateTimeRange(earliestRunStart_, latestRunStart_);
	ui.FilterToDateTimeEdit->setDateTimeRange(earliestRunStart_, latestRunStart_);

	// Set run number limits
	ui.FilterFromRunSpin->setRange(firstRunNumber_, lastRunNumber_);
	ui.FilterToRunSpin->setRange(firstRunNumber_, lastRunNumber_);
}

// Store current filter values
void JournalViewer::storeFilters()
{
	// Store current filter values for combos, so we can get them back (if they still exist in the list)
	storedUserFilter_ = ui.FilterUserCombo->currentText();
	storedRBFilter_ = ui.FilterRBCombo->currentText();
	storedFromRunFilter_ = ui.FilterFromRunSpin->value();
	storedToRunFilter_ = ui.FilterToRunSpin->value();
	storedFromDateFilter_ = ui.FilterFromDateTimeEdit->dateTime();
	storedToDateFilter_ = ui.FilterFromDateTimeEdit->dateTime();
}

// Retrieve stored filter values
void JournalViewer::retrieveFilters()
{
	// Set stored run number and date limits (if lock button is checked) or reset to extreme limits otherwise
	ui.FilterFromRunSpin->setValue(ui.FilterFromRunLockButton->isChecked() ? storedFromRunFilter_ : firstRunNumber_);
	ui.FilterToRunSpin->setValue(ui.FilterToRunLockButton->isChecked() ? storedToRunFilter_ : lastRunNumber_);
	ui.FilterFromDateTimeEdit->setDateTime(ui.FilterFromDateTimeLockButton->isChecked() ? storedFromDateFilter_ : earliestRunStart_);
	ui.FilterToDateTimeEdit->setDateTime(ui.FilterToDateTimeLockButton->isChecked() ? storedToDateFilter_ : latestRunStart_);
}

// Reset filters
void JournalViewer::resetFilters()
{
	// Uncheck all lock buttons
	ui.FilterFromDateTimeLockButton->setChecked(false);
	ui.FilterToDateTimeLockButton->setChecked(false);
	ui.FilterFromRunLockButton->setChecked(false);
	ui.FilterToRunLockButton->setChecked(false);

	// Set limits dates and run numbers
	ui.FilterFromDateTimeEdit->setDateTime(earliestRunStart_);
	ui.FilterToDateTimeEdit->setDateTime(latestRunStart_);
	ui.FilterFromRunSpin->setValue(firstRunNumber_);
	ui.FilterToRunSpin->setValue(lastRunNumber_);

	for (auto user : availableUsers_)
	{
		if (user.contains(storedUserFilter_, Qt::CaseInsensitive))
		{	
			ui.FilterUserCombo->setCurrentText(storedUserFilter_);
			break;
		}
	}
	for (auto rb : availableRB_)
	{
		if (rb==storedRBFilter_.toInt())
		{	
			ui.FilterRBCombo->setCurrentText(storedRBFilter_);
			break;
		}
	}
	if (ui.FilterUserCombo->currentText()=="") ui.FilterUserCombo->setCurrentText("<All>");
	if (ui.FilterRBCombo->currentText()=="") ui.FilterRBCombo->setCurrentText("<All>");
}

// Create groups over visible RunData
void JournalViewer::createGroups()
{
	int index, row, nRows = ui.DataTable->rowCount();
	int column = visibleProperties_.nItems();
	TTableWidgetItem* item;
	RunData* runData;
	
	// Create a list of unique Run Titles from the visible table items, and assign group numbers from that
	QList<QString> runTitles;
	for (row = 0; row < nRows; ++row)
	{
		item = (TTableWidgetItem*) ui.DataTable->item(row, 0);
		if (!item)
		{
			msg.print("JournalViewer::createGroups() - Failed to get item from DataTable");
			continue;
		}
		runData = item->source();
		index = runTitles.indexOf(runData->title());
		if (index == -1)
		{
			runTitles << runData->title();
			index = runTitles.count()-1;
		}
		runData->setGroup(index);
	}
	
	// Loop over DataTable items and set new Group numbers
	for (row = 0; row < nRows; ++row)
	{
		item = (TTableWidgetItem*) ui.DataTable->item(row, column);
		if (!item)
		{
			msg.print("JournalViewer::createGroups() - Failed to get item from DataTable");
			continue;
		}
		item->setText(QString::number(item->source()->group()));
	}
}

// Update data table
void JournalViewer::updateDataTable()
{
	// Store existing sort column and direction
	QHeaderView* headerView = ui.DataTable->horizontalHeader();
	RunProperty::Property sortProperty = RunProperty::nProperties;
	Qt::SortOrder sortOrder = Qt::AscendingOrder;
	if (headerView)
	{
		sortOrder = headerView->sortIndicatorOrder();
		QTableWidgetItem* headerItem = ui.DataTable->horizontalHeaderItem(headerView->sortIndicatorSection());
		if (headerItem) sortProperty = RunProperty::property(headerItem->text());
	}

	// Keep the current item selection in the table - construct a new list of the selected items.
	RefList<RunData,int> selectedData = getTableContents(true);

	// Clear table and set headers
	ui.DataTable->clear();
	ui.DataTable->setRowCount(nRunDataVisible_);
	ui.DataTable->setSortingEnabled(false);
	ui.DataTable->setColumnCount(visibleProperties_.nItems()+1);
	ui.DataTable->setSelectionMode(QAbstractItemView::MultiSelection);

	QStringList headers;
	for (RunProperty* rp = visibleProperties_.first(); rp != NULL; rp = rp->next) headers << RunProperty::property(rp->type());
	headers << RunProperty::property(RunProperty::GroupNumber);
	ui.DataTable->setHorizontalHeaderLabels(headers);

	QTableWidgetItem* item;
	int col, row = 0;
	for (RefListItem<RunData,Journal*>* ri = runData_.first(); ri != NULL; ri = ri->next)
	{
		RunData* rd = ri->item;
		// If item is not visible (has been filtered) then continue
		if (!rd->visible()) continue;

		col = 0;
		for (RunProperty* rp = visibleProperties_.first(); rp != NULL; rp = rp->next)
		{
			// Create table entry with proper sorting type...
			switch (rp->type())
			{
				case (RunProperty::RunNumber):
				case (RunProperty::RBNumber):
					item = new TTableWidgetItem(rd, rd->propertyAsString(rp->type()), TTableWidgetItem::IntegerSort);
					break;
				case (RunProperty::Duration):
					item = new TTableWidgetItem(rd, rd->propertyAsString(rp->type()), TTableWidgetItem::DurationSort);
					break;
				case (RunProperty::StartTimeAndDate):
				case (RunProperty::StartTime):
				case (RunProperty::StartDate):
				case (RunProperty::EndTimeAndDate):
				case (RunProperty::EndTime):
				case (RunProperty::EndDate):
					item = new TTableWidgetItem(rd, rd->propertyAsString(rp->type()), TTableWidgetItem::DateSort);
					break;
				default:
					item = new TTableWidgetItem(rd, rd->propertyAsString(rp->type()), TTableWidgetItem::StringSort);
					break;
			}
			
			ui.DataTable->setItem(row, col, item);
			++col;
		}
		
		// Add Group data to last, hidden column
		item = new TTableWidgetItem(rd, rd->propertyAsString(RunProperty::GroupNumber), TTableWidgetItem::IntegerSort);
		ui.DataTable->setItem(row, col, item);

		// Select item?
		if (selectedData.contains(rd)) ui.DataTable->selectRow(row);

		++row;
	}

	// Final changes to table
	ui.DataTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.DataTable->setSortingEnabled(true);
	ui.DataTable->sortItems(0);
	ui.DataTable->setRowCount(row);
	// -- Does previous sort column still exist in new table?
	if (sortProperty == RunProperty::GroupNumber) ui.DataTable->sortByColumn(visibleProperties_.nItems(), sortOrder);
	else for (col = 0; col < visibleProperties_.nItems(); ++col) if (visibleProperties_[col]->type() == sortProperty) ui.DataTable->sortByColumn(col, sortOrder);
	ui.DataTable->resizeColumnsToContents();

	// Show all columns except the Group column
	for (col = 0; col < visibleProperties_.nItems() + 1; ++col)
	{
		if (ui.DataTable->horizontalHeaderItem(col) == NULL) continue;
		ui.DataTable->setColumnHidden(col, ui.DataTable->horizontalHeaderItem(col)->text() == "Group");
	}

	// Create groups if necessary
	if (viewByGroup_) createGroups();

	// Redo highlighting
	updateDataTableHighlighting();

	// Update status bar
	updateStatusBarPermanentWidgets();
}

// Filter run data 
void JournalViewer::filterRunData()
{
	// Grab current search/filter parameters
	QString search = ui.SearchEdit->text();
	JournalViewer::SearchStyle style = (JournalViewer::SearchStyle) ui.SearchStyleCombo->currentIndex();
	bool filterUser = (ui.FilterUserCombo->currentIndex() > 0);
	QString filterUserString = ui.FilterUserCombo->currentText();
	bool filterRB = (ui.FilterRBCombo->currentIndex() > 0);
	QString filterRBString = ui.FilterRBCombo->currentText();
	QDateTime filterFromDateTime = ui.FilterFromDateTimeEdit->dateTime();
	QDateTime filterToDateTime = ui.FilterToDateTimeEdit->dateTime();
	int filterFromRunInt = ui.FilterFromRunSpin->value();
	int filterToRunInt = ui.FilterToRunSpin->value();
	bool dateFilterOnRunning = ui.FilterDateTypeCombo->currentIndex() == 0;

	// Create a regular expression from the 'search' string
	bool hasTitleSearch = (!search.isEmpty());
	QRegExp titleExpr(search);
	if (style == JournalViewer::TextStyle) titleExpr.setPatternSyntax(QRegExp::FixedString);
	else if (style == JournalViewer::WildStyle) titleExpr.setPatternSyntax(QRegExp::WildcardUnix);
	else titleExpr.setPatternSyntax(QRegExp::RegExp);
	titleExpr.setCaseSensitivity(ui.SearchCaseSensitiveCheck->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);

	// Check that RegExp is valid - if not, highlight text in red and exit
	static QPalette palette;
	if (!titleExpr.isValid())
	{
		palette.setColor(QPalette::Text, Qt::red);
		ui.SearchEdit->setPalette(palette);
		return;
	}
	palette.setColor(QPalette::Text, Qt::black);
	ui.SearchEdit->setPalette(palette);

	// Create a search pattern from the 'User' and RBNo strings
	QRegExp userExpr(filterUserString);
	userExpr.setPatternSyntax(QRegExp::FixedString);
	userExpr.setCaseSensitivity(Qt::CaseInsensitive);
	QRegExp rbExpr(filterRBString);
	rbExpr.setPatternSyntax(QRegExp::FixedString);
	rbExpr.setCaseSensitivity(Qt::CaseInsensitive);

	// Loop over loaded RunData and hide those for which the string matches
	nRunDataVisible_ = 0;
	bool visible;
	for (RefListItem<RunData,Journal*>* ri = runData_.first(); ri != NULL; ri = ri->next)
	{
		RunData* rd = ri->item;

		// Simple filtering based on search string in title
		if (hasTitleSearch) visible = (titleExpr.indexIn(rd->title()) != -1);
		else visible = true;

		// Now additional filters (unless no match to simple search string)
		if (visible)
		{
			if (filterUser && (userExpr.indexIn(rd->user()) == -1)) visible = false;
			else if (rd->endDateTime() < filterFromDateTime) visible = false;
			else if (rd->startDateTime() > filterToDateTime) visible = false;
// 			else if (dateFilterOnRunning && (rd->startDateTime() > filterToDateTime)) visible = false;
			else if ((!dateFilterOnRunning) &&  (rd->startDateTime() < filterFromDateTime)) visible = false;
			else if (filterRB && (rbExpr.indexIn(QString::number(rd->rbNumber())) == -1)) visible = false;
			else if (rd->runNumber() < filterFromRunInt) visible = false;
			else if (rd->runNumber() > filterToRunInt) visible = false;
		}

		rd->setVisible(visible);
		if (visible) ++nRunDataVisible_;
	}
}

// Update data table highlighting
void JournalViewer::updateDataTableHighlighting()
{
	// Setup brushes for colouring rows
	QBrush normalRow(Qt::white), highlightedRow(highlightBGColour_);
	
	// Determine highlighting style
	bool highlight = true;

	// Loop over TTableWidgetItems
	TTableWidgetItem* item;
	int nRows = ui.DataTable->rowCount(), nColumns = ui.DataTable->columnCount(), column;
	for (int row = 0; row < nRows; ++row)
	{
		// Set highlight status
		if (viewByGroup_)
		{
			// Get item from table so we can check group number
			item = (TTableWidgetItem*) ui.DataTable->item(row, 0);
			if (!item)
			{
				msg.print("JournalViewer::updateDataTableHighlighting() - Failed to get item from DataTable.");
				continue;
			}
			highlight = item->source()->group()%2 == 0;
		}
		else highlight = !highlight;

		// Loop over columns...
		for (column = 0; column < nColumns; ++column)
		{
			// Get item from table so we can check group number
			item = (TTableWidgetItem*) ui.DataTable->item(row, column);
			if (!item)
			{
				msg.print("JournalViewer::updateDataTableHighlighting() - Failed to get item from DataTable.");
				continue;
			}
			item->setBackground(highlight ? highlightedRow : normalRow);
		}
	}
}

// Plot info from selected run data
void JournalViewer::plotSelectedRunData(RunData::BlockDataSource source, bool forceReload)
{
	// Get current table selection
	RefList<RunData,int> selectedData = getTableContents(true);

	if (selectedData.nItems() == 0) return;

	QProgressDialog progress("Loading data...", "Cancel", 0, selectedData.nItems(), this);
	progress.setWindowModality(Qt::WindowModal);
     
	// Loop over selected RunData
	RunData* rd;
	int nLoaded = 0, n=0;
	bool loaded;
	for (RefListItem<RunData,int>* ri = selectedData.first(); ri != NULL; ri = ri->next)
	{
		rd = ri->item;

		// Check for progress dialog being canceled
		QApplication::processEvents();
		if (progress.wasCanceled()) break;

		if (rd->loadBlockData(source, forceReload)) ++nLoaded;
		
		++n;
		progress.setValue(n);
	}
	progress.setValue(selectedData.nItems());

	// Was the progress dialog canceled?
	if (n != selectedData.nItems()) return;

	// Did we load all (any?) data
	if (nLoaded == 0)
	{
		QMessageBox::warning(this, "Failed to Load Data", QString("Couldn't load any data for the selected runs.\nCheck the path to the data directories in Settings.\n"));
		return;
	}
	else if (nLoaded != selectedData.nItems())
	{
		QMessageBox::StandardButton button = QMessageBox::question(this, "Failed to Load Data", QString("Not all log/Nexus files could be loaded - ") + QString::number(selectedData.nItems() - nLoaded) + " of " + QString::number(selectedData.nItems()) + " failed.\nPlot anyway?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		if (button == QMessageBox::No) return;
	}
	
	// Now create a RunDataWindow to display the data
	RunDataWindow* runDataWin = new RunDataWindow(this, plotFont_);
	runDataWin->setWindowTitle(rd->instrument()->capitalisedName() + " Run Data");
	for (RefListItem<RunData,int>* ri = selectedData.first(); ri != NULL; ri = ri->next) runDataWin->addRunData(ri->item);
	runDataWin->finaliseAndShow();
}

// Create list of RunData from table
RefList<RunData,int> JournalViewer::getTableContents(bool selectionOnly)
{
	RefList<RunData,int> data;

	// Depending on whether we're creating a document for the entire table, or just a selection of it, set up loop differently
	QList<QTableWidgetItem*> selectedItems = ui.DataTable->selectedItems();
	int totalItems = selectionOnly ? selectedItems.count() : ui.DataTable->rowCount();
	RunData* rd = NULL;
	TTableWidgetItem* item = NULL, *nextItem;
	int lastRunNumber = -1;
	for (int index = 0; index < totalItems; ++index)
	{
		// Grab an item in the row (it doesn't matter which) and grab its source RunData pointer
		if (selectionOnly) nextItem = (TTableWidgetItem*) selectedItems[index];
		else nextItem = (TTableWidgetItem*) ui.DataTable->item(index, 0);

		// Check for valid item
		if (nextItem == NULL)
		{
			msg.print("Failed to retrieve TTableWidgetItem for index %i", index);
			continue;
		}

		// Since the list contains each cell in the row, we need to exclude duplicates...
		if (selectionOnly && (nextItem->source() == rd)) continue;
		item = nextItem;

		// Get RunData pointer
		rd = item->source();
		if (rd == NULL)
		{
			msg.print("Strange - this TTableWidgetItem has no runData pointer. Table index = ", index);
			continue;
		}

		// Check this run number with the last found (since multiple tablewidgetitems can have the same RunNumber)
		// We don't know which order they'll be in in the list, though, so must still addUniqiue() to our list.
		if (rd->runNumber() == lastRunNumber) continue;

		data.addUnique(rd);
		lastRunNumber = rd->runNumber();
	}

	return data;
}

// Set visible flags of all current rundata
void JournalViewer::setAllRunDataVisible(bool state)
{
	bool visible;
	for (RefListItem<RunData,Journal*>* ri = runData_.first(); ri != NULL; ri = ri->next)
	{
		RunData* rd = ri->item;
		rd->setVisible(state);
	}
}

// Return column containing specified RunProperty (or -1 if not visible)
int JournalViewer::runPropertyColumn(RunProperty::Property property)
{
	int index = 0;
	for (RunProperty* rp = visibleProperties_.first(); rp != NULL; rp = rp->next)
	{
		if (rp->type() == property) return index;
		++index;
	}

	return -1;
}

// Return RefList of RunData
RefList<RunData,Journal*>& JournalViewer::runData()
{
	return runData_;
}

// Return list of visible RunProperties
List<RunProperty>& JournalViewer::visibleRunProperties()
{
	return visibleProperties_;
}

// Set visible properties from column characters
bool JournalViewer::setVisibleProperties(const char* propertyString)
{
	// Clear current visible properties
	visibleProperties_.clear();

	for (int n=0; n < strlen(propertyString); ++n)
	{
		RunProperty::Property property = RunProperty::property(propertyString[n]);
		if (property != RunProperty::nProperties) visibleProperties_.add()->setType(property);
		else
		{
			msg.print("Error: Unrecognised column character '%c'.\n", propertyString[n]);
			return false;
		}
	}

	return true;
}

// Set current Find, returning number of matches
int JournalViewer::find(QString text, JournalViewer::SearchStyle style, bool caseSensitive)
{
	// Clear old list of row matches
	findMatches_.clear();

	findText_ = text;
	findSearchStyle_ = style;
	findCaseSensitive_ = caseSensitive;
	lastFindMatchIndex_ = -1;

	// If search string is empty, return now.
	if (findText_.isEmpty()) return 0;

	// Get column number containing run title (if visible)
	int titleColumn = runPropertyColumn(RunProperty::Title);
	if (titleColumn == -1) return -1;

	// Create a regular expression from the 'search' string
	QRegExp searchExpr(findText_);
	if (style == JournalViewer::TextStyle) searchExpr.setPatternSyntax(QRegExp::FixedString);
	else if (style == JournalViewer::WildStyle) searchExpr.setPatternSyntax(QRegExp::WildcardUnix);
	else searchExpr.setPatternSyntax(QRegExp::RegExp);
	searchExpr.setCaseSensitivity(ui.SearchCaseSensitiveCheck->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);

	// Loop over current rows (in their display order)
	int nRows = ui.DataTable->rowCount();
	QTableWidgetItem* item;
	for (int n=0; n < nRows; ++n)
	{
		// See if this item matches
		item = ui.DataTable->item(n, titleColumn);

		if (item->text().contains(findText_)) findMatches_.add(n);
	}
}

// Return current number of find matches
int JournalViewer::nFindMatches()
{
	return findMatches_.nItems();
}

// Select (exclusively) next item in the current findMatches_
bool JournalViewer::findNext()
{
	// Deselect everything
	ui.DataTable->clearSelection();

	// Check for presence of any matches
	if (findMatches_.nItems() == 0) return false;

	// Determine where to start looking in the findMatches_ list
	++lastFindMatchIndex_;
	if (lastFindMatchIndex_ >= findMatches_.nItems())
	{
		ui.statusbar->showMessage("Bottom of list reached. Continuing from top...", 2000);
		lastFindMatchIndex_ = 0;
	}

	// Get table row from the findMatches_ array, and select all of its columns
	int row = findMatches_[lastFindMatchIndex_];
	ui.DataTable->selectRow(row);

	return true;
}

// Select (exclusively) previous item in the current findMatches_
bool JournalViewer::findPrevious()
{
	// Deselect everything
	ui.DataTable->clearSelection();

	// Check for presence of any matches
	if (findMatches_.nItems() == 0) return false;

	// Determine where to start looking in the findMatches_ list
	--lastFindMatchIndex_;
	if (lastFindMatchIndex_ < 0)
	{
		ui.statusbar->showMessage("Top of list reached. Continuing from bottom...", 2000);
		lastFindMatchIndex_ = findMatches_.nItems()-1;
	}

	// Get table row from the findMatches_ array, and select all of its columns
	int row = findMatches_[lastFindMatchIndex_];
	ui.DataTable->selectRow(row);

	return true;
}

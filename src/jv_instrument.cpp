/*
	*** JournalViewer Instrument Functions
	*** src/jv_instrument.cpp
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

/*
 * Instruments
 */

// Add new instrument
bool JournalViewer::addInstrument(ISIS::ISISInstrument instrmnt)
{
	QDir localDir;
	QUrl httpDir;
	QString journalIndexFile;

	if (instrmnt == ISIS::LOCAL)
	{
		localDir = userDataDirectory_;
		httpDir = "";
		journalIndexFile = "local_main.xml";
	}
	else
	{
		localDir = journalDirectory_.absoluteFilePath(ISIS::ndxName(instrmnt));
		httpDir = journalUrl_.toString()+ISIS::ndxName(instrmnt)+"/";
		journalIndexFile = "journal_main.xml";
	}

	Instrument* inst = instruments_.add();
	inst->set(instrmnt);
	inst->setIndex(localDir, httpDir, journalIndexFile);

	return true;
}

// Set current Instrument
void JournalViewer::setInstrument(Instrument* inst)
{
	// Refreshing?
	if (refreshing_) return;

	// Is instrument valid?
	if (inst == NULL) return;
	   
	// Is instrument different from current?
	if (inst == currentInstrument_) return;

	refreshing_ = true;

	// Different instrument, so update InstrumentCombo and JournalCombo
	currentInstrument_ = inst;
	currentJournal_ = NULL;
	if (currentInstrument_ != NULL)
	{
		ui.InstrumentCombo->setCurrentIndex(instruments_.indexOf(currentInstrument_));
		msg.print("Current instrument changed to " + currentInstrument_->capitalisedName());
		ui.statusbar->showMessage("Current instrument changed to " + currentInstrument_->capitalisedName(), 3000);
	}
	else ui.InstrumentCombo->setCurrentIndex(-1);

	// Change text on journal / experiment label (full JV only)
#ifndef LITE
	ui.DisplayCycleLabel->setText(currentInstrument_->capitalisedName() == "LOCAL" ? "Data" : "Cycle");
#endif

	// Stop progress hide timer, in case we re-use the progress bar here...
	hideProgressTimer_.stop();

	// Probe journal file
	QByteArray data;
	bool result;
	QDateTime modificationTime;
	JournalViewer::JournalAccess accessType, sourceType;
	accessType = (currentInstrument_->instrument() == ISIS::LOCAL ? JournalViewer::DiskOnlyAccess : journalAccessType_);
	
	if (dataInterface_->mostRecent(accessType, currentInstrument_->indexLocalFile(), currentInstrument_->indexHttpFile(), modificationTime, sourceType))
	{
		if (sourceType == JournalViewer::NoAccess)
		{
			result = false;
			msg.print("Error: No source available for index file for instrument " + currentInstrument_->capitalisedName());
			ui.statusbar->showMessage(QString("No source available for index file for instrument ") + currentInstrument_->capitalisedName(), 3000);
		}
		else if (modificationTime == currentInstrument_->indexModificationTime())
		{
			result = true;
			msg.print("Index file is up to date for instrument " + currentInstrument_->capitalisedName());
			ui.statusbar->showMessage(QString("Index file is up to date for instrument ") + currentInstrument_->capitalisedName(), 3000);
		}
		else if (sourceType == JournalViewer::DiskOnlyAccess)
		{
			// Clear journals from current instrument
			currentInstrument_->clearJournals();

			// Local copy is newer (somehow) so load it in
			result = dataInterface_->readFile(currentInstrument_->indexLocalFile(), data);
			if (result) result = ISIS::parseJournalIndex(inst, data);

			// Did we succeed?
			if (result)
			{
				// Add default 'All' journal option
				currentInstrument_->addJournal("All");

				msg.print("Updated index loaded from disk for instrument " + currentInstrument_->capitalisedName());
				ui.statusbar->showMessage("Updated index loaded from disk for instrument " + currentInstrument_->capitalisedName(), 3000);
			}
			else if (accessType == JournalViewer::DiskAndNetAccess)
			{
				// Failed to load local copy - we can update it from the net, since the access type permits it
				result = dataInterface_->readHttp(currentInstrument_->indexHttpFile(), data);
				if (result) result = result = ISIS::parseJournalIndex(inst, data);

				// Check overall success of reading local copy
				if (result)
				{
					// Add default 'All' journal option
					currentInstrument_->addJournal("All");

					msg.print("Updated index loaded from net (failed to read local copy) for instrument " + currentInstrument_->capitalisedName());
					ui.statusbar->showMessage("Updated index loaded from net (failed to read local copy) for instrument " + currentInstrument_->capitalisedName(), 3000);
					if (dataInterface_->readHttpModificationTime(currentInstrument_->indexHttpFile(), modificationTime)) dataInterface_->saveLocalCopy(data, currentInstrument_->indexLocalFile(), modificationTime);
				}
				else 
				{
					msg.print("Failed to load index for instrument " + currentInstrument_->capitalisedName() + " from both local disk and net.");
					ui.statusbar->showMessage("Failed to load index for instrument " + currentInstrument_->capitalisedName() + " from both local disk and net.", 3000);
				}
			}
		}
		else if (sourceType == JournalViewer::NetOnlyAccess)
		{
			// Clear journals from current instrument
			currentInstrument_->clearJournals();

			// Net copy is newer
			result = dataInterface_->readHttp(currentInstrument_->indexHttpFile(), data);
			if (result) result = result = ISIS::parseJournalIndex(inst, data);

			// Check overall success of reading net copy
			if (result)
			{
				msg.print("Updated index loaded from net for instrument " + currentInstrument_->capitalisedName());
				ui.statusbar->showMessage("Updated index loaded from net for instrument " + currentInstrument_->capitalisedName(), 3000);

				// Add default 'All' journal option
				currentInstrument_->addJournal("All");

				// Save local copy, if access type permits
				if (accessType == JournalViewer::DiskAndNetAccess)
				{
					if (dataInterface_->readHttpModificationTime(currentInstrument_->indexHttpFile(), modificationTime)) dataInterface_->saveLocalCopy(data, currentInstrument_->indexLocalFile(), modificationTime);
				}
			}
			else
			{
				msg.print("Failed to load index instrument " + currentInstrument_->capitalisedName() + " from net.");
				ui.statusbar->showMessage("Failed to load index instrument " + currentInstrument_->capitalisedName() + " from net.", 3000);
			}
		}
		
		// Set modification time
		currentInstrument_->setIndexModificationTime(result ? modificationTime : QDateTime());

		// Check final result
		if (result)
		{
			msg.print("Successfully found/probed journal index for " + ISIS::capitalisedName(currentInstrument_->instrument()));
			ui.statusbar->showMessage("Journal index successfully loaded for instrument " + ISIS::capitalisedName(currentInstrument_->instrument()), 3000);
		}
		else
		{
			msg.print("Failed to parse journal index for " + ISIS::capitalisedName(currentInstrument_->instrument()));
			ui.statusbar->showMessage("Journal index not available for instrument " + ISIS::capitalisedName(currentInstrument_->instrument()), 3000);
		}

	}
	else
	{
		msg.print("Failed to load journal index file for " + ISIS::capitalisedName(currentInstrument_->instrument()));
// 		currentInstrument_ = NULL;
	}

	ui.JournalCombo->setEnabled(currentInstrument_);
	ui.JournalCombo->clear();
	if (currentInstrument_ != NULL)
	{
		for (Journal* jrnl = currentInstrument_->journals(); jrnl != NULL; jrnl = jrnl->next) ui.JournalCombo->addItem(jrnl->name());
	}

	// Start the progress hide timer, if the progressBar is visible
	if (statusHttpProgress_->isVisible()) hideProgressTimer_.start();

	refreshing_ = false;

	// Update current Journal
	setJournal(currentInstrument_->currentCycleJournal());
}

// Return current instrument
Instrument* JournalViewer::currentInstrument()
{
	return currentInstrument_;
}

// Return Instrument requested
Instrument* JournalViewer::instrument(ISIS::ISISInstrument instrmnt)
{
	return instruments_[instrmnt];
}

// Set current Journal (loads data)
void JournalViewer::setJournal(Journal* jrnl)
{
	// Refreshing?
	if (refreshing_) return;

	// Is journal valid?
	if (jrnl == NULL)
	{
		currentJournal_ = NULL;
		runData_.clear();
		updateDataTable();
		return;
	}

	// Is journal different from current?
	if (jrnl == currentJournal_) return;

	// Check journal parent against current instrument
	if (currentInstrument_ != jrnl->parent())
	{
		msg.print("JournalViewer::setJournal() Error - mismatch between current instrument and current journal parent.");
		return;
	}

	setJournalControlsEnabled(false);
	refreshing_ = true;

	// Different journal selected, so update JournalCombo and load new data
	currentJournal_ = jrnl;
	ui.JournalCombo->setCurrentIndex(currentInstrument_->journalIndex(currentJournal_));

	// Stop progress hide timer, in case we re-use the progress bar here...
	hideProgressTimer_.stop();

	// Check for 'All' being selected
	runData_.clear();
	updateDataTable();
	if (currentJournal_->name() == "All")
	{
		for (Journal* journal = currentInstrument_->journals(); journal != NULL; journal = journal->next)
		{
			// Skip 'All' journal entry
			if (journal->name() == "All") continue;
			
			loadJournalData(journal, false);
		}
	}
	else
	{
		if (loadJournalData(currentJournal_, false)) ui.statusbar->showMessage(QString("Journal '") + currentJournal_->name() + QString("' loaded for instrument ") + currentInstrument_->capitalisedName(), 3000);
		else
		{
			msg.print("Failed to load journal '" + currentJournal_->name() + "' for instrument " + currentInstrument_->capitalisedName());
			ui.statusbar->showMessage("Failed to load journal '" + currentJournal_->name() + "' for instrument " + currentInstrument_->capitalisedName(), 3000);
		}
	}

	// New journal data has been loaded (hopefully), so must update limits and unique lists
	storeFilters();
	findFilterLimits();
	resetFilters();
	filterRunData();
	updateDataTable();
	
	hideProgressTimer_.start();

	refreshing_ = false;
	setJournalControlsEnabled(true);
}

// Load specified journal data
bool JournalViewer::loadJournalData(Journal* jrnl, bool updateOnly)
{
	dataInterface_->setLabelText(jrnl->fileName());

	QByteArray data;
	bool result = false;
	QDateTime modificationTime;

	// Stop progress hide timer, in case we re-use the progress bar here...
	hideProgressTimer_.stop();

	JournalViewer::JournalAccess accessType, sourceType;
	accessType = (currentInstrument_->instrument() == ISIS::LOCAL ? JournalViewer::DiskOnlyAccess : journalAccessType_);


	if (dataInterface_->mostRecent(accessType, jrnl->filePath(), jrnl->httpPath(), modificationTime, sourceType))
	{
		if (sourceType == JournalViewer::NoAccess)
		{
			result = false;
			msg.print("No source available for Journal '" + jrnl->name() + "' for instrument " + currentInstrument_->capitalisedName());
			ui.statusbar->showMessage("No source available for Journal '" + jrnl->name() + "' for instrument " + currentInstrument_->capitalisedName(), 3000);
		}
		else if (modificationTime == jrnl->modificationTime())
		{
			result = true;
			msg.print("Current data for Journal '" + jrnl->name() + "' for instrument " + currentInstrument_->capitalisedName() + " is up to date");
			ui.statusbar->showMessage("Current data for Journal '" + jrnl->name() + "' for instrument " + currentInstrument_->capitalisedName() + " is up to date", 3000);
		}
		else if (sourceType == JournalViewer::DiskOnlyAccess)
		{
			// Local copy is newer (somehow) so load it in
			result = DataInterface::readFile(jrnl->filePath(), data);
			if (result) result = ISIS::parseJournalData(jrnl, data, updateOnly, forceISOEncoding_);

			// Did we succeed?
			if (result)
			{
				msg.print("Updated Journal '" + jrnl->name() + "' loaded from disk for instrument " + currentInstrument_->capitalisedName());
				ui.statusbar->showMessage("Updated Journal '" + jrnl->name() + "' loaded from disk for instrument " + currentInstrument_->capitalisedName(), 3000);
			}
			else if (accessType == JournalViewer::DiskAndNetAccess)
			{
				// Failed to load local copy - we can update it from the net, since the access type permits it
				result = dataInterface_->readHttp(jrnl->httpPath(), data);
				if (result) result = ISIS::parseJournalData(jrnl, data, updateOnly, forceISOEncoding_);

				// Check overall success of reading local copy
				if (result)
				{
					msg.print("Updated Journal '" + jrnl->name() + "' loaded from net (failed to read local copy) for instrument " + currentInstrument_->capitalisedName());
					ui.statusbar->showMessage("Updated Journal '" + jrnl->name() + "' loaded from net (failed to read local copy) for instrument " + currentInstrument_->capitalisedName(), 3000);
					if (dataInterface_->readHttpModificationTime(jrnl->httpPath(), modificationTime)) dataInterface_->saveLocalCopy(data, jrnl->filePath(), modificationTime);
				}
				else
				{
					msg.print("Failed to load Journal '" + jrnl->name() + "' instrument " + currentInstrument_->capitalisedName() + " from both local disk and net.");
					ui.statusbar->showMessage("Failed to load Journal '" + jrnl->name() + "' instrument " + currentInstrument_->capitalisedName() + " from both local disk and net.", 3000);
				}
			}
		}
		else if (sourceType == JournalViewer::NetOnlyAccess)
		{
			// Net copy is newer
			result = dataInterface_->readHttp(jrnl->httpPath(), data);
			if (result) result = ISIS::parseJournalData(jrnl, data, updateOnly, forceISOEncoding_);

			// Check overall success of reading net copy
			if (result)
			{
				msg.print("Updated Journal '" + jrnl->name() + "' loaded from net for instrument " + currentInstrument_->capitalisedName());
				ui.statusbar->showMessage("Updated Journal '" + jrnl->name() + "' loaded from net for instrument " + currentInstrument_->capitalisedName(), 3000);

				// Save local copy, if access type permits
				if (accessType == JournalViewer::DiskAndNetAccess)
				{
					if (dataInterface_->readHttpModificationTime(jrnl->httpPath(), modificationTime)) dataInterface_->saveLocalCopy(data, jrnl->filePath(), modificationTime);
				}
			}
			else
			{
				msg.print("Failed to load Journal '" + jrnl->name() + "' instrument " + currentInstrument_->capitalisedName() + " from net.");
				ui.statusbar->showMessage("Failed to load Journal '" + jrnl->name() + "' instrument " + currentInstrument_->capitalisedName() + " from net.", 3000);
			}
		}
		
		// Set modification time
		jrnl->setModificationTime(result ? modificationTime : QDateTime());

		// Success?
		// Add data to run list, if we were successful
		if (result) for (RunData* rd = jrnl->runData().first(); rd != NULL; rd = rd->next) runData_.add(rd, jrnl);
		else msg.print(("Failed to load journal data '") + jrnl->fileName() + "' for instrument " + jrnl->parent()->capitalisedName());
	}

	// Start the progress hide timer, if the progressBar is now visible
	if (statusHttpProgress_->isVisible()) hideProgressTimer_.start();

	return result;
}

// Update current journal data
bool JournalViewer::updateJournalData()
{
	// Refreshing?
	if (refreshing_) return false;

	// Is current journal valid?
	if (currentJournal_ == NULL) return false;

	// Check journal parent against current instrument
	if (currentInstrument_ != currentJournal_->parent())
	{
		msg.print("JournalViewer::setJournal() Error - mismatch between current instrument and current journal parent.");
		return false;
	}

	refreshing_ = true;

	// Stop progress hide timer, in case we re-use the progress bar here...
	hideProgressTimer_.stop();
	
	// Disable datatable and reload journal button
	ui.DataTable->setEnabled(false);
	ui.ReloadJournalButton->setEnabled(false);
	runData_.clear();

	// Check for 'All' being selected
	QDateTime modificationTime;
	JournalViewer::JournalAccess sourceType;
	if (currentJournal_->name() == "All")
	{
		for (Journal* journal = currentInstrument_->journals(); journal != NULL; journal = journal->next)
		{
			// Skip 'All' journal entry
			if (journal->name() == "All") continue;

			if (!loadJournalData(journal, true)) msg.print("Failed to check for most recent version of journal.");
		}
	}
	else
	{
		if (!loadJournalData(currentJournal_, true))
		{
			QMessageBox::warning(this, "Update Failed", QString("Failed to update journal '") + currentJournal_->name() + QString("' for instrument ") + currentInstrument_->capitalisedName());
			msg.print("Failed to update journal '" + currentJournal_->name() + "' for instrument " + currentInstrument_->capitalisedName());
			ui.statusbar->showMessage("Failed to update journal '" + currentJournal_->name() + "' for instrument " + currentInstrument_->capitalisedName(), 3000);
		}
	}

	// New journal data has been loaded (hopefully), so must update limits and unique lists
	storeFilters();
	findFilterLimits();
	retrieveFilters();
	filterRunData();
	updateDataTable();
	ui.DataTable->setEnabled(true);
	ui.ReloadJournalButton->setEnabled(true);

	// Restart reload timer (if active)
	if (autoReload_) autoReloadTimer_.start();
	
	// Start the progress hide timer, if the progressBar is now visible
	if (statusHttpProgress_->isVisible()) hideProgressTimer_.start();

	refreshing_ = false;

	return true;
}

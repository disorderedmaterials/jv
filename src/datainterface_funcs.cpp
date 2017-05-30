/*
	*** Data Interface Functions
	*** src/datainterface_funcs.cpp
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

#include "datainterface.h"
#include "logwindow.h"
#include "instrument.h"
#include "messenger.hui"
#include "version.h"
#include "treplytimeout.hui"
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkProxyFactory>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QSettings>
#include <QMessageBox>

// Local Dummy Values
QByteArray dummyByteArray;
QDateTime dummyDateTime;

// Constructor
DataInterface::DataInterface(QProgressBar* progressBar, QLabel* progressLabel) : QObject()
{
	dataThread_ = NULL;
	progressBar_ = progressBar;
	progressLabel_ = progressLabel;
// 	labelText_ = labelText;
	if (progressBar_)
	{
		progressBar_->setRange(0, 1);
		progressBar_->setValue(0);
	}
	if (progressLabel_) progressLabel_->setText(labelText_);
}

// Return progress bar
QProgressBar* DataInterface::progressBar()
{
	return progressBar_;
}

// Set label text
void DataInterface::setLabelText(QString text)
{
	labelText_ = text;
}

// Load file data into specified QByteArray
bool DataInterface::readFile(QString fileName, QByteArray& data)
{
	// First, check to see if file exists...
	if (!QFile::exists(fileName))
	{
		msg.print("DataInterface::readFile() - File '" + fileName + "' doesn't exist.");
		return false;
	}

	// It does exist - can we open it?
	QFile localFile;
	localFile.setFileName(fileName);
	localFile.open(QIODevice::ReadOnly);
	if (!localFile.isReadable())
	{
		msg.print("DataInterface::readFile() - Can't open '" + fileName + "' for reading (permissions?).");
		return false;
	}

	// It exists, and we can open it, so read in the entire thing
	data = localFile.readAll();
	localFile.close();
	msg.print("DataInterface::readFile() - Successfully read file '" + fileName + "'");

	return true;
}

// Load data from net into QByteArray
bool DataInterface::readHttp(QUrl location, QByteArray& data)
{
	// Set target label
	if (progressLabel_ && progressBar_)
	{
		progressLabel_->setVisible(true);
		progressLabel_->setText(location.toString());
		progressBar_->setVisible(true);
	}

	// Prepare a LoadDataThread so we retrieve data in the background whilst displaying progress in a nice way
	dataThread_ = new LoadDataThread(this, location, data);

	// Connect the thread's 'finished' signal to its own 'deleteLater' slot so it is nicely cleaned up
	QObject::connect(dataThread_, SIGNAL(finished()), dataThread_, SLOT(deleteLater()));

	// Construct an event loop, connecting the 'done' signal of the thread to the 'quit' slot
	QEventLoop loop;
	QObject::connect(dataThread_, SIGNAL(done()), &loop, SLOT(quit()));

	// Connect the thread's 'completed' and 'failed' signals to our own slots, and connect our cancel signal to the thread's
	QObject::connect(dataThread_, SIGNAL(completed()), this, SLOT(threadSuccess()));
	QObject::connect(dataThread_, SIGNAL(failed()), this, SLOT(threadFailed()));
	QObject::connect(this, SIGNAL(cancelRequest()), dataThread_, SLOT(cancel()));

	// Start thread, and show the window
	httpSuccess_ = false;
	dataThread_->start();
	loop.exec();

	return httpSuccess_;
}

// Get modification time from HTTP header
bool DataInterface::readHttpModificationTime(QUrl location, QDateTime& httpModificationTime)
{
	// Set target label
	if (progressLabel_ && progressBar_)
	{
		progressLabel_->setVisible(true);
		progressLabel_->setText("MODTIME:"+location.toString());
		progressBar_->setVisible(true);
	}
	
	// Prepare a LoadDataThread so we retrieve data in the background whilst displaying progress in a nice way
	dataThread_ = new LoadDataThread(this, location, httpModificationTime);

	// Connect the thread's 'finished' signal to its own 'deleteLater' slot so it is nicely cleaned up
	QObject::connect(dataThread_, SIGNAL(finished()), dataThread_, SLOT(deleteLater()));

	// Construct an event loop, connecting the custom 'done' signal of the thread to the 'quit' slot
	QEventLoop loop;
	QObject::connect(dataThread_, SIGNAL(done()), &loop, SLOT(quit()));

	// Connect the thread's 'completed' and 'failed' signals to our own slots, and connect our cancel signal to the thread's
	QObject::connect(dataThread_, SIGNAL(completed()), this, SLOT(threadSuccess()));
	QObject::connect(dataThread_, SIGNAL(failed()), this, SLOT(threadFailed()));
	QObject::connect(this, SIGNAL(cancelRequest()), dataThread_, SLOT(cancel()));

	// Start thread, and enter our event loop
	httpSuccess_ = false;
	dataThread_->start();
	loop.exec();

	return httpSuccess_;
}

// Get modification time of most recent version of specified source
bool DataInterface::mostRecent(JournalViewer::JournalAccess accessType, QString localFile, QUrl httpFile, QDateTime& modTime, JournalViewer::JournalAccess& sourceType)
{
	sourceType = JournalViewer::NoAccess;
	bool result = false;
	modTime = QDateTime();

	// Probe availability of sources
	bool diskAvailable = false, httpAvailable = false;
	QDateTime diskModificationTime, httpModificationTime;
	
	if (accessType != JournalViewer::NetOnlyAccess)
	{
		// Was a valid localFile given?
		// If so, check it's existence, and get modification time
		if (localFile.isEmpty()) diskAvailable = false;
		else if (QFile::exists(localFile))
		{
			// Do we have a stored modification time?
			QSettings settings;
			if (settings.contains(QString("modtime/")+localFile))
			{
				diskModificationTime = settings.value(QString("modtime/")+localFile).toDateTime();
				diskAvailable = true;
			}
			else msg.print("File '%s' found, but no modtime available.\n", qPrintable(localFile));
		}
	}

	if (accessType != JournalViewer::DiskOnlyAccess)
	{
		// Was a valid httpFile given?
		// If so, get HTTP source modification time
		if (httpFile.isEmpty()) httpAvailable = false;
		else if (readHttpModificationTime(httpFile, httpModificationTime)) httpAvailable = true;
	}

	// Return suitable result...
	switch (accessType)
	{
		case (JournalViewer::DiskOnlyAccess):
			if (diskAvailable)
			{
				modTime = diskModificationTime;
				sourceType = JournalViewer::DiskOnlyAccess;
				return true;
			}
			else msg.print("Warning - No local copy of '%s' available, and access type is local only.\n", qPrintable(localFile));
			break;
		case (JournalViewer::NetOnlyAccess):
			if (httpAvailable)
			{
				modTime = httpModificationTime;
				sourceType = JournalViewer::NetOnlyAccess;
				return true;
			}
			else msg.print("Warning - No net copy of '%s' available, and access type is net only.\n", qPrintable(httpFile.toString()));
			break;
		case (JournalViewer::DiskAndNetAccess):
			// What have we got?
			if (diskAvailable && httpAvailable)
			{
				// Check modification time of local copy...
				if (diskModificationTime == httpModificationTime) modTime = diskModificationTime;
				else modTime = httpModificationTime;
				sourceType = (diskModificationTime == httpModificationTime ? JournalViewer::DiskOnlyAccess : JournalViewer::NetOnlyAccess);
				return true;
			}
			else if (diskAvailable)
			{
				modTime = diskModificationTime;
				sourceType = JournalViewer::DiskOnlyAccess;
				return true;
			}
			else if (httpAvailable)
			{
				modTime = httpModificationTime;
				sourceType = JournalViewer::NetOnlyAccess;
				return true;
			}
			else msg.print("DataInterface::mostRecent() - No local or http sources available for file '" + localFile + "'.");
	}
	return false;
}

// Save local copy of specified data
bool DataInterface::saveLocalCopy(QByteArray& data, QString localFile, QDateTime modificationTime)
{
	msg.print("Saving local copy of data...");

	// Extract directory from local filename
	QDir dir = QFileInfo(localFile).absoluteDir();

	msg.print("Checking to see if local directory '" + dir.path() + "' exists...");
	if (!dir.exists())
	{
		if (!dir.mkpath(dir.path()))
		{
			QMessageBox::StandardButton button = QMessageBox::warning(NULL, "Error", QString("Failed to create the directory '") + dir.path() + "' .\nCheck the path and your permissions to write to it.", QMessageBox::Cancel | QMessageBox::Ok, QMessageBox::Ok);
			if (button == QMessageBox::Cancel) return false;
		}
	}

	// Write new file?
	if (dir.exists())
	{
		msg.print("Writing local data file '" + localFile + "'");

		// Save modification time to settings
		QSettings settings;
		settings.setValue(QString("modtime/")+localFile, modificationTime);
		
		// Save data file
		QFile file;
		file.setFileName(localFile);
		file.open(QIODevice::WriteOnly);
		if (file.isWritable())
		{
			file.write(data);
			file.close();
			msg.print("Successfully wrote file '" + localFile + "'");
		}
		else
		{
			msg.print("Error: Failed to write file '" + localFile + "'");
			return false;
		}
	}

	return true;
}

// Cancel current retrieval
void DataInterface::cancel()
{
	emit(cancelRequest());
}

void DataInterface::threadSuccess()
{
	httpSuccess_ = true;
}

void DataInterface::threadFailed()
{
	httpSuccess_ = false;
}

// // Cancel button pressed
// void DataInterface::on_CancelButton_clicked(bool checked)
// {
// // 	ui.CancelButton->setEnabled(false);
// 	printf("Cancel button pressed.\n");
// 	if (dataThread_ != NULL)
// 	{
// 		printf("Emitting cancelled signal.\n");
// 		emit(cancelled());
// 		printf("waiting for thread...\n");
// 		dataThread_->wait();
// 		printf("Finished waiting for thread.\n");
// 	}
// }

/*
 * LoadDataThread
 */

// Constructor (retrieve data)
LoadDataThread::LoadDataThread(DataInterface* parent, QUrl location, QByteArray& array) : byteArray_(array), dateTime_(dummyDateTime)
{
	error_ = false;
	parent_ = parent;
	location_ = location;
	headerOnly_ = false;
	networkReply_= NULL;
}

// Constructor (retrieve header)
LoadDataThread::LoadDataThread(DataInterface* parent, QUrl location, QDateTime& dateTime) : byteArray_(dummyByteArray), dateTime_(dateTime)
{
	error_ = false;
	parent_ = parent;
	location_ = location;
	headerOnly_ = true;
	networkReply_ = NULL;
}

// Execute thread
void LoadDataThread::run()
{
	// Create and set-up our QNetworkManager
	QNetworkAccessManager networkManager;
	QNetworkProxyFactory::setUseSystemConfiguration(true);

	error_ = false;

	msg.print("Retrieving data from location " + location_.toString());

	// Make the request
	QEventLoop loop;
	if (headerOnly_) networkReply_ = networkManager.head(QNetworkRequest(location_));
	else networkReply_ = networkManager.get(QNetworkRequest(location_));

	// Create reply timeout object (30 seconds)
	TReplyTimeout replyTimeout(networkReply_, 30000);

	// Connect the downloadProgress signal to our own handler (which will update the relevant ProgressBar in the parent_)
	if (parent_ && parent_->progressBar())
	{
		connect(networkReply_, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadUpdate(qint64,qint64)));
		connect(this, SIGNAL(setDownloadSize(int)), parent_->progressBar(), SLOT(setMaximum(int)));
		connect(this, SIGNAL(setDownloadProgress(int)), parent_->progressBar(), SLOT(setValue(int)));
	}
	if (parent_) connect(parent_, SIGNAL(cancelRequest()), this, SLOT(cancel()));
	connect(networkReply_, SIGNAL(finished()), &loop, SLOT(quit()));

	// Execute the blocking event loop
	loop.exec();
	
	// Check for valid source
	if (networkReply_->error() != QNetworkReply::NoError)
	{
		msg.print("LoadDataThread::run() - NetworkReply returned an error.");
		error_ = true;
		emit failed();
		emit done();
		return;
	}

	if (headerOnly_) dateTime_ = networkReply_->header(QNetworkRequest::LastModifiedHeader).toDateTime();
	else byteArray_ = networkReply_->readAll();
	
	networkReply_->close();
	
	networkReply_->deleteLater();

	// All finished - signal the parent dialog that we're done.
	if (error_) emit failed();
	else emit completed();
	emit done();
}

// Cancel current download (slot)
void LoadDataThread::cancel()
{
	msg.print("LoadDataThread::cancel() - Thread received cancel signal.");
	if (networkReply_ != NULL)
	{
		msg.print("LoadDataThread::cancel() - Aborting networkReply...");
		networkReply_->abort();
		msg.print("LoadDataThread::cancel() - NetworkReply aborted.");
	}
// 	emit cancelled();
// 	emit failed();
// 	emit finished();
}

// Journal index download updated
void LoadDataThread::downloadUpdate(qint64 bytesRecvd, qint64 bytesTotal)
{
	emit(setDownloadSize(bytesTotal));
	emit(setDownloadProgress(bytesRecvd));
}

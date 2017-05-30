/*
	*** Data Interface / Dialog
	*** src/datainterface.h
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

#ifndef JOURNALVIEWER_DATAINTERFACE_H
#define JOURNALVIEWER_DATAINTERFACE_H

#include "ui_datainterface.h"
#include "list.h"
#include "rundata.h"
#include "instrument.h"
#include "jv.h"
#include <QThread>
#include <QUrl>
#include <QXmlStreamReader>
#include <QMessageBox>

// Forward Declarations
class QNetworkAccessManager;
class QNetworkReply;
class LoadDataThread;

class DataInterface : public QObject
{
	// All Qt declarations must include this macro
	Q_OBJECT


	/*
	// Window Functions
	*/
	public:
	// Constructor / Destructor
	DataInterface(QProgressBar* progressBar, QLabel* progressLabel);
	
	private:
	// Thread for data loading
	LoadDataThread* dataThread_;
	// Target progressBar, if any
	QProgressBar* progressBar_;
	// Target label for progressBar, if any
	QLabel* progressLabel_;
	// General text to prepend to label
	QString labelText_;
	// Success of last threaded http retrieval
	bool httpSuccess_;

	public:
	// Return progress bar
	QProgressBar* progressBar();
	// Set label text
	void setLabelText(QString text);
	// Load file data into specified QByteArray
	static bool readFile(QString fileName, QByteArray& data);
	// Load data from net into QByteArray
	bool readHttp(QUrl location, QByteArray& data);
	// Get modification time from HTTP header
	bool readHttpModificationTime(QUrl location, QDateTime& httpModificationTime);
	// Get modification time of most recent version of specified source
	bool mostRecent(JournalViewer::JournalAccess accessType, QString localFile, QUrl httpFile, QDateTime& modTime, JournalViewer::JournalAccess& sourceType);
	// Save local copy of specified data
	static bool saveLocalCopy(QByteArray& data, QString localFile, QDateTime modificationTime);

	public slots:
	// Cancel current retrieval
	void cancel();

	signals:
	// Cancel current retrieval thread
	void cancelRequest();

	private slots:
	void threadSuccess();
	void threadFailed();
};

// Load data thread
class LoadDataThread : public QThread
{
	Q_OBJECT

	public:
	// Constructor (retrieve data)
	LoadDataThread(DataInterface* parent, QUrl location, QByteArray& array);
	// Constructor (retrieve header)
	LoadDataThread(DataInterface* parent, QUrl location, QDateTime& dateTime);

	private:
	// Whether to retrieve full data or just header
	bool headerOnly_;
	// Flag containing result of last read attempt
	bool error_;
	// Target file URL
	QUrl location_;
	// QByteArray to put data in to
	QByteArray& byteArray_;
	// QDateTime to put modification time in to
	QDateTime& dateTime_;
	// Dialog parent
	DataInterface* parent_;
	// QNetworkReply object pointer
	QNetworkReply* networkReply_;
	
	public:
	// Execute thread
	void run();

	public slots:
	// Cancel current download
	void cancel();
	// Update progress bar (http download)
	void downloadUpdate(qint64 bytesRecvd, qint64 bytesTotal);

	signals:
	// Signal used to cancel running thread
	void cancelRequest();
	// Local signal, controlling event loop in run()
	void done();
	// Signal used to signal parent dialog that all went well
	void completed();
	// Signal used to signal parent dialog that errors occurred
	void failed();
	// Set maximum size on associated progress bar
	void setDownloadSize(int);
	// Set download progress
	void setDownloadProgress(int);
};

#endif

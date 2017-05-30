/*
	*** Document Commands
	*** src/documentcommands.h
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

#ifndef JOURNALVIEWER_DOCUMENTCOMMANDS_H
#define JOURNALVIEWER_DOCUMENTCOMMANDS_H

#include <QString>
#include <QColor>
#include "list.h"

// Forward Declarations
class QPainter;
class DocumentColumn;
class Document;
class PlotWidget;

// Basic Document Command
class DocumentCommand : public ListItem<DocumentCommand>
{
	public:
	// Constructor
	DocumentCommand();
	// Destructor
	virtual ~DocumentCommand();

	// Command Type
	enum Command { AddColumnCommand, BackgroundColourCommand, ClearColumnsCommand, EndRowCommand, FontCommand, ForegroundColourCommand, PlotCommand, TextCommand, nCommands };
	
	protected:
	// Command
	Command command_;
	// Column span of content (if required)
	int span_;

	public:
	// Return command
	Command command();
	// Return span
	int span();


	/*
	 * Virtuals
	 */
	public:
	// Execute command on target Document (pure)
	virtual void execute(Document& target) = 0;
	// Return contents rectangle for display
	virtual QRect contentsRect(QPainter& painter, QRect initialRect, bool wordWrap = false);
	// Draw content within specified bounding rectangle
	virtual void drawContent(QPainter& painter, QRect boundingRect);
};

// Add Column Command
class AddColumnCommand : public DocumentCommand
{
	public:
	// Constructor
	AddColumnCommand(bool fitContents);
	// Destructor
	~AddColumnCommand();

	private:
	// Related column
	DocumentColumn* column_;
	
	public:
	// Return column
	DocumentColumn* column();
	// Execute command on target Document
	void execute(Document& target);
};

// Background Colour Command
class BackgroundColourCommand : public DocumentCommand
{
	public:
	// Constructor
	BackgroundColourCommand(QColor& colour);
	// Destructor
	~BackgroundColourCommand();

	private:
	// Stored colour
	QColor colour_;
	
	public:
	// Execute command on target Document
	void execute(Document& target);
};

// Clear Columns Command
class ClearColumnsCommand : public DocumentCommand
{
	public:
	// Constructor
	ClearColumnsCommand();
	// Destructor
	~ClearColumnsCommand();

	public:
	// Execute command on target Document
	void execute(Document& target);
};

// End Row Command
class EndRowCommand : public DocumentCommand
{
	public:
	// Constructor
	EndRowCommand();
	// Destructor
	~EndRowCommand();

	public:
	// Execute command on target Document
	void execute(Document& target);
};

// Font Command
class FontCommand : public DocumentCommand
{
	public:
	// Constructor
	FontCommand(QFont& font);
	// Destructor
	~FontCommand();

	private:
	// Stored font
	QFont font_;
	
	public:
	// Execute command on target Document
	void execute(Document& target);
};

// Foreground Colour Command
class ForegroundColourCommand : public DocumentCommand
{
	public:
	// Constructor
	ForegroundColourCommand(QColor& colour);
	// Destructor
	~ForegroundColourCommand();

	private:
	// Stored colour
	QColor colour_;
	
	public:
	// Execute command on target Document
	void execute(Document& target);
};

// Text Command
class TextCommand : public DocumentCommand
{
	public:
	// Constructor
	TextCommand(QString& text, int span = 1, Qt::AlignmentFlag alignment = Qt::AlignLeft, bool bold = false, bool italic = false);
	// Destructor
	~TextCommand();

	private:
	// Stored text
	QString text_;
	// Alignment for text
	Qt::AlignmentFlag alignment_;
	// Whether text is bold
	bool bold_;
	// Whether text is italic
	bool italic_;
	
	public:
	// Execute command on target Document
	void execute(Document& target);
	// Return contents rectangle for display
	QRect contentsRect(QPainter& painter, QRect initialRect, bool wordWrap = false);
	// Draw content within specified bounding rectangle
	void drawContent(QPainter& painter, QRect boundingRect);
};

// Plot Command
class PlotCommand : public DocumentCommand
{
	public:
	// Constructor
	PlotCommand(PlotWidget& source, int span, double fractionalHeight);
	// Destructor
	~PlotCommand();

	private:
	// Stored PlotWidget reference
	PlotWidget& plot_;
	// Height of plot, in fraction of device height
	double fractionalHeight_;
	
	public:
	// Execute command on target Document
	void execute(Document& target);
	// Return contents rectangle for display
	QRect contentsRect(QPainter& painter, QRect initialRect, bool wordWrap = false);
	// Draw content within specified bounding rectangle
	void drawContent(QPainter& painter, QRect boundingRect);
};


#endif

/*
	*** PlotWidget Functions
	*** src/plotwidget_funcs.cpp
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

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMenu>
#include <QPushButton>
#include <QLabel>
#include <QApplication>
#include <QClipboard>
#include "plotwidget.hui"
#include "messenger.hui"
#include <math.h>

// Static Singletons
QColor PlotWidget::lineColours_[PlotWidget::nLineColours];

// Constructor
PlotWidget::PlotWidget(QWidget* parent) : QWidget(parent)
{
	// Context Menu / Extra Widgets
	contextMenu_ = new QMenu(this);
	connect(contextMenu_->addAction("Show &All"), SIGNAL(triggered(bool)), this, SLOT(contextMenuShowAllClicked(bool)));
	connect(contextMenu_->addAction("Copy to Clipboard"), SIGNAL(triggered(bool)), this, SLOT(contextMenuCopyToClipboardClicked(bool)));
	coordinatesLabel_ = NULL;

	// Set plot defaults
	spacing_ = 4;
	backgroundColour_ = Qt::white;
	foregroundColour_ = Qt::black;
	font_.setPointSize(8);
	xMin_ = -10.0;
	xMax_ = 10.0;
	yMin_ = -10.0;
	yMax_ = 10.0;
	xScale_ = 1.0;
	yScale_ = 1.0;
	xLogarithmic_ = false;
	yLogarithmic_ = false;
	lastXScale_ = -1.0;
	lastYScale_ = -1.0;
	xAxisTickStart_ = -10.0;
	yAxisTickStart_ = -10.0;
	xAxisTickDelta_ = 2.0;
	yAxisTickDelta_ = 2.0;
	xLabelFormat_ = "%6.3f";
	yLabelFormat_ = "%6.3f";
	showLegend_ = true;
	verticalSpacing_ = 1.0;
	xMinLimit_ = 0.0;
	xMaxLimit_ = 0.0;
	yMinLimit_ = 0.0;
	yMaxLimit_ = 0.0;
	limitXMin_ = false;
	limitXMax_ = false;
	limitYMin_ = false;
	limitYMax_ = false;
	absoluteTime_ = true;
	absoluteTimeOrigin_ = QDateTime();
	nDataSetGroupsVisible_ = 0;
	nDataSetBlocksVisible_ = 0;
	showBeginEnd_ = false;
	
	// Source data
	onChangeData_ = false;

	// Style
	// -- Pre-defined colours
	lineColours_[PlotWidget::BlackColour] = Qt::black;
	lineColours_[PlotWidget::RedColour] = Qt::red;
	lineColours_[PlotWidget::BlueColour] = Qt::blue;
	lineColours_[PlotWidget::GreenColour].setRgb(32,114,53);
	lineColours_[PlotWidget::PurpleColour].setRgb(126,12,179);
	lineColours_[PlotWidget::OrangeColour].setRgb(224,133,25);
	lineColours_[PlotWidget::MetallicBlueColour].setRgb(61,129,160);
	lineColours_[PlotWidget::MintColour].setRgb(119,189,137);
	lineColours_[PlotWidget::GreyColour].setRgb(180,180,180);
	singleProperty_ = false;
	shadeBackground_ = false;
	shadedBackgroundMinimumY_ = 0.0;
	shadedBackgroundMaximumY_ = 1.0;
	shadedBackgroundMinimumColour_ = Qt::red;
	shadedBackgroundMaximumColour_ = lineColours_[PlotWidget::GreenColour];

	// Titles
	mainTitle_ = "NewGraph";
	xAxisTitle_ = "Time";
	yAxisTitle_ = "YAxis";
}

// Destructor
PlotWidget::~PlotWidget()
{
}

/*
 * Widgets / Slots / Reimplementations
 */

// Mouse press event
void PlotWidget::mousePressEvent(QMouseEvent* event)
{
	buttons_ = event->buttons();
	clickedWidgetPosition_ = event->pos();

	if (buttons_&Qt::LeftButton)
	{
		// Where is mouse cursor (use original click position to check)?
		if (graphArea_.contains(clickedWidgetPosition_)) clickedDataPosition_ = widgetToGraph(event->pos());
		else clickedWidgetPosition_ = QPoint();
	}
	else if (buttons_&Qt::RightButton)
	{
		// Raise context menu...
		contextMenu_->exec(event->globalPos());
	}
	else if (buttons_&Qt::MiddleButton)
	{
		// Where is mouse cursor (use original click position to check)?
		if (graphArea_.contains(clickedWidgetPosition_)) clickedDataPosition_ = widgetToGraph(event->pos());
		else clickedWidgetPosition_ = QPoint();
	}
	
	//printf("Clicked graph coords = %f,%f\n", clickedDataPosition_.x(), clickedDataPosition_.y());
}

// Mouse release event
void PlotWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (buttons_&Qt::LeftButton)
	{
		// Set current mouse position (in graph coordinates)
		currentDataPosition_ = widgetToGraph(event->pos());
		//printf("Unclicked graph coords = %f,%f\n", currentDataPosition_.x(), currentDataPosition_.y());

		// Where is mouse cursor (use original click position to check)?
		if (graphArea_.contains(clickedWidgetPosition_))
		{
			// If the box is too small in either direction, assume that we are translating to the point instead...
			bool smallArea = false;
			double delta, valMax, valMin, fraction;
			// -- X
			valMax = std::max(currentDataPosition_.x(), clickedDataPosition_.x());
			valMin = std::min(currentDataPosition_.x(), clickedDataPosition_.x());
			delta = xLogarithmic_ ? log10(valMax / valMin) : valMax - valMin;
			fraction = (xLogarithmic_ ? log10(xMax_/xMin_) : xMax_ - xMin_) * 0.001;
			if (delta < fraction) smallArea = true;
			// -- Y
			valMax = std::max(currentDataPosition_.y(), clickedDataPosition_.y());
			valMin = std::min(currentDataPosition_.y(), clickedDataPosition_.y());
			delta = yLogarithmic_ ? log10(valMax / valMin) : valMax - valMin;
			fraction = (yLogarithmic_ ? log10(yMax_/yMin_) : yMax_ - yMin_) * 0.001;
			if (delta < fraction) smallArea = true;

			if (smallArea)
			{
				double dx = currentDataPosition_.x() - (xMin_+xMax_)*0.5;
				double dy = currentDataPosition_.y() - (yMin_+yMax_)*0.5;
				xMin_ += dx;
				xMax_ += dx;
				yMin_ += dy;
				yMax_ += dy;
			}
			else zoomToGraph(clickedDataPosition_.x(), clickedDataPosition_.y(), currentDataPosition_.x(), currentDataPosition_.y());
		}
	}
	else if (buttons_&Qt::MiddleButton)
	{
	}

	// Reset values
	buttons_ = 0;
	clickedWidgetPosition_ = QPoint();

	// Update widget
	repaint();
}

// Mouse move event
void PlotWidget::mouseMoveEvent(QMouseEvent* event)
{
	currentWidgetPosition_ = event->pos();
	currentDataPosition_ = widgetToGraph(currentWidgetPosition_);
	setFocus();
	if (buttons_&Qt::MiddleButton)
	{
		if (!clickedWidgetPosition_.isNull())
		{
			double dx = currentDataPosition_.x() - clickedDataPosition_.x();
			double dy = currentDataPosition_.y() - clickedDataPosition_.y();
			xMin_ -= dx;
			xMax_ -= dx;
			yMin_ -= dy;
			yMax_ -= dy;

			clickedDataPosition_ = widgetToGraph(event->pos());
			repaint();
		}
	}
	
	// Set coordinates label (if it exists)
	if (coordinatesLabel_)
	{
		QString label;
		if (absoluteTime_)
		{
			QDateTime xDateTime = absoluteTimeOrigin_.addSecs(currentDataPosition_.x());
			label.sprintf("x = %s %s, y = %6.2e", qPrintable(xDateTime.time().toString("hh:mm:ss")), qPrintable(xDateTime.date().toString("dd/MM/yy")), currentDataPosition_.y());
		}
		else
		{
			int hh = int(fabs(currentDataPosition_.x()) / 3600);
			int mm = int((fabs(currentDataPosition_.x()) - hh*3600) / 60);
			int ss = int(fabs(currentDataPosition_.x()) - hh*3600 - mm*60);
			if (currentDataPosition_.x() < 0.0) label.sprintf("x=-%2i:%02i:%02i, y=%6.2e", hh, mm, ss, currentDataPosition_.y());
			else label.sprintf("x = %02i:%02i:%02i, y = %6.2e", hh, mm, ss, currentDataPosition_.y());
		}
		coordinatesLabel_->setText(label);
	}
}

// Mouse wheel event
void PlotWidget::wheelEvent(QWheelEvent* event)
{
	// Get a fraction of the current y range
	double yBit = (yMax_ - yMin_) * 0.1;
	if (event->delta() < 0) yBit *= -1.0;

	// Shift y range of graph....
	yMin_ += yBit;
	yMax_ += yBit;

	repaint();
}

// Key press event
void PlotWidget::keyPressEvent(QKeyEvent* event)
{
	bool accept = true;
	switch (event->key())
	{
		// Show all data (optionally obeying soft limits)
		case (Qt::Key_A):
			if (event->modifiers().testFlag(Qt::ShiftModifier)) fitData(false);
			else fitData(true);
			update();
			break;
		// Toggle X axis soft limits
// 		case (Qt::Key_X):
// 			if (event->modifiers().testFlag(Qt::ShiftModifier)) limitXMax_ = !limitXMax_;
// 			else limitXMin_ = !limitXMin_;
// 			fitData(true);
// 			update();
// 			break;
		// Toggle Y axis soft limits
// 		case (Qt::Key_Y):
// 			if (event->modifiers().testFlag(Qt::ShiftModifier)) limitYMax_ = !limitYMax_;
// 			else limitYMin_ = !limitYMin_;
// 			fitData(true);
// 			update();
// 			break;
		// Toggle logarithmic X axis
		case (Qt::Key_K):
			xLogarithmic_ = !xLogarithmic_;
			fitData(true);
			update();
			break;
		// Toggle logarithmic Y axis
		case (Qt::Key_L):
			yLogarithmic_ = !yLogarithmic_;
			fitData(true);
			update();
			break;
		default:
			accept = false;
			break;
	}
	if (accept) event->accept();
	else event->ignore();
}

// Key release event
void PlotWidget::keyReleaseEvent(QKeyEvent* event)
{
	event->ignore();
}

// Context Menu Show All clicked
void PlotWidget::contextMenuShowAllClicked(bool checked)
{
	fitData(true);
}

// Context Menu CopyToClipboard clicked
void PlotWidget::contextMenuCopyToClipboardClicked(bool checked)
{
	// Create bitmap image of plot and then print image to QPainter
	const double inchesPerMetre = 39.3700787;
	int scaling = 1;
	QImage image(width()*scaling, height()*scaling, QImage::Format_RGB32);
// 	image.setDotsPerMeterX(inchesPerMetre * scaling*90);
// 	image.setDotsPerMeterY(inchesPerMetre * scaling*90);
	draw(image);

	// Copy image to clipboard
	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setImage(image);
}

// Set coordinates label
void PlotWidget::setCoordinatesLabel(QLabel* label)
{
	coordinatesLabel_ = label;
}

/*
 * Style
 */

// Return nth pre-defined colour
QColor PlotWidget::lineColour(int n)
{
	return lineColours_[n%PlotWidget::nLineColours];
}

// Set single property status
void PlotWidget::setSingleProperty(bool b)
{
	singleProperty_ = b;
}

// Set whether to shade background (if single property)
void PlotWidget::setShadedBackground(bool b)
{
	shadeBackground_ = b;
}

// Set shaded background minimum and colour
void PlotWidget::setShadedBackgroundMinimum(double value, QColor colour)
{
	shadedBackgroundMinimumY_ = value;
	shadedBackgroundMinimumColour_ = colour;
}

// Return shaded background minimum value
double PlotWidget::shadedBackgroundMinimum()
{
	return shadedBackgroundMinimumY_;
}

// Return shaded background minimum colour
QColor PlotWidget::shadedBackgroundMinimumColour()
{
	return shadedBackgroundMinimumColour_;
}

// Set shaded background maximum and colour
void PlotWidget::setShadedBackgroundMaximum(double value, QColor colour)
{
	shadedBackgroundMaximumY_ = value;
	shadedBackgroundMaximumColour_ = colour;
}

// Return shaded background maximum value
double PlotWidget::shadedBackgroundMaximum()
{
	return shadedBackgroundMaximumY_;
}

// Return shaded background maximum colour
QColor PlotWidget::shadedBackgroundMaximumColour()
{
	return shadedBackgroundMaximumColour_;
}

/*
 * Plot Area
 */

// Determine suitable tick deltas based on current graph limits
void PlotWidget::calculateTickDeltas(int maxTicks)
{
	const int nBaseValues = 5, maxIterations = 10;
	int power = 1, baseValues[nBaseValues] = { 1, 2, 3, 4, 5 }, baseValueIndex = 0, nTicks, iteration, minTicks = maxTicks/2;

	// Y axis first
	baseValueIndex = 0;
	power = int(log10((yMax_-yMin_) / maxTicks) - 1);
	iteration = 0;
	
	if ((yMax_-yMin_) > 1.0e-10)
	{
		do
		{
			// Calculate current tickDelta
			yAxisTickDelta_ = baseValues[baseValueIndex]*pow(10.0,power);

			// Get first tickmark value
			yAxisTickStart_ = int(yMin_ / yAxisTickDelta_) * yAxisTickDelta_;
			if (yAxisTickStart_ < yMin_) yAxisTickStart_ += yAxisTickDelta_;
			
			// How many ticks now fit between the firstTick and max value?
			// Add 1 to get total ticks for this delta (i.e. including firstTick)
			nTicks = int((yMax_-yAxisTickStart_) / yAxisTickDelta_);
			++nTicks;
			
			// Check n...
			if (nTicks > maxTicks)
			{
				++baseValueIndex;
				if (baseValueIndex == nBaseValues) ++power;
				baseValueIndex = baseValueIndex%nBaseValues;
			}
			else if (nTicks < minTicks)
			{
				--baseValueIndex;
				if (baseValueIndex == -1)
				{
					--power;
					baseValueIndex += nBaseValues;
				}
			}
			
			++iteration;
			if (iteration == maxIterations) break;
			
		} while ((nTicks > maxTicks) || (nTicks < minTicks));
	}
	else
	{
		yAxisTickStart_ = yMin_;
		yAxisTickDelta_ = 1.0;
	}

	// Now x (time) axis
	// The exact 'units' we use will depend on the timespan of the axis
	const int nTimeDeltas = 16, timeDeltas[] = { 1, 2, 3, 4, 5, 10, 30, 60, 120, 180, 240, 300, 600, 1200, 1800, 3600 };
	int range = int(floor(xMax_) - ceil(xMin_));
	int snappedXMin;
	if (absoluteTime_)
	{
		// Each dataset will use its own time origin, so use literal range
		// Divide range up into five segments, an
	}

	// Loop over timeDeltas, and search for a sensible division
	nTicks = 0;
	int requiredTicks;
	for (int n=0; n<100; ++n)
	{
		if (n < nTimeDeltas) xAxisTickDelta_ = timeDeltas[n];
		else xAxisTickDelta_ = (1+n-nTimeDeltas) * 3600;

		// Get 'snapped' value for xMin - i.e. first time value divisible by this time delta
		snappedXMin = int(ceil(xMin_) / xAxisTickDelta_) * xAxisTickDelta_;
		xAxisTickStart_ = snappedXMin;
		
		// Calculate number of ticks needed between here and the maximum value
		nTicks = int((xMax_ - snappedXMin) / xAxisTickDelta_);
		if (nTicks < maxTicks) break;
	}
}

// Set main title
void PlotWidget::setMainTitle(QString title)
{
	mainTitle_ = title;
	repaint();
}

// Set x-axis title
void PlotWidget::setXAxisTitle(QString title)
{
	xAxisTitle_ = title;
	repaint();
}

// Set y-axis title
void PlotWidget::setYAxisTitle(QString title)
{
	yAxisTitle_ = title;
	repaint();
}

// Set x and y axis titles
void PlotWidget::setTitles(QString mainTitle, QString xTitle, QString yTitle)
{
	mainTitle_ = mainTitle;
	xAxisTitle_ = xTitle;
	yAxisTitle_ = yTitle;
	update();
}

// Set soft X limits for plot area
void PlotWidget::setXLimits(bool setMinLimit, bool applyMinLimit, double minLimit, bool setMaxLimit, bool applyMaxLimit, double maxLimit)
{
	if (setMinLimit)
	{
		limitXMin_ = applyMinLimit;
		xMinLimit_ = minLimit;
	}
	if (setMaxLimit)
	{
		limitXMax_ = applyMaxLimit;
		xMaxLimit_ = maxLimit;
	}
	fitData(true);
}

// Set soft Y limits for plot area
void PlotWidget::setYLimits(bool setMinLimit, bool applyMinLimit, double minLimit, bool setMaxLimit, bool applyMaxLimit, double maxLimit)
{
	if (setMinLimit)
	{
		limitYMin_ = applyMinLimit;
		yMinLimit_ = minLimit;
	}
	if (setMaxLimit)
	{
		limitYMax_ = applyMaxLimit;
		yMaxLimit_ = maxLimit;
	}
	fitData(true);
}

// Set whether absolute time should be used on the x axis
void PlotWidget::setAbsoluteTime(bool on)
{
	absoluteTime_ = on;
	fitData(true);
}

// Return whether absolute time should be used on the x axis
bool PlotWidget::absoluteTime()
{
	return absoluteTime_;
}

// Set whether begin/end periods are shown
void PlotWidget::setShowBeginEnd(bool on)
{
	showBeginEnd_ = on;
	update();
}

// Return whether begin/end periods are shown
bool PlotWidget::showBeginEnd()
{
	return showBeginEnd_;
}

// Set whether legend is visible
void PlotWidget::setShowLegend(bool on)
{
	showLegend_ = on;
	update();
}

// Return whether legend is visible
bool PlotWidget::showLegend()
{
	return showLegend_;
}

// Set whether to assume 'On Change' data
void PlotWidget::setOnChangeData(bool b)
{
	onChangeData_ = b;
	
	// Invalidate the current QPainterPaths, since they will need to be regenerated
	invalidatePainterPaths();
	update();
}

// Return whether to assume 'On Change' data
bool PlotWidget::onChangeData()
{
	return onChangeData_;
}

// Enable/disable soft x minimum limit
void PlotWidget::setXMinLimit(bool enabled)
{
	limitXMin_ = enabled;
	fitData(true);
}

// Enable/disable soft x maxiimum limit
void PlotWidget::setXMaxLimit(bool enabled)
{
	limitXMax_ = enabled;
	fitData(true);
}

// Enable/disable soft y minimum limit
void PlotWidget::setYMinLimit(bool enabled)
{
	limitYMin_ = enabled;
	fitData(true);
}

// Enable/disable soft y maxiimum limit
void PlotWidget::setYMaxLimit(bool enabled)
{
	limitYMax_ = enabled;
	fitData(true);
}

/*
// Source Data
*/

// Invalidate painter paths for all dataset, forcing their regeneration on next redraw
void PlotWidget::invalidatePainterPaths()
{
	for (PlotData* pd = dataSets_.first(); pd != NULL; pd = pd->next) pd->invalidatePainterPath();
}

// Create new dataset group
PlotDataGroup* PlotWidget::addPlotDataGroup(QString name, bool visible, QDateTime start, QDateTime end)
{
	PlotDataGroup* group = new PlotDataGroup(name, start, end);
	group->setVisible(visible);
	if (visible)
	{
		group->setColour( PlotWidget::lineColour(nDataSetGroupsVisible_%PlotWidget::nLineColours) );
		++nDataSetGroupsVisible_;
	}
	dataSetGroups_.own(group);
	return group;
}

// Return group list
const List<PlotDataGroup> PlotWidget::dataSetGroups()
{
	return dataSetGroups_;
}

// Add data to Plot (local Data2D)
PlotData* PlotWidget::addDataSet(PlotDataGroup* parent, Data2D& data, QString runNumber, QDateTime timeMax, QString blockName, int yOffset)
{
	PlotData* pd = dataSets_.add();
	pd->setData(parent, data, runNumber, timeMax);
	pd->setVerticalOffset(yOffset);
	
	// Determine limits for this data, and adjust parent's relative limits if necessary
	pd->determineLimits();

	// Set group index - does the group exist already?
	PlotDataBlock* pdb;
	for (pdb = dataSetBlocks_.first(); pdb != NULL; pdb = pdb->next) if (pdb->blockName() == blockName) break;
	if (pdb == NULL)
	{
		pdb = new PlotDataBlock(blockName);
		dataSetBlocks_.own(pdb);
	}
	pd->setBlock(pdb);
	pdb->addPlotData(pd);

	// Repaint widget
	repaint();

	return pd;
}

// Remove all data from plot
void PlotWidget::removeAllDataSets()
{
	dataSets_.clear();

	repaint();
}

// Return dataset list
const List<PlotData>& PlotWidget::dataSets()
{
	return dataSets_;
}

// Determine dataset limits
void PlotWidget::determineDataSetLimits()
{
	for (PlotData* pd = dataSets_.first(); pd != NULL; pd = pd->next) pd->determineLimits();
}

// Return list of data set groups
const List<PlotDataBlock>& PlotWidget::dataSetBlocks()
{
	return dataSetBlocks_;
}

// Return named block (if it exists)
PlotDataBlock* PlotWidget::dataSetBlock(QString name)
{
	for (PlotDataBlock* pdb = dataSetBlocks_.first(); pdb != NULL; pdb = pdb->next) if (pdb->blockName() == name) return pdb;
	return NULL;
}

// Return list of visible data blocks
const RefList<PlotDataBlock,int> PlotWidget::visibleDataSetBlocks()
{
	RefList<PlotDataBlock,int> result;
	for (PlotDataBlock* pdb = dataSetBlocks_.first(); pdb != NULL; pdb = pdb->next) if (pdb->visible()) result.add(pdb);
	return result;
}

// Hide all datasets
void PlotWidget::hideAllDataSets()
{
	for (PlotDataGroup* group = dataSetGroups_.first(); group != NULL; group = group->next) group->setVisible(false);
	nDataSetGroupsVisible_ = 0;
	for (PlotDataBlock* pdb = dataSetBlocks_.first(); pdb != NULL; pdb = pdb->next) pdb->setVisible(false);
	nDataSetBlocksVisible_ = 0;
}

// Set availability for data from specified run number
void PlotWidget::setGroupVisible(QString runNumber, bool visible)
{
	// Loop over time periods, and set the one for this runnumber
	// Recalculate number of visible groups while we're at it...
	nDataSetGroupsVisible_ = 0;
	for (PlotDataGroup* group = dataSetGroups_.first(); group != NULL; group = group->next)
	{
		if (group->name() == runNumber) group->setVisible(visible);
		
		// Set group colour
		if (group->visible())
		{
			group->setColour( PlotWidget::lineColour(nDataSetGroupsVisible_%PlotWidget::nLineColours) );
			++nDataSetGroupsVisible_;
		}
	}

	fitData(true);
	update();
}

// Show all datasets containing specified block data
void PlotWidget::setBlockVisible(QString blockName, bool visible)
{
	nDataSetBlocksVisible_ = 0;
	for (PlotDataBlock* pdb = dataSetBlocks_.first(); pdb != NULL; pdb = pdb->next)
	{
		if (pdb->blockName() == blockName)
		{
			pdb->setVisible(visible);

			// Update PlotDataGroup visibility counters
			for (PlotData* pd = dataSets_.first(); pd != NULL; pd = pd->next)
			{
				if (pd->block() != pdb) continue;
				if (pd->parent() == NULL) continue;
				pd->parent()->changeVisibleCount(visible);
			}
		}
		
		// Set block linestyle
		if (pdb->visible())
		{
			pdb->setLineStyle( (PlotDataBlock::BlockLineStyle) (nDataSetBlocksVisible_%PlotDataBlock::nBlockLineStyles) );
			++nDataSetBlocksVisible_;
		}
	}

	fitData(true);
	
	update();
}

// Return earliest time origin over all shown datasets
QDateTime PlotWidget::absoluteTimeOrigin()
{
	return absoluteTimeOrigin_;
}

/*
// Functions
*/

// Convert widget coordinates to graph coordinates
QPointF PlotWidget::widgetToGraph(QPoint pos)
{
	QPointF result;
	if (xLogarithmic_) result.setX(pow(10.0, (pos.x() - graphArea_.left()) / xScale_ + xMin_));
	else result.setX((pos.x() - graphArea_.left()) / xScale_ + xMin_);
	if (yLogarithmic_) result.setY(pow(10.0, (graphArea_.bottom() - pos.y()) / yScale_ + yMin_));
	else result.setY((graphArea_.bottom() - pos.y()) / yScale_ + yMin_);
	return result;
}

// Zoom to specified graph coordinates
void PlotWidget::zoomToGraph(double x1, double y1, double x2, double y2)
{
	// Set range
	xMin_ = xLogarithmic_ ? log10(x1) : x1;
	xMax_ = xLogarithmic_ ? log10(x2) : x2;
	yMin_ = yLogarithmic_ ? log10(y1) : y1;
	yMax_ = yLogarithmic_ ? log10(y2) : y2;

	// Swap values if necessary
	double temp;
	if (xMin_ > xMax_)
	{
		temp = xMin_;
		xMin_ = xMax_;
		xMax_ = temp;
	}
	if (yMin_ > yMax_)
	{
		temp = yMin_;
		yMin_ = yMax_;
		yMax_ = temp;
	}

	msg.print("Zoom To  %f/%f and %f/%f\n", xMin_, xMax_, yMin_, yMax_);
}

// Rescale axes to fit all current data
void PlotWidget::fitData(bool obeySoftLimits)
{
	// Set initial values
	int nVisible = 0;
	absoluteTimeOrigin_ = QDateTime();
	for (PlotData* pd = dataSets_.first(); pd != NULL; pd = pd->next)
	{
		// If dataset isn't currently shown, move on
		if (!pd->visible()) continue;
		
		// If this is the first visible dataset encountered, set initial limits - otherwise, compare
		double off = verticalSpacing_ * pd->verticalOffset();
		if (nVisible == 0)
		{
			absoluteTimeOrigin_ = pd->runTimeStart();
			xMin_ = pd->xMin();
			xMax_ = pd->xMax();
			yMin_ = pd->yMin() + verticalSpacing_*pd->verticalOffset();
			yMax_ = pd->yMax() + verticalSpacing_*pd->verticalOffset();
		}
		else
		{
			// X axis first
			if (absoluteTime_)
			{
				if (pd->runTimeStart() < absoluteTimeOrigin_)
				{
					xMax_ = pd->runTimeStart().secsTo( absoluteTimeOrigin_.addSecs(xMax_) );
					absoluteTimeOrigin_ = pd->runTimeStart();
				}
				
				double secs = absoluteTimeOrigin_.secsTo(pd->runTimeStart().addSecs(pd->xMax()));
				if (secs > xMax_) xMax_ = secs;
			}
			else
			{
				if (pd->xMin() < xMin_) xMin_ = pd->xMin();
				if (pd->xMax() > xMax_) xMax_ = pd->xMax();
			}
			
			// Now Y
			if ((pd->yMin() + off) < yMin_) yMin_ = pd->yMin() + off;
			if ((pd->yMax() + off) > yMax_) yMax_ = pd->yMax() + off;
		}
		++nVisible;
	}
	
	if (nVisible > 0)
	{
		if (yLogarithmic_)
		{
			// TODO
// 				if ((yMin_ - 1.0) > 0) yMin_ -= 1.0;
// 				if ((yMax_ + 1.0) > 0) yMax_ += 1.0;
		}
		else
		{
			// Increase y limits by 5% of difference (or actual value if difference is 'zero')
			double diff = ((yMax_ - yMin_) < yMax_*1.0e-3 ? yMax_*0.05 : (yMax_ - yMin_) * 0.05);
			if (diff < 1.0e-8)
			{
				yMin_ -= 1.0;
				yMax_ += 1.0;
			}
			else
			{
				if (fabs(yMin_) > 1.0e-8) yMin_ -= diff;
				if (fabs(yMax_) > 1.0e-8) yMax_ += diff;
			}
		}
	}

	// Obey soft limits?
	if (obeySoftLimits)
	{
		if (limitXMin_ && (xMin_ < xMinLimit_)) xMin_ = xMinLimit_;
		if (limitXMax_ && (xMax_ > xMaxLimit_)) xMax_ = xMaxLimit_;
		if (limitYMin_ && (yMin_ < yMinLimit_)) yMin_ = yMinLimit_;
		if (limitYMax_ && (yMax_ > yMaxLimit_)) yMax_ = yMaxLimit_;
	}

	// Adjust limits to make suitable for log axes
	if (xLogarithmic_)
	{
		xMin_ = xMin_ <= 0 ? 1.0e-6 : log10(xMin_);
		xMax_ = xMax_ <= 0 ? 1.0e-6 : log10(xMax_);
	}
	if (yLogarithmic_)
	{
		yMin_ = yMin_ <= 0 ? 1.0e-6 : log10(yMin_);
		yMax_ = yMax_ <= 0 ? 1.0e-6 : log10(yMax_);
	}

	msg.print("New axis limits are %f/%f and %f/%f (scales = %f/%f), %i/%i\n", xMin_, xMax_, yMin_, yMax_, xScale_, yScale_, graphArea_.width(), graphArea_.height());

	repaint();
}

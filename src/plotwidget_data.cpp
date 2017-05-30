/*
	*** PlotData
	*** src/plotwidget_data.cpp
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

#include "plotwidget.hui"
#include <QPen>
#include <math.h>

/*
// Plot Data Group
*/

// Constructor
PlotDataGroup::PlotDataGroup(QString name, QDateTime begin, QDateTime end) : ListItem<PlotDataGroup>()
{
	name_ = name;
	runBeginDateTime_ = begin;
	runEndDateTime_ = end;
	runXMin_ = 0.0;
	runXMax_ = 0.0;
	runTrueXMin_ = false;
	runTrueXMax_ = false;
	visible_ = false;
	nDataSetsVisible_ = 0;
}

// Return name of this period
QString PlotDataGroup::name()
{
	return name_;
}

// Set availability
void PlotDataGroup::setVisible(bool avail)
{
	visible_ = avail;
}

// Return availability
bool PlotDataGroup::visible()
{
	return visible_;
}

// Modify dataset visibility count
void PlotDataGroup::changeVisibleCount(bool newDataSetVisibility)
{
	if (newDataSetVisibility) ++nDataSetsVisible_;
	else --nDataSetsVisible_;
}

// Return number of dataSets_ which are currently visible
int PlotDataGroup::nDataSetsVisible()
{
	return nDataSetsVisible_;
}

// Set general colour for this group
void PlotDataGroup::setColour(QColor colour)
{
	colour_ = colour;
}

// Return general colour for this group
QColor PlotDataGroup::colour()
{
	return colour_;
}

// Return start time and date for run
QDateTime PlotDataGroup::runBeginDateTime()
{
	return runBeginDateTime_;
}

// Return end time and date for run
QDateTime PlotDataGroup::runEndDateTime()
{
	return runEndDateTime_;
}

// Set x axis minimum for run
void PlotDataGroup::setRunXMin(double xmin, bool trueLimit)
{
	runXMin_ = xmin;
	runTrueXMin_ = trueLimit;
}

// Set x axis maximum for run
void PlotDataGroup::setRunXMax(double xmax, bool trueLimit)
{
	runXMax_ = xmax;
	runTrueXMax_ = trueLimit;
}

// Return xMin
double PlotDataGroup::runXMin()
{
	return runXMin_;
}

// Return xMin true limit
double PlotDataGroup::runTrueXMin()
{
	return runTrueXMin_;
}

// Return xMax
double PlotDataGroup::runXMax()
{
	return runXMax_;
}

// Return xMax true limit
double PlotDataGroup::runTrueXMax()
{
	return runTrueXMax_;
}

/*
 * PlotDataBlock
 */

// Static member
QVector<qreal> PlotDataBlock::lineStyles_[] = {
	QVector<qreal>() << 10 << 1,
	QVector<qreal>() << 1 << 3,
	QVector<qreal>() << 3 << 3,
	QVector<qreal>() << 1 << 3 << 3 << 3
};

// Constructor
PlotDataBlock::PlotDataBlock(QString blockName) : ListItem<PlotDataBlock>()
{
	blockName_ = blockName;
	lineStyle_ = PlotDataBlock::SolidStyle;
	visible_ = false;
}

// Destructor
PlotDataBlock::~PlotDataBlock()
{
}

// Return name of block
QString PlotDataBlock::blockName()
{
	return blockName_;
}

// Set associated line style
void PlotDataBlock::setLineStyle(PlotDataBlock::BlockLineStyle style)
{
	lineStyle_ = style;
}

// Return associated line dash pattern
const QVector<qreal>& PlotDataBlock::dashes()
{
	return lineStyles_[lineStyle_];
}

// Set visibility of block
void PlotDataBlock::setVisible(bool visible)
{
	visible_ = visible;
}

// Return visibility of block
bool PlotDataBlock::visible()
{
	return visible_;
}

// Add reference to PlotData, storing it in run number order
void PlotDataBlock::addPlotData(PlotData* pd)
{
	data_.add(pd);
}

// Return list of referenced PlotData
const RefList<PlotData,int>& PlotDataBlock::plotData()
{
	return data_;
}

/*
// Plot Data
*/

// Constructor
PlotData::PlotData() : ListItem<PlotData>()
{
	lineColour_ = Qt::black;
	lineStyle_ = Qt::SolidLine;
	parent_ = NULL;
	lastXScale_ = 0.0;
	lastYScale_ = 0.0;
}

// Destructor
PlotData::~PlotData()
{
}

// Set source data
void PlotData::setData(PlotDataGroup* parent, Data2D& source, QString runNumber, QDateTime tMax)
{
	parent_ = parent;
	data_ = source;
	name_ = runNumber;
	timeMax_ = tMax;
}

// Return reference to contained data
Data2D& PlotData::data()
{
	return data_;
}

// Determine data limits
void PlotData::determineLimits()
{
	const int* xarray = data_.arrayX().array();
	const Data2DValue* yarray = data_.arrayY().array();
	double x, y;
	if ((xarray != NULL) && (yarray != NULL))
	{
		// Get first point and use to set initial limits
		x = xarray[0];
		y = yarray[0].y();
		xMin_ = x;
		xMax_ = runTimeStart().secsTo(timeMax_);
		yMin_ = y;
		yMax_ = y;
		for (int n=1; n<data_.arrayX().nItems(); ++n)
		{
			// Grab modified array values
			x = xarray[n];
			y = yarray[n].y();

			// Recalculate min/max values
			if (x > xMax_) xMax_ = x;
			if (x < xMin_) xMin_ = x;
			if (y > yMax_) yMax_ = y;
			if (y < yMin_) yMin_ = y;
		}
	}
}

// Regenerate painter path
void PlotData::generatePainterPaths(double xScale, bool xLogarithmic, double yScale, bool yLogarithmic, bool changesOnly)
{
	// Generate QPainterPath
	// Check the scale values at which the path was last created at - if it hasn't changed, don't bother recreating the path again...
	if ( (fabs(xScale-lastXScale_) < 1.0e-10) && (fabs(yScale-lastYScale_) < 1.0e-10)) return;

	linePath_ = QPainterPath();
	symbolPath_ = QPainterPath();
	QRect symbolRect(0, 0, 7, 7);
	const int* xarray = data_.arrayX().array();
	const Data2DValue* yarray = data_.arrayY().array();
	int enumY;
	double x, y, lastX, lastScaledY, scaledX, scaledY;
	if ((xarray != NULL) && (yarray != NULL))
	{
		// Get first point and use to set painter path origin
		x = xLogarithmic ? log10(xarray[0]) : xarray[0];
		y = yLogarithmic ? log10(yarray[0].y()) : yarray[0].y();
		linePath_.moveTo(x * xScale, y * yScale);
		lastX = x;
		lastScaledY = y * yScale;
		for (int n=1; n<data_.arrayX().nItems(); ++n)
		{
			// Grab modified array values
			x = xLogarithmic ? log10(xarray[n]) : xarray[n];
			y = yLogarithmic ? log10(yarray[n].y()) : yarray[n].y();

			// Use scaled (screen) coordinates for point
			scaledX = x*xScale;
			scaledY = y*yScale;

			if (changesOnly)
			{
				// If the 'changesOnly' option is selected, it is assumed that datapoints indicate *changes* in Y values, rather than
				// a continuously measured set. As such, if this X point is more than 1 second away from the last, we must draw a line
				// to just before the x value of the current point first.
				if ((x - lastX) > 1.5) linePath_.lineTo((x-0.5)*xScale, lastScaledY);
			}

			// Draw the next line
			linePath_.lineTo(scaledX, scaledY);

			if (yarray[n].constEnumeratedY() != NULL)
			{
				symbolRect.moveCenter(QPoint( int(scaledX), int(scaledY)));
				symbolPath_.addEllipse(symbolRect);
			}

			// Store current X and scaled Y values
			lastX = x;
			lastScaledY = scaledY;
		}

		// Add on final point, extending the data to the end of the run period (only if we are still inside the run period)
		if (changesOnly && (lastX < data_.runTimeEndAsX())) linePath_.lineTo(data_.runTimeStart().secsTo(timeMax_)*xScale, lastScaledY);
	}

	lastXScale_ = xScale;
	lastYScale_ = yScale;
}

// Set block
void PlotData::setBlock(PlotDataBlock* block)
{
	block_ = block;
}

// Return block
PlotDataBlock* PlotData::block()
{
	return block_;
}

// Return line QPainterPath
QPainterPath& PlotData::linePath()
{
	return linePath_;
}

// Return line QPainterPath
QPainterPath& PlotData::symbolPath()
{
	return symbolPath_;
}

// Invalidate painter path, forcing it to be recreated
void PlotData::invalidatePainterPath()
{
	// Just reset the scaling factors
	lastXScale_ = 0.0;
	lastYScale_ = 0.0;
}

// Return whether this data is visible
bool PlotData::visible()
{
	if (parent_)
	{
		if (block_) return (parent_->visible() && block_->visible());
		else return parent_->visible();
	}
	else if (block_) return block_->visible();
	else return true;
}

// Set vertical offset to apply to data
void PlotData::setVerticalOffset(int offset)
{
	verticalOffset_ = offset;
}

// Return vertical offset to apply to data
double PlotData::verticalOffset()
{
	return verticalOffset_;
}

// Return minimum x value for data
double PlotData::xMin()
{
	return xMin_;
}

// Return maximum x value for data
double PlotData::xMax()
{
	return xMax_;
}

// Return minimum y value for data
double PlotData::yMin()
{
	return yMin_;
}

// Return maximum y value for data
double PlotData::yMax()
{
	return yMax_;
}

// Return name
QString PlotData::name()
{
	return name_;
}

// Return parent
PlotDataGroup* PlotData::parent()
{
	return parent_;
}

/*
 * Data2D Access
 */

// Return time origin from Data2D
QDateTime PlotData::runTimeStart()
{
	return data_.runTimeStart();
}

// Return enumerated Y values from Data2D
const RefList<EnumeratedValue,int>& PlotData::enumeratedY()
{
	return data_.enumeratedY();
}

/*
// Style
*/

// Set line colour
void PlotData::setLineColour(QColor color)
{
	lineColour_ = color;
}

// Return line colour
QColor PlotData::lineColour()
{
	return lineColour_;
}

// Set line style
void PlotData::setLineStyle(Qt::PenStyle style)
{
	lineStyle_ = style;
}

// Return line style
Qt::PenStyle PlotData::lineStyle()
{
	return lineStyle_;
}

// Set supplied pen colour and line style
void PlotData::stylePen(QPen& pen)
{
	// Get colour from PlotDataGroup parent (if available)
	if (parent_) pen.setColor(parent_->colour());
	else pen.setColor(lineColour_);
	
	// Get line style from PlotDataBlock (if available)
	if (block_) pen.setDashPattern(block_->dashes());
}

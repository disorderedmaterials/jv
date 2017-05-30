/*
	*** Two-dimensional Data Storage
	*** src/data2d.cpp
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

// Constants
#define PI 3.141592653589793
#define TWOPI 6.283185307179586

#include "data2d.h"
#include "enumeration.h"
#include "messenger.hui"
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <QFile>
#include <QTextStream>
#include <QStringList>

/*
 * Data2DValue
 */

// Constructors
Data2DValue::Data2DValue(double y, EnumeratedValue* enumeratedY)
{
	y_ = y;
	enumeratedY_ = enumeratedY;
}

// Copy constructor
Data2DValue::Data2DValue(const Data2DValue& source)
{
	y_ = source.y_;
	enumeratedY_ = source.enumeratedY_;
}

// Assignment Operator (Data2DValue)
void Data2DValue::operator=(const Data2DValue& source)
{
	y_ = source.y_;
	enumeratedY_ = source.enumeratedY_;
}

// Assignment Operaor
void Data2DValue::operator=(double y)
{
	y_ = y;
	enumeratedY_ = NULL;
}

// Assignment Operaor
void Data2DValue::operator=(EnumeratedValue* value)
{
	y_ = 0.0;
	enumeratedY_ = value;
}

// Return value of y, from whichever is the valid source
double Data2DValue::y() const
{
	return enumeratedY_ == NULL ? y_ : enumeratedY_->value();
};

// Return real value
double Data2DValue::constY() const
{
	return y_;
}

// Return enumerated index
EnumeratedValue* Data2DValue::constEnumeratedY() const
{
	return enumeratedY_;
}

/*
 * Data2D
 */

// Constructor
Data2D::Data2D() : ListItem<Data2D>()
{
	name_ = "Untitled";
}

// Destructor
Data2D::~Data2D()
{
}

// Copy Constructor
Data2D::Data2D(const Data2D& source)
{
	(*this) = source;
}

// Clear Data
void Data2D::clear()
{
	x_.clear();
	y_.clear();
	enumeratedY_.clear();
	runTimeStart_ = QDateTime();
	runTimeEnd_ = QDateTime();
}

/*
// Data
*/

// Resize arrays
void Data2D::resize(int size)
{
	clear();
	x_.createEmpty(size);
	y_.createEmpty(size);
}

// Reset arrays to zero
void Data2D::reset()
{
	for (int n=0; n<x_.nItems(); ++n) x_[n] = 0.0;
	for (int n=0; n<y_.nItems(); ++n) y_[n] = 0.0;
	enumeratedY_.clear();
}

// Initialise arrays to specified size
void Data2D::initialise(int size)
{
	resize(size);
}

// Return current array size
int Data2D::arraySize()
{
	return x_.nItems();
}

// Return number of defined datapoints
int Data2D::nPoints() const
{
	return x_.nItems();
}

// Return x value specified
int Data2D::x(int index) const
{
#ifdef CHECKS
	if ((index < 0) || (index >= x_.nItems()))
	{
		printf("OUT_OF_RANGE - Index %i is out of range for x_ array in Data2D::x().\n", index);
		return 0.0;
	}
#endif
	return x_.value(index);
}

// Return x Array
Array<int>& Data2D::arrayX()
{
	return x_;
}

// Set y value
void Data2D::setY(int index, double y)
{
#ifdef CHECKS
	if ((index < 0) || (index >= y_.nItems()))
	{
		printf("OUT_OF_RANGE - Index %i is out of range for y_ array in Data2D::setY().\n", index);
		return;
	}
#endif
	y_[index] = y;
}

// Return y value specified
Data2DValue Data2D::y(int index) const
{
#ifdef CHECKS
	if ((index < 0) || (index >= y_.nItems()))
	{
		printf("OUT_OF_RANGE - Index %i is out of range for y_ array in Data2D::y().\n", index);
		return 0.0;
	}
#endif
	return y_.value(index);
}

// Return y Array
Array<Data2DValue>& Data2D::arrayY()
{
	return y_;
}

// Add relative data point
void Data2D::addRelativePoint(QDateTime time, double y)
{
	x_.add(runTimeStart_.secsTo(time));
	y_.add(Data2DValue(y, NULL));
}

// Add relative data point (enumerated value)
void Data2D::addRelativePoint(QDateTime time, EnumeratedValue* enumy)
{
	x_.add(runTimeStart_.secsTo(time));
	y_.add(Data2DValue(0.0, enumy));

	// Store unique link to EnumeratedValue
	enumeratedY_.addUnique(enumy);
}

// Add new data point
void Data2D::addPoint(int x, double y)
{
	x_.add(x);
	y_.add(Data2DValue(y, NULL));
}

// Set time origin for data
void Data2D::setRunTimeSpan(QDateTime origin, QDateTime endTime)
{
	runTimeStart_ = origin;
	runTimeEnd_ = endTime;
}

// Return time origin for data
QDateTime Data2D::runTimeStart()
{
	return runTimeStart_;
}

// Return run time end as relative x offset from runTimeStart_
int Data2D::runTimeEndAsX()
{
	return runTimeStart_.secsTo(runTimeEnd_);
}

// Set name of data
void Data2D::setName(QString name)
{
	name_ = name;
}

// Return name of data
QString Data2D::name() const
{
	return name_;
}

// Return list of enumerated Y values
const RefList<EnumeratedValue,int>& Data2D::enumeratedY()
{
	return enumeratedY_;
}

// Set name of parent group
void Data2D::setGroupName(QString name)
{
	groupName_ = name;
}

// Return name of parent group
QString Data2D::groupName() const
{
	return groupName_;
}

/*
// Operators
*/

// Operator =
void Data2D::operator=(const Data2D& source)
{
	x_ = source.x_;
	y_ = source.y_;
	name_ = source.name_;
	enumeratedY_ = source.enumeratedY_;
	runTimeStart_ = source.runTimeStart_;
	runTimeEnd_ = source.runTimeEnd_;
}

/*
 * Functions
 */

// Calculate and return average
double Data2D::totalAverage()
{
	if (nPoints() == 0) return 0.0;
	else if (nPoints() == 1) return x_[0];

	double avg = 0.0, secs = 0.0;
	double dx;
	for (int n=1; n<x_.nItems(); ++n)
	{
		dx = x_[n] - x_[n-1];
		secs += dx;
		avg += y_[n].y() * dx;
	}
	return avg/secs;
}

// Determine number of datapoints within run begin/end
int Data2D::runNPoints()
{
	int nAdded = 0, endX = runTimeEndAsX();
	for (int n=0; n<x_.nItems(); ++n)
	{
		if (x_[n] < 0) continue;
		if (x_[n] > endX) break;
		++nAdded;
	}
	return nAdded;
}

// Calculate time covered by datapoints within run begin/end
double Data2D::runTimeCoverage()
{
	return runTimeStart_.secsTo(runTimeEnd_);
}

// Calculate and return average over run begin/end
double Data2D::runAverage(bool& noValue)
{
	noValue = true;
	if (x_.nItems() == 0) return 0.0;

	double avg = 0.0, secs = 0.0, dx;
	int endX = runTimeEndAsX();
	double x1, x2;

	// Loop over segments of data
	for (int n=0; n<x_.nItems()-1; ++n)
	{
		// Clamp time limits of this region to start and end times of run
		x1 = x_[n];
		x2 = x_[n+1];
		if (x1 < 0) x1 = 0.0;
		else if (x1 > endX) x1 = endX;
		if (x2 < 0) x2 = 0.0;
		else if (x2 > endX) x2 = endX;

		// Now get time delta between x1 and x2, which will be the length of time that this value was 'measured' in the run
		dx = x2 - x1;
		if (dx > 1.0e-2)
		{
			secs += dx;
			avg += y_[n].y() * dx;
		}
	}
	if (secs > 1.0e-2)
	{
		avg /= secs;
		noValue = false;
	}
	return avg;
}

// Return minimum value over run begin/end
double Data2D::runMinimum(bool& noValue)
{
	noValue = true;

	double minimum = 1.0e99;
	int endX = runTimeEndAsX();
	for (int n=0; n<x_.nItems(); ++n)
	{
		if (x_[n] < 0) continue;
		else if (x_[n] > endX) break;

		noValue = false;
		if (y_[n].y() < minimum) minimum = y_[n].y();
	}

	return minimum;
}

// Return maximum value over run begin/end
double Data2D::runMaximum(bool& noValue)
{
	noValue = true;

	double maximum = -1.0e99;
	int endX = runTimeEndAsX();
	for (int n=0; n<x_.nItems(); ++n)
	{
		if (x_[n] < 0) continue;
		else if (x_[n] > endX) break;

		noValue = false;
		if (y_[n].y() > maximum) maximum = y_[n].y();
	}

	return maximum;
}

// Return standard deviation over run begin/end
double Data2D::runStandardDeviation()
{
	return 0.0;
}

// Calculate and return average over all points
double Data2D::average(bool& noValue)
{
	noValue = true;
	if (x_.nItems() == 0) return 0.0;

	double avg = 0.0, secs = 0.0, dx;
	int endX = runTimeEndAsX();
	double x1, x2;
	noValue = false;

	// Loop over segments of data
	for (int n=0; n<x_.nItems()-1; ++n)
	{
		x1 = x_[n];
		x2 = x_[n+1];

		// Now get time delta between x1 and x2, which will be the length of time that this value was 'measured' in the run
		dx = x2 - x1;
		if (dx > 1.0e-2)
		{
			secs += dx;
			avg += y_[n].y() * dx;
		}
	}

	return avg / secs;
}

// Return minimum value over all points
double Data2D::minimum(bool& noValue)
{
	noValue = true;
	if (y_.nItems() == 0) return 0.0;

	double minimum = y_[0].y();
	noValue = false;
	
	for (int n=1; n<y_.nItems(); ++n) if (y_[n].y() < minimum) minimum = y_[n].y();
	
	return minimum;
}

// Return maximum value over all points
double Data2D::maximum(bool& noValue)
{
	noValue = true;
	if (y_.nItems() == 0) return 0.0;

	double maximum = y_[0].y();
	noValue = false;
	
	for (int n=1; n<y_.nItems(); ++n) if (y_[n].y() > maximum) maximum = y_[n].y();
	
	return maximum;
}

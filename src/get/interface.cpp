/*
	*** GetRoutines Interface
	*** src/get/interface.cpp
	Copyright T. Youngs 2012-2014

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

#include "get/interface.h"
#include <messenger.hui>
#include <cstring>
#include <cstdio>
#include <cassert>

// Constructor
GetInterface::GetInterface(const char* rawFile)
{
	openErrorCode_ = -1;
	lastErrorCode_ = -1;

	// Safety checks
	if (sizeof(int) != (4*sizeof(char)))
	{
		msg.print("GetInterface() : sizeof(int) != sizeof(char)*4\n");
		// error = true;
	}

	// Open the specified file....
	rawFileLength_ = strlen(rawFile);
	rawFile_ = new char[rawFileLength_+1];
	strcpy(rawFile_, rawFile);
	open_data_file_(rawFile_, &nTimeChannels_, &nDetectors_, &nUserTables_, &openErrorCode_, rawFileLength_);

	// Retrieve other information if we successfully opened the file
	if (openErrorCode_ == 0)
	{
		// Get run parameter block
		getParameterArray(GetInterface::RPB, runParameterBlock_, 32);
	}
	else
	{
		msg.print("Error (%i) opening rawFile '%s' in GetInterface().\n", openErrorCode_, rawFile_);
	}
}

// Destructor
GetInterface::~GetInterface()
{
}


/*
 * Basic Information
 */

// Return error code reported on file open
int GetInterface::openErrorCode()
{
	return openErrorCode_;
}

// Return number of time channels present in raw file
int GetInterface::nTimeChannels()
{
	return nTimeChannels_;
}

// Return number of detectors present in raw file
int GetInterface::nDetectors()
{
	return nDetectors_;
}

// Return number of user tables present in raw file
int GetInterface::nUserTables()
{
	return nUserTables_;
}

// Return error code reported by last routine
int GetInterface::lastErrorCode()
{
	return lastErrorCode_;
}

// Return run parameter information array
int* GetInterface::runParameterBlock()
{
	return runParameterBlock_;
}

/*
 * Convenience Functions
 */

// Retrieve parameter as integer
int GetInterface::getIntegerParameter(GetInterface::ParameterName param)
{
	if (openErrorCode_ != 0)
	{
		msg.print("GetInterface::getIntegerParameter() : file not open.\n");
		return 0;
	}

	int value, arrayUsed;
	int arraySize = 1;
	getpari_(rawFile_, parameterName(param), &value, &arraySize, &arrayUsed, &lastErrorCode_, rawFileLength_, parameterNameLength(param));
	if (lastErrorCode_ != 0)
	{
		msg.print("Error (%i) reading integer parameter '%s' from rawFile '%s'.\n", lastErrorCode_, parameterName(param), rawFile_);
		return 0;
	}
	return value;
}

// Retrieve parameter as double
double GetInterface::getDoubleParameter(GetInterface::ParameterName param)
{
	if (openErrorCode_ != 0)
	{
		msg.print("GetInterface::getDoubleParameter() : file not open.\n");
		return 0;
	}

	int arrayUsed;
	int arraySize = 1;
	float tempVariable = 0.0;
	getparr_(rawFile_, parameterName(param), &tempVariable, &arraySize, &arrayUsed, &lastErrorCode_, rawFileLength_, parameterNameLength(param));
	double result = (double) tempVariable;
	if (lastErrorCode_ != 0)
	{
		msg.print("Error (%i) reading integer parameter '%s' from rawFile '%s'.\n", lastErrorCode_, parameterName(param), rawFile_);
		return 0.0;
	}
	return result;
}

// Retrieve parameter (as character array)
char* GetInterface::getCharParameter(GetInterface::ParameterName param)
{
	if (openErrorCode_ != 0)
	{
		msg.print("GetInterface::getCharParameter() : file not open.\n");
		return 0;
	}

	int arrayUsed;
	int arraySize = 511;
	static char value[512];
	value[0] = '\0';
	value[parameters_[param].primaryArraySize] = '\0';
	getparc_(rawFile_, parameterName(param), value, &arraySize, &arrayUsed, &lastErrorCode_, rawFileLength_, parameterNameLength(param), parameters_[param].primaryArraySize);
	if (lastErrorCode_ != 0)
	{
		msg.print("Error (%i) reading char* parameter array %s from rawFile '%s'.\n", lastErrorCode_, parameterName(param), rawFile_);
		return NULL;
	}
	return value;
}

// Retrieve parameter array (as integer array)
bool GetInterface::getParameterArray(GetInterface::ParameterName param, int* destinationArray, int destinationArraySize)
{
	if (openErrorCode_ != 0)
	{
		msg.print("GetInterface::getParameterArray(int) : file not open.\n");
		return 0;
	}

	int arrayUsed;
	getpari_(rawFile_, parameterName(param), destinationArray, &destinationArraySize, &arrayUsed, &lastErrorCode_, rawFileLength_, parameterNameLength(param));
	if (lastErrorCode_ != 0)
	{
		msg.print("Error (%i) reading int* parameter array %s from rawFile '%s'.\n", lastErrorCode_, parameterName(param), rawFile_);
		return false;
	}
	return true;
}

// Retrieve parameter array (as double array)
bool GetInterface::getParameterArray(GetInterface::ParameterName param, double* destinationArray, int destinationArraySize)
{
	if (openErrorCode_ != 0)
	{
		msg.print("GetInterface::getParameterArray(double) : file not open.\n");
		return 0;
	}

	int arrayUsed;
	float* tempArray = new float[destinationArraySize];
	getparr_(rawFile_, parameterName(param), tempArray, &destinationArraySize, &arrayUsed, &lastErrorCode_, rawFileLength_, parameterNameLength(param));
	if (lastErrorCode_ != 0)
	{
		msg.print("Error (%i) reading float* parameter array %s from rawFile '%s'.\n", lastErrorCode_, parameterName(param), rawFile_);
		return false;
	}
	// Implicit conversion from float to double
	for (int n=0; n<destinationArraySize; ++n) destinationArray[n] = tempArray[n];
	return true;
}

/*
 * Conversion Routines
 */

// Return character string from integer array
char* GetInterface::intToChar(int* sourceArray, int sourceLength)
{
	static char result[512];
	int stringLength = sourceLength*4;
	if (stringLength >= 512)
	{
		msg.print("GetInterface::intToChar() : Not enough room to store resulting string.\n");
		return NULL;
	}

	// Copy character data from int array, utilising reinterpret_cast
	strncpy(result, (reinterpret_cast<const char*>(sourceArray)), stringLength);

	// Add on terminator
	result[stringLength] = '\0';
	return result;
}

// Return double value converted from VAX float (stored in int)
double GetInterface::intToDouble(int sourceValue)
{
	// Copy and reinterpret data in sourceValue
	float f = *reinterpret_cast<float*>(&sourceValue);
	int length = 1, ecode;
	vaxf_to_local_(&f, &length, &ecode);

	// Return and implicit cast to double
	return f;
}

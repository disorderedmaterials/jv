/*
	*** GetRoutines Interface
	*** src/get/interface.h
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

// Basic wrappers to Fortran routines
#ifdef __cplusplus
extern"C" {
#endif
	// Open file for reading
	void open_data_file_(char* rawFile, int* nTimeChannels, int* nDetectors, int* nUserTables, int* errorCode, int rawFileLength);

	// Extract run parameters
	void getpari_(char* rawFile, char* parameterName, int* inputArray, int* inputArrayLength, int* inputArrayUsedLength, int* errorCode, int rawFileLength, int parameterNameLength);
	void getparr_(char* rawFile, char* parameterName, float* inputArray, int* inputArrayLength, int* inputArrayUsedLength, int* errorCode, int rawFileLength, int parameterNameLength);
	void getparc_(char* rawFile, char* parameterName, char* inputArray, int* inputArrayLength, int* inputArrayUsedLength, int* errorCode, int rawFileLength, int parameterNameLength, int CinputArrayLength);

	// Floating point conversion
	int vaxf_to_local_(float* sourceArray, int* sourceArrayLength, int* errorCode);
#ifdef __cplusplus
}
#endif

// SectionParameter
class SectionParameter
{
	public:
	// Type enum
	enum ParameterType
	{
		Character,
		Integer,
		Float
	};
	// Array size enum
	enum ParameterArraySize
	{
		NMON = -1,
		NDET = -2,
		NTC = -3,
		NTCSP = -4
	};

	public:
	// Parameter name
	char name[5];
	// Parameter name length
	int nameLength;
	// Parameter type
	ParameterType type;
	// Parameter primary array size
	int primaryArraySize;
	// Second array size
	int secondaryArraySize;
};

// Helper class for getroutines
class GetInterface
{
	public:
	// Constructor
	GetInterface(const char* rawFile);
	// Destructor
	~GetInterface();

	/*
	 * Parameters
	 */
	private:
	// Array of instrument data
	static SectionParameter parameters_[];

	public:
	// Parameter Names
	enum ParameterName
	{ 
		HDR,	// HEADER BLOCK                       
		VER1,	// FORMAT version number
		ADD,	// SECTION addresses

		VER2,	// RUN section version #
		RUN,	// RUN #
		TITL,	// RUN title
		USER,	// USER information
		RPB,	// RUN parameter block
		RRPB,	// RUN parameter block (as reals)

		VER3,	// INSTRUMENT section version #
		NAME,	// INSTRUMENT name
		IVPB,	// INSTRUMENT parameter block
		NDET,	// No. of detectors
		NMON,	// No. of monitors                    
		NUSE,	// NO. of USER defined (UTn) tables  ***B
		MDET,	// spectrum no. for each monitor      
		MONP,	// prescale value for each monitor    
		SPEC,	// SPECTRUM # table
		DELT,	// HOLD OFF table                    ***B
		LEN2,	// L2         table
		CODE,	// CODE for UTn tables               ***B
		TTHE,	// 2Theta     table (scattering ang)
		UT1,	// USER defined table 1              ***B ..cont.	to	UTn

		VER4,	// SE section version #
		SPB,	// SAMPLE parameter block
		NSEP,	// number of controlled SEPs
		SE01,	// SE parameter block #1 ..cont.	to	SEnn

		VER5,	// DAE section version #
		DAEP,	// DAE parameter block
		CRAT,	// crate no.for each detector
		MODN,	// module no. for each detector
		MPOS,	// position in module for each detector
		TIMR,	// TIME REGIME # table
		UDET,	// 'USER detector #' for each detector

		VER6,	// TCB secton version #
		NTRG,	// # of time regimes (normally =1)
		NFPP,	// No. of frames per period           
		NPER,	// No. of periods                     
		PMAP,	// period # for each basic period     
		NSP1,	// No. of spectra for time reg.= 1(tr=1)
		NTC1,	// No. of time chan.              (tr=1)
		TCM1,	// time channel mode (0,1,2, ..) (tr=1) *A
		TCP1,	// time channel parameters       (tr=1) *A
		PRE1,	// prescale value for 32MHz clock (tr=1)
		TCB1,	// Time channel boundaries        (tr=1)
// 		USER,	// User defined (max length =400 R*4 wds.)

		VER7,	// DATA version #
		DAT1	// (ntc1,nsp1)	raw data for first time regime
	};
	// Return string corresponding to parameter name
	static char* parameterName(ParameterName pname);
	// Return length of parameter name
	int parameterNameLength(ParameterName pname);


	/*
	 * Basic Information
	 */
	private:
	// Name of rawfile targetted by this instance
	char* rawFile_;
	// Length of rawFile_ name
	int rawFileLength_;
	// Error code reported on file open
	int openErrorCode_;
	// Number of time channels present in raw file
	int nTimeChannels_;
	// Number of detectors present in raw file
	int nDetectors_;
	// Number of user tables present in raw file
	int nUserTables_;
	// Error code reported by last routine
	int lastErrorCode_;
	// Run parameter information
	int runParameterBlock_[32];

	public:
	// Return error code reported on file open
	int openErrorCode();
	// Return number of time channels present in raw file
	int nTimeChannels();
	// Return number of detectors present in raw file
	int nDetectors();
	// Return number of user tables present in raw file
	int nUserTables();
	// Return error code reported by last routine
	int lastErrorCode();
	// Return run parameter information array
	int* runParameterBlock();


	/*
	 * Convenience Functions
	 */
	public:
	// Retrieve parameter as int
	int getIntegerParameter(GetInterface::ParameterName param);
	// Retrieve parameter as double
	double getDoubleParameter(GetInterface::ParameterName param);
	// Retrieve parameter (as characterarray)
	char* getCharParameter(GetInterface::ParameterName param);
	// Retrieve parameter array (as integer array)
	bool getParameterArray(GetInterface::ParameterName param, int* destinationArray, int destinationArraySize);
	// Retrieve parameter array (as double array)
	bool getParameterArray(GetInterface::ParameterName param, double* destinationArray, int destinationArraySize);


	/*
	 * Conversion Routines
	 */
	public:
	// Return character string from integer array
	static char* intToChar(int* sourceArray, int sourceLength);
	// Return double value(s) converted from VAX float (stored in int)
	static double intToDouble(int sourceValue);
};

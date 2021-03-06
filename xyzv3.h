#ifndef XYZV3_H
#define XYZV3_H

// forward declerations

class Serial; // from serial.h

// some functions require user interaction
// use a callback to handle that
typedef void (*XYZCallback)(void);

// printer status word
// items marked with state have substatus words as well.
enum XYZPrintStatus
{
	PRINT_INITIAL = 9500,
	PRINT_HEATING = 9501,
	PRINT_PRINTING = 9502,
	PRINT_CALIBRATING = 9503,
	PRINT_CALIBRATING_DONE = 9504,
	PRINT_IN_PROGRESS = 9505,
	PRINT_COOLING_DONE = 9506,
	PRINT_COOLING_END = 9507,
	PRINT_ENDING_PROCESS = 9508,
	PRINT_ENDING_PROCESS_DONE = 9509,
	PRINT_JOB_DONE = 9510,
	PRINT_NONE = 9511,
	PRINT_STOP = 9512,
	PRINT_LOAD_FILAMENT = 9513,
	PRINT_UNLOAD_FILAMENT = 9514,
	PRINT_AUTO_CALIBRATION = 9515,
	PRINT_JOG_MODE = 9516,
	PRINT_FATAL_ERROR = 9517,

	STATE_PRINT_FILE_CHECK = 9520,

	STATE_PRINT_LOAD_FIALMENT = 9530,
	STATE_PRINT_UNLOAD_FIALMENT = 9531,
	STATE_PRINT_JOG_MODE = 9532,
	STATE_PRINT_FATAL_ERROR = 9533,
	STATE_PRINT_HOMING = 9534,
	STATE_PRINT_CALIBRATE = 9535,
	STATE_PRINT_CLEAN_NOZZLE = 9536,
	STATE_PRINT_GET_SD_FILE = 9537,
	STATE_PRINT_PRINT_SD_FILE = 9538,
	STATE_PRINT_ENGRAVE_PLACE_OBJECT = 9539,
	STATE_PRINT_ADJUST_ZOFFSET = 9540,

	PRINT_TASK_PAUSED = 9601,
	PRINT_TASK_CANCELING = 9602,

	STATE_PRINT_BUSY = 9700,

	STATE_PRINT_SCANNER_IDLE = 9800,
	STATE_PRINT_SCANNER_RUNNING = 9801,
};

// status of current printer
struct XYZStatus
{
	bool isValid;

	int bedTemp_C;
	int extruderActualTemp_C;
	int extruderTargetTemp_C;

	int fillimantRemaining_mm;
//	int materialType;
	int isFillamentPLA;
	int nozelID;
	float nozelDiameter_mm;

	int calib[9];
	bool autoLevelEnabled;
	bool buzzerEnabled;

	int printPercentComplete;
	int printElapsedTime_m;
	int printTimeLeft_m;

	int errorStatus;
	int printerStatus; // XYZPrintStatus
	int printerSubStatus;

	int packetSize;

	int printerLifetimePowerOnTime_min;
	int printerLastPowerOnTime_min;
	int extruderLifetimePowerOnTime_min;

	char lang[32]; //4

	char machineName[64]; //32
	char machineModelNumber[64]; //12

	char machineSerialNum[64]; //20
	char filamentSerialNumber[64]; //16
	char nozelSerialNumber[64]; //32

	char firmwareVersion[64]; //10

	// m ???
	// o t1, c1
	// s ???
	// z ???
	// W
	// 4
};

// info on each printer type
struct XYZPrinterInfo
{
	// string constants
	const char *modelNum;
	const char *fileNum;
	const char *serialNum;
	const char *screenName;

	bool fileIsV5; // v2 is 'normal' file format, v5 is for 'pro' systems
	bool fileIsZip; // older file format, zips header
	bool comIsV3; // old or new serial protocol, v3 is new

	// build volume
	int length;
	int width;
	int height;
};

// master class
// most functions will block while serial IO happens
class XYZV3
{
public:
	XYZV3();

	//===========================
	// serial communication

	// port is com port number, -1 is auto detect
	bool connect(int port = -1);
	void disconnect();
	bool isConnected(); // return true if connected
	bool checkConnection(); // connect if not connected

	// update status struct
	bool updateStatus();
	// update status struct and print to console
	bool printStatus();

	// run auto bed leveling routine
	bool calibrateBed(XYZCallback cbPress, XYZCallback cbRelease);

	// heats up nozzle so you can clean it out with a wire
	bool cleanNozzle(XYZCallback cbDone);

	// home printer so you can safely move head
	bool homePrinter();

	// move head in given direction, axis is one of x,y,z
	bool jogPrinter(char axis, int dist_mm);

	// unloads fillament without any user interaction
	bool unloadFillament();

	// loads fillament, be sure to hit enter when fillament comes out of extruder!!!
	bool loadFillament(XYZCallback ldDone);

	// increment/decrement z offset by one step (1/100th of  a mm?)
	int incrementZOffset(bool up);
	int getZOffset(); // in hundredths of a mm
	bool setZOffset(int offset);

	bool restoreDefaults();
	bool enableBuzzer(bool enable);
	bool enableAutoLevel(bool enable);
	bool setLanguage(const char *lang); //lang is one of en, fr, it, de, es, jp

	bool cancelPrint();
	bool pausePrint();
	bool resumePrint();
	bool readyPrint();

	// send a .3w file to printer
	bool printFile(const char *path, XYZCallback cbStatus);
	bool printString(const char *data, int len, XYZCallback cbStatus);

	// dump current status of print job to screen, returns false when job is finished
	bool monitorPrintJob();

	// load new firmware into the printer.
	// probably not a good idea to mess with this!
	bool writeFirmware(const char *path);

	//=====================
	// file i/o

	const XYZPrinterInfo* modelToInfo(const char *modelNum);

	// convert a gcode file to 3w format
	//****FixMe, modify encryptFile to take either file bools or machine info or at least override format in some way.
	bool encryptFile(const char *inPath, const char *outPath = NULL, const XYZPrinterInfo *info = NULL);

	// convert a 3w file to gcode
	bool decryptFile(const char *inPath, const char *outPath = NULL);

protected:
	void setCursorXY(int x, int y);

	float nozelIDToDiameter(int id);
	const char* statusCodesToStr(int status, int subStatus);
	bool getJsonVal(const char *str, const char *key, char *val);

	// serial functions
	const char* waitForLine(bool waitForEndCom, int timeout_s = 10);
	bool waitForVal(const char *val, bool waitForEndCom, int timeout_s = 10);
	bool waitForJsonVal(const char *key, const char *val, bool waitForEndCom, int timeout_s = 10);

	// file functions
	unsigned int swap16bit(unsigned int in);
	unsigned int swap32bit(unsigned int in);

	int readWord(FILE *f);
	void writeWord(FILE *f, int i);
	int readByte(FILE *f);
	void writeByte(FILE *f, char c);
	void writeRepeatByte(FILE *f, char byte, int count);

	int roundUpTo16(int in);
	void pkcs7unpad(char *buf, int len);
	int pkcs7pad(char *buf, int len);

	unsigned int calcXYZcrc32(char *buf, int len);
	const char* readLineFromBuf(const char* buf, char *lineBuf, int lineLen);
	bool checkLineIsHeader(const char* lineBuf);
	bool processGCode(const char *gcode, const int gcodeLen, const XYZPrinterInfo *info,  char **headerBuf, int *headerLen, char **bodyBuf, int *bodyLen);

	Serial m_serial;
	XYZStatus m_status;
	const XYZPrinterInfo *m_info;

	static const int m_infoArrayLen = 19;
	static const XYZPrinterInfo m_infoArray[m_infoArrayLen];
};


#endif // XYZV3_H
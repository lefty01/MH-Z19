/*   Version: 1.5.3  |  License: LGPLv3  |  Author: JDWifWaf@gmail.com   */

#ifndef MHZ19_H
#define MHZ19_H

#include <Arduino.h>

#ifdef ESP32
#include "esp32-hal-log.h"
#define TAG_MHZ19 "MH-Z19"
#endif

#define MHZ19_ERRORS 1			// Set to 0 to disable error prints
#define TEMP_ADJUST 40			// This is the value used to adjust the temperature.
#define TIMEOUT_PERIOD 500		// Time out period for response (ms)
#define DEFAULT_RANGE 2000		// For range function (sensor works best in this range)
#define MHZ19_DATA_LEN 9		// Data protocol length

// Command bytes -------------------------- //
#define MHZ19_ABC_PERIOD_OFF    0x00
#define MHZ19_ABC_PERIOD_DEF    0xA0

/* enum alias for error code definitions */
enum ERRORCODE
{
	RESULT_NULL = 0,
	RESULT_OK = 1,
	RESULT_TIMEOUT = 2,
	RESULT_MATCH = 3,
	RESULT_CRC = 4,
	RESULT_FILTER = 5
};

class MHZ19
{
  public:
	/*###########################-Variables-##########################*/

	/* Holds last received error code from recieveResponse() */
	byte errorCode;

	/* for keeping track of the ABC run interval */
	unsigned long ABCRepeatTimer;

	/*#####################-Initiation Functions-#####################*/

	/* essential begin, return 0 on success, non-zero on error */
	int begin(Stream &stream);

	/*########################-Set Functions-##########################*/

	/* Sets Range to desired value*/
	void setRange(int range = 2000);

	/* Sets Span to desired value below 10,000*/
	void zeroSpan(int span = 2000);

    /* Sets "filter mode" to ON or OFF & mode type (see example) */
	void setFilter(bool isON = true, bool isCleared = true);

	/*########################-Get Functions-##########################*/

	/* request CO2 values, 2 types of CO2 can be returned, isLimted = true (command 134) and is Limited = false (command 133) */
	int getCO2(bool isunLimited = true, bool force = true);

	/* returns the "raw" CO2 value of unknown units */
	unsigned int getCO2Raw(bool force = true);

	/* returns Raw CO2 value as a % of transmittance */		//<--- needs work to understand
	float getTransmittance(bool force = true);

	/*  returns temperature using command 133 or 134 */
	float getTemperature(bool force = true);

	/* reads range using command 153 */
	int getRange();

	/* reads ABC-Status using command 125 / 0x7D */
	bool getABC();

	/* Returns accuracy value if available */
	byte getAccuracy(bool force = true);

	/* not yet implemented */
	byte getPWMStatus();

	/* returns MH-Z19 version using command 160, to the entered array */
	void getVersion(char rVersion[]);

	/* returns background CO2 used by sensor using command 156 */
	int getBackgroundCO2();

	/* returns temperature using command 163 (Note: this library deducts -2 when the value is used) */
	byte getTempAdjustment();

	/* returns last recorded response from device using command 162 */
	byte getLastResponse(byte bytenum);

	/*######################-Utility Functions-########################*/

	/* ensure communication is working (included in begin())
	 * return 0 on success, non-zero on error
	 */
	int verify();

	/* disables calibration or sets ABCPeriod */
	void autoCalibration(bool isON = true, byte ABCPeriod = 24);

	/* Calibrates "Zero" (Note: Zero refers to 400ppm for this sensor)*/
	void calibrate();

	/*  Calibrate Backwards compatibility */
	void inline calibrateZero(){ calibrate(); };

	/* requests a reset */
	void recoveryReset();

	/* use to show communication between MHZ19 and  Device */
	void printCommunication(bool isDec = true, bool isPrintComm = true);

  private:
	/*###########################-Variables-##########################*/

	/* pointer for Stream class to accept reference for hardware and software ports */
  Stream* mySerial;

  /* alias for command types */
	typedef enum COMMAND_TYPE
	{
		RECOVER = 0,			// 0 Recovery Reset
		ABC = 1,				// 1 ABC (Automatic Baseline Correction) Mode ON/OFF
		GETABC = 2,				// 2 Get ABC - Status 0x79
		RAWCO2 = 3,				// 3 Raw CO2
		CO2UNLIM = 4,			// 4 Temperature for unsigned, CO2 Unlimited
		CO2LIM = 5,				// 5 Temperature for signed, CO2 limited
		ZEROCAL = 6,			// 6 Zero Calibration
		SPANCAL = 7,			// 7 Span Calibration
		RANGE = 8,				// 8 Range
		GETRANGE = 9,			// 9 Get Range
		GETCALPPM = 10,			// 10 Get Background CO2
		GETFIRMWARE = 11,		// 11 Get Firmware Version
		GETLASTRESP = 12,		// 12 Get Last Response
		GETEMPCAL = 13			// 13 Get Temperature Calibration
	} Command_Type;

	/* Memory Pool */
	struct mempool
	{
		struct config
		{
			bool ABCRepeat = false;					// A flag which represents whether auto calibration ABC period was checked
			bool filterMode = false;				// Flag set by setFilter() to signify is "filter mode" was made active
			bool filterCleared = true;				// Additional flag set by setFilter() to store which mode was selected
			bool printcomm = false;					// Communication print options
			bool _isDec = true;						// Holds preference for communication printing
			uint8_t fw_ver = 0;                     // holds the major version of the firmware
		} settings;

		byte constructedCommand[MHZ19_DATA_LEN];	// holder for new commands which are to be sent

		struct indata
		{
			byte CO2UNLIM[MHZ19_DATA_LEN];			// Holds command 133 response values "CO2 unlimited and temperature for unsigned"
			byte CO2LIM[MHZ19_DATA_LEN];			// Holds command 134 response values "CO2 limited and temperature for signed"
			byte RAW[MHZ19_DATA_LEN];				// Holds command 132 response values "CO2 Raw"
			byte STAT[MHZ19_DATA_LEN];				// Holds other command response values such as range, background CO2 etc
		} responses;

	} storage;

	/*######################-Internal Functions-########################*/

	/* Coordinates  sending, constructing and receiving commands */
	void provisioning(Command_Type commandtype, int inData = 0);

	/* Constructs commands using command array and entered values */
	void constructCommand(Command_Type commandtype, int inData = 0);

	/* generates a checksum for sending and verifying incoming data */
	byte getCRC(byte inBytes[]);

	/* Sends commands to the sensor */
	void write(byte toSend[]);

	/* Call retrieveData to retrieve values from the sensor and check return code */
	byte read(byte inBytes[9], Command_Type commandnumber);

	/* Assigns response to the correct communication arrays */
	void handleResponse(Command_Type commandtype);

	/* prints sending / receiving messages if enabled */
	void printstream(byte inbytes[9], bool isSent, byte pserrorCode);

	/* Checks whether time elapse for next ABC OFF cycle has occurred */
	void ABCCheck();

	/* converts integers to bytes according to /256 and %256 */
	void makeByte(int inInt, byte *high, byte *low);

	/* converts bytes to integers according to *256 and + value */
	unsigned int makeInt(byte high, byte low);

	void cleanUp(uint8_t cnt);
};
#endif

#ifndef FUNCTIONSSD_H
#define FUNCTIONSSD_H

#include <Arduino.h>
#include <DS3231.h>
#include <SdFat.h>

/*!
 * @function    dateFileName
 * @abstract    Generate the filename with the date given
 * @discussion  Takes the day,month and year put in the char
				array  given with "log.DD-MM-YYYY.csv" format
 * @param       FileName    Char pointer for store filename
 * @param       day     	Day for timestamp
 * @param       month     	Month for timestamp
 * @param       year     	Year for timestamp
*/
void DateFileName(char * FileName, int day, int month, int year);

/*!
 * @function    dateFileExist
 * @abstract    Check if certains date file exist on SD
 * @discussion  This function check for existence of file
				(with standard format) on SD, uses dateFileName
				for generate name
 * @param       SDhandler   Handler to SdFat for use
 * @param       day     	Day for timestamp
 * @param       month     	Month for timestamp
 * @param       year     	Year for timestamp
 * @result      Returns 1 if exist, 0 if not or error
*/
int DateFileExist(SdFat SDhandler, int day, int month, int year);

/*!
 * @function    getReadableTime
 * @abstract    Translate from seconds to HH:MM:SS string
 * @discussion  This function take an certains number of seconds and
				translate it to HH:MM:SS format, It do not check for
				hours>24, this means days are not included
 * @param       seconds    Seconds to translate
 * @result      String with readable HH:MM:SS
*/
int GetReadableTime(unsigned long seconds,char * buff, int buffSize);

/*!
 * @function    transDoW
 * @abstract    Translate from number of week to day's name
 * @discussion  This function take an input number of week day, starting from monday
				and returns day's name, or ERROR if day < 1 or day > 7.
 * @param       day    Day of the week
 * @param		buff		buffer to store string
 * @param		buffSize	buffer size (not in use)
 * @result      1 for Ok, 0 for Error
*/
int TransDoW(int day, char * buff, int buffSize);

/*!
 * @function    printDateTimeOverUART
 * @abstract    Takes DS3231 clock time and print it over UART
 * @discussion  This Function takes a DS3231 handler and print over Serial UART
				It needs Serial already init
 * @param       DS3231 handler
*/
void PrintDateTimeOverUART(DS3231 _reloj);

/*!
 * @function    setTime
 * @abstract    Function that block execution and set time over UART
 * @discussion  This Function takes a string over UART in format:
				"YYYY MM DD DOW HH MM SS" and set it on ds3231 handler
				It needs Serial already init
 * @param       DS3231 handler
*/
void SetTime(DS3231 _reloj);

/*!
 * @function    transDoW
 * @abstract    Translate from number of event to event's name
 * @discussion  This function translate the number of event to envents name
				0 --> OFF
				1 --> ON
				2 --> RESET
				3 --> AUTORST
				4 --> START
				default --> ERROR
 * @param       event   	int Number of the event
 * @param		buff		buffer to store string
 * @param		buffSize	buffer size (not in use)
 * @result      1 for Ok, 0 for Error
*/
int TransTypeEvent(int event, char * buff, int buffSize);


#endif

#define SET_PIN 3
#define BUTTON_PIN 6
#define CHAIN_IN_PIN 7
#define INT_PIN 2
#define LED_PIN 13

#include <Arduino.h>

#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DS3231.h>
#include <SdFat.h>
#include <CircularBuffer.h>
#include "structs.h"
#include "functionsSD.h"

//Declarations of internal functions
void CounterISR(void);
int AddToBuffer(byte event);
int AddToDailyBuffer(void);
int WriteBuffers(void);
int BufferToFile(logSD_t bufferSample, File &fileOutput);
int DailyBufferToFile(dailyLogSD_t bufferSample, File &fileOutput);
void dateTime(uint16_t *date, uint16_t *time);

//Declare SdFat type as SD so we can use Arduino SD library as "transparent"
SdFat SD;
//Liquid Crystal I2C creation
LiquidCrystal_I2C lcd(0x27, 16, 2);
//RTC library types creations
RTClib myRTC;
DS3231 reloj;

//VARIABLES
//For check chain TURN ON or OFF
int chainStatus = 0;
int lastChainStatus = 0;
//For check button pressed, and backlight control
int buttonStatus = 0;
int lastButtonStatus = 0;
int buttonPressed = 0;
unsigned long lastMillis = 0;
//For store chain's seconds in use reseteable for the user and not reseteable for the user (at EoD)
volatile unsigned long secondsInUse = 0;
volatile unsigned long secondsInUseDaily = 0;
//DateTime datatype for RTC comunication
DateTime timeNow;
// Flag needed for reset SecondsInUse and SecondsInUse at EoD, and store values on SD
int autoResetFlag = 0;
int alreadyResetFlag = 0;

//CircularBuffers for save logs and Daily logs on SD
CircularBuffer<logSD_t, 15> bufferForSD;
CircularBuffer<dailyLogSD_t, 3> dailyBufferForSD;

void setup()
{
	//Sets inputs
	pinMode(SET_PIN, INPUT_PULLUP);
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	pinMode(CHAIN_IN_PIN, INPUT_PULLUP);
	pinMode(INT_PIN, INPUT);
	pinMode(LED_PIN, OUTPUT);

	Serial.begin(115200);
	Wire.begin();
	delay(300);

	reloj.setClockMode(false); //24HS mode
	lcd.begin(16, 2);
	lcd.clear();
	delay(200);
	if (digitalRead(SET_PIN) == 0)
	{ //If We need to set time
		lcd.print("SETTING");
		Serial.println(F("Setting Time Date"));
		SetTime(reloj);
	}

	reloj.enableOscillator(true, false, 0); //Start Osc on DS3231 at 1Hz

	timeNow = myRTC.now();
	Serial.println(F("Time Now:"));
	PrintDateTimeOverUART(reloj);
	Serial.println(F("Starting Operation"));
	//SAVE TIMESTAMP POWERON
	AddToBuffer(4);
	WriteBuffers();
	Serial.println(F("Power-ON timestamp saved"));
	attachInterrupt(digitalPinToInterrupt(INT_PIN), CounterISR, RISING);
}

void loop()
{

	char strtmp[10];
	//For RESET acumulator
	timeNow = myRTC.now();

	if ((timeNow.hour() == 23) && (timeNow.minute() == 59) && (timeNow.second() > 50))
	{
		autoResetFlag = true;
	}
	else
	{
		autoResetFlag = false;
		alreadyResetFlag = false;
	}
	if (autoResetFlag && !alreadyResetFlag)
	{
		AddToBuffer(3); //Add AutoReset Event
		AddToDailyBuffer();
		WriteBuffers();
		Serial.println(F("Auto-Reset timestamp saved"));
		secondsInUse = 0;
		secondsInUseDaily = 0;
		alreadyResetFlag = true;
	}

	chainStatus = !digitalRead(CHAIN_IN_PIN);
	//If chain starts
	if ((chainStatus == 1) && (lastChainStatus == 0))
	{
		//SAVE TIMESTAMP ON uSD
		AddToBuffer(1);
		WriteBuffers();
		Serial.println(F("Chain-ON timestamp saved"));
	}
	//If chain stops
	if ((chainStatus == 0) && (lastChainStatus == 1))
	{
		//SAVE TIMESTAMP OFF uSD
		AddToBuffer(0);
		WriteBuffers();
		Serial.println(F("Chain-OFF timestamp saved"));
	}
	lastChainStatus = chainStatus;

	//For backlight control
	buttonStatus = !digitalRead(BUTTON_PIN);
	if (!lastButtonStatus && buttonStatus)
	{
		if (buttonPressed)
		{
			//SAVE TIMESTAMP MANUAL RESET
			AddToBuffer(2);
			WriteBuffers();
			Serial.println(F("Manual-Reset timestamp saved"));
			secondsInUse = 0;
			buttonPressed = false;
		}
		else
		{
			lcd.backlight();
			lastMillis = millis();
			buttonPressed = true;
		}
	}
	lastButtonStatus = buttonStatus;
	//FOR PREVENT MILLIS OVERFLOW
	if (millis() < lastMillis)
	{
		lastMillis = millis();
	}
	//For automatic TurnOff display backlight
	if ((millis() - lastMillis) > 20000)
	{
		lcd.noBacklight();
		buttonPressed = false;
	}

	lcd.clear();
	lcd.print(F("Tiempo ON:"));
	lcd.setCursor(4, 1);
	GetReadableTime(secondsInUse, strtmp, sizeof(strtmp));
	lcd.print(strtmp);
	delay(200);
}

//ISR function attached to 1Hz clock oscillator
void CounterISR(void)
{
	secondsInUse += !digitalRead(CHAIN_IN_PIN);
	secondsInUseDaily += !digitalRead(CHAIN_IN_PIN);
}

int AddToBuffer(byte event)
{
	logSD_t auxBuffer;
	auxBuffer.segundos = timeNow.second();
	auxBuffer.minutos = timeNow.minute();
	auxBuffer.horas = timeNow.hour();
	auxBuffer.dia = timeNow.day();
	auxBuffer.mes = timeNow.month();
	auxBuffer.anio = timeNow.year() - 2000;
	auxBuffer.tipoEvento = event;
	auxBuffer.secondsInUse = secondsInUse;
	return bufferForSD.push(auxBuffer);
}
int AddToDailyBuffer(void)
{
	dailyLogSD_t auxBuffer;
	auxBuffer.dia = timeNow.day();
	auxBuffer.mes = timeNow.month();
	auxBuffer.anio = timeNow.year() - 2000;
	auxBuffer.secondsInUse = secondsInUseDaily;
	return dailyBufferForSD.push(auxBuffer);
}

int WriteBuffers(void)
{
	//If SD is not present
	if (!SD.begin(SS))
	{
		return -1;
	}
	//Two auxiliar structs for read buffer info, a filename char array, and a file handler
	logSD_t auxForSD;
	dailyLogSD_t auxDailyForSD;
	char fileName[20];
	File loggerFile;
	while (bufferForSD.size() > 0)
	{									//If buffer is not empty
		auxForSD = bufferForSD.shift(); //Retrieve first element loaded on buffer,
		delay(100);
		DateFileName(fileName, auxForSD.dia, auxForSD.mes, auxForSD.anio); //Create the filename
		SdFile::dateTimeCallback(dateTime);
		if (!SD.exists(fileName))
		{
			loggerFile = SD.open(fileName, FILE_WRITE);
			loggerFile.println(F("HORA;EVENTO;ACUMULADO;NROEVENTO"));
			loggerFile.close();
		}
		loggerFile = SD.open(fileName, FILE_WRITE); //And open it, if it doesnt exist it will create it
		BufferToFile(auxForSD, loggerFile);
		loggerFile.close();
		delay(100);
	}
	while (dailyBufferForSD.size() > 0)
	{
		auxDailyForSD = dailyBufferForSD.shift();
		SdFile::dateTimeCallback(dateTime);
		if (!SD.exists(F("Acumulados Diarios.csv")))
		{
			loggerFile = SD.open(F("Acumulados Diarios.csv"), FILE_WRITE);
			loggerFile.println(F("FECHA;ACUMULADO"));
			loggerFile.close();
			delay(100);
		}
		loggerFile = SD.open(F("Acumulados Diarios.csv"), FILE_WRITE);
		DailyBufferToFile(auxDailyForSD, loggerFile);
		loggerFile.close();
	}
	return 0;
}

int BufferToFile(logSD_t bufferSample, File &fileOutput)
{
	char localbuffer[40];
	char auxiliar1[12];
	char auxiliar2[12];
	TransTypeEvent(bufferSample.tipoEvento, auxiliar1, sizeof(auxiliar1));
	GetReadableTime(bufferSample.secondsInUse, auxiliar2, sizeof(auxiliar2));
	sprintf(localbuffer, "%02d:%02d:%02d;%s;%s;%d", bufferSample.horas, bufferSample.minutos, bufferSample.segundos, auxiliar1, auxiliar2, bufferSample.tipoEvento);
	fileOutput.println(localbuffer);
	return 1;
}

int DailyBufferToFile(dailyLogSD_t bufferSample, File &fileOutput)
{
	char auxiliar[16];
	if (bufferSample.dia < 10)
	{
		fileOutput.print("0");
	}
	fileOutput.print(bufferSample.dia);
	fileOutput.print("-");
	if (bufferSample.mes < 10)
	{
		fileOutput.print("0");
	}
	fileOutput.print(bufferSample.mes);
	fileOutput.print("-");
	fileOutput.print(bufferSample.anio + 2000);
	fileOutput.print(";");
	GetReadableTime(bufferSample.secondsInUse, auxiliar, sizeof(auxiliar));
	fileOutput.println(auxiliar);
	return 1;
}

//Function to be used on callback, for set files modification timestamp
void dateTime(uint16_t *date, uint16_t *time)
{
	DateTime now = myRTC.now();

	// return date using FAT_DATE macro to format fields
	*date = FAT_DATE(now.year(), now.month(), now.day());

	// return time using FAT_TIME macro to format fields
	*time = FAT_TIME(now.hour(), now.minute(), now.second());
}

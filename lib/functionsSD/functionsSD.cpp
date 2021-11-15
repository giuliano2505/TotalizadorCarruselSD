#include "functionsSD.h"



//Function for generate the filename with the date given
void DateFileName(char * localFileName, int day, int month, int year) {
	sprintf(localFileName, "Fecha.%02d-%02d-%d.csv", day, month, year + 2000);
	return 0;
}

//Check if certains date file exist on SD
int DateFileExist(SdFat SDhandler, int day, int month, int year) {
  char localFileName[25] = {'\0'};
  DateFileName(localFileName,day,month,year);
  return SDhandler.exists(localFileName);
}

//Translate from seconds to HH:MM:SS string
int GetReadableTime(unsigned long seconds,char * buff, int buffSize) {
  int minutes;
  int hours;
  char strtmp[4];

  minutes = seconds / 60;
  hours = minutes / 60;
  seconds %= 60;
  minutes %= 60;
  
  sprintf(buff, "%02d:%02d:%02d",hours,minutes,seconds);

  return 1;
}

//Function for translate number to day of week, start with Monday
int TransDoW(int day, char * buff, int buffSize) {
  switch (day) {
    case 1:
		strcpy(buff,"Lunes");
		return 1;
    case 2:
		strcpy(buff,"Martes");
		return 1;
    case 3:
		strcpy(buff,"Miercoles");
		return 1;
    case 4:
		strcpy(buff,"Jueves");
		return 1;
    case 5:
		strcpy(buff,"Viernes");
		return 1;
    case 6:
		strcpy(buff,"Sabado");
		return 1;
    case 7:
		strcpy(buff,"Domingo");
		return 1;
    default:
		strcpy(buff,"ERROR");
		return 0;
  }
}


void PrintDateTimeOverUART(DS3231 _reloj) {
  bool century = false;
  bool h12Flag = false;
  bool pmFlag = false;
  char auxiliar[10];
  TransDoW(_reloj.getDoW(),auxiliar,sizeof(auxiliar));
  Serial.print(auxiliar);
  Serial.print(" ");
  Serial.print(_reloj.getDate(), DEC);
  Serial.print('/');
  Serial.print(_reloj.getMonth(century), DEC);
  Serial.print('/');
  Serial.print(_reloj.getYear(), DEC);
  Serial.print(" ");
  Serial.print(_reloj.getHour(h12Flag, pmFlag), DEC); //24-hr
  Serial.print(":");
  Serial.print(_reloj.getMinute(), DEC);
  Serial.print(":");
  Serial.println(_reloj.getSecond(), DEC);
}

void SetTime(DS3231 _reloj) {
  int anio, mes, dia, dow, hh, mm, ss;
  Serial.println(F("Ingrese hora y fecha:"));
  Serial.println(F("YYYY MM DD DoW hh mm ss"));
  Serial.flush();
  while (1) {
    if (Serial.available() > 0) {
		delay(100);
      anio = Serial.parseInt() - 2000;
      mes = Serial.parseInt();
      dia = Serial.parseInt();
      dow = Serial.parseInt();
      hh = Serial.parseInt();
      mm = Serial.parseInt();
      ss = Serial.parseInt();
      _reloj.setYear(anio);
      _reloj.setMonth(mes);
      _reloj.setDate(dia);
      _reloj.setDoW(dow);
      _reloj.setHour(hh);
      _reloj.setMinute(mm);
      _reloj.setSecond(ss);
      PrintDateTimeOverUART(_reloj);
      while (1);
    }
  }
}


//Function for translate number of event to string
int TransTypeEvent(int event, char * buff, int buffSize) {
  switch (event) {
    case 0:
		strcpy(buff,"APAGADO");
		return 1;
    case 1:
		strcpy(buff,"ENCENDIDO");
		return 1;
    case 2:
		strcpy(buff,"RESETMANUAL");
		return 1;
    case 3:
		strcpy(buff,"AUTOMATICO");
		return 1;
    case 4:
		strcpy(buff,"POWERON");
		return 1;
    default:
		strcpy(buff,"ERROR");
		return 1;
  }
}


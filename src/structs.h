#ifndef STRUCTS_H
#define STRUCTS_H

#include <Arduino.h>

/*!
 * @struct      logSD_t
 * @abstract    Type of data for store logs
 * @discussion  This type of data is used for store datailed logs
 *              readys for SD write later.
 * @field       segundos      Seconds for timestamps
 * @field       minutos       Minutes for timestamps
 * @field       horas         Hours for timestamps
 * @field       dia           Days for timestamps
 * @field       mes           Months for timestamps
 * @field       anio          Years for timestamps
 * @field       tipoEvento    Type of event for store
 * @field       secondsInUse  Total seconds in use before event
*/
struct logSD_t {
	byte segundos;
	byte minutos;
	byte horas;
	byte dia;
	byte mes;
	byte  anio;
	byte tipoEvento;
	long secondsInUse;
};
typedef struct logSD_t logSD_t;



/*!
 * @struct      dailyLogSD_t
 * @abstract    Type of data for store total log at EoD
 * @discussion  This type of data is used for store EoD logs
 *              readys for SD write later.
 * @field       dia           Days for timestamps
 * @field       mes           Months for timestamps
 * @field       anio          Years for timestamps
 * @field       secondsInUse  Total seconds in use before event
*/
struct dailyLogSD_t {
	byte dia;
	byte mes;
	byte anio;
	long secondsInUse;
};
typedef struct dailyLogSD_t dailyLogSD_t;
#endif

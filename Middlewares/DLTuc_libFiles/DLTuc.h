/****************************************************************************
 * dlt_logs_mcu.h
 *
 *  Created on: 30 Jan 2023
 *      Author: teodor
 * This file is a part of DLTuc library
 ****************************************************************************/

/**
 * @file dlt_logs_mcu.h
 * @author teodor
 * @date 1 Jul 2022
 * @brief File is a part of DLTuc library
 *
 * In this header you can find types, configuration and Api function provided by DLTuc library
 */

#ifndef INC_DLT_LOGS_MCU_H_
#define INC_DLT_LOGS_MCU_H_


#include "stdint.h"
#include <stdarg.h>
#include <string.h>

/**
 * *******************************************************************************************
 * Debug dlt log level macros start
 * *******************************************************************************************
 * */

#define logs_enable


#define DL_FATALd   0x01
#define DL_ERRORd  0x02
#define DL_WARNd    0x03
#define DL_INFOd    0x04
#define DL_DEBUGd   0x05
#define DL_VERBOSEd 0x06
/**
 * *******************************************************************************************
 * Debug dlt typedef start
 * *******************************************************************************************
 * */

typedef enum
{
	DL_FATAL   = 1,
	DL_ERROR   =2,
	DL_WARN    =3,
	DL_INFO    =4,
	DL_DEBUG   =5,
	DL_VERBOSE =6,
}DltLogLevel_t;


// typedef
// {
// 	uint32_t HeaderStart;
// 	BaseHeaderConfig_t BaseHeaderConfig;
// 	uint8_t MessageCounter;
// 	uint16_t MessageLength;
// 	uint32_t EcuId;
// 	uint32_t TimeStamp;
// }DLT_BaseHeader_t;

// typedef
// {
// 	DLT_DebugLevel_t DebugLevel;
// 	uint8_t NumberOfArguments;
// 	uint32_t AppId;
// 	uint32_t ContexID;
// 	TypeInfo_t TypeInfo;
// 	uint16_t Argument1;
// }DLT_ExtendedHeader_t;


/**
 * *******************************************************************************************
 * Configuration defines 
 * *******************************************************************************************
 * */
#define DLT_LOG_ECUID           "_ETH" /*Electronic Controller Unit ID*/

/*@brief DLT_LOG_ECUID_VALUE
 *
 *  - convert the ECUID string to uint32_t
 */
#define DLT_LOG_ECUID_VALUE	((uint32_t)((((uint32_t) ((uint8_t)DLT_LOG_ECUID[0])) << 24UL) | \
						 (((uint32_t) ((uint8_t)DLT_LOG_ECUID[1])) << 16UL) | \
					  (((uint32_t) ((uint8_t)DLT_LOG_ECUID[2])) << 8UL) | \
				  ((uint32_t)((uint8_t)DLT_LOG_ECUID[3]))))


/*
* @brief DLT_ACT_HOLE_HEADER_SIZE hole header sum
* Standard Header + Extended + TypeInfo
* Don't modify it if you aren't sure what are you doing!!!
*/
#define DLT_ACT_HOLE_HEADER_SIZE 32

/*
*@brief DLT_MAX_SINGLE_MESSAGE_SIZE & DLT_RING_BUFFER_SIZE
* these values define the size of the circular buffer and the maximum size of a single DLT message
* Size of out Circular DLT messages buffer is equal: DLT_MAX_SINGLE_MESSAGE_SIZE*DLT_RING_BUFFER_SIZE
*/
#define DLT_MAX_SINGLE_MESSAGE_SIZE 255
#define DLT_RING_BUFFER_SIZE 15

/**
 * *******************************************************************************************
 * API Function prototypes section 
 * *******************************************************************************************
 * */

/*!
 ************************************************************************************************
 * \brief DLTuc_RegisterTransmitSerialDataCallback
 * \details This simple stack/library must be initialized by "DLTuc_RegisterTransmitSerialDataCallback"
 *          As a parameter must be pass function which will transmit serial data
 * \param in LLSerialTrDataFunctionC transmit function pointer
 
 ************************************************************************************************/
void DLTuc_RegisterTransmitSerialDataCallback(void LLSerialTrDataFunctionC(uint8_t *DltLogData, uint8_t Size));

/*!
 ************************************************************************************************
 * \brief DLTuc_MessageTransmitDone
 * \details IMPORTANT!!!!!
 *  Call this function when the transsmision is end
 *  For example in "DMA transmission end callback" to inform the lib that the message is transmitted
 ************************************************************************************************
 * */
void DLTuc_MessageTransmitDone(void);


/*!
 ************************************************************************************************
 * \brief DLTuc_UpdateTimeStampMs
 * \details function to update time stamp in library
 * \param Time - system time in ms
 *************************************************************************************************/
void DLTuc_UpdateTimeStampMs(uint32_t Time);


/*!
 ************************************************************************************************
 * \brief DLTuc_LogOutVarArgs
 * \details function to create DLT Log
 * \param DltLogLevel_t Level - of Dlt log
 * \param in AppId - size of the "DltLogData" (return value)
 * \param in ContextId - pointer to the message stored in RingBuffer (return value)
 * \param in Payload String to send as dlt log
 * \param in ... parameters same as in printf function
 *************************************************************************************************/
void DLTuc_LogOutVarArgs(DltLogLevel_t Level, uint32_t AppId, uint32_t ContextId, uint8_t *Payload, ...);



/**
 * *******************************************************************************************
 * Frew usefull macros to easer create DLT log
 * *******************************************************************************************
 * */

/**!
* \brief DLT_LOG_ENABLE_LEVEL
* \details to define the minimum level log which will printed using the debug macros
*
*/

#define DLT_LOG_ENABLE_LEVEL     DL_VERBOSE


/*This values - it's only example*/
#define DLT_LOG_CONTEX      "TEST"
/**!
* \brief DLT_LOG_ENABLE_LEVEL
* \details to define the minimum level log which will printed using the debug macros
*
*/
#define DLT_LOG_APPID      "BASE"
/**!
* \brief DLT_LOG_ENABLE_LEVEL
* \details to define the minimum level log which will printed using the debug macros
*
*/

#define DLT_LOG_APPID_VALUE	((uint32_t)((((uint32_t) ((uint8_t)DLT_LOG_APPID[0])) << 24UL) | \
						 (((uint32_t) ((uint8_t)DLT_LOG_APPID[1])) << 16UL) | \
					  (((uint32_t) ((uint8_t)DLT_LOG_APPID[2])) << 8UL) | \
				  ((uint32_t)((uint8_t)DLT_LOG_APPID[3]))))


#define DLT_LOG_CONTEX_VALUE	((uint32_t)((((uint32_t) ((uint8_t)DLT_LOG_CONTEX[0])) << 24UL) | \
						 (((uint32_t) ((uint8_t)DLT_LOG_CONTEX[1])) << 16UL) | \
					  (((uint32_t) ((uint8_t)DLT_LOG_CONTEX[2])) << 8UL) | \
				  ((uint32_t)((uint8_t)DLT_LOG_CONTEX[3]))))



#ifdef logs_enable

/**!
 * \brief DEBUGL(level, str, ...)
 * \details Transmit DltLog using function DLTuc_LogOutVarArgs but user don't have add DLT_LOG_APPID_VALUE and DLT_LOG_CONTEX_VALUE
 *
 * */
#define DEBUGL(log_level, str, ...)\
	if(log_level <= DLT_LOG_ENABLE_LEVEL){\
		DLTuc_LogOutVarArgs(log_level, DLT_LOG_APPID_VALUE, DLT_LOG_CONTEX_VALUE,(uint8_t *) str, ##__VA_ARGS__);\
	}

/**!
 * @brief DEBUGF(log_level, str, ...)
 * \details Transmit DltLog using function DLTuc_LogOutVarArgs but user don't have add DLT_LOG_APPID_VALUE and DLT_LOG_CONTEX_VALUE
 * Additionally add the name of the calling function
 *
 *
 * */
#define DEBUGF(log_level, str, ...)\
	if(log_level <= DLT_LOG_ENABLE_LEVEL){\
		DLTuc_LogOutVarArgs(log_level, DLT_LOG_APPID_VALUE, DLT_LOG_CONTEX_VALUE,(uint8_t *) "FUN:%s() LOG: "str, __FUNCTION__,##__VA_ARGS__);\
	}

/**!
 * \brief DEBUGFF(log_level, str, ...)
 * \details Transmit DltLog using function DLTuc_LogOutVarArgs but user don't have add DLT_LOG_APPID_VALUE and DLT_LOG_CONTEX_VALUE
 * Additionally add the name of the calling function and file name
 *
 *
 * */
#define DEBUGFF(log_level, str, ...)\
	if(log_level <= DLT_LOG_ENABLE_LEVEL){\
		DLTuc_LogOutVarArgs(log_level, DLT_LOG_APPID_VALUE, DLT_LOG_CONTEX_VALUE,(uint8_t *)"FILE:%s() FUN:%s() LOG: "str,__FILE__,__FUNCTION__,##__VA_ARGS__);\
	}

#else

DEBUGL(log_level, str, ...)
DEBUGF(log_level, str, ...)
DEBUGFF(log_level, str, ...)
#endif

#endif /* INC_DLT_LOGS_MCU_H_ */

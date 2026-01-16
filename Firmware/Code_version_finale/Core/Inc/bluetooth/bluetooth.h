/*
 * HC05.h
 *
 *  Created on: Oct 13, 2025
 *      Author: hugof
 */

#ifndef INC_HC05_H_
#define INC_HC05_H_

////////////////////////////////////////////// INCLUDES
#include "stdint.h"
#include "string.h"
#include "stdio.h"

////////////////////////////////////////////// DEFINES
#define txData_LENGTH 50
#define rxData_LENGTH 1

////////////////////////////////////////////// STRUCTURES
typedef struct
{
	// Envoyer command
	void (*SendCommand)(const char* cmd);
	// Envoyer trame
	void (*SendTrame)(const uint8_t* cmd);
    // Commandes de base
    void (*AT)(void);
    void (*Reset)(void);
    void (*RestoreDefaults)(void);
    void (*Sleep)(void);

    // Nom & identifiants
    void (*ReadVersion)(void);
    void (*ReadName)(void);
    void (*SetName)(const char* name);
    void (*ReadSPPName)(void);
    void (*SetSPPName)(const char* name);

    // Adresse MAC
    void (*ReadMAC)(void);
    void (*ReadSPPMAC)(void);
    void (*SetHC05MAC)(const char* mac12);
    void (*SetSPPMAC)(const char* mac12);

    // Baudrate
    void (*ReadBaudrate)(void);
    void (*SetBaudrate)(uint32_t baudrate);

    // Mode HC05 / HC05+SPP
    void (*ReadMode)(void);
    void (*SetMode)(uint8_t mode);

    // Sécurité
    void (*SetPIN)(const char* pin6digits);
    void (*ReadPIN)(void);
    void (*SetSecurity)(uint8_t level);
    void (*ReadSecurity)(void);

    // Rôle
    void (*ReadRole)(void);
    void (*SetRole)(uint8_t role);

    // UUIDs
    void (*ReadUUID)(void);
    void (*SetUUID)(uint8_t index, const char* uuid);
    void (*SetTargetUUID)(const char* uuid);

    // Broadcasting (advertising)
    void (*ReadAdvData)(void);
    void (*SetAdvData)(const char* adv_data_hex);
    void (*ReadAdvInterval)(void);
    void (*SetAdvInterval)(uint16_t interval_ms);
    void (*SetBroadcast)(uint8_t enaHC05);

    // Indicateur LED
    void (*SetLED)(uint8_t mode);
    void (*ReadLED)(void);

    // RF Power
    void (*SetRFPower)(uint8_t level);

    // Low power
    void (*SetLowPowerMode)(uint8_t enaHC05);
    void (*ReadLowPowerMode)(void);

    // Logs
    void (*SetLog)(uint8_t enaHC05);
    void (*ReadLog)(void);

    // Canaux
    void (*EnterATMode)(void);
    void (*EnterHC05Mode)(void);

    // Scan / Connexion
    void (*ScanStart)(void);
    void (*SetScanLimits)(uint8_t count, uint8_t timeout_sec);
    void (*SetScanParams)(uint8_t active, uint8_t interval, uint8_t window);
    void (*ClearBindings)(void);
    void (*SetAutoConnect)(const char* mac);
    void (*ConnectTo)(const char* mac);
    void (*Disconnect)(uint8_t mode);
} HC05_Interface_t;

typedef struct
{
	UART_HandleTypeDef * huart;
	uint8_t txData[txData_LENGTH];
	uint8_t rxData[rxData_LENGTH];
} h_hc05_t;

////////////////////////////////////////////// COMMANDES PROPRES HC05
// Envoie et réception trames
void HC05_SendCommand(h_hc05_t * h_hc05, const char* cmd);
void HC05_SendTrame(h_hc05_t * h_hc05, const uint8_t* cmd);

// Commandes de base
void HC05_AT(h_hc05_t * h_hc05);
void HC05_Reset(h_hc05_t * h_hc05);
void HC05_RestoreDefaults(h_hc05_t * h_hc05);
void HC05_Sleep(h_hc05_t * h_hc05);

// Nom et identifiants
void HC05_ReadVersion(h_hc05_t * h_hc05);
void HC05_ReadName(h_hc05_t * h_hc05);
void HC05_SetName(h_hc05_t * h_hc05, const char* name);
void HC05_ReadSPPName(h_hc05_t * h_hc05);
void HC05_SetSPPName(h_hc05_t * h_hc05, const char* name);

// Adresse MAC
void HC05_ReadMAC(h_hc05_t * h_hc05);
void HC05_ReadSPPMAC(h_hc05_t * h_hc05);
void HC05_SetHC05MAC(h_hc05_t * h_hc05, const char* mac12);
void HC05_SetSPPMAC(h_hc05_t * h_hc05, const char* mac12);

// Baudrate
void HC05_ReadBaudrate(h_hc05_t * h_hc05);
void HC05_SetBaudrate(h_hc05_t * h_hc05, uint32_t baudrate);

// Mode HC05 / HC05+SPP
void HC05_ReadMode(h_hc05_t * h_hc05);
void HC05_SetMode(h_hc05_t * h_hc05, uint8_t mode);

// Sécurité
void HC05_SetPIN(h_hc05_t * h_hc05, const char* pin6digits);
void HC05_ReadPIN(h_hc05_t * h_hc05);
void HC05_SetSecurity(h_hc05_t * h_hc05, uint8_t level);
void HC05_ReadSecurity(h_hc05_t * h_hc05);

// Rôle
void HC05_ReadRole(h_hc05_t * h_hc05);
void HC05_SetRole(h_hc05_t * h_hc05, uint8_t role);

// UUIDs
void HC05_ReadUUID(h_hc05_t * h_hc05);
void HC05_SetUUID(h_hc05_t * h_hc05, uint8_t index, const char* uuid);
void HC05_SetTargetUUID(h_hc05_t * h_hc05, const char* uuid);

// Broadcasting (advertising)
void HC05_ReadAdvData(h_hc05_t * h_hc05);
void HC05_SetAdvData(h_hc05_t * h_hc05, const char* adv_data_hex);
void HC05_ReadAdvInterval(h_hc05_t * h_hc05);
void HC05_SetAdvInterval(h_hc05_t * h_hc05, uint16_t interval_ms);
void HC05_SetBroadcast(h_hc05_t * h_hc05, uint8_t enaHC05);

// Indicateur LED
void HC05_SetLED(h_hc05_t * h_hc05, uint8_t mode);
void HC05_ReadLED(h_hc05_t * h_hc05);

// Puissance RF
void HC05_SetRFPower(h_hc05_t * h_hc05, uint8_t level);

// Low power
void HC05_SetLowPowerMode(h_hc05_t * h_hc05, uint8_t enaHC05);
void HC05_ReadLowPowerMode(h_hc05_t * h_hc05);

// Logs
void HC05_SetLog(h_hc05_t * h_hc05, uint8_t enaHC05);
void HC05_ReadLog(h_hc05_t * h_hc05);

// Canaux / modes
void HC05_EnterATMode(h_hc05_t * h_hc05);
void HC05_EnterHC05Mode(h_hc05_t * h_hc05);

// Connexion / scan
void HC05_ConnectTo(h_hc05_t * h_hc05, const char* mac_addr);
void HC05_Disconnect(h_hc05_t * h_hc05, uint8_t mode);
void HC05_ScanStart(h_hc05_t * h_hc05);
void HC05_SetScanLimits(h_hc05_t * h_hc05, uint8_t count, uint8_t timeout_sec);
void HC05_SetScanParams(h_hc05_t * h_hc05, uint8_t active, uint8_t interval, uint8_t window);
void HC05_ClearBindings(h_hc05_t * h_hc05);
void HC05_SetAutoConnect(h_hc05_t * h_hc05, const char* mac);

////////////////////////////////////////////// COMMANDES TRAITEMENTS
void HC05_Tasks_Create (h_hc05_t * h_hc05);
void send_coordinates (h_hc05_t * h_hc05);
void rx_process (h_hc05_t * h_hc05);
void hc05_RX_callback(h_hc05_t * h_hc05);

#endif /* INC_HC05_H_ */

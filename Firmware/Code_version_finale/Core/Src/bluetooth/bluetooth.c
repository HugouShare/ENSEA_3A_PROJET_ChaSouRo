/*
 * HC05.c
 *
 *  Created on: Oct 13, 2025
 *      Author: hugof
 */

#include "main.h"
#include "bluetooth/bluetooth.h"
#include "actuators/control.h"

#include "stm32g4xx.h"
#include "FreeRTOS.h"
#include "event_groups.h"

#include "task.h"
#include "usart.h"

////////////////////////////////////////////// VARIABLES
static TaskHandle_t h_task_hc05_tx = NULL;
static TaskHandle_t h_task_hc05_rx = NULL;

////////////////////////////////////////////// COMMANDES PROPRES HC05
// Envoie commande
void HC05_SendCommand(h_hc05_t * h_hc05, const char* cmd)
{
	HAL_UART_Transmit(h_hc05->huart, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);
}

// Envoie trame
void HC05_SendTrame(h_hc05_t * h_hc05, const uint8_t* cmd)
{
	HAL_UART_Transmit(h_hc05->huart, cmd, strlen((char*)cmd), HAL_MAX_DELAY);
}

// Test AT
void HC05_AT(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT\r\n");
}

// Redémarrage du module
void HC05_Reset(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+RESET\r\n");
}

// Restauration usine
void HC05_RestoreDefaults(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+RESTORE\r\n");
}

// Lire version du firmware
void HC05_ReadVersion(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+VER\r\n");
}

// Lire nom HC05
void HC05_ReadName(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+HC05NAME\r\n");
}

// Définir nom HC05
void HC05_SetName(h_hc05_t * h_hc05, const char* name)
{
	char cmd[64];
	snprintf(cmd, sizeof(cmd), "AT+HC05NAME=%s\r\n", name);
	HC05_SendCommand(h_hc05,cmd);
}

// Lire l’adresse MAC HC05
void HC05_ReadMAC(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+HC05MAC\r\n");
}

// Lire le rôle actuel
void HC05_ReadRole(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+ROLE\r\n");
}

// Définir rôle (0 = slave, 1 = master)
void HC05_SetRole(h_hc05_t * h_hc05, uint8_t role)
{
	char cmd[32];
	snprintf(cmd, sizeof(cmd), "AT+ROLE=%d\r\n", (char)role);
	HC05_SendCommand(h_hc05,cmd);
}

// Définir mot de passe
void HC05_SetPIN(h_hc05_t * h_hc05,const char* pin6digits)
{
	char cmd[32];
	snprintf(cmd, sizeof(cmd), "AT+PIN=%s\r\n", pin6digits);
	HC05_SendCommand(h_hc05,cmd);
}

// Définir le niveau de sécurité
void HC05_SetSecurity(h_hc05_t * h_hc05,uint8_t level)
{
	char cmd[32];
	snprintf(cmd, sizeof(cmd), "AT+SECURITY=%d\r\n", (char)level);
	HC05_SendCommand(h_hc05,cmd);
}

// Lire niveau de sécurité
void HC05_ReadSecurity(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+SECURITY\r\n");
}

// Lire PIN actuel
void HC05_ReadPIN(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+PIN\r\n");
}

// Passer en mode AT
void HC05_EnterATMode(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT>9\r\n");
}

// Revenir au mode HC05
void HC05_EnterHC05Mode(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT>8\r\n");
}

// Activer/désactiver le log
void HC05_SetLog(h_hc05_t * h_hc05, uint8_t enaHC05)
{
	char cmd[32];
	snprintf(cmd, sizeof(cmd), "AT+LOG=%d\r\n", (char)enaHC05);
	HC05_SendCommand(h_hc05,cmd);
}

// Lire état du log
void HC05_ReadLog(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+LOG\r\n");
}

// Réglage LED d’état
void HC05_SetLED(h_hc05_t * h_hc05, uint8_t mode)
{
	char cmd[32];
	snprintf(cmd, sizeof(cmd), "AT+LED=%d\r\n", (char)mode);
	HC05_SendCommand(h_hc05,cmd);
}

// Lire mode LED
void HC05_ReadLED(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+LED\r\n");
}

// Définir puissance d’émission (0 à 9)
void HC05_SetRFPower(h_hc05_t * h_hc05, uint8_t level)
{
	char cmd[32];
	snprintf(cmd, sizeof(cmd), "AT+RFPWR=%d\r\n", (char)level);
	HC05_SendCommand(h_hc05,cmd);
}

// Lire UUID
void HC05_ReadUUID(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+UUID\r\n");
}

// Modifier UUID d’un service ou caractéristique
void HC05_SetUUID(h_hc05_t * h_hc05, uint8_t index, const char* uuid)
{
	char cmd[128];
	snprintf(cmd, sizeof(cmd), "AT+UUID=%d,%s\r\n", (char)index, uuid);
	HC05_SendCommand(h_hc05,cmd);
	HC05_Reset(h_hc05);  // UUID settings require reboot
}

// Lire données du paquet de broadcast
void HC05_ReadAdvData(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+ADVDATA\r\n");
}

// Définir données broadcast personnalisées
void HC05_SetAdvData(h_hc05_t * h_hc05, const char* adv_data_hex)
{
	char cmd[128];
	snprintf(cmd, sizeof(cmd), "AT+ADVDATA=%s\r\n", adv_data_hex);
	HC05_SendCommand(h_hc05,cmd);
}

// Lire intervalle broadcast
void HC05_ReadAdvInterval(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+ADVPARAM\r\n");
}

// Définir intervalle broadcast (10~4000 ms)
void HC05_SetAdvInterval(h_hc05_t * h_hc05, uint16_t interval_ms)
{
	char cmd[64];
	snprintf(cmd, sizeof(cmd), "AT+ADVPARAM=%d\r\n", (char)interval_ms);
	HC05_SendCommand(h_hc05,cmd);
}

// Activer/désactiver le broadcast
void HC05_SetBroadcast(h_hc05_t * h_hc05, uint8_t enaHC05)
{
	char cmd[32];
	snprintf(cmd, sizeof(cmd), "AT+ADV=%d\r\n", (char)enaHC05);
	HC05_SendCommand(h_hc05,cmd);
}

// Activer ou désactiver le mode low power
void HC05_SetLowPowerMode(h_hc05_t * h_hc05, uint8_t enaHC05)
{
	char cmd[32];
	snprintf(cmd, sizeof(cmd), "AT+LPM=%d\r\n", (char)enaHC05);
	HC05_SendCommand(h_hc05,cmd);
}

// Lire état low power
void HC05_ReadLowPowerMode(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+LPM\r\n");
}

// Rechercher des périphériques HC05 autour (master mode)
void HC05_Scan(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+SCAN=1\r\n");
}

// Connexion à un périphérique HC05 (master mode)
void HC05_ConnectTo(h_hc05_t * h_hc05, const char* mac_addr)
{
	char cmd[64];
	snprintf(cmd, sizeof(cmd), "AT+CONN=%s\r\n", mac_addr);
	HC05_SendCommand(h_hc05,cmd);
}

// Déconnexion (nécessite d’abord AT>9)
void HC05_Disconnect(h_hc05_t * h_hc05, uint8_t mode)
{
	char cmd[32];
	snprintf(cmd, sizeof(cmd), "AT+DISC=%d\r\n", (char)mode);
	HC05_SendCommand(h_hc05,cmd);
}

// Mise en veille (soft shutdown)
void HC05_Sleep(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+SLEEP\r\n");
}

// Lire le baudrate
void HC05_ReadBaudrate(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+BAUD\r\n");
}

// Définir le baudrate (ex: 9600, 115200, etc.)
void HC05_SetBaudrate(h_hc05_t * h_hc05, uint32_t baudrate)
{
	char cmd[32];
	snprintf(cmd, sizeof(cmd), "AT+BAUD=%lu\r\n", baudrate);
	HC05_SendCommand(h_hc05,cmd);
}

// Lire le mode (0 = HC05 / 1 = HC05&SPP)
void HC05_ReadMode(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+MODE\r\n");
}

// Définir le mode HC05 ou HC05+SPP
void HC05_SetMode(h_hc05_t * h_hc05, uint8_t mode)
{
	char cmd[32];
	snprintf(cmd, sizeof(cmd), "AT+MODE=%d\r\n", (char)mode);
	HC05_SendCommand(h_hc05,cmd);
}

// Lire nom SPP
void HC05_ReadSPPName(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+SPPNAME\r\n");
}

// Définir nom SPP
void HC05_SetSPPName(h_hc05_t * h_hc05, const char* name)
{
	char cmd[64];
	snprintf(cmd, sizeof(cmd), "AT+SPPNAME=%s\r\n", name);
	HC05_SendCommand(h_hc05,cmd);
}

// Lire MAC SPP
void HC05_ReadSPPMAC(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+SPPMAC\r\n");
}

// Définir MAC HC05
void HC05_SetHC05MAC(h_hc05_t * h_hc05, const char* mac12)
{
	char cmd[64];
	snprintf(cmd, sizeof(cmd), "AT+HC05MAC=%s\r\n", mac12);
	HC05_SendCommand(h_hc05,cmd);
}

// Définir MAC SPP
void HC05_SetSPPMAC(h_hc05_t * h_hc05, const char* mac12)
{
	char cmd[64];
	snprintf(cmd, sizeof(cmd), "AT+SPPMAC=%s\r\n", mac12);
	HC05_SendCommand(h_hc05,cmd);
}

// Définir UUID ciHC05 pour connexion (client mode)
void HC05_SetTargetUUID(h_hc05_t * h_hc05, const char* uuid)
{
	char cmd[128];
	snprintf(cmd, sizeof(cmd), "AT+TARGETUUID=%s\r\n", uuid);
	HC05_SendCommand(h_hc05,cmd);
}

// Scanner les périphériques HC05
void HC05_ScanStart(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+SCAN=1\r\n");
}

// Configurer paramètres de scan (nombre, timeout)
void HC05_SetScanLimits(h_hc05_t * h_hc05, uint8_t count, uint8_t timeout_sec)
{
	char cmd[32];
	snprintf(cmd, sizeof(cmd), "AT+SCANM=%d,%d\r\n", (char)count, (char)timeout_sec);
	HC05_SendCommand(h_hc05,cmd);
}

// Configurer scan params (mode, interval, window)
void HC05_SetScanParams(h_hc05_t * h_hc05, uint8_t active, uint8_t interval, uint8_t window)
{
	char cmd[64];
	snprintf(cmd, sizeof(cmd), "AT+SCANPARAM=%d,%d,%d\r\n", (char)active, (char)interval, (char)window);
	HC05_SendCommand(h_hc05,cmd);
}

// Effacer les liaisons enregistrées
void HC05_ClearBindings(h_hc05_t * h_hc05)
{
	HC05_SendCommand(h_hc05,"AT+CLRBIND\r\n");
}

// Configurer auto-connexion (client mode uniquement)
void HC05_SetAutoConnect(h_hc05_t * h_hc05, const char* mac)
{
	char cmd[64];
	snprintf(cmd, sizeof(cmd), "AT+SERVER=%s\r\n", mac);
	HC05_SendCommand(h_hc05,cmd);
	HC05_Reset(h_hc05);  // Reboot pour appliquer
}

// Dictionnaire de commandes => UNUSED
/*
HC05_Interface_t HC05 =
{
		// Envoyer trame
		.SendCommand         = HC05_SendCommand,
		.SendTrame           = HC05_SendTrame,
		// Commandes de base
		.AT                  = HC05_AT,
		.Reset               = HC05_Reset,
		.RestoreDefaults     = HC05_RestoreDefaults,
		.Sleep               = HC05_Sleep,

		// Nom & identifiants
		.ReadVersion         = HC05_ReadVersion,
		.ReadName            = HC05_ReadName,
		.SetName             = HC05_SetName,
		.ReadSPPName         = HC05_ReadSPPName,
		.SetSPPName          = HC05_SetSPPName,

		// Adresse MAC
		.ReadMAC             = HC05_ReadMAC,
		.ReadSPPMAC          = HC05_ReadSPPMAC,
		.SetHC05MAC           = HC05_SetHC05MAC,
		.SetSPPMAC           = HC05_SetSPPMAC,

		// Baudrate
		.ReadBaudrate        = HC05_ReadBaudrate,
		.SetBaudrate         = HC05_SetBaudrate,

		// Mode HC05 / HC05+SPP
		.ReadMode            = HC05_ReadMode,
		.SetMode             = HC05_SetMode,

		// Sécurité
		.SetPIN              = HC05_SetPIN,
		.ReadPIN             = HC05_ReadPIN,
		.SetSecurity         = HC05_SetSecurity,
		.ReadSecurity        = HC05_ReadSecurity,

		// Rôle
		.ReadRole            = HC05_ReadRole,
		.SetRole             = HC05_SetRole,

		// UUIDs
		.ReadUUID            = HC05_ReadUUID,
		.SetUUID             = HC05_SetUUID,
		.SetTargetUUID       = HC05_SetTargetUUID,

		// Broadcasting
		.ReadAdvData         = HC05_ReadAdvData,
		.SetAdvData          = HC05_SetAdvData,
		.ReadAdvInterval     = HC05_ReadAdvInterval,
		.SetAdvInterval      = HC05_SetAdvInterval,
		.SetBroadcast        = HC05_SetBroadcast,

		// LED
		.SetLED              = HC05_SetLED,
		.ReadLED             = HC05_ReadLED,

		// RF
		.SetRFPower          = HC05_SetRFPower,

		// Low power
		.SetLowPowerMode     = HC05_SetLowPowerMode,
		.ReadLowPowerMode    = HC05_ReadLowPowerMode,

		// Logs
		.SetLog              = HC05_SetLog,
		.ReadLog             = HC05_ReadLog,

		// Canaux
		.EnterATMode         = HC05_EnterATMode,
		.EnterHC05Mode        = HC05_EnterHC05Mode,

		// Scan / Connexion
		.ScanStart           = HC05_ScanStart,
		.SetScanLimits       = HC05_SetScanLimits,
		.SetScanParams       = HC05_SetScanParams,
		.ClearBindings       = HC05_ClearBindings,
		.SetAutoConnect      = HC05_SetAutoConnect,
		.ConnectTo           = HC05_ConnectTo,
		.Disconnect          = HC05_Disconnect
};
*/

////////////////////////////////////////////// COMMANDES TRAITEMENTS

volatile uint8_t emergency_stop = 0;

// Tâche liée à la transmission via module HC05
void task_hc05_tx (void *argument)
{
	/*
	 * Le fonctionnement de cette tâche-ci est le suivant :
	 * - Un timer déclenche une interruption toutes les x ms
	 * - L'interruption active alors cette tâche-ci
	 * - Cette tâche envoie alors au téléphone, via Bluetooth, un buffer contenant (x,y) = la position du robot sur la map
	 */

    h_hc05_t *h_hc05 = (h_hc05_t *)argument;

	for (;;)
	{
		send_coordinates(h_hc05);
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

// Tâche liée à la réception via module HC05
void task_hc05_rx (void * argument)
{
	/*
	 * Le fonctionnement de cette tâche-ci est le suivant :
	 * - Cette tâche est activée suite au déclenchement d'une interruption RX sur USART1 (usart relié au HC05)
	 * - Une fois le buffer reçu il est traité
	 */

    h_hc05_t *h_hc05 = (h_hc05_t *)argument;

	for (;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		rx_process(h_hc05);
	}
}

// Permet l'envoi des coordonnées (x,y) du robot sur la table
void send_coordinates (h_hc05_t * h_hc05)
{
	uint16_t size = snprintf ((char*)h_hc05->txData, txData_LENGTH, "\n\r (X : %d, Y : %d) \n\r",58,678); /* cordonnées (x,y) */
	HAL_UART_Transmit_DMA(h_hc05->huart, h_hc05->txData, size);
}

// Process du RX
void rx_process (h_hc05_t * h_hc05)
{
    h_hc05->rxData[rxData_LENGTH - 1] = '\0';

    if (strstr((char*)h_hc05->rxData, "STOP") != NULL)
    {
        emergency_stop = 1;  // Active arrêt d’urgence
        Control_Stop();
        printf(">>> EMERGENCY STOP RECEIVED <<<\r\n");
        return;
    }
    if (strstr((char*)h_hc05->rxData, "START") != NULL)
    {
        emergency_stop = 0;
        printf(">>> SYSTEM RE-ENABLED <<<\r\n");
        return;
    }
	if (h_hc05->rxData[0]=='0')
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, RESET);
	}
	if (h_hc05->rxData[0]=='1')
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, SET);
	}
}

// Initialisation du module HC05
void HC05_Tasks_Create (h_hc05_t * h_hc05)
{
	HAL_UART_Receive_IT(h_hc05->huart, h_hc05->rxData, rxData_LENGTH);
	if (xTaskCreate(task_hc05_tx, "HC05_TX", 256, h_hc05, 1, &h_task_hc05_tx) != pdPASS)
	{
		printf("\n\r Error creating TX bluetooth task \n\r");
		Error_Handler();
	}
	if (xTaskCreate(task_hc05_rx, "HC05_RX", 256, h_hc05, 2, &h_task_hc05_rx) != pdPASS)
	{
		printf("\n\r Error creating RX bluetooth task \n\r");
		Error_Handler();
	}
}

// Interruption RX qui active la tâche "task_hc05_rx"
void hc05_RX_callback(h_hc05_t * h_hc05)
{
	BaseType_t higher_priority_task_woken = pdFALSE;
	vTaskNotifyGiveFromISR(h_task_hc05_rx, &higher_priority_task_woken);
	HAL_UART_Receive_IT(h_hc05->huart, h_hc05->rxData, rxData_LENGTH);
	portYIELD_FROM_ISR(higher_priority_task_woken);
}

/*
 * OLED.c
 *
 *  Created on: Sep 30, 2025
 *      Author: nelven
 */

#include "oled_screen/oled.h"
#include "oled_screen/bitmaps.h"
#include <string.h>
#include <stdio.h>
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "cmsis_os.h"

// Ajout FreeRTOS
//SemaphoreHandle_t sem_select_menu;
//SemaphoreHandle_t sem_enter_menu;
TaskHandle_t hmenuTaskHandle = NULL;
#define MENU_SELECT   (1UL << 0)
#define MENU_ENTER    (1UL << 1)


SystFlag Flags = {0};

#define IS_SUBMENU(m)     ((m) >= MENU_START && (m) <= MENU_END)
#define IS_DISPLAY(m)     ((m) >= DISP_START && (m) <= DISP_END)
// Variables globales pour Init bitmap
uint8_t InitFrame = 0;
uint32_t lastUpdateInit = 0;

// Variables globales pour Kirby
uint8_t kirbyFrame = 0;
uint32_t lastUpdateKirby = 0;

// Variables globales pour le chat effrayé
uint8_t catFrame = 0;
uint32_t lastUpdateCat = 0;

extern const unsigned char github_logo_64x64[];
extern const unsigned char garfield_128x64[];

// Variable pour la luminosité
uint8_t brightnessLevel = 255;  // Luminosité initiale (0-255)

MenuState currentMenu = MENU_PRINCIPAL;
uint8_t cursorIndex = 0;   // position du curseur
uint8_t topIndex = 0;      // premier élément affiché
const uint8_t maxVisibleItems = 5; // écran peut afficher 5 lignes

// Menus
const char* mainMenuItems[] = {
		"Demarrer",
		"Settings",
		"Mode",
		"Animations",
		"Attendre"};
const char* settingsMenuItems[] = {
		"Luminosite +",
		"Luminosite -",
		"Retour"};
const char* modeMenuItems[] = {
		"Chat",
		"Souris",
		"Retour"};
const char* AnimMenuItems[] = {
		"Chat",
		"Souris",
		"ChaSouRo",
		"Scared Cat",
		"Initialising",
		"Kirby sleeping",
		"Retour"};

const uint8_t mainMenuLength = sizeof(mainMenuItems) / sizeof(mainMenuItems[0]);
const uint8_t settingsMenuLength = sizeof(settingsMenuItems) / sizeof(settingsMenuItems[0]);
const uint8_t modeMenuLength = sizeof(modeMenuItems) / sizeof(modeMenuItems[0]);
const uint8_t AnimMenuLength = sizeof(AnimMenuItems) / sizeof(AnimMenuItems[0]);

// Gestion du timeout
uint32_t lastPressBtnPA1 = 0;
uint32_t lastPressBtnPB9 = 0;
uint32_t lastMenuActivity = 0;


//Init
void task_screen(void * unused){

	for(;;){

//		// Attend soit un "select", soit un "enter"
//		if (xSemaphoreTake(sem_select_menu, 0) == pdTRUE)
//		{
//			select_Menu();
//		}
//
//		if (xSemaphoreTake(sem_enter_menu, 0) == pdTRUE)
//		{
//			enter_Menu();
//		}
		uint32_t notif;

	    xTaskNotifyWait(
	        0x00,
	        MENU_SELECT | MENU_ENTER,
	        &notif,
	        0
	    );

	    if (notif & MENU_SELECT)
	    {
	    	select_Menu();
	    }
	    if (notif & MENU_ENTER)
	    {
	    	enter_Menu();
	    }
		// Affichage + gestion de veille
		displayMenu();
		checkMenuTimeout();

		vTaskDelay(pdMS_TO_TICKS(80));
	}
}

void OLED_Tasks_Create(void){
//  sem_select_menu = xSemaphoreCreateBinary();
//  sem_enter_menu = xSemaphoreCreateBinary();

  if (xTaskCreate(task_screen, "SCREEN", OLED_STACK_SIZE, NULL,task_screen_PRIORITY,&hmenuTaskHandle) != pdPASS){
	 printf("Error creating task screen\r\n");
	 Error_Handler();
  }
}

void OLED_Init(void){
	ssd1306_Init();

    Init_bitmap();

    ssd1306_SetCursor(20,20);
    ssd1306_Fill(Black);
    ssd1306_WriteString("Ready", Font_16x26, White);
    ssd1306_UpdateScreen();
    HAL_Delay(100);
}

static int brightnessToPercent(uint8_t brightness) {
    switch (brightness) {
        case 255: return 100;
        case 170: return 67;
        case 85:  return 33;
        case 20:  return 8;
        default:  return (brightness * 100) / 255;
    }
}

void oled_git(){
    ssd1306_Fill(Black);
	ssd1306_DrawBitmap(32,0,github_logo_64x64,64,64,White);
	ssd1306_UpdateScreen();
}

void oled_garfield(){
	ssd1306_Fill(White);
	ssd1306_DrawBitmap(0,0,garfield_128x64,128,64,Black);
	ssd1306_UpdateScreen();
}

void oled_cat(){
	ssd1306_Fill(Black);
	ssd1306_DrawBitmap(0,0,epd_bitmap_cat_head_64,128,64,White);
	ssd1306_UpdateScreen();
}

void oled_mouse(){
	ssd1306_Fill(Black);
	ssd1306_DrawBitmap(0,0,epd_bitmap_mouse_head_64,128,64,White);
	ssd1306_UpdateScreen();
}

void kirby_bitmap(){
	int i = 0;
	while (i < kirby_bitmap_allArray_LEN){
		ssd1306_Fill(Black);
		ssd1306_DrawBitmap(0,0,kirby_bitmap_allArray[i],128,64,White);
		ssd1306_UpdateScreen();
		vTaskDelay(40);
		i++;
	}

}

//void scared_cat_bitmap(){
//	int i = 0;
//	while (i < scared_cat_bitmap_allArray_LEN){
//		ssd1306_Fill(Black);
//		ssd1306_DrawBitmap(0,0,scared_cat_bitmap_allArray[i],128,64,White);
//		ssd1306_UpdateScreen();
//		vTaskDelay(40);
//		i++;
//	}
//
//}

void kirby_bitmap_tick(void) {
    TickType_t now = xTaskGetTickCount();

    if (now - lastUpdateKirby > pdMS_TO_TICKS(120)) {  // avance toutes les 40 ms
        lastUpdateKirby = now;

        ssd1306_Fill(Black);
        ssd1306_DrawBitmap(0, 0,
            kirby_bitmap_allArray[kirbyFrame],
            128, 64, White);
        ssd1306_UpdateScreen();

        kirbyFrame++;
        if (kirbyFrame >= kirby_bitmap_allArray_LEN) {
            kirbyFrame = 0;  // pour relancer la boucle
        }
    }
}

//void scared_cat_bitmap_tick(void) {
//    TickType_t now = xTaskGetTickCount();
//
//    if (now - lastUpdateCat > pdMS_TO_TICKS(120)) {  // avance toutes les 40 ms sans bloquer comme un while
//        lastUpdateCat = now;
//
//        ssd1306_Fill(Black);
//        ssd1306_DrawBitmap(0, 0,
//            scared_cat_bitmap_allArray[catFrame],
//            128, 64, White);
//        ssd1306_UpdateScreen();
//
//        catFrame++;
//        if (catFrame >= scared_cat_bitmap_allArray_LEN) {
//            catFrame = 0;
//        }
//    }
//}

void Init_bitmap_tick(void) {
    TickType_t now = xTaskGetTickCount();


    if (now - lastUpdateInit > pdMS_TO_TICKS(120)) {  // avance toutes les 40 ms sans bloquer comme un while
        lastUpdateInit = now;

        ssd1306_Fill(Black);
        ssd1306_DrawBitmap(0, 0,
            Init_bitmap_allArray[InitFrame],
            128, 64, White);
        ssd1306_UpdateScreen();

        InitFrame++;
        if (InitFrame >= Init_bitmap_allArray_LEN) {
            InitFrame = 0;
        }
    }
}

void Init_bitmap(){
	int i = 0;
	while (i < Init_bitmap_allArray_LEN){
		ssd1306_Fill(Black);
		ssd1306_DrawBitmap(0,0,Init_bitmap_allArray[i],128,64,White);
		ssd1306_UpdateScreen();
		HAL_Delay(40);
		i++;
	}

}

void screen_stats(){
	if (Flags.User_Button == 1){
		  vTaskDelay(100);
		  oled_cat();
		  vTaskDelay(50);
	};
	if (Flags.User_Button == 2){
		  vTaskDelay(100);
		  oled_mouse();
		  vTaskDelay(50);

	};
	if (Flags.User_Button == 3){
		  vTaskDelay(100);
		  ssd1306_Fill(Black);
		  ssd1306_SetCursor(20,20);
		  ssd1306_WriteString("ChaSouRo", Font_11x18, White);
		  ssd1306_UpdateScreen();
		  vTaskDelay(50);
	};
	if (Flags.User_Button == 4){
//		  vTaskDelay(100);
//		  kirby_bitmap();
//		  vTaskDelay(40);
		kirby_bitmap_tick();
	};
	if (Flags.User_Button == 5){
//		  vTaskDelay(100);
//		  scared_cat_bitmap();
//		  vTaskDelay(40);
//		scared_cat_bitmap_tick();

		};
	if (Flags.User_Button == 6){
//		  vTaskDelay(100);
//		  scared_cat_bitmap();
//		  vTaskDelay(40);
		Init_bitmap_tick();

		};
}

void adjustBrightness(bool increase) {
    if (increase) {
        // Si la luminosité est inférieure à 255, on peut augmenter de 85
        if (brightnessLevel == 170) brightnessLevel = 255;
        else if (brightnessLevel == 85) brightnessLevel = 170;
        else if (brightnessLevel == 20) brightnessLevel = 85;
    } else {
        // Si la luminosité est plus grande que 20, on peut diminuer de 85
        if (brightnessLevel == 255) brightnessLevel = 170;
        else if (brightnessLevel == 170) brightnessLevel = 85;
        else if (brightnessLevel == 85) brightnessLevel = 20;
    }
    // Appliquer le contraste
    ssd1306_SetContrast(brightnessLevel);
}
//
//void oled_HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
////	if (GPIO_Pin == GPIO_PIN_1) {
////		Flags.Select_Menu = 1;
////	}
//
////	if (GPIO_Pin == GPIO_PIN_9){
////		Flags.Enter_Menu = 1;
////	}
//	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//
//	    if (GPIO_Pin == GPIO_PIN_1)  // bouton PA1
//	    {
//	        xSemaphoreGiveFromISR(sem_select_menu, &xHigherPriorityTaskWoken);
//	    }
//	    else if (GPIO_Pin == GPIO_PIN_9)  // bouton PB9
//	    {
//	        xSemaphoreGiveFromISR(sem_enter_menu, &xHigherPriorityTaskWoken);
//	    }
//
//	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//}


void oled_HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (GPIO_Pin == GPIO_PIN_1)
    {
        xTaskNotifyFromISR(hmenuTaskHandle,
                           MENU_SELECT,
                           eSetBits,
                           &xHigherPriorityTaskWoken);
    }
    else if (GPIO_Pin == GPIO_PIN_9)
    {
        xTaskNotifyFromISR(hmenuTaskHandle,
                           MENU_ENTER,
                           eSetBits,
                           &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

//void select_Menu(void)
//{
//    TickType_t now = xTaskGetTickCount();
//
//    if (now - lastPressBtnPA1 < pdMS_TO_TICKS(100)) return;  // anti-rebond
//    lastPressBtnPA1 = now;
//    lastMenuActivity = now;
//
//    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);
//
//    switch (currentMenu)
//    {
//        case MENU_PRINCIPAL:
//            switch (cursorIndex)
//            {
//                case 0: currentMenu = MENU_DEMARRER; break;
//                case 1: currentMenu = MENU_SETTINGS; cursorIndex = 0; break;
//                case 2: currentMenu = MENU_MODE; cursorIndex = 0; break;
//                case 3: currentMenu = MENU_ANIMATIONS; cursorIndex = 0; topIndex = 0; break;
//                case 4: currentMenu = MENU_ATTENDRE; break;
//            }
//            break;
//
//        case MENU_SETTINGS:
//            switch (cursorIndex)
//            {
//                case 0: adjustBrightness(true); break;
//                case 1: adjustBrightness(false); break;
//                case 2: currentMenu = MENU_PRINCIPAL; cursorIndex = 0; break;
//            }
//            break;
//
//        case MENU_ANIMATIONS:
//            switch (cursorIndex)
//            {
//                case 0: currentMenu = DISP_CAT; break;
//                case 1: currentMenu = DISP_MOUSE; break;
//                case 2: currentMenu = DISP_CHASOURO; break;
//                case 3: currentMenu = DISP_SCAREDCAT; break;
//                case 4: currentMenu = DISP_INIT; break;
//                case 5: currentMenu = DISP_KIRBY; break;
//                case 6: currentMenu = MENU_PRINCIPAL; cursorIndex = 0; break;
//            }
//            break;
//
//        case MENU_MODE:
//            switch (cursorIndex)
//            {
//                case 2: currentMenu = MENU_PRINCIPAL; cursorIndex = 0; break;
//            }
//            break;
//    }
//}

void enter_Menu(void)
{
    TickType_t now = xTaskGetTickCount();

    if (now - lastPressBtnPB9 < pdMS_TO_TICKS(100)) return; // anti-rebond
    lastPressBtnPB9 = now;
    lastMenuActivity = now;

    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

    Flags.User_Button++;
    if (Flags.User_Button > 6) Flags.User_Button = 1;

    cursorIndex++;

    // Gestion du débordement selon le menu
    if (currentMenu == MENU_PRINCIPAL && cursorIndex >= mainMenuLength)
        cursorIndex = 0;
    else if (currentMenu == MENU_SETTINGS && cursorIndex >= settingsMenuLength)
        cursorIndex = 0;
    else if (currentMenu == MENU_MODE && cursorIndex >= modeMenuLength)
        cursorIndex = 0;
    else if (currentMenu == MENU_ANIMATIONS && cursorIndex >= AnimMenuLength)
        cursorIndex = 0;

    // Gestion du scroll
    if (cursorIndex >= topIndex + maxVisibleItems)
        topIndex = cursorIndex - (maxVisibleItems - 1);
    else if (cursorIndex < topIndex)
        topIndex = cursorIndex;

    // Comportement selon type de menu
    if (currentMenu == MENU_ATTENDRE ||
        IS_SUBMENU(currentMenu) ||
        IS_DISPLAY(currentMenu))
    {
        currentMenu = MENU_PRINCIPAL;
        cursorIndex = 0;
        topIndex = 0;
    }
}

void checkMenuTimeout(void) {
    TickType_t now = xTaskGetTickCount();

    //Ajouter condition sur le menu car on veut pas se mettre en veille dans le mode lancement par exemple

    // Si plus de 20 secondes sans appuyer mode veille
    if ((now - lastMenuActivity) <= pdMS_TO_TICKS(20000))
        return;

    currentMenu = MENU_ATTENDRE;


    lastMenuActivity = now;
}

void drawTextMenu(const char** items, uint8_t length, uint8_t cursor) {
    for (uint8_t i = 0; i < length; i++) {
        ssd1306_SetCursor(5, i * 12);
        if (i == cursor)
            ssd1306_WriteString(">", Font_7x10, White);
        ssd1306_SetCursor(20, i * 12);
        ssd1306_WriteString((char*)items[i], Font_7x10, White);
    }
}

void displayMenu(void) {

    ssd1306_Fill(Black);

    switch (currentMenu) {
        case MENU_PRINCIPAL:
            drawTextMenu(mainMenuItems, mainMenuLength, cursorIndex);
            break;

        case MENU_SETTINGS:
            drawTextMenu(settingsMenuItems, settingsMenuLength, cursorIndex);
            if (cursorIndex == 0 || cursorIndex == 1) {
                ssd1306_SetCursor(5, 50);
                char contrastStr[32];
                int percent = brightnessToPercent(brightnessLevel);
                snprintf(contrastStr, sizeof(contrastStr), "Luminosite: %d%%", percent);
                ssd1306_WriteString(contrastStr, Font_7x10, White);

                // Barre de progression
                uint8_t barWidth = (brightnessLevel * 80) / 255;
                uint8_t barHeight = 10;
                for (uint8_t i = 0; i < barHeight; i++)
                    ssd1306_Line(20, 60 + i, 20 + barWidth, 60 + i, White);
            }
            break;

        case MENU_MODE:
            ssd1306_SetCursor(0, 54);
            ssd1306_WriteString("[ Mode ]", Font_7x10, White);
            drawTextMenu(modeMenuItems, modeMenuLength, cursorIndex);
            break;

        case MENU_ANIMATIONS:
            for (uint8_t i = 0; i < maxVisibleItems; i++) {
                uint8_t itemIndex = topIndex + i;
                if (itemIndex >= AnimMenuLength) break;
                ssd1306_SetCursor(5, i * 12);
                if (itemIndex == cursorIndex)
                    ssd1306_WriteString(">", Font_7x10, White);
                ssd1306_SetCursor(20, i * 12);
                ssd1306_WriteString((char*)AnimMenuItems[itemIndex], Font_7x10, White);
            }
            break;

        case MENU_DEMARRER:
            ssd1306_SetCursor(10, 20);
            ssd1306_WriteString("Lancement...", Font_11x18, White);
            break;

        // --- Affichages spéciaux / animations ---
        case DISP_CAT:        oled_cat(); break;
        case DISP_MOUSE:      oled_mouse(); break;
        case DISP_CHASOURO:
            ssd1306_SetCursor(20, 20);
            ssd1306_WriteString("ChaSouRo", Font_11x18, White);
            break;
        case DISP_SCAREDCAT:

//        	scared_cat_bitmap_tick();
        	ssd1306_SetCursor(20, 20);
			ssd1306_WriteString("Out of FLASH", Font_11x18, White);
        	break;
        case DISP_INIT:       Init_bitmap_tick(); break;
        case DISP_KIRBY:      kirby_bitmap_tick(); break;
        case MENU_ATTENDRE:   kirby_bitmap_tick(); break;
        case MENU_START:	  break;
		case MENU_END:		  break;
        case DISP_START:	  break;
        case DISP_END:		  break;

    }

    ssd1306_UpdateScreen();
}


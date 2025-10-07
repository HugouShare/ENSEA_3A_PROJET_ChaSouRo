/*
 * OLED.c
 *
 *  Created on: Sep 30, 2025
 *      Author: nelven
 */

#include "oled.h"
#include <string.h>
#include <stdio.h>
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "bitmaps.h"

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
uint32_t lastButtonPress = 0;

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
		HAL_Delay(40);
		i++;
	}

}

void scared_cat_bitmap(){
	int i = 0;
	while (i < scared_cat_bitmap_allArray_LEN){
		ssd1306_Fill(Black);
		ssd1306_DrawBitmap(0,0,scared_cat_bitmap_allArray[i],128,64,White);
		ssd1306_UpdateScreen();
		HAL_Delay(40);
		i++;
	}

}

void kirby_bitmap_tick(void) {
    uint32_t now = HAL_GetTick();

    if (now - lastUpdateKirby > 120) {  // avance toutes les 40 ms
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

void scared_cat_bitmap_tick(void) {
    uint32_t now = HAL_GetTick();

    if (now - lastUpdateCat > 120) {  // avance toutes les 40 ms sans bloquer comme un while
        lastUpdateCat = now;

        ssd1306_Fill(Black);
        ssd1306_DrawBitmap(0, 0,
            scared_cat_bitmap_allArray[catFrame],
            128, 64, White);
        ssd1306_UpdateScreen();

        catFrame++;
        if (catFrame >= scared_cat_bitmap_allArray_LEN) {
            catFrame = 0;
        }
    }
}

void Init_bitmap_tick(void) {
    uint32_t now = HAL_GetTick();

    if (now - lastUpdateInit > 120) {  // avance toutes les 40 ms sans bloquer comme un while
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
		  HAL_Delay(100);
		  oled_cat();
		  HAL_Delay(50);
	};
	if (Flags.User_Button == 2){
		  HAL_Delay(100);
		  oled_mouse();
		  HAL_Delay(50);

	};
	if (Flags.User_Button == 3){
		  HAL_Delay(100);
		  ssd1306_Fill(Black);
		  ssd1306_SetCursor(20,20);
		  ssd1306_WriteString("ChaSouRo", Font_11x18, White);
		  ssd1306_UpdateScreen();
		  HAL_Delay(50);
	};
	if (Flags.User_Button == 4){
//		  HAL_Delay(100);
//		  kirby_bitmap();
//		  HAL_Delay(40);
		kirby_bitmap_tick();
	};
	if (Flags.User_Button == 5){
//		  HAL_Delay(100);
//		  scared_cat_bitmap();
//		  HAL_Delay(40);
		scared_cat_bitmap_tick();

		};
	if (Flags.User_Button == 6){
//		  HAL_Delay(100);
//		  scared_cat_bitmap();
//		  HAL_Delay(40);
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


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

    if (GPIO_Pin == GPIO_PIN_13){
    	Flags.User_Button++;
    	if (Flags.User_Button > 6) Flags.User_Button = 1;
    }

    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {

        // === Navigation du curseur ===
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

        // Ajustement de la "fenêtre de scroll"
        if (cursorIndex >= topIndex + maxVisibleItems)
            topIndex = cursorIndex - (maxVisibleItems - 1);
        else if (cursorIndex < topIndex)
            topIndex = cursorIndex;

        // === Comportement selon le type de menu ===
        if (currentMenu == MENU_ATTENDRE) {
            currentMenu = MENU_PRINCIPAL;
            cursorIndex = 0;
            topIndex = 0;
        }
        else if (IS_SUBMENU(currentMenu)) {
            // Tout sous-menu retourne au menu principal
            currentMenu = MENU_PRINCIPAL;
            cursorIndex = 0;
            topIndex = 0;
        }
        else if (IS_DISPLAY(currentMenu)) {
            // Tout écran d’affichage retourne au menu des animations
            currentMenu = MENU_ANIMATIONS;
            cursorIndex = 0;
            topIndex = 0;
        }

        // Mettre à jour le dernier appui
        lastButtonPress = HAL_GetTick();
    }
}


void checkMenuTimeout(void) {
    uint32_t now = HAL_GetTick();

    // Si plus de 5 secondes sans appuyer → valider la sélection
    if ((now - lastButtonPress) <= 5000)
        return;

    switch (currentMenu) {

        // === Menu principal ===
        case MENU_PRINCIPAL:
            switch (cursorIndex) {
                case 0: currentMenu = MENU_DEMARRER; break;
                case 1: currentMenu = MENU_SETTINGS;  cursorIndex = 0; break;
                case 2: currentMenu = MENU_MODE;      cursorIndex = 0; break;
                case 3: currentMenu = MENU_ANIMATIONS; cursorIndex = 0; topIndex = 0; break;
                case 4: currentMenu = MENU_ATTENDRE; break;
                default: break;
            }
            break;

        // === Menu Settings ===
        case MENU_SETTINGS:
            switch (cursorIndex) {
                case 0:
                    adjustBrightness(true);
                    lastButtonPress = HAL_GetTick();
                    break;
                case 1:
                    adjustBrightness(false);
                    lastButtonPress = HAL_GetTick();
                    break;
                case 2:
                    currentMenu = MENU_PRINCIPAL;
                    cursorIndex = 0;
                    break;
                default: break;
            }
            break;

        // === Menu Animations ===
        case MENU_ANIMATIONS:
            switch (cursorIndex) {
                case 0: currentMenu = DISP_CAT; break;
                case 1: currentMenu = DISP_MOUSE; break;
                case 2: currentMenu = DISP_CHASOURO; break;
                case 3: currentMenu = DISP_SCAREDCAT; break;
                case 4: currentMenu = DISP_INIT; break;
                case 5: currentMenu = DISP_KIRBY; break;
                case 6:
                    currentMenu = MENU_PRINCIPAL;
                    cursorIndex = 0;
                    break;
                default: break;
            }
            break;

        // === Menu Mode ===
        case MENU_MODE:
            switch (cursorIndex) {
                case 0: /* futur mode 1 */ break;
                case 1: /* futur mode 2 */ break;
                case 2:
                    currentMenu = MENU_PRINCIPAL;
                    cursorIndex = 0;
                    break;
                default: break;
            }
            break;

        default:
            break;
    }

    // Évite de valider en boucle
    lastButtonPress = now;
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
        case DISP_SCAREDCAT:  scared_cat_bitmap_tick(); break;
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


/* Brouillon avec pleins de if --> compliqué pour évoluer et ajouter de nouveaux menus */

//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//    if (GPIO_Pin == GPIO_PIN_13) {
//    	Flags.User_Button++;
//    			if (Flags.User_Button > 6) {
//    				Flags.User_Button = 1;
//    			}
//		if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
//					// Avancer le curseur
//					cursorIndex++;
//					if (cursorIndex >= AnimMenuLength) {cursorIndex = 0;topIndex = 0;}
//
//					// Ajuster la fenêtre
//					if (cursorIndex >= topIndex + maxVisibleItems) {
//					    topIndex = cursorIndex - (maxVisibleItems - 1);
//					}
//
//					if (currentMenu == MENU_PRINCIPAL && cursorIndex >= mainMenuLength) cursorIndex = 0;
//					if (currentMenu == MENU_SETTINGS && cursorIndex >= settingsMenuLength) cursorIndex = 0;
//					if (currentMenu == MENU_MODE && cursorIndex >= modeMenuLength) cursorIndex = 0;
////					if (currentMenu == MENU_ANIMATIONS && topIndex >= AnimMenuLength) cursorIndex = 0;
//					if (currentMenu == MENU_ATTENDRE) {currentMenu = MENU_PRINCIPAL; cursorIndex = 0;}
//					if (IS_SUBMENU(currentMenu)) {
//						currentMenu = MENU_PRINCIPAL;cursorIndex = 0; topIndex = 0;
//					}
//			        if (IS_DISPLAY(currentMenu)) {
//			            // n’importe quel DISP_ ramène au menu anim
//			            currentMenu = MENU_ANIMATIONS;cursorIndex = 0; topIndex = 0;
//			        }
//
//					// Mettre à jour le "dernier appui"
//					lastButtonPress = HAL_GetTick();
//				}
//
//    }
//}

//void checkMenuTimeout(void) {
//    uint32_t now = HAL_GetTick();
//
//    // Si plus de 5 secondes sans appuyer → valider
//    if ((now - lastButtonPress) > 5000) {
//        if (currentMenu == MENU_PRINCIPAL) {
//            if (cursorIndex == 0) currentMenu = MENU_DEMARRER;
//            else if (cursorIndex == 1) { currentMenu = MENU_SETTINGS; cursorIndex = 0; }
//            else if (cursorIndex == 2) { currentMenu = MENU_MODE; cursorIndex = 0; }
//            else if (cursorIndex == 3) { currentMenu = MENU_ANIMATIONS; cursorIndex = 0; topIndex = 0; }
//            else if (cursorIndex == 4) currentMenu = MENU_ATTENDRE;
//        }
//        else if (currentMenu == MENU_SETTINGS) {
//        	if (cursorIndex == 0){
//        		adjustBrightness(true);
//				lastButtonPress = HAL_GetTick();
//        	}
//        	if (cursorIndex == 1){
//        		adjustBrightness(false); // false -> decrease brightness
//				lastButtonPress = HAL_GetTick();
//        	}
//            if (cursorIndex == 2) { // Retour
//                currentMenu = MENU_PRINCIPAL;
//                cursorIndex = 0;
//            }
//        }
//        else if (currentMenu == MENU_ANIMATIONS) {
//        	if (cursorIndex == 0) {
//        		currentMenu = DISP_CAT;
//        	}
//        	if (cursorIndex == 1) {
//        		currentMenu = DISP_MOUSE;
//			}
//        	if (cursorIndex == 2) {
//        		currentMenu = DISP_CHASOURO;
//			}
//        	if (cursorIndex == 3) {
//        		currentMenu = DISP_SCAREDCAT;
//			}
//        	if (cursorIndex == 4) {
//        		currentMenu = DISP_INIT;
//			}
//        	if (cursorIndex == 5) {
//				currentMenu = DISP_KIRBY;
//			}
//        	if (cursorIndex == 6) {
//				currentMenu = MENU_PRINCIPAL;
//				cursorIndex = 0;
//			}
//        }
//        else if (currentMenu == MENU_MODE){
//        	if (cursorIndex == 0) {
////				currentMenu = DISP_CAT;
//			}
//        	if (cursorIndex == 1) {
////				currentMenu = DISP_MOUSE;
//			}
//        	if (cursorIndex == 2) {
//				currentMenu = MENU_PRINCIPAL;
//			}
//        }
//
//        // Réinitialiser le timer (évite de revalider en boucle)
//        lastButtonPress = now;
//    }
//}

//void displayMenu() {
//    ssd1306_Fill(Black);
//
//    if (currentMenu == MENU_PRINCIPAL) {
//        for (uint8_t i = 0; i < mainMenuLength; i++) {
//            ssd1306_SetCursor(5, i * 12);
//            if (i == cursorIndex) {
//                ssd1306_WriteString(">", Font_7x10, White);
//            }
//            ssd1306_SetCursor(20, i * 12);
//            ssd1306_WriteString(mainMenuItems[i], Font_7x10, White);
//        }
//    }
////    else if (currentMenu == MENU_SETTINGS) {
////        for (uint8_t i = 0; i < settingsMenuLength; i++) {
////            ssd1306_SetCursor(5, i * 12);
////            if (i == cursorIndex) {
////                ssd1306_WriteString(">", Font_7x10, White);
////            }
////            ssd1306_SetCursor(20, i * 12);
////            ssd1306_WriteString(settingsMenuItems[i], Font_7x10, White);
////        }
////    }
//    else if (currentMenu == MENU_SETTINGS) {
//            for (uint8_t i = 0; i < settingsMenuLength; i++) {
//                ssd1306_SetCursor(5, i * 12);
//                if (i == cursorIndex) {
//                    ssd1306_WriteString(">", Font_7x10, White);  // Curseur
//                }
//                ssd1306_SetCursor(20, i * 12);
//                ssd1306_WriteString(settingsMenuItems[i], Font_7x10, White);
//            }
//
//            // Si on est sur le menu "Luminosité"
//            if (cursorIndex == 0 || cursorIndex == 1) {
//                ssd1306_SetCursor(5, 50);
//                char contrastStr[32];
//				int percent = brightnessToPercent(brightnessLevel);
//				snprintf(contrastStr, sizeof(contrastStr), "Luminosite: %d%%", percent);
//                ssd1306_WriteString(contrastStr, Font_7x10, White);
//
//                // Afficher la barre de progression pour la luminosité avec la fonction de Bresenham
//                uint8_t barWidth = (brightnessLevel * 80) / 255;  // Largeur de la barre
//                uint8_t barHeight = 10;  // Hauteur de la barre (plus grande)
//
//                // Dessiner une barre plus épaisse
//                for (uint8_t i = 0; i < barHeight; i++) {
//                    ssd1306_Line(20, 60 + i, 20 + barWidth, 60 + i, White);
//                }
//            }
//	}
//    else if (currentMenu == MENU_ANIMATIONS) {
//        for (uint8_t i = 0; i < maxVisibleItems; i++) {
//            uint8_t itemIndex = topIndex + i;
//            //if (itemIndex >= mainMenuLength) break; // éviter débordement
//
//            ssd1306_SetCursor(5, i * 12);
//            if (itemIndex == cursorIndex) {
//                ssd1306_WriteString(">", Font_7x10, White);
//            }
//            ssd1306_SetCursor(20, i * 12);
//            ssd1306_WriteString(AnimMenuItems[itemIndex], Font_7x10, White);
//        }
//    }
////            for (uint8_t i = 0; i < AnimMenuLength; i++) {
////                ssd1306_SetCursor(5, i * 12);
////                if (i == cursorIndex) {
////                    ssd1306_WriteString(">", Font_7x10, White);
////                }
////                ssd1306_SetCursor(20, i * 12);
////                ssd1306_WriteString(AnimMenuItems[i], Font_7x10, White);
////            }
////        }
//    else if (currentMenu == MENU_DEMARRER) {
//        ssd1306_SetCursor(10, 20);
//        ssd1306_WriteString("Lancement...", Font_11x18, White);
//    }
//    else if (currentMenu == MENU_MODE) {
//    	ssd1306_SetCursor(0, 54);
//    	ssd1306_WriteString("[ Mode ]", Font_7x10, White);
//    	for (uint8_t i = 0; i < modeMenuLength; i++) {
//			ssd1306_SetCursor(5, i * 12);
//			if (i == cursorIndex) {
//				ssd1306_WriteString(">", Font_7x10, White);  // Curseur
//			}
//			ssd1306_SetCursor(20, i * 12);
//			ssd1306_WriteString(modeMenuItems[i], Font_7x10, White);
//			}
//    	}
//
//    else if (currentMenu == MENU_ATTENDRE) {
//    		kirby_bitmap_tick();
//        }
//    else if (currentMenu == DISP_CAT) {
//			oled_cat();
//		}
//    else if (currentMenu == DISP_MOUSE) {
//			oled_mouse();
//		}
//    else if (currentMenu == DISP_CHASOURO) {
//			ssd1306_Fill(Black);
//		  	ssd1306_SetCursor(20,20);
//		    ssd1306_WriteString("ChaSouRo", Font_11x18, White);
//		  	ssd1306_UpdateScreen();
//		}
//    else if (currentMenu == DISP_SCAREDCAT) {
//			scared_cat_bitmap_tick();
//    	}
//    else if (currentMenu == DISP_INIT) {
//			Init_bitmap_tick();
//    	}
//    else if (currentMenu == DISP_KIRBY) {
//    		kirby_bitmap_tick();
//    	}
//    ssd1306_UpdateScreen();
//}



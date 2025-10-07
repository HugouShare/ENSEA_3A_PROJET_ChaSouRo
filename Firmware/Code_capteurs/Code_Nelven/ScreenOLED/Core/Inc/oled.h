/*
 * OLED.h
 *
 *  Created on: Sep 30, 2025
 *      Author: nelven
 */

#ifndef INC_OLED_H_
#define INC_OLED_H_

#include <stdbool.h>
#include <stdint.h>

// fonctions publiques existantes (tu peux les laisser comme avant)
void oled_git(void);
void oled_garfield(void);
void oled_cat(void);
void kirby_bitmap(void);
void scared_cat_bitmap(void);
void screen_stats(void);
void Init_bitmap(void);
void Init_bitmap_tick(void);
void adjustBrightness(bool);
void checkMenuTimeout(void);
void displayMenu(void);
void drawTextMenu(const char** items, uint8_t length, uint8_t cursor);

// nouvelle fonction pour démarrer les tâches FreeRTOS
void oled_StartTasks(void);

// structures et enum (inchangés)
typedef struct {
    int User_Button;
} SystFlag;

extern SystFlag Flags;

typedef enum {
    MENU_PRINCIPAL,
    MENU_SETTINGS,
    MENU_MODE,
    MENU_ANIMATIONS,
    MENU_ATTENDRE,

    MENU_START,
    MENU_DEMARRER,
    MENU_END,

    DISP_START,
    DISP_CAT,
    DISP_MOUSE,
    DISP_CHASOURO,
    DISP_SCAREDCAT,
    DISP_INIT,
    DISP_KIRBY,
    DISP_END,
} MenuState;

#endif /* INC_OLED_H_ */

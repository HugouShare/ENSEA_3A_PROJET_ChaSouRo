# Fonctionnement

## Init 

- Chaque moteur tourne à son tour et quand un moteur tourne, une LED associée s'allume

## Principe
- Une structure de pointeurs motorDriver à été mis en place qui associe à un moteur L ou R, son timer et son FWD/REV.
- 2 fonctions
	- motorRun qui séléctionne le moteur et la vitesse (CCR [0:1000])
	- motorSet utilise motor Run et la durée pendant laquelle il tourne (en ms) puis met le moteur à l'arrêt
- Fait en FreeRTOS (mais utilise HAL_Delay donc les passer vTaskDelay())

## Utilisation

- Tâche motor créée qui va faire tourner l'un des deux moteurs lorsque son bouton (ISR avec sémaphore) associé à été appuyé.
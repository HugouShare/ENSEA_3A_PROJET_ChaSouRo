# Contexte général du projet  

Comme expliqué précédemment, le but de notre projet est de concevoir un robot capable de jouer au jeu du chat et de la souris de manière complètement autonome sur une table.   
Pour plus d'informations sur le projet : [cliquez ici](Descriptif%20du%20projet.pdf)  

Pour se faire, seuls les composants utilisés dans le projet nous sont imposés : [liste des composants](Liste%20des%20composants%20disponibles.pdf)  
Charge à nous :  
- de développer une PCB
- de programmer les différents modules : capteurs, actionneurs, écran OLED et module de communication bluetooth
- d'intégrer tous les modules ensemble, de tester et vérifier le bon fonctionnement de l'ensemble  

# Aspects techniques liés au projet

## Schéma d'architecture fonctionnelle  

Après une première réunion portant sur l'architecture fonctionnelle de notre robot, voici le schéma fonctionnel que nous adoptons pour notre robot :  
<img width="1113" height="593" alt="image" src="https://github.com/user-attachments/assets/ec6f46da-f3cf-4715-b50a-5d9e955e3537" />  

## Diagramme des tâches  

Insérer ici un diagramme des tâches  

## Organisation du projet sur STM32CubeIDE  

D'un point de vue global, nous avons choisi d'organiser notre projet de la manière suivante :  

| Fichiers sources | Fichiers headers |
|--------|---------|
| <img width="276" height="641" src="https://github.com/user-attachments/assets/1488964b-004f-4a89-a848-ebb90d078acb" /> | <img width="225" height="597" src="https://github.com/user-attachments/assets/9307b93e-7e89-43c4-908d-4a65494c34fd" /> |  

### 📝 Description détaillée fichiers sources

#### 📦 Dossier `actuators/` – Gestion des actionneurs

| Fichier | Rôle |
|--------|------|
| **behavior.c** | Implémente les comportements de haut niveau (navigation et stratégie de déplacement). |
| **control.c** | Contient les fonctions de contrôle moteur, coordination PID, gestion des vitesses/couple. |
| **encoder.c** | Gestion des encodeurs : lecture des ticks, calcul de vitesse et position. |
| **motor.c** | Interface bas niveau pour piloter les moteurs (PWM, direction, enable/disable). |
| **pid.c** | Implémentation des régulateurs PID utilisés par `control.c` et `behavior.c`. |

#### 📦 Dossier `bluetooth/` – Communication Bluetooth

| Fichier | Rôle |
|--------|------|
| **bluetooth.c** | Gestion du module Bluetooth : initialisation, envoi/réception de données. |

#### 📦 Dossier `oled_screen/` – Afficheur OLED

| Fichier | Rôle |
|--------|------|
| **oled.c** | Gestion d’un écran OLED : affichage de texte, images, et initialisation. |

#### 📦 Dossier `sensors/` – Capteurs du robot

| Fichier | Rôle |
|--------|------|
| **accelerometers.c** | Lecture de l’accéléromètre (IMU), calibration, filtrage. |
| **lidar.c** | Interface avec un LiDAR : récupération des distances, gestion du capteur. |
| **tofs.c** | Gestion des capteurs Time-of-Flight. |

#### 🧵 FreeRTOS

| Fichier | Rôle |
|--------|------|
| **app_freertos.c** | Définition des tâches FreeRTOS, files de messages, mutex et scheduling. |

#### ⚙️ Core système

| Fichier | Rôle |
|--------|------|
| **main.c** | Point d’entrée du programme, initialisations des différents capteurs, créations des différentes tâches et démarrage du RTOS. |
| **syscalls.c** | Implémente les fonctions système nécessaires (malloc, printf…). |
| **sysem.c** | Gestion du système et interrupt handlers (auto-généré). |
| **system_stm32g4xx.c** | Configuration de l’horloge système et initialisation MCU. |

#### 🔧 Drivers Hardware Abstraction Layer (HAL)

| Fichier | Rôle |
|--------|------|
| **dma.c** | Initialisation du DMA pour les transferts mémoire ↔ périphériques. |
| **gpio.c** | Configuration des broches GPIO (mode, pull-up/down, vitesse…). |
| **i2c.c** | Initialisation du bus I2C utilisé par IMU, OLED, ToF. |
| **tim.c** | Configuration des timers : PWM moteurs, interruptions, timebase. |
| **usart.c** | Gestion de la communication série UART (console, Bluetooth). |
| **stm32g4xx_hal_msp.c** | Fonctions MSP auto-générées : clocks, GPIO, interruptions. |
| **stm32g4xx_it.c** | Gestion des interruptions globales du microcontrôleur. |
| **stm32g4xx_hal_timebase_tim.c** | Gestion de la base de temps HAL via TIM. |  

### 📝 Description détaillée fichiers headers

#### 📦 Dossier `oled_screen/` – Gestion de l’OLED

| Fichier | Rôle |
|--------|------|
| **bitmaps.h** | Ressources graphiques (icônes, images) utilisées par l’écran OLED. |  

#### 🧵 FreeRTOS

| Fichier | Rôle |
|--------|------|
| **FreeRTOSConfig.h** | Configuration du kernel FreeRTOS (priorités, timers, heap…). |
| **freeRTOS_tasks_priority.h** | Définition des priorités de tâche et organisation du multitâche. |

#### ⚙️ Core du programme

| Fichier | Rôle |
|--------|------|
| **main.h** | Déclarations globales, includes principaux et prototypes de `main.c`. |

# Point HARDWARE   

Décrire ici les choix faits lors de la conception du PCB : choix de placement, d'agencement, de protocole, d'empreintes... + pourquoi avoir choisi les composants que nous avons pris.  

# Point SOFTWARE & FIRMWARE

Décrire ici les choix faits lors de l'implémentation du code C : pourquoi avoir organisé notre code comme ça par rapport aux fichiers et à la structure globales, pourquoi avoir choisi de faire des drivers et structure et du FreeRTOS, décrire + explique pourquoi les comportements que nous avons choisi pour le robot : ROOMBA, CHAT, SOURIS, EDGE.  

# Problèmes rencontrés lors du projet    

Décrire ici les problèmes rencontrés lors du projet 

# Rapport individuel des tâches réalisées au sein du projet  

Suite à cela, après une nouvelle réunion, chaque membre du groupe se voit attribuer diverses missions.  
Voici ce que chaque membre du groupe réalise au sein du projet...  

### Nelven THEBAULT  

- Faire partie Hardware : PCB, schematic, footprint, soudure
- Partie écran
- Bipper 
- Coordination avec méca et intégration

### Hugo CARVALHO FONTES  

Hardware : 
- Conception d'une PCB : schematique & routage
- Soudure des composants sur la carte finale

Software/Firmware : 
- Module bluetooth HC-05
  - Création d'une application sous android studio afin de pouvoir lancer le robot ou l'arrêter d'urgence, mais aussi afin de recevoir en temps-réel les coordonnés du robot sur la table
  - Implémentation du code C en Free-RTOS et avec driver sur STM32G431CBU6
- Module TOFs VL53L0X
  - Implémentation du code C en Free-RTOS et avec driver sur STM32G431CBU6 afin de faire fonctionner 4 TOFs ensemble
- Module accéléromètre ADXL345
  - Implémentation du code C en Free-RTOS et avec driver sur STM32G431CBU6
- Intégration de tous les différents modules afin d'obtenir un robot fonctionnel  

### Arthur Cesar NKWA NJITCHOU  

Hardware :
- Conception d'un PCB : Réalisation de la schématique et du routage du circuit imprimé (PCB).
- Recherche de composants : Participation active à la sélection des composants électroniques.
- Soudure : Assemblage et soudure des composants sur le PCB final avec l'équipe.

Software :
- Développement en C : Implementation du code pour l'accéléromètre ADXL345, permettant de détecter les chocs brusques lors des collisions du robot.

Mécanique :
- Conception des supports : Création de supports pour les capteurs TOF et l'écran, ainsi que perçage pour l'interrupteur et le buzzer.
- Amélioration des pattes : Optimisation des pattes des bases supérieures pour une meilleure stabilité.
- Impression 3D : Impression des différentes parties du robot à l'aide d'une imprimante 3D.
- Assemblage partie mécanique du robot.


### Hugo CORDI  



# Résultat final le jour J  



# Conclusion du projet   

Décrire ici une conclusion

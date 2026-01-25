# Modifications sur la V1

- Problèmes sur la taille des empreintes pour les capacités >= 10uF -> Passage en 0603
- Changement pad sw wurth vers un connecteur (plus propre) -> JST_PH Batterie +/- (Pin_x2) en JS_PH_ Batterie +/- et +/IN du switch wurth (Pin_x4)
- Erreur sur un des deux drivers où les silkscreen des OUT1 et OUT2 sont inversé par rapport au pin (Quand on se place dans le sens avec TX4/RX4 du ST-Link -> Driver gauche)
- Ajout Silkscreen sur ST-Link pour simplifier la connexion
- Modification schématique/Nom du bon NMOSFET -> SQ2310ES
- Changement de la capacité de découplage du VDDA : 47uF -> 22uF
- Oublie des capacités pour limiter les rebonds sur USER1 et USER2 -> ajout de 0.1uF
- Inversion sur le PCB des RX3/TX3 sur les pins PB10/PB11 du STM32
- Problème d'angle droit sur la ligne SDA3 enlevé (peut-être incidence sur l'I2C3)
---
Modification non faite mais à connaître pour de futurs projets 
- Utilisation du PB4 (BOOT0) pour le xshunt du ToF1 → pose problème car relié en interne à un GPIO relié ici au FWD ou REV d'un driver moteur → réinitialise la carte.
    - Solution : ne jamais utiliser la broche `BOOT0` pour autre chose même si l'on est censé pouvoir le faire
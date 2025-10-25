# Modifications sur la V1

- Problèmes sur la taille des empreintes pour les capacités >= 10uF -> Passage en 0603
- Changement pad sw wurth vers un connecteur (plus propre) -> JST_PH Batterie +/- (Pin_x2) en JS_PH_ Batterie +/- et +/IN du switch wurth (Pin_x4)
- Erreur sur un des deux drivers où les silkscreen des OUT1 et OUT2 sont inversé par rapport au pin (Quand on se place dans le sens avec TX4/RX4 du ST-Link -> Driver gauche)
- Ajout Silkscreen sur ST-Link pour simplifier la connexion
- Modification schématique/Nom du bon NMOSFET -> SQ2310ES
- Changement de la capacité de découplage du VDDA : 47uF -> 22uF
- Oublie des capacités pour limiter les rebonds sur USER1 et USER2 -> ajout de 0.1uF
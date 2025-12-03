# Fonctionnement

## Init 

- Fonction `Init_motors()` qui démarre les PWM et fais tourner le robot sur lui même à gauche puis à droite.

## Principe
- Une structure de pointeurs motorDriver à été mis en place qui associe à un moteur L ou R, son timer et son FWD/REV.
- 2 fonctions
	- `Motors_Set()` qui vient modifier une structure avec les valeurs de vitesses et durées souhaitées dans les deux moteurs
	- `Motor_SetSpeed()` qui applique dans la tâche `Task_motor` dédiée et permet au tout de ne pas être bloquant 

## Utilisation

Ici configuré dans le callback des boutons 2 consignes :  
- une où le robot tourne sur lui même sens droite
- une où le robot avance tout droit lentement

Reste à mettre en plac ue fonction pour corrige la trajectoire qui n'est pas parfaitement droite.
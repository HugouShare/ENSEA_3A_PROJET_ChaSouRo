# 🤖 ENSEA_3AProject_ChaSouRo 🤖

![Version](https://img.shields.io/badge/version-1.0-blue)
![Status](https://img.shields.io/badge/status-en%20développement-yellow)

## 📋 Table des matières

- [Équipe](#équipe)
- [Introduction](#introduction)
- [Missions & Objectifs](#missions-et-objectifs)
- [Timeline](#planning-d'avancement)
- [Journal de bord](#journal-de-bord)

---

## 👨🏽 Équipe

<div align="left">

   **🙋‍♂️ [Hugo CARVALHO FONTES](https://github.com/HugouShare)**

   **🙋‍♂️ [Nelven THÉBAULT](https://github.com/NelvTheb)**

   **🙋‍♂️ [Hugo CORDI](https://github.com/Lynxlegrand)**

   **🙋‍♂️ [Arthur Cesar NJITCHOU NKWA](https://github.com/ArthurNjitchou)**

</div>

## ⚙️ Introduction

- Nous réalisons ce projet dans le cadre de notre dernière année d'étude en spécialité électronique et systèmes embarqués à l'[ENSEA](https://www.ensea.fr/fr) située à Cergy.
- Le but du projet est de réaliser deux robots : l'un étant le robot chat et l'autre le robot souris. Les deux robots évoluent ensuite de manière autonome sur une table. Le but du jeu est alors le suivant : le robot chat doit tenter d'attraper le robot souris qui doit donc tenter d'échapper au robot chat. Une fois que le robot chat attrape le robot souris, les rôles s'inversent et la partie continue. 

## 🎯 Missions & Objectifs

Les missions principales sont les suivantes (réparties selon différents niveaux) :
  - **Niveau 0 :**
    - Le robot se déplace
    - Il ne tombe pas de la table
  - **Niveau 1 :**
    - Il détecte et se dirige vers un objet
    - Ou s’en éloigne s’il n’est pas le chat
  - **Niveau 2 :**
    - Il change de comportement (proie/prédateur) après un contact
    - Il fonctionne avec plusieurs robots sur la table
  - **Niveau 3 :**
    - Il n’est pas affecté par les obstacles hors de la table
    - Il est donc capable de se localiser

## ⌚ Timeline

```mermaid
%%{init: {"theme":"default", "themeVariables": {
  "primaryColor": "#4F46E5",
  "primaryTextColor": "#ffffff",
  "primaryBorderColor": "#312E81",
  "lineColor": "#6366F1",
  "secondaryColor": "#E0E7FF",
  "tertiaryColor": "#A5B4FC"
}}}%%

timeline
    title 🚀 Évolution du projet
    section Phase 1 - conception du PCB
      Séance 1 : Point réunion + schéma architectural / BOM
      Séance 2 et 3 : Schéma électronique annoté
      Séance 4 : Corrections Schéma / BOM Finale
      Séance 5 : Point réunion + placement
      Séance 6 : Placement corrigé
      Séance 7 et 8 : Routage
      Séance 9 et 10 : Corrections Routage, export...
    section Phase 2 - mécanique 3D + prise en main et programmation des différents capteurs et actionneurs
      Séance 1 : Point réunion + architecture de subsomption + architecture schéma logicielle + soudure composants sur PCBs + vérification du bon fonctionnement des PCBs
      Séance 2 et 3 et 4 : Prise en main & programmation du LiDar + accéléromètre + écran OLED + module bluetooth avec développement application apk
      Séance 5 : Point réunion + vérification du bon fonctionnement des différents modules
      Séance 6 et 7 et 8 : Développement de l'odométrie + programmation des moteurs + 1x TOF puis 4x TOFs + modification de la structure mécanique du robot pour intégration des 4 TOFs et écran OLED
      Séance 9 et 10 : Impression 3D de la structure mécanique du robot + début de l'intégration des différents modules + finalisation de la programmation des 4x TOFs ensemble 
    section Phase 3 - Intégration, tests & vérifications
      Séance 1 et 2 : Point réunion + intégration, tests & vérifications
      Séance 3 : DEMONSTRATION FINALE
```

## 📝 Journal des activités

Pour consulter notre journal des activités [cliquez ici](Documents/Journal%20de%20bord.md)  

## ✅ A faire 

- Diagramme UML (des fichiers, des drivers, pas forcément au format normé)
- Diagramme des tâches
- Diagramme hardware (signaux, composants, ...)

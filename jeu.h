#ifndef JEU_H
#define JEU_H

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "joueur.h"
#include "niveau.h"
#include "struct.h"

void lancer_partie(char* pseudo);
// Lance une nouvelle partie, initialise le joueur et le premier niveau

int lancer_niveau(int num_niveau, Joueur* joueur);
// Lance et gère un niveau

void fin_niveau(int resultat, Joueur* joueur);
// Traite la fin d’un niveau : victoire ou défaite

void sauvegarder_partie(Joueur* joueur, int niveau);
// Sauvegarde le pseudo du joueur et le niveau atteint dans un fichier

int charger_partie(char* pseudo, Joueur* joueur);
// Charge la dernière partie sauvegardée du joueur

#endif
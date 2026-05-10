#ifndef JOUEUR_H
#define JOUEUR_H

#include <string.h>
#include "struct.h"

void initialiser_joueur(Joueur* joueur, char* pseudo);
// Initialise le joueur

void deplacer_joueur(Joueur* joueur, int direction, float dt); // direction=0 (gauche), direction=1 (droite)
// Déplace le joueur horizontalement

Projectile tirer(Joueur* joueur);
// Crée un projectile tiré par le joueur

#endif

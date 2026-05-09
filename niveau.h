#ifndef NIVEAU_H
#define NIVEAU_H

#include <stdio.h>
#include <stdlib.h>
#include "joueur.h"
#include "struct.h"
#include "interface_graphique.h"

void initialiser_niveau(Niveau* niveau, int num_niveau);
// Initialise un niveau 

void maj_niveau(Niveau* niveau, Joueur* joueur, float dt);
// Met à jour un niveau avec les positions des bulles, projectiles et boss 

int niveau_termine(Niveau* niveau, Joueur* joueur);
// Retourne 0 si défaite, 1 si victoire, -1 si en cours

void deplacer_bulle(Bulle* bulle, float dt);
// Gère les déplacements d'une bulle

void diviser_bulle(Bulle* bulle, ListeBulles* liste);
// Divise une bulle en plusieurs bulles plus petites 

int collision_bulle_projectile(Bulle* bulle, Projectile* proj);
// Vérifie la collision entre une bulle et un projectile 

int collision_bulle_joueur(Bulle* bulle, Joueur* joueur);
// Vérifie la collision entre une bulle et le joueur 

int collision_bulle_bulle(Bulle* bulle1, Bulle* bulle2);
// Vérifie la collision entre deux bulles

int collision_boss_projectile(Boss* boss, Projectile* proj);
// Retourne 1 si le projectile touche le boss

void deplacer_boss(Boss* boss, float dt);
// Déplace le boss 

void boss_attaque(Boss* boss, ListeBulles* bulles);
// Fait attaquer le boss

void eclair_bulle(Bulle* bulle, Projectile* projectiles, int* nb_projectiles);
// Génère des éclairs provenant des bulles

void liberer_niveau(Niveau* niveau);
// Libère la mémoire allouée pour les bulles et projectiles d'un niveau

#endif

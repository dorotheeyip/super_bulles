#ifndef NIVEAU_H
#define NIVEAU_H

#include <stdio.h>
#include <stdlib.h>
#include "joueur.h"
#include "struct.h"

#define BOSS_PV_MAX 25
#define RAYON_HITBOX_PROJECTILE_BOSS 30.0f
#define ECLAIR_HITBOX_LARGEUR 24.0f
#define ECLAIR_HITBOX_HAUTEUR 240.0f
#define ECLAIR_DUREE_VIE 0.24f
#define ECLAIR_DELAI_HITBOX 0.08f

typedef void (*EffetCallback)(int x, int y);

void niveau_set_effets_callbacks(EffetCallback explosion_cb, EffetCallback eclair_cb);
// Permet de définir les fonctions à appeler pour les effets d'explosion et d'éclair

void initialiser_niveau(Niveau* niveau, int num_niveau);
// Initialise un niveau 

void maj_niveau(Niveau* niveau, Joueur* joueur, float dt);
// Met à jour un niveau avec les positions des bulles, projectiles et boss 

void maj_niveau_duel(Niveau* niveau, Joueur* joueur1, Joueur* joueur2, float dt);
// Met à jour un niveau en mode duel sans défaite instantanée

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

int collision_boss_projectile(Boss* boss, Projectile* proj);
// Retourne 1 si le projectile touche le boss

int collision_projectile_boss_joueur(Projectile* proj, Joueur* joueur);
// Vérifie la collision entre un projectile du boss et le joueur

void deplacer_boss(Boss* boss, float dt);
// Déplace le boss 

int boss_attaque(Boss* boss, ListeBulles* bulles, Projectile* projectiles, int nb_projectiles, Joueur* joueur, float dt, float* timer_tir);
// Déplace et génère les attaques du boss, retourne 1 si le joueur est touché

void eclair_bulle(Bulle* bulle, Projectile* projectiles, int* nb_projectiles, float dt);
// Génère des éclairs provenant des bulles

void liberer_niveau(Niveau* niveau);
// Libère la mémoire allouée pour les bulles et projectiles d'un niveau

#endif

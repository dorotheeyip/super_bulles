#ifndef STRUCT_H
#define STRUCT_H

// Structure d'un joueur
typedef struct Joueur {
    char pseudo[20];
    float x, y;          // position
    int tx, ty;          // taille (largeur, hauteur) 
    int score;           // score actuel
    int arme;            // type d’arme active
} Joueur;

// Structure d'une bulle
typedef struct Bulle {
    float x, y;          // position
    float vx, vy;        // vitesse
    int r;               // rayon
    int tx, ty;          // taille (largeur, hauteur)
    float xcoin, ycoin;  // coin superieur gauche du bounding box
    int type;            // 0=normale, 1=lance des éclairs
    int actif;
    float delai_spawn;   // temps restant avant activation
    int nb_splits;       // nombre de divisions déjà faites
    int charge_eclair;    // 1 si la bulle prépare un éclair
    float timer_eclair;   // temps de préparation restant
} Bulle;

// Liste dynamique de bulles
typedef struct ListeBulles {
    Bulle* tab;
    int nb;
    int capacite;
} ListeBulles;

// Structure d'un boss
typedef struct Boss {
    float x, y;
    int pv;
    float vitesse;
} Boss;

// Structure d'un projectile
typedef struct Projectile {
    float x, y;
    float vitesse;
    float duree_vie;
    float delai_activation;
    int actif;       // 1=en mouvement, 0=détruit
    int type;        // 0=joueur, 1=eclair, 2=boss
} Projectile;

// Structure d'un niveau
typedef struct Niveau {
    ListeBulles bulles;
    Boss boss;
    Projectile* projectiles;
    int nb_projectiles;
    float temps_restant;
} Niveau;

#endif

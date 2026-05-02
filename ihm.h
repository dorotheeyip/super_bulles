#ifndef IHM_H
#define IHM_H

// Permet la différenciation des états de jeu
typedef enum {
    ETAT_MENU = 0,
    ETAT_JEU,
    ETAT_PAUSE,
    ETAT_FIN_NIVEAU,
    ETAT_SAISIE,
    ETAT_REGLES
} EtatJeu;

// Permet le déplacement du joueur
typedef struct {
    int gauche;
    int droite;
    int pause;
} Input;

typedef struct {
    char buffer[50];
    char affichage[50];
    int index;
    int fini;
} SaisiePseudo;


void ihm_init_saisie_pseudo(SaisiePseudo *s);
void ihm_update_saisie_pseudo(SaisiePseudo *s);
void ihm_afficher_saisie_pseudo(SaisiePseudo *s, int x, int y);

// Initialisation de l'IHM
void ihm_init();

// Permet la lecture et le traitement de tous les évènements Allegro
void ihm_update();

// Libère toutes les ressources allouées par le module IHM
void ihm_fermer();

// Permet de mettre à jour l'état courant du jeu
void ihm_set_etat(EtatJeu etat); 

// Retourne l'etat courant du jeu
EtatJeu ihm_get_etat();

// Affiche le menu principal et gère les intéractions avec l'utilisateur
int  ihm_menu_principal();

// Affiche le menu de pause et gère les intéractions
int  ihm_menu_pause();

// Affiche le menu de fin et gère les interactions
int  ihm_menu_fin_niveau(int victoire);

// Affiche les règles du jeu
void ihm_afficher_regles();

// Permet la saisie du pseudo
void ihm_saisir_pseudo(char *pseudo, int taille_max);

// Permet de lire les commandes du joueur 1
void ihm_lire_inputs(Input *input);

// Permet de lire les commandes du joueur 2 (mode multijoueur)
void ihm_lire_inputs_joueur2(Input *input);

// Permet la détection de l'appui sur une touche
int ihm_touche_appuyee(int keycode);

// Permet la détection d'un appui continu sur une touche
int ihm_touche_maintenue(int keycode);

// Permet de détecter les appuis sur la sourie
void ihm_get_souris(int *x, int *y, int *clic_gauche);

// Permet de détecter les appuis de la sourie dans la zone (rx, ry, rw, rh)
int  ihm_souris_clique_zone(int rx, int ry, int rw, int rh);

// Permet de détecter une demande de quitter la page actuelle
int ihm_quitter_demande();

// Permet de détecter lorsqu'une pause est demandée
int ihm_pause_demandee();

// Bloque l'exécution jusqu'à ce qu'une touche soit appuyée
void ihm_attendre_touche();

// Remet à zéro tous les états d'input
void ihm_reset_inputs();

void ihm_maj_etats_precedents();

#endif 

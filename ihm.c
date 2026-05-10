#include <stdio.h>
#include "ihm.h"
#include <allegro.h>
#include <string.h>

#define TOUCHE_GAUCHE_J1 KEY_LEFT
#define TOUCHE_DROITE_J1 KEY_RIGHT
#define TOUCHE_PAUSE_J1  KEY_SPACE

#define TOUCHE_GAUCHE_J2 KEY_A //Q
#define TOUCHE_DROITE_J2 KEY_D //D
#define TOUCHE_PAUSE_J2  KEY_SPACE

static EtatJeu etat_courant;
static Input inputs_j1;
static Input inputs_j2;

static int key_old[KEY_MAX];
static int mouse_old = 0;

/* ===================== */
/* INIT / UPDATE */
/* ===================== */

void ihm_init() {
    install_keyboard();
    install_mouse();
    etat_courant = ETAT_MENU;
    ihm_reset_inputs();

    for (int i = 0; i < KEY_MAX; i++) {
        key_old[i] = 0;
    }
}

void ihm_update() {
    poll_keyboard();
    poll_mouse();
}

void ihm_fermer() {
    remove_keyboard();
    remove_mouse();
}

/* ===================== */
/* ETAT */
/* ===================== */

void ihm_set_etat(EtatJeu etat) {
    etat_courant = etat;
}

EtatJeu ihm_get_etat() {
    return etat_courant;
}

/* ===================== */
/* MENUS */
/* ===================== */

int ihm_menu_principal(int *selection, int nb_options) {
    if (ihm_touche_appuyee(KEY_UP)) {
        (*selection)--;
        if (*selection < 0) *selection = nb_options - 1;
    }

    if (ihm_touche_appuyee(KEY_DOWN)) {
        (*selection)++;
        if (*selection >= nb_options) *selection = 0;
    }

    if (ihm_touche_appuyee(KEY_ENTER)) {
        return *selection + 1;
    }

    return 0;
}

int ihm_menu_pause(int *selection, int nb_options) {
    if (ihm_touche_appuyee(KEY_UP)) {
        (*selection)--;
        if (*selection < 0) *selection = nb_options - 1;
    }

    if (ihm_touche_appuyee(KEY_DOWN)) {
        (*selection)++;
        if (*selection >= nb_options) *selection = 0;
    }

    if (ihm_touche_appuyee(KEY_ENTER)) {
        return *selection + 1;
    }

    return 0;
}

int ihm_menu_fin_niveau(int *selection, int nb_options) {
    if (ihm_touche_appuyee(KEY_UP)) {
        (*selection)--;
        if (*selection < 0) *selection = nb_options - 1;
    }

    if (ihm_touche_appuyee(KEY_DOWN)) {
        (*selection)++;
        if (*selection >= nb_options) *selection = 0;
    }

    if (ihm_touche_appuyee(KEY_ENTER)) {
        return *selection + 1;
    }

    return 0;
    return 0;
}

void ihm_afficher_regles() {
    ihm_attendre_touche();
}

/* ===================== */
/* SAISIE PSEUDO (PROPRE + PROGRESSIVE) */
/* ===================== */

/* INIT */
void ihm_init_saisie_pseudo(SaisiePseudo *s) {
    memset(s->buffer, 0, 50);
    memset(s->affichage, 0, 50);
    s->index = 0;
    s->fini = 0;
}

/* UPDATE */
void ihm_update_saisie_pseudo(SaisiePseudo *s) {
    int c;

    ihm_update();

    if (keypressed()) {
        c = readkey() & 0xFF;

        if (c == '\r' || c == '\n') {
            s->fini = 1;
        }
        else if (c == 8) { // backspace
            int len = strlen(s->buffer);
            if (len > 0) {
                s->buffer[len - 1] = '\0';
            }
        }
        else if (strlen(s->buffer) < 49 && c >= 32 && c <= 126) {
            int len = strlen(s->buffer);
            s->buffer[len] = (char)c;
            s->buffer[len + 1] = '\0';
        }
    }

    /* effet machine à écrire */
    if (s->index < (int)strlen(s->buffer)) {
        s->index++;
    }

    strncpy(s->affichage, s->buffer, s->index);
    s->affichage[s->index] = '\0';
}

/* AFFICHAGE */
void ihm_afficher_saisie_pseudo(SaisiePseudo *s, int x, int y) {
    textout_ex(screen, font, "Entrez votre pseudo :", x, y, makecol(255,255,255), -1);
    textout_ex(screen, font, s->affichage, x, y + 40, makecol(255,255,255), -1);
}

/* ===================== */
/* INPUT JEU */
/* ===================== */

void ihm_lire_inputs(Input *input) {
    input->gauche = ihm_touche_maintenue(TOUCHE_GAUCHE_J1);
    input->droite = ihm_touche_maintenue(TOUCHE_DROITE_J1);
    input->pause  = ihm_touche_appuyee(TOUCHE_PAUSE_J1);
}

void ihm_lire_inputs_joueur2(Input *input) {
    input->gauche = ihm_touche_maintenue(TOUCHE_GAUCHE_J2);
    input->droite = ihm_touche_maintenue(TOUCHE_DROITE_J2);
    input->pause  = ihm_touche_appuyee(TOUCHE_PAUSE_J2);
}

/* ===================== */
/* CLAVIER */
/* ===================== */

int ihm_touche_appuyee(int keycode) {
    return key[keycode] && !key_old[keycode];
}

int ihm_touche_maintenue(int keycode) {
    return key[keycode];
}

/* ===================== */
/* SOURIS */
/* ===================== */

void ihm_get_souris(int *x, int *y, int *clic_gauche) {
    *x = mouse_x;
    *y = mouse_y;
    *clic_gauche = mouse_b & 1;
}

int ihm_souris_clique_zone(int rx, int ry, int rw, int rh) {
    int x, y, clic;
    ihm_get_souris(&x, &y, &clic);

    int clic_unique = clic && !mouse_old;
    mouse_old = clic;

    return clic_unique &&
           x >= rx && x <= rx + rw &&
           y >= ry && y <= ry + rh;
}

/* ===================== */
/* EVENEMENTS */
/* ===================== */

int ihm_quitter_demande() {
    return ihm_touche_appuyee(KEY_Q);
}

int ihm_pause_demandee() {
    return ihm_touche_appuyee(KEY_ESC);
}

/* ===================== */
/* UTILITAIRES */
/* ===================== */

void ihm_attendre_touche() {
    clear_keybuf();
    readkey();
}

void ihm_reset_inputs() {
    inputs_j1.gauche = 0;
    inputs_j1.droite = 0;
    inputs_j1.pause  = 0;

    inputs_j2.gauche = 0;
    inputs_j2.droite = 0;
    inputs_j2.pause  = 0;
}

/* ===================== */
/* MAJ ETATS PRECEDENTS */
/* ===================== */

void ihm_maj_etats_precedents() {
    for (int i = 0; i < KEY_MAX; i++) {
        key_old[i] = key[i];
    }
}

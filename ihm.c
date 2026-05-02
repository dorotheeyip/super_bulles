#include <stdio.h>
#include "ihm.h"
#include <allegro.h>
#include <string.h>

#define TOUCHE_GAUCHE_J1 KEY_A
#define TOUCHE_DROITE_J1 KEY_D
#define TOUCHE_PAUSE_J1  KEY_Z

#define TOUCHE_GAUCHE_J2 KEY_J
#define TOUCHE_DROITE_J2 KEY_L
#define TOUCHE_PAUSE_J2  KEY_I

#define LARGEUR 800
#define HAUTEUR 600

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

int ihm_menu_principal() {
    if (ihm_touche_appuyee(KEY_1)) return 1;
    if (ihm_touche_appuyee(KEY_2)) return 2;
    if (ihm_touche_appuyee(KEY_3)) return 3;
    if (ihm_touche_appuyee(KEY_4)) return 4;
    return 0;
}

int ihm_menu_pause() {
    if (ihm_touche_appuyee(KEY_1)) return 1;
    if (ihm_touche_appuyee(KEY_2)) return 2;
    return 0;
}

int ihm_menu_fin_niveau(int victoire) {
    if (ihm_touche_appuyee(KEY_1)) return 1;
    if (ihm_touche_appuyee(KEY_2)) return 2;
    if (ihm_touche_appuyee(KEY_3)) return 3;
    if (ihm_touche_appuyee(KEY_4)) return 4;
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


int main_ihm() {
    allegro_init();
    install_keyboard();
    install_mouse();
    install_timer();

    set_color_depth(32);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, LARGEUR, HAUTEUR, 0, 0);

    ihm_init();

    int running = 1;

    while (running) {

        ihm_update();

        switch (ihm_get_etat()) {

        /* ===================== */
        /* MENU */
        /* ===================== */
        case ETAT_MENU:

            clear(screen);

            textout_ex(screen, font, "MENU", 350, 100, makecol(255,255,255), -1);
            textout_ex(screen, font, "1: JEU", 350, 200, makecol(255,255,255), -1);
            textout_ex(screen, font, "4: QUIT", 350, 260, makecol(255,255,255), -1);

            if (ihm_touche_appuyee(KEY_1)) {
                ihm_set_etat(ETAT_SAISIE);
            }

            if (ihm_touche_appuyee(KEY_4)) {
                running = 0;
            }

            break;

        /* ===================== */
        /* SAISIE PSEUDO */
        /* ===================== */
        case ETAT_SAISIE: {

            static SaisiePseudo saisie;
            static int init = 0;

            if (!init) {
                ihm_init_saisie_pseudo(&saisie);
                init = 1;
            }

            clear(screen);

            ihm_update_saisie_pseudo(&saisie);
            ihm_afficher_saisie_pseudo(&saisie, 250, 200);

            if (saisie.fini) {
                init = 0;
                ihm_set_etat(ETAT_JEU);
            }

        } break;

        /* ===================== */
        /* JEU */
        /* ===================== */
        case ETAT_JEU: {

            clear(screen);

            textout_ex(screen, font, "JEU ACTIF", 350, 50, makecol(255,255,255), -1);

            Input j1, j2;

            ihm_lire_inputs(&j1);
            ihm_lire_inputs_joueur2(&j2);

            /* J1 */
            if (j1.gauche) textout_ex(screen, font, "J1 GAUCHE", 100, 150, makecol(255,0,0), -1);
            if (j1.droite) textout_ex(screen, font, "J1 DROITE", 100, 180, makecol(0,255,0), -1);

            /* J2 */
            if (j2.gauche) textout_ex(screen, font, "J2 GAUCHE", 500, 150, makecol(255,0,0), -1);
            if (j2.droite) textout_ex(screen, font, "J2 DROITE", 500, 180, makecol(0,255,0), -1);

            /* PAUSE */
            if (ihm_touche_appuyee(TOUCHE_PAUSE_J1)) {
                ihm_set_etat(ETAT_PAUSE);
            }

            if (ihm_touche_appuyee(TOUCHE_PAUSE_J2)) {
                ihm_set_etat(ETAT_PAUSE);
            }

        } break;

        /* ===================== */
        /* PAUSE */
        /* ===================== */
        case ETAT_PAUSE:

            clear(screen);

            textout_ex(screen, font, "PAUSE", 380, 200, makecol(255,255,0), -1);
            textout_ex(screen, font, "1: Reprendre", 350, 250, makecol(255,255,255), -1);
            textout_ex(screen, font, "2: Menu", 350, 280, makecol(255,255,255), -1);

            if (ihm_touche_appuyee(KEY_1)) {
                ihm_set_etat(ETAT_JEU);
            }

            if (ihm_touche_appuyee(KEY_2)) {
                ihm_set_etat(ETAT_MENU);
            }

            break;

        default:
            break;
        }

        ihm_maj_etats_precedents();
        rest(16);
    }

    ihm_fermer();
    return 0;
}
END_OF_MAIN();
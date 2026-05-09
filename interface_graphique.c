#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <allegro.h>
#include "interface_graphique.h"
#include "struct.h"
#include "joueur.h"
#include "niveau.h"
#include "jeu.h"

typedef enum {
    ETAT_MENU,
    ETAT_JEU,
    ETAT_FIN,
    ETAT_REGLES,
    ETAT_PSEUDO,
    ETAT_DUO
} EtatJeu;

int fin_selection = 0;
float fin_scale = 0.1f;
int fin_anim_done = 0;

char pseudo[20] = "";
int pseudo_index = 0;

#define NB_BULLES 5

#define NB_ECLAIRS 5

BITMAP *img_eclair[4];

int eclair_x[NB_ECLAIRS];
int eclair_y[NB_ECLAIRS];
int eclair_active[NB_ECLAIRS];
int eclair_anim[NB_ECLAIRS];   /* frame d'animation par éclair */
int eclair_cpt[NB_ECLAIRS];    /* compteur animation par éclair */
int cpt_spawn_eclair = 0;

#define BOSS_VIE_MAX 6  /* nombre d'états de vie */

BITMAP *img_boss_vie[BOSS_VIE_MAX];
int boss_vie = BOSS_VIE_MAX;  /* commence à vie pleine */

int bulle_x[NB_BULLES];
int bulle_y[NB_BULLES];
int bulle_dx[NB_BULLES]; /* vitesse horizontale */
int bulle_dy[NB_BULLES]; /* vitesse verticale */
int bulle_active[NB_BULLES];
int cpt_spawn_bulle = 0;  /* compteur pour espacer les spawns */

#define NB_EXPLOSIONS 5

BITMAP *img_explosion[7];

int explo_x[NB_EXPLOSIONS];
int explo_y[NB_EXPLOSIONS];
int explo_active[NB_EXPLOSIONS];
int explo_anim[NB_EXPLOSIONS];
int explo_cpt[NB_EXPLOSIONS];

#define NB_PROJ 10 /* nombre de projectiles simultanés */

int proj_x[NB_PROJ];
int proj_y[NB_PROJ];
int proj_active[NB_PROJ];

#define NB_PROJ_BOSS 5

int proj_boss_x[NB_PROJ_BOSS];
int proj_boss_y[NB_PROJ_BOSS];
int proj_boss_active[NB_PROJ_BOSS];
int cpt_tir_boss = 0;  /* cadence de tir du boss */


int boss_x = 400;
int boss_y = 50;
int boss_dir = 1;
int boss_moving = 1;

/* ============================= */
/* ===== VARIABLES GLOBALES ==== */
/* ============================= */
#define NB_OPTIONS 5

int dir = 1;
int moving = 0;
int moving_boss = 0;

BITMAP *buffer = NULL;

/* Sprites */
BITMAP *img_bubble = NULL;
BITMAP *img_banner = NULL;
BITMAP *img_victoire = NULL;
BITMAP *img_defaite = NULL;

/* Animations */
BITMAP *img_player[2];
BITMAP *img_boss[2];
BITMAP *img_projectile = NULL;
BITMAP *img_projectile_boss = NULL;

BITMAP *img_background_menu = NULL;
BITMAP *img_background_lvl1 = NULL;
BITMAP *img_background_lvl3 = NULL;
BITMAP *img_background_lvl4 = NULL;

/* Compteurs animation */
int anim_player = 0;
int anim_projectile = 0;
int anim_projectile_boss = 0;
int anim_boss = 0;
int cpt_player = 0;
int cpt_projectile = 0;
int cpt_projectile_boss = 0;
int cpt_boss = 0;
/* ============================= */
/* ===== INITIALISATION ========= */
/* ============================= */

int init_graphics() {
    if (allegro_init() != 0) return 0;

    install_keyboard();
    set_color_depth(32);

    if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, 800, 600, 0, 0) != 0) {
        return 0;
    }

    if (!screen) return 0;

    buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) return 0;

    /* Chargement des sprites */

    /* Player animation */
    img_player[0] = load_bitmap("assets/bowser_1.bmp", NULL);
    img_player[1] = load_bitmap("assets/bowser_2.bmp", NULL);

    img_boss[0] = load_bitmap("assets/boss1.bmp", NULL);
    img_boss[1] = load_bitmap("assets/boss2.bmp", NULL);

    img_projectile_boss = load_bitmap("assets/boule_feu.bmp", NULL);

    img_boss_vie[0] = load_bitmap("assets/vie1.bmp", NULL); /* vie pleine */
    img_boss_vie[1] = load_bitmap("assets/vie2.bmp", NULL);
    img_boss_vie[2] = load_bitmap("assets/vie3.bmp", NULL);
    img_boss_vie[3] = load_bitmap("assets/vie4.bmp", NULL);
    img_boss_vie[4] = load_bitmap("assets/vie5.bmp", NULL);
    img_boss_vie[5] = load_bitmap("assets/vie6.bmp", NULL); /* presque mort */

    img_bubble = load_bitmap("assets/bulle.bmp", NULL);

    img_explosion[0] = load_bitmap("assets/boom1.bmp", NULL);
    img_explosion[1] = load_bitmap("assets/boom2.bmp", NULL);
    img_explosion[2] = load_bitmap("assets/boom3.bmp", NULL);
    img_explosion[3] = load_bitmap("assets/boom4.bmp", NULL);
    img_explosion[4] = load_bitmap("assets/boom5.bmp", NULL);
    img_explosion[5] = load_bitmap("assets/boom6.bmp", NULL);
    img_explosion[6] = load_bitmap("assets/boom7.bmp", NULL);

    img_eclair[0] = load_bitmap("assets/eclair1.bmp", NULL);
    img_eclair[1] = load_bitmap("assets/eclair2.bmp", NULL);
    img_eclair[2] = load_bitmap("assets/eclair3.bmp", NULL);
    img_eclair[3] = load_bitmap("assets/eclair4.bmp", NULL);

    img_banner = load_bitmap("assets/baniere_menu.bmp", NULL);

    img_victoire = load_bitmap("assets/victoire.bmp", NULL);
    img_defaite = load_bitmap("assets/defaite.bmp", NULL);

    /* Projectile animation */
    img_projectile = load_bitmap("assets/projectile1.bmp", NULL);

    /* background */
    img_background_menu = load_bitmap("assets/menu.bmp", NULL);
    img_background_lvl1 = load_bitmap("assets/niveau1et2.bmp", NULL);
    img_background_lvl3 = load_bitmap("assets/niveau3.bmp", NULL);
    img_background_lvl4 = load_bitmap("assets/niveau4.bmp", NULL);

    if (!img_player[0] || !img_player[1] ||
    !img_boss[0] || !img_boss[1] ||
    !img_boss_vie[0] || !img_boss_vie[1] ||
    !img_boss_vie[2] || !img_boss_vie[3] ||
    !img_boss_vie[4] || !img_boss_vie[5] ||
    !img_eclair[0] || !img_eclair[1] ||
    !img_eclair[2] || !img_eclair[3] ||
    !img_explosion[0] || !img_explosion[1] || !img_explosion[2] ||
    !img_explosion[3] || !img_explosion[4] || !img_explosion[5] ||
    !img_explosion[6] ||
    !img_projectile||
    !img_defaite||
    !img_victoire||
    !img_projectile_boss ||
    !img_background_menu ||
    !img_background_lvl1 ||
    !img_background_lvl3 ||
    !img_background_lvl4) {

        return 0;
    }

    return 1;
}


/* ============================= */
/* ===== NETTOYAGE ============== */
/* ============================= */

void destroy_graphics() {
    if (buffer) destroy_bitmap(buffer);

    for (int i = 0; i < 2; i++) {
        if (img_player[i]) destroy_bitmap(img_player[i]);
        if (img_boss[i]) destroy_bitmap(img_boss[i]);
    }
    for (int i = 0; i < BOSS_VIE_MAX; i++)
        if (img_boss_vie[i]) destroy_bitmap(img_boss_vie[i]);

    for (int i = 0; i < 4; i++)
        if (img_eclair[i]) destroy_bitmap(img_eclair[i]);

    for (int i = 0; i < 7; i++)
        if (img_explosion[i]) destroy_bitmap(img_explosion[i]);

    if (img_bubble) destroy_bitmap(img_bubble);
    if (img_background_menu) destroy_bitmap(img_background_menu);
    if (img_background_lvl1) destroy_bitmap(img_background_lvl1);
    if (img_background_lvl3) destroy_bitmap(img_background_lvl3);
    if (img_background_lvl4) destroy_bitmap(img_background_lvl4);
    if (img_banner) destroy_bitmap(img_banner);
    if (img_victoire) destroy_bitmap(img_victoire);
    if (img_defaite) destroy_bitmap(img_defaite);
    if (img_projectile) destroy_bitmap(img_projectile);
}


/* ============================= */
/* ===== GESTION ECRAN ========= */
/* ============================= */

void clear_screen() {
    clear_bitmap(buffer);
}

void update_display() {
    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}


/* ============================= */
/* ===== BACKGROUND ============ */
/* ============================= */



void draw_background_level(int level) {

    BITMAP *bg = NULL;

    if (level == 0) bg = img_background_menu;
    else if (level == 1 || level == 2) bg = img_background_lvl1;
    else if (level == 3) bg = img_background_lvl3;
    else if (level == 4) bg = img_background_lvl4;

    if (bg) {
        stretch_blit(bg, buffer,
                     0, 0, bg->w, bg->h,
                     0, 0, SCREEN_W, SCREEN_H);
    }
}


/* ============================= */
/* ===== MENU ================== */
/* ============================= */

void draw_menu(int selection) {

    clear_bitmap(buffer);

    if (img_background_menu) {
        stretch_blit(img_background_menu, buffer,
                     0,0,img_background_menu->w,img_background_menu->h,
                     0,0,SCREEN_W,SCREEN_H);
    }

    if (img_banner) {
        int x = (SCREEN_W - img_banner->w) / 2;
        int y = 20;
        draw_sprite(buffer, img_banner, x, y);
    }

    textout_centre_ex(buffer, font, "SUPER BULLES",
                      SCREEN_W/2, 140, makecol(255,255,255), -1);

    /* === AJOUT DES OPTIONS === */

    const char *options[NB_OPTIONS] = {
        "Nouvelle partie",
        "Charger partie",
        "Mode Duo",
        "Regles",
        "Quitter"
    };

    for (int i = 0; i < NB_OPTIONS; i++) {

        int color;

        if (i == selection)
            color = makecol(255, 0, 0); // option sélectionnée en rouge
        else
            color = makecol(255, 255, 255); // blanc

        textout_centre_ex(buffer, font, options[i],
                          SCREEN_W/2,
                          200 + i * 40,
                          color, -1);
    }

    update_display();
}


/* ============================= */
/* ===== ENTITES =============== */
/* ============================= */

void draw_player(int x, int y, int moving, int dir) {

    if (moving) {
        cpt_player++;
        if (cpt_player > 10) {
            cpt_player = 0;
            anim_player = (anim_player + 1) % 2;
        }
    } else {
        anim_player = 0;
    }

    BITMAP *frame = img_player[anim_player];

    int w = frame->w * 3;
    int h = frame->h * 3;

    BITMAP *tmp = create_bitmap(w, h);
    if (!tmp) return;

    /* Remplir en magenta au lieu de noir */
    clear_to_color(tmp, makecol(255, 0, 255));

    stretch_sprite(tmp, frame, 0, 0, w, h);

    if (dir == 1) {
        draw_sprite(buffer, tmp, x, y);
    } else {
        draw_sprite_h_flip(buffer, tmp, x, y);
    }

    destroy_bitmap(tmp);
}

void draw_boss(int x, int y, int moving_boss, int dir) {

    if (moving_boss) {
        cpt_boss++;
        if (cpt_boss > 10) {
            cpt_boss = 0;
            anim_boss = (anim_boss + 1) % 2;
        }
    } else {
        anim_boss = 0;
    }

    BITMAP *frame = img_boss[anim_boss];

    int w = frame->w * 3;
    int h = frame->h * 3;

    BITMAP *tmp = create_bitmap(w, h);
    if (!tmp) return;

    /* Remplir en magenta au lieu de noir */
    clear_to_color(tmp, makecol(255, 0, 255));

    stretch_sprite(tmp, frame, 0, 0, w, h);

    if (dir == 1) {
        draw_sprite(buffer, tmp, x, y);
    } else {
        draw_sprite_h_flip(buffer, tmp, x, y);
    }

    destroy_bitmap(tmp);
}

void draw_boss_vie(int vie) {
    if (vie <= 0 || vie > BOSS_VIE_MAX) return;

    /* index = vie pleine → image 0, vie presque morte → image 3 */
    int index = BOSS_VIE_MAX - vie;
    BITMAP *img = img_boss_vie[index];

    if (img)
        draw_sprite(buffer, img, (SCREEN_W - img->w) / 2, 10);
}

void draw_bubble(int x, int y, int active) {
    if (active && img_bubble) {
        draw_sprite(buffer, img_bubble, x, y);
    }
}

void draw_explosion(int x, int y, int active, int anim) {
    if (!active) return;

    BITMAP *frame = img_explosion[anim];
    if (!frame) return;

    int w = frame->w * 2;
    int h = frame->h * 2;

    BITMAP *tmp = create_bitmap(w, h);
    if (!tmp) return;
    clear_to_color(tmp, makecol(255, 0, 255));
    stretch_sprite(tmp, frame, 0, 0, w, h);
    draw_sprite(buffer, tmp, x, y);
    destroy_bitmap(tmp);
}

void draw_eclair(int x, int y, int active, int anim) {
    if (!active) return;

    BITMAP *frame = img_eclair[anim];
    if (!frame) return;

    int w = frame->w * 2;
    int h = frame->h * 2;

    BITMAP *tmp = create_bitmap(w, h);
    if (!tmp) return;
    clear_to_color(tmp, makecol(255, 0, 255));
    stretch_sprite(tmp, frame, 0, 0, w, h);
    draw_sprite(buffer, tmp, x, y);
    destroy_bitmap(tmp);
}

void draw_projectile(int x, int y, int active) {

    if (!active) return;

    cpt_projectile++;
    if (cpt_projectile > 5) {
        cpt_projectile = 0;
        anim_projectile = (anim_projectile + 1) % 2;
    }

    int w = img_projectile->w * 2;  /* Change le multiplicateur pour ajuster la taille */
    int h = img_projectile->h * 2;

    BITMAP *tmp = create_bitmap(w, h);
    if (!tmp) return;
    clear_to_color(tmp, makecol(255, 0, 255));

    stretch_sprite(tmp, img_projectile, 0, 0, w, h);
    draw_sprite(buffer, tmp, x, y);

    destroy_bitmap(tmp);
}

void draw_projectile_boss(int x, int y, int active) {

    if (!active) return;

    cpt_projectile_boss++;
    if (cpt_projectile_boss > 5) {
        cpt_projectile_boss = 0;
        anim_projectile_boss = (anim_projectile_boss + 1) % 2;
    }

    int w = img_projectile_boss->w * 2;  /* Change le multiplicateur pour ajuster la taille */
    int h = img_projectile_boss->h * 2;

    BITMAP *tmp = create_bitmap(w, h);
    if (!tmp) return;
    clear_to_color(tmp, makecol(255, 0, 255));

    stretch_sprite(tmp, img_projectile_boss, 0, 0, w, h);
    draw_sprite(buffer, tmp, x, y);

    destroy_bitmap(tmp);
}

void spawn_explosion(int x, int y) {
    for (int i = 0; i < NB_EXPLOSIONS; i++) {
        if (!explo_active[i]) {
            explo_x[i] = x;
            explo_y[i] = y;
            explo_active[i] = 1;
            explo_anim[i] = 0;
            explo_cpt[i] = 0;
            break;
        }
    }
}

void spawn_eclair(int x, int y) {
    for (int i = 0; i < NB_ECLAIRS; i++) {
        if (!eclair_active[i]) {
            eclair_x[i] = x;
            eclair_y[i] = y;
            eclair_active[i] = 1;
            eclair_anim[i] = 0;
            eclair_cpt[i] = 0;
            break;
        }
    }
}



/* ============================= */
/* ===== INTERFACE ============= */
/* ============================= */

void draw_ui(int score, int time_left, const char *pseudo) {
    char text[100];

    sprintf(text, "Score : %d", score);
    textout_ex(buffer, font, text, 10, 10, makecol(255,255,255), -1);

    sprintf(text, "Temps : %d", time_left);
    textout_ex(buffer, font, text, 10, 30, makecol(255,255,255), -1);

    sprintf(text, "Joueur : %s", pseudo);
    textout_ex(buffer, font, text, 10, 50, makecol(255,255,255), -1);
}


/* ============================= */
/* ===== FIN =================== */
/* ============================= */

void draw_menu_fin(int victoire, int niveau_actuel, int score, BITMAP *img_annonce) {

    draw_background_level(niveau_actuel);

    set_trans_blender(0, 0, 0, 128);
    drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    rectfill(buffer, 0, 0, SCREEN_W, SCREEN_H, makecol(0, 0, 0));
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);

    if (img_annonce) {

        int final_w = (int)(img_annonce->w * fin_scale);
        int final_h = (int)(img_annonce->h * fin_scale);

        int bx = (SCREEN_W - final_w) / 2;
        int by = 100 - (final_h / 2);

        stretch_sprite(buffer, img_annonce, bx, by, final_w, final_h);
    }

    if (fin_anim_done) {

        char msg_score[50];
        sprintf(msg_score, "SCORE FINAL : %d", score);
        textout_centre_ex(buffer, font, msg_score,
                          SCREEN_W/2, 220,
                          makecol(255,255,255), -1);

        const char *choix[2];

        if (victoire) {
            choix[0] = "NIVEAU SUIVANT";
            choix[1] = "REVENIR AU MENU";
        } else {
            choix[0] = "RECOMMENCER";
            choix[1] = "REVENIR AU MENU";
        }

        for (int i = 0; i < 2; i++) {
            int color = (i == fin_selection)
                ? makecol(255,255,0)
                : makecol(200,200,200);

            textout_centre_ex(buffer, font,
                              choix[i],
                              SCREEN_W/2,
                              300 + i * 40,
                              color, -1);
        }
    }

    update_display();
}

void reset_game(Joueur *joueur, Niveau *niveau, int *niveau_actuel) {

    joueur->x = 100;
    joueur->y = 425;
    joueur->score = 0;
    dir = 1;
    *niveau_actuel = 1;

    niveau->boss.x = 400;
    niveau->boss.y = 50;
    niveau->boss.vitesse = 2;
    niveau->boss.pv = BOSS_VIE_MAX;

    /* Reset projectiles joueur */
    if (niveau->projectiles) {
        for (int i = 0; i < 20; i++) {
            niveau->projectiles[i].actif = 0;
        }
        niveau->nb_projectiles = 0;
    }

    /* Reset projectiles boss */
    for (int i = 0; i < NB_PROJ_BOSS; i++) {
        proj_boss_active[i] = 0;
        proj_boss_x[i] = 0;
        proj_boss_y[i] = 0;
    }

    cpt_tir_boss = 0;

    /* Reset bulles */
    if (niveau->bulles.tab) {
        niveau->bulles.nb = 0;
    }
    cpt_spawn_bulle = 0;

    /* Reset éclairs */
    for (int i = 0; i < NB_ECLAIRS; i++) {
        eclair_active[i] = 0;
        eclair_x[i] = 0;
        eclair_y[i] = 0;
        eclair_anim[i] = 0;
        eclair_cpt[i] = 0;
    }

    cpt_spawn_eclair = 0;

    /* Reset explosions */
    for (int i = 0; i < NB_EXPLOSIONS; i++) {
        explo_active[i] = 0;
        explo_anim[i] = 0;
        explo_cpt[i] = 0;
        explo_x[i] = 0;
        explo_y[i] = 0;
    }

    /* Reset animations */
    anim_player = 0;
    anim_boss = 0;
    anim_projectile = 0;
    anim_projectile_boss = 0;

    cpt_player = 0;
    cpt_boss = 0;
    cpt_projectile = 0;
    cpt_projectile_boss = 0;
}


/* ============================= */
/* ===== CONSTANTES MENU ======= */
/* ============================= */

void update_menu_fin() {
    if (fin_scale < 1.0f) {
        fin_scale += 0.05f;
        if (fin_scale >= 1.0f) {
            fin_scale = 1.0f;
            fin_anim_done = 1;
        }
    }
}


int main() {

    if (!init_graphics()) {
        printf("Erreur initialisation graphique\n");
        return -1;
    }

    srand(time(NULL));

    clock_t last_time = clock();

    EtatJeu etat = ETAT_MENU;
    int selection = 0;

    Joueur joueur;
    Niveau niveau_struct;
    int niveau_actuel = 1;
    int mode_charger = 0;
    int victoire = 0;
    int key_pressed = 0;

    memset(&joueur, 0, sizeof(Joueur));
    memset(&niveau_struct, 0, sizeof(Niveau));

    reset_game(&joueur, &niveau_struct, &niveau_actuel);

    while (!key[KEY_ESC]) {

        switch (etat) {

        /* ================= MENU ================= */
        case ETAT_MENU:

            if (key[KEY_UP]) {
                selection--;
                if (selection < 0) selection = NB_OPTIONS - 1;
                rest(150);
            }

            if (key[KEY_DOWN]) {
                selection++;
                if (selection >= NB_OPTIONS) selection = 0;
                rest(150);
            }

            if (key[KEY_ENTER]) {

                if (selection == 0) { // Nouvelle partie
                    initialiser_joueur(&joueur, "");
                    initialiser_niveau(&niveau_struct, niveau_actuel);
                    etat = ETAT_JEU;
                }

                else if (selection == 1) {
                    mode_charger = 1;
                    etat = ETAT_PSEUDO;
                    pseudo_index = 0;
                    pseudo[0] = '\0';

                    clear_keybuf();
                }

                else if (selection == 2) { // MODE DUO
                    etat = ETAT_DUO;
                }

                else if (selection == 3) { // REGLES
                    etat = ETAT_REGLES;
                }

                else if (selection == 4) { // Quitter
                    return 0;
                }

                rest(200);
            }

            draw_menu(selection);
            break;

            case ETAT_PSEUDO: {

                clear_bitmap(buffer);

                // fond menu
                if (img_background_menu) {
                    stretch_blit(img_background_menu, buffer,
                                 0,0,img_background_menu->w,img_background_menu->h,
                                 0,0,SCREEN_W,SCREEN_H);
                }

                // cadre (rectangle)
                rectfill(buffer,
                         SCREEN_W/2 - 150,
                         SCREEN_H/2 - 40,
                         SCREEN_W/2 + 150,
                         SCREEN_H/2 + 40,
                         makecol(200,200,200));

                rect(buffer,
                     SCREEN_W/2 - 150,
                     SCREEN_H/2 - 40,
                     SCREEN_W/2 + 150,
                     SCREEN_H/2 + 40,
                     makecol(0,0,0));

                textout_centre_ex(buffer, font,
                    "ENTRE TON PSEUDO",
                    SCREEN_W/2, SCREEN_H/2 - 80,
                    makecol(0,0,0), -1);

                textout_centre_ex(buffer, font,
                    pseudo,
                    SCREEN_W/2, SCREEN_H/2 - 10,
                    makecol(0,0,0), -1);

                textout_centre_ex(buffer, font,
                    "ENTER pour valider",
                    SCREEN_W/2, SCREEN_H/2 + 80,
                    makecol(0,0,0), -1);

                // saisie clavier simple
                if (keypressed()) {

                    int c = readkey() & 0xff;

                    if (c == 13) { // ENTER
                        if(mode_charger) {
                            int niv = charger_partie(pseudo, &joueur);
                            initialiser_joueur(&joueur, pseudo);
                            if(niv != -1) {
                                niveau_actuel = niv;
                            } else {
                                niveau_actuel = 1;
                            }
                        } else {
                            initialiser_joueur(&joueur, pseudo);
                        }
                        initialiser_niveau(&niveau_struct, niveau_actuel);
                        etat = ETAT_JEU;
                        mode_charger = 0;
                    }
                    else if (c == 8 && pseudo_index > 0) { // BACKSPACE
                        pseudo_index--;
                        pseudo[pseudo_index] = '\0';
                    }
                    else if (pseudo_index < 19 && c >= 32 && c <= 126) {
                        pseudo[pseudo_index++] = c;
                        pseudo[pseudo_index] = '\0';
                    }

                    rest(50);
                }

                update_display();

            break;
            }

            case ETAT_REGLES: {

                clear_bitmap(buffer);

                // fond identique au menu
                if (img_background_menu) {
                    stretch_blit(img_background_menu, buffer,
                                 0,0,img_background_menu->w,img_background_menu->h,
                                 0,0,SCREEN_W,SCREEN_H);
                }

                textout_centre_ex(buffer, font,
                    "REGLES DU JEU",
                    SCREEN_W/2, 80,
                    makecol(0,0,0), -1);

                textout_centre_ex(buffer, font,
                    "- Deplacer le joueur avec LEFT / RIGHT",
                    SCREEN_W/2, 150,
                    makecol(0,0,0), -1);

                textout_centre_ex(buffer, font,
                    "- Tir automatique des projectiles",
                    SCREEN_W/2, 180,
                    makecol(0,0,0), -1);

                textout_centre_ex(buffer, font,
                    "- Detruire les bulles pour gagner des points",
                    SCREEN_W/2, 210,
                    makecol(0,0,0), -1);

                textout_centre_ex(buffer, font,
                    "- Eviter les attaques du boss",
                    SCREEN_W/2, 240,
                    makecol(0,0,0), -1);

                textout_centre_ex(buffer, font,
                    "Appuie sur ENTER pour revenir",
                    SCREEN_W/2, 320,
                    makecol(255,255,0), -1);

                update_display();

                if (key[KEY_ENTER]) {
                    etat = ETAT_MENU;
                    rest(200);
                }

            break;
            }


        /* ================= JEU ================= */
        case ETAT_JEU: {

            clear_screen();

            draw_background_level(niveau_actuel);

            clock_t current_time = clock();

            float dt = (float)(current_time - last_time) / CLOCKS_PER_SEC;

            last_time = current_time;

            if(dt > 0.1f) dt = 0.1f;

            /* input */

            if (key[KEY_LEFT]) {

                deplacer_joueur(&joueur, 0);

                dir = -1;

                moving = 1;

            } else if (key[KEY_RIGHT]) {

                deplacer_joueur(&joueur, 1);

                dir = 1;

                moving = 1;

            } else {

                moving = 0;

            }

            if (joueur.x < 0) joueur.x = 0;

            if (joueur.x > SCREEN_W - 100) joueur.x = SCREEN_W - 100;

            if (key[KEY_SPACE] && !key_pressed) {

                key_pressed = 1;

                if(niveau_struct.nb_projectiles < 20) {

                    niveau_struct.projectiles[niveau_struct.nb_projectiles] = tirer(&joueur);

                    niveau_struct.nb_projectiles++;

                }

            }

            if (!key[KEY_SPACE]) key_pressed = 0;

            /* update boss projectiles */

            for (int i = 0; i < NB_PROJ_BOSS; i++) {

                if (proj_boss_active[i]) {

                    proj_boss_y[i] += 3;

                    if (proj_boss_y[i] > SCREEN_H) proj_boss_active[i] = 0;

                }

            }

            cpt_tir_boss++;

            if (cpt_tir_boss > 50) {

                for (int i = 0; i < NB_PROJ_BOSS; i++) {

                    if (!proj_boss_active[i]) {

                        proj_boss_x[i] = (int)niveau_struct.boss.x;

                        proj_boss_y[i] = (int)niveau_struct.boss.y;

                        proj_boss_active[i] = 1;

                        cpt_tir_boss = 0;

                        break;

                    }

                }

            }

            /* update */

            maj_niveau(&niveau_struct, &joueur, dt);

            /* check end */

            int res = niveau_termine(&niveau_struct, &joueur);

            if(res == 1) {

                victoire = 1;

                joueur.score += 100;

                niveau_actuel++;

                liberer_niveau(&niveau_struct);

                if(niveau_actuel <= 3) {

                    initialiser_niveau(&niveau_struct, niveau_actuel);

                } else {

                    etat = ETAT_MENU;

                }

            } else if(res == 0) {

                victoire = 0;

                etat = ETAT_FIN;

            }

            /* draw */

            draw_player((int)joueur.x, (int)joueur.y, moving, dir);

            if(niveau_struct.boss.pv > 0) {

                boss_dir = (niveau_struct.boss.vitesse > 0) ? 1 : -1;

                boss_moving = 1;

                draw_boss((int)niveau_struct.boss.x, (int)niveau_struct.boss.y, boss_moving, boss_dir);

                draw_boss_vie(niveau_struct.boss.pv);

            }

            for(int i = 0; i < niveau_struct.bulles.nb; i++) {

                draw_bubble((int)niveau_struct.bulles.tab[i].x, (int)niveau_struct.bulles.tab[i].y, niveau_struct.bulles.tab[i].actif);

            }

            for(int i = 0; i < niveau_struct.nb_projectiles; i++) {

                draw_projectile((int)niveau_struct.projectiles[i].x, (int)niveau_struct.projectiles[i].y, niveau_struct.projectiles[i].actif);

            }

            for(int i = 0; i < NB_PROJ_BOSS; i++) {

                draw_projectile_boss(proj_boss_x[i], proj_boss_y[i], proj_boss_active[i]);

            }

            for(int i = 0; i < NB_EXPLOSIONS; i++) {

                draw_explosion(explo_x[i], explo_y[i], explo_active[i], explo_anim[i]);

            }

            for(int i = 0; i < NB_ECLAIRS; i++) {

                draw_eclair(eclair_x[i], eclair_y[i], eclair_active[i], eclair_anim[i]);

            }

            draw_ui(joueur.score, (int)niveau_struct.temps_restant, joueur.pseudo);

            update_display();

            rest(16);

            break;

            case ETAT_FIN: {

                update_menu_fin();
                draw_menu_fin(victoire, niveau_actuel, joueur.score, victoire ? img_victoire : img_defaite);

                if (fin_anim_done) {

                    if (key[KEY_UP]) {
                        fin_selection = 0;
                        rest(150);
                    }

                    if (key[KEY_DOWN]) {
                        fin_selection = 1;
                        rest(150);
                    }

                    if (key[KEY_ENTER]) {

                        int choix = fin_selection;

                        rest(200);

                        if (choix == 0) {
                            reset_game(&joueur, &niveau_struct, &niveau_actuel);
                            initialiser_niveau(&niveau_struct, niveau_actuel);
                            etat = ETAT_JEU;
                        } else {
                            sauvegarder_partie(&joueur, niveau_actuel);
                            fin_selection = 0;
                            etat = ETAT_MENU;
                        }
                    }
                }

                rest(20);

                break;
            }
        }
    }
}

    destroy_graphics();
    return 0;
}

END_OF_MAIN();
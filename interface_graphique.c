#include <stdio.h>
#include <stdlib.h>
#include <allegro.h>
#include "interface_graphique.h"

typedef enum {
    ETAT_MENU,
    ETAT_JEU,
    ETAT_FIN
} EtatJeu;

#define NB_BULLES 5

int bulle_x[NB_BULLES];
int bulle_y[NB_BULLES];
int bulle_dx[NB_BULLES]; /* vitesse horizontale */
int bulle_dy[NB_BULLES]; /* vitesse verticale */
int bulle_active[NB_BULLES];
int cpt_spawn_bulle = 0;  /* compteur pour espacer les spawns */

#define NB_PROJ 10 /* nombre de projectiles simultanés */

int proj_x[NB_PROJ];
int proj_y[NB_PROJ];
int proj_active[NB_PROJ];

/* ============================= */
/* ===== VARIABLES GLOBALES ==== */
/* ============================= */
#define NB_OPTIONS 4

BITMAP *buffer = NULL;

/* Sprites */
BITMAP *img_bubble = NULL;
BITMAP *img_banner = NULL;

/* Animations */
BITMAP *img_player[2];
BITMAP *img_projectile = NULL;

BITMAP *img_background_menu = NULL;
BITMAP *img_background_lvl1 = NULL;
BITMAP *img_background_lvl3 = NULL;
BITMAP *img_background_lvl4 = NULL;

/* Compteurs animation */
int anim_player = 0;
int anim_projectile = 0;
int cpt_player = 0;
int cpt_projectile = 0;
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

    buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) return 0;

    /* Chargement des sprites */

    /* Player animation */
    img_player[0] = load_bitmap("assets/bowser_1.bmp", NULL);
    img_player[1] = load_bitmap("assets/bowser_2.bmp", NULL);

    img_bubble = load_bitmap("assets/bulle.bmp", NULL);
    img_banner = load_bitmap("assets/baniere_menu.bmp", NULL);

    /* Projectile animation */
    img_projectile = load_bitmap("assets/projectile1.bmp", NULL);

    /* background */
    img_background_menu = load_bitmap("assets/menu.bmp", NULL);
    img_background_lvl1 = load_bitmap("assets/niveau1et2.bmp", NULL);
    img_background_lvl3 = load_bitmap("assets/niveau3.bmp", NULL);
    img_background_lvl4 = load_bitmap("assets/niveau4.bmp", NULL);

    if (!img_player[0] || !img_player[1] ||
    // !img_projectile[0] || !img_projectile[1] ||
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
    }
    if (img_bubble) destroy_bitmap(img_bubble);
    if (img_background_menu) destroy_bitmap(img_background_menu);
    if (img_background_lvl1) destroy_bitmap(img_background_lvl1);
    if (img_background_lvl3) destroy_bitmap(img_background_lvl3);
    if (img_background_lvl4) destroy_bitmap(img_background_lvl4);
    if (img_banner) destroy_bitmap(img_banner);
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

int moving = 0;

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

void draw_bubble(int x, int y, int active) {
    if (!active || !img_bubble) return;

    int w = img_bubble->w / 4;  /* divise par 2 pour réduire, ajuste à ta guise */
    int h = img_bubble->h / 4;

    BITMAP *tmp = create_bitmap(w, h);
    clear_to_color(tmp, makecol(255, 0, 255));

    stretch_sprite(tmp, img_bubble, 0, 0, w, h);
    draw_sprite(buffer, tmp, x, y);

    destroy_bitmap(tmp);
}

void spawn_bulle(int index) {
    int cote = rand() % 3; /* 0=gauche, 1=droite, 2=haut */

    if (cote == 0) {         /* depuis la gauche */
        bulle_x[index] = -32;
        bulle_y[index] = rand() % (SCREEN_H - 100);
        bulle_dx[index] = 2 + rand() % 3;
        bulle_dy[index] = 1 + rand() % 2;
    }
    else if (cote == 1) {    /* depuis la droite */
        bulle_x[index] = SCREEN_W;
        bulle_y[index] = rand() % (SCREEN_H - 100);
        bulle_dx[index] = -(2 + rand() % 3);
        bulle_dy[index] = 1 + rand() % 2;
    }
    else {                   /* depuis le haut */
        bulle_x[index] = rand() % (SCREEN_W - 32);
        bulle_y[index] = -32;
        bulle_dx[index] = 0;
        bulle_dy[index] = 2 + rand() % 3;
    }

    bulle_active[index] = 1;
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
    clear_to_color(tmp, makecol(255, 0, 255));

    stretch_sprite(tmp, img_projectile, 0, 0, w, h);
    draw_sprite(buffer, tmp, x, y);

    destroy_bitmap(tmp);
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

void draw_end_screen(int score, int victoire) {
    clear_bitmap(buffer);

    if (victoire)
        textout_centre_ex(buffer, font, "VICTOIRE !",
                          SCREEN_W/2, 200, makecol(0,255,0), -1);
    else
        textout_centre_ex(buffer, font, "DEFAITE !",
                          SCREEN_W/2, 200, makecol(255,0,0), -1);

    char text[50];
    sprintf(text, "Score : %d", score);

    textout_centre_ex(buffer, font, text,
                      SCREEN_W/2, 250,
                      makecol(255,255,255), -1);

    update_display();
}

/* ============================= */
/* ===== CONSTANTES MENU ======= */
/* ============================= */

#define NB_OPTIONS 4

int main() {

    if (!init_graphics()) {
        allegro_message("Erreur initialisation graphique");
        return -1;
    }

    EtatJeu etat = ETAT_MENU;
    int selection = 0;

    /* Variables jeu */
    int player_x = 100;
    int player_y = 425;
    int score = 0;
    int temps = 60;
    int dir = 1; /* 1 = droite, -1 = gauche */

    /* Init projectiles */
    for (int i = 0; i < NB_PROJ; i++)
        proj_active[i] = 0;

    /* Init bulles */
    srand(time(NULL));
    for (int i = 0; i < NB_BULLES; i++)
        bulle_active[i] = 0;
    cpt_spawn_bulle = 0;

    while (!key[KEY_ESC]) {

        /* ================= MENU ================= */
        if (etat == ETAT_MENU) {

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
                if (selection == 0) {
                    etat = ETAT_JEU;
                    player_x = 100;
                    player_y = 425;
                    score = 0;
                    temps = 60;
                    dir = 1;
                    for (int i = 0; i < NB_PROJ; i++)
                        proj_active[i] = 0;
                    for (int i = 0; i < NB_BULLES; i++)
                        bulle_active[i] = 0;
                    cpt_spawn_bulle = 0;
                }
                else if (selection == 3) {
                    break;
                }
                rest(200);
            }

            draw_menu(selection);
        }

        /* ================= JEU ================= */
        else if (etat == ETAT_JEU) {

            clear_screen();
            draw_background_level(1);

            int moving = 0;

            if (key[KEY_RIGHT]) {
                player_x += 5;
                moving = 1;
                dir = -1;
            }

            if (key[KEY_LEFT]) {
                player_x -= 5;
                moving = 1;
                dir = 1;
            }

            if (player_x < 0) player_x = 0;
            if (player_x > SCREEN_W - 100) player_x = SCREEN_W - 100;

            /* === PROJECTILES === */
            for (int i = 0; i < NB_PROJ; i++) {
                if (proj_active[i]) {
                    proj_y[i] -= 8;
                    if (proj_y[i] < -32)
                        proj_active[i] = 0;
                }
            }

            int peut_tirer = 1;
            for (int i = 0; i < NB_PROJ; i++) {
                if (proj_active[i] && proj_y[i] > player_y - 60) {
                    peut_tirer = 0;
                    break;
                }
            }

            if (peut_tirer) {
                for (int i = 0; i < NB_PROJ; i++) {
                    if (!proj_active[i]) {
                        proj_x[i] = player_x + 40;
                        proj_y[i] = player_y;
                        proj_active[i] = 1;
                        break;
                    }
                }
            }

            for (int i = 0; i < NB_PROJ; i++)
                draw_projectile(proj_x[i], proj_y[i], proj_active[i]);

            /* === BULLES ENNEMIES === */
            cpt_spawn_bulle++;
            if (cpt_spawn_bulle > 60) {
                cpt_spawn_bulle = 0;
                for (int i = 0; i < NB_BULLES; i++) {
                    if (!bulle_active[i]) {
                        spawn_bulle(i);
                        break;
                    }
                }
            }

            for (int i = 0; i < NB_BULLES; i++) {
                if (bulle_active[i]) {
                    bulle_x[i] += bulle_dx[i];
                    bulle_y[i] += bulle_dy[i];

                    if (bulle_x[i] > SCREEN_W + 32 || bulle_x[i] < -32 || bulle_y[i] > SCREEN_H + 32)
                        bulle_active[i] = 0;

                    draw_bubble(bulle_x[i], bulle_y[i], bulle_active[i]);
                }
            }

            draw_player(player_x, player_y, moving, dir);
            draw_ui(score, temps, "Player1");
            update_display();

            rest(20);
        }

        /* ================= FIN ================= */
        else if (etat == ETAT_FIN) {
            draw_end_screen(score, 1);
            rest(20);
        }
    }

    destroy_graphics();
    return 0;
}

END_OF_MAIN();
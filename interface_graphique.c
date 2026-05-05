#include <stdio.h>
#include <stdlib.h>
#include <allegro.h>
#include "interface_graphique.h"

typedef enum {
    ETAT_MENU,
    ETAT_JEU,
    ETAT_FIN
} EtatJeu;

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
BITMAP *img_projectile[2];

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
    /*img_projectile[0] = load_bitmap("assets/projectile1.bmp", NULL);
    /*img_projectile[1] = load_bitmap("assets/projectile2.bmp", NULL);

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
        if (img_projectile[i]) destroy_bitmap(img_projectile[i]);
    }
    if (img_bubble) destroy_bitmap(img_bubble);
    if (img_background_menu) destroy_bitmap(img_background_menu);
    if (img_background_lvl1) destroy_bitmap(img_background_lvl1);
    if (img_background_lvl3) destroy_bitmap(img_background_lvl3);
    if (img_background_lvl4) destroy_bitmap(img_background_lvl4);
    if (img_banner) destroy_bitmap(img_banner);
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
                      SCREEN_W/2, 150, makecol(255,255,255), -1);

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

void draw_player(int x, int y) {

    cpt_player++;

    if (cpt_player > 10) {
        cpt_player = 0;
        anim_player = (anim_player + 1) % 2;
    }

    stretch_sprite(buffer, img_player[anim_player],
               x, y,
               img_player[anim_player]->w * 2,
               img_player[anim_player]->h * 2);
}

void draw_bubble(int x, int y, int active) {
    if (active && img_bubble) {
        draw_sprite(buffer, img_bubble, x, y);
    }
}

void draw_projectile(int x, int y, int active) {

    if (!active) return;

    cpt_projectile++;

    if (cpt_projectile > 5) {
        cpt_projectile = 0;
        anim_projectile = (anim_projectile + 1) % 2;
    }

    draw_sprite(buffer, img_projectile[anim_projectile], x, y);
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

    /* Variables de jeu */
    int player_x = 100;
    int player_y = 400;
    int score = 0;
    int temps = 60;

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
                    etat = ETAT_JEU; // 👉 LANCER LE JEU
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

            /* Fond niveau 1 */
            draw_background_level(1);

            /* Joueur */
            draw_player(player_x, player_y);

            /* UI */
            draw_ui(score, temps, "Player1");

            update_display();

            rest(20);
        }

        /* ================= FIN ================= */
        else if (etat == ETAT_FIN) {
            draw_end_screen(score, 1);
        }
    }

    destroy_graphics();
    return 0;
}
END_OF_MAIN();
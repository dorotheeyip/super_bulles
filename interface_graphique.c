#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <allegro.h>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOGDI
#define NOGDI
#endif
#include <windows.h>
#else
#include <sys/time.h>
#endif
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

static void lancer_partie_graphique(Joueur *joueur, Niveau *niveau, int niveau_depart, int *niveau_courant, int *resultat_niveau) {
    *niveau_courant = niveau_depart;
    *resultat_niveau = -1;
    initialiser_niveau(niveau, *niveau_courant);
}

static void ajouter_projectile_joueur(Niveau *niveau, Joueur *joueur) {
    for (int i = 0; i < niveau->nb_projectiles; i++) {
        if (!niveau->projectiles[i].actif) {
            niveau->projectiles[i] = tirer(joueur);
            return;
        }
    }

    if (niveau->nb_projectiles < 20) {
        niveau->projectiles[niveau->nb_projectiles] = tirer(joueur);
        niveau->nb_projectiles++;
    }
}

static int lancer_niveau_graphique(Niveau *niveau, Joueur *joueur, float dt, float *timer_tir_auto) {
    *timer_tir_auto += dt;
    if (*timer_tir_auto > 0.1f) {
        ajouter_projectile_joueur(niveau, joueur);
        *timer_tir_auto = 0.0f;
    }

    maj_niveau(niveau, joueur, dt);
    return niveau_termine(niveau, joueur);
}

static double temps_actuel_secondes(void) {
#ifdef _WIN32
    return (double)GetTickCount() / 1000.0;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
#endif
}

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
float eclair_cpt[NB_ECLAIRS];  /* timer d'animation par éclair */
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
float explo_cpt[NB_EXPLOSIONS];

#define NB_PROJ 10 /* nombre de projectiles simultanés */

int proj_x[NB_PROJ];
int proj_y[NB_PROJ];
int proj_active[NB_PROJ];

#define NB_PROJ_BOSS 5

Projectile proj_boss[NB_PROJ_BOSS];
float timer_tir_boss = 0.0f;  /* cadence de tir du boss */


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
        draw_sprite_h_flip(buffer, tmp, x, y);
    } else {
        draw_sprite(buffer, tmp, x, y);
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
        draw_sprite(buffer, tmp, x - w/2, y - h/2);
    } else {
        draw_sprite_h_flip(buffer, tmp, x - w/2, y - h/2);
    }

    destroy_bitmap(tmp);
}

void draw_boss_vie(int vie) {
    if (vie <= 0) return;
    if (vie > BOSS_PV_MAX) vie = BOSS_PV_MAX;

    /* index = vie pleine -> image 0, vie presque morte -> dernière image */
    int index = ((BOSS_PV_MAX - vie) * BOSS_VIE_MAX) / BOSS_PV_MAX;
    if (index >= BOSS_VIE_MAX) index = BOSS_VIE_MAX - 1;

    BITMAP *img = img_boss_vie[index];

    if (img)
        draw_sprite(buffer, img, (SCREEN_W - img->w) / 2, 10);
}

void draw_bubble(Bulle *bulle) {
    if (!bulle || !bulle->actif || !img_bubble) return;

    /* The bubble position is its center; draw sprite centered and scaled to hitbox diameter. */
    int diametre = bulle->r * 2;
    int x = (int)(bulle->x - bulle->r);
    int y = (int)(bulle->y - bulle->r);

    stretch_sprite(buffer, img_bubble, x, y, diametre, diametre);

    if(bulle->charge_eclair){
        int clignote = ((int)(bulle->timer_eclair * 12.0f)) % 2;
        if(clignote){
            circlefill(buffer, (int)bulle->x, (int)bulle->y, bulle->r, makecol(255, 230, 0));
            circle(buffer, (int)bulle->x, (int)bulle->y, bulle->r, makecol(255, 0, 0));
        }
    }
}

void draw_bubble_hitbox(Bulle* bulle) {
    if (!bulle->actif) return;
    
    // Draw circular hitbox (green)
    circle(buffer, (int)bulle->x, (int)bulle->y, bulle->r, makecol(0, 255, 0));
}

void draw_player_hitbox(Joueur* joueur) {
    // Draw rectangular hitbox (yellow)
    rect(buffer, (int)joueur->x, (int)joueur->y, (int)(joueur->x + joueur->tx), (int)(joueur->y + joueur->ty), makecol(255, 255, 0));
}

void draw_boss_hitbox(Boss* boss) {
    // Draw circular hitbox (blue) with radius 40
    circle(buffer, (int)boss->x, (int)boss->y, 40, makecol(0, 0, 255));
}

void draw_projectile_hitbox(Projectile* proj) {
    if (!proj->actif) return;
    
    // Draw circular hitbox (cyan) with radius 5
    circle(buffer, (int)proj->x, (int)proj->y, 5, makecol(0, 255, 255));
}

void draw_projectile_boss_hitbox(int x, int y, int active) {
    if (!active) return;
    
    // Draw circular hitbox (purple)
    circle(buffer, x, y, (int)RAYON_HITBOX_PROJECTILE_BOSS, makecol(255, 0, 255));
}

void draw_eclair_hitbox(Projectile* proj) {
    if (!proj->actif) return;
    if (proj->delai_activation > 0.0f) return;

    // Draw rectangular hitbox (red)
    rect(buffer,
         (int)(proj->x - ECLAIR_HITBOX_LARGEUR / 2.0f),
         (int)proj->y,
         (int)(proj->x + ECLAIR_HITBOX_LARGEUR / 2.0f),
         (int)(proj->y + ECLAIR_HITBOX_HAUTEUR),
         makecol(255, 0, 0));
}

void draw_explosion(int x, int y, int active, int anim) {
    if (!active) return;

    BITMAP *frame = img_explosion[anim];
    if (!frame) return;

    int w = frame->w;
    int h = frame->h;

    BITMAP *tmp = create_bitmap(w, h);
    if (!tmp) return;
    clear_to_color(tmp, makecol(255, 0, 255));
    stretch_sprite(tmp, frame, 0, 0, w, h);
    draw_sprite(buffer, tmp, x - w/2, y - h/2);
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
    draw_sprite(buffer, tmp, x-w/2, y);
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
    draw_sprite(buffer, tmp, x - w/2, y - h/2);

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
    draw_sprite(buffer, tmp, x-w/2, y-h/2);

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

static void update_explosions(float dt) {
    const float frame_duration = 0.05f;
    for (int i = 0; i < NB_EXPLOSIONS; i++) {
        if (!explo_active[i]) continue;
        explo_cpt[i] += dt;
        while (explo_cpt[i] >= frame_duration) {
            explo_cpt[i] -= frame_duration;
            explo_anim[i]++;
            if (explo_anim[i] >= 7) {
                explo_active[i] = 0;
                explo_anim[i] = 0;
                explo_cpt[i] = 0.0f;
                break;
            }
        }
    }
}

static void update_eclairs(float dt) {
    const float frame_duration = 0.06f;
    for (int i = 0; i < NB_ECLAIRS; i++) {
        if (!eclair_active[i]) continue;
        eclair_cpt[i] += dt;
        while (eclair_cpt[i] >= frame_duration) {
            eclair_cpt[i] -= frame_duration;
            eclair_anim[i]++;
            if (eclair_anim[i] >= 4) {
                eclair_active[i] = 0;
                eclair_anim[i] = 0;
                eclair_cpt[i] = 0.0f;
                break;
            }
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

    int niveau_fond = niveau_actuel;
    if(niveau_fond > 3) niveau_fond = 3;
    draw_background_level(niveau_fond + 1);

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
            choix[0] = (niveau_actuel <= 3) ? "NIVEAU SUIVANT" : "REVENIR AU MENU";
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
    joueur->y = SCREEN_H-170;
    joueur->score = 0;
    dir = 1;
    *niveau_actuel = 0;

    niveau->boss.x = 400;
    niveau->boss.y = 50;
    niveau->boss.vitesse = 0;
    niveau->boss.pv = 0;

    /* Reset projectiles joueur */
    if (niveau->projectiles) {
        for (int i = 0; i < 20; i++) {
            niveau->projectiles[i].actif = 0;
            niveau->projectiles[i].type = 0;
            niveau->projectiles[i].duree_vie = 0.0f;
            niveau->projectiles[i].delai_activation = 0.0f;

        }



        niveau->nb_projectiles = 0;

    }

    /* Reset projectiles boss */
    for (int i = 0; i < NB_PROJ_BOSS; i++) {
        proj_boss[i].actif = 0;
        proj_boss[i].x = 0;
        proj_boss[i].y = 0;
        proj_boss[i].vitesse = 0;
        proj_boss[i].duree_vie = 0.0f;
        proj_boss[i].delai_activation = 0.0f;
        proj_boss[i].type = 2;
    }

    timer_tir_boss = 0.0f;

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

void update_menu_fin(float dt) {
    if (fin_scale < 1.0f) {
        fin_scale += 2.5f * dt;
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

    double last_time_sec = temps_actuel_secondes();

    EtatJeu etat = ETAT_MENU;
    int selection = 0;

    Joueur joueur;
    Niveau niveau_struct;
    int niveau_actuel = 0;
    int mode_charger = 0;
    int victoire = 0;
    int resultat_niveau = -1;
    float timer_tir_auto = 0.0f;
    int prev_up = 0;
    int prev_down = 0;
    int prev_enter = 0;
    float menu_nav_cooldown = 0.0f;
    float menu_enter_cooldown = 0.0f;

    memset(&joueur, 0, sizeof(Joueur));
    memset(&niveau_struct, 0, sizeof(Niveau));

    reset_game(&joueur, &niveau_struct, &niveau_actuel);

    while (!key[KEY_ESC]) {
        double now_time_sec = temps_actuel_secondes();
        float frame_dt = (float)(now_time_sec - last_time_sec);
        last_time_sec = now_time_sec;
        if (frame_dt < 0.0f) frame_dt = 0.0f;
        if (frame_dt > 0.1f) frame_dt = 0.1f;

        if (menu_nav_cooldown > 0.0f) menu_nav_cooldown -= frame_dt;
        if (menu_enter_cooldown > 0.0f) menu_enter_cooldown -= frame_dt;

        int up_pressed = key[KEY_UP] && !prev_up;
        int down_pressed = key[KEY_DOWN] && !prev_down;
        int enter_pressed = key[KEY_ENTER] && !prev_enter;

        switch (etat) {

        /* ================= MENU ================= */
        case ETAT_MENU:

            if (up_pressed && menu_nav_cooldown <= 0.0f) {
                selection--;
                if (selection < 0) selection = NB_OPTIONS - 1;
                menu_nav_cooldown = 0.2f;
            }

            if (down_pressed && menu_nav_cooldown <= 0.0f) {
                selection++;
                if (selection >= NB_OPTIONS) selection = 0;
                menu_nav_cooldown = 0.2f;
            }

            if (enter_pressed && menu_enter_cooldown <= 0.0f) {
                menu_enter_cooldown = 0.2f;

                if (selection == 0) { // Nouvelle partie
                    initialiser_joueur(&joueur, "");
                    lancer_partie_graphique(&joueur, &niveau_struct, 0, &niveau_actuel, &resultat_niveau); // remettre à 0
                    timer_tir_auto = 0.0f;
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
                                niveau_actuel = 0;
                            }
                        } else {
                            initialiser_joueur(&joueur, pseudo);
                            niveau_actuel = 0;
                        }
                        lancer_partie_graphique(&joueur, &niveau_struct, niveau_actuel, &niveau_actuel, &resultat_niveau);
                        timer_tir_auto = 0.0f;
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

                if (enter_pressed && menu_enter_cooldown <= 0.0f) {
                    menu_enter_cooldown = 0.2f;
                    etat = ETAT_MENU;
                }

            break;
            }


        /* ================= JEU ================= */
        case ETAT_JEU: {

            clear_screen();

            draw_background_level(niveau_actuel + 1);

            float dt = frame_dt;

            /* input */

            if (key[KEY_LEFT]) {

                deplacer_joueur(&joueur, 0, dt);

                dir = -1;

                moving = 1;

            } else if (key[KEY_RIGHT]) {

                deplacer_joueur(&joueur, 1, dt);

                dir = 1;

                moving = 1;

            } else {

                moving = 0;

            }

            if (joueur.x < 0) joueur.x = 0;

            if (joueur.x > SCREEN_W - 100) joueur.x = SCREEN_W - 100;

            /* update boss projectiles */
            int touche_projectile_boss = boss_attaque(&niveau_struct.boss, &niveau_struct.bulles, proj_boss, NB_PROJ_BOSS, &joueur, dt, &timer_tir_boss);

            /* update */

            if(touche_projectile_boss) {
                resultat_niveau = 0;
            } else {
                resultat_niveau = lancer_niveau_graphique(&niveau_struct, &joueur, dt, &timer_tir_auto);
            }
            update_explosions(dt);
            update_eclairs(dt);

            if(resultat_niveau == 1) {

                victoire = 1;
                if(niveau_struct.temps_restant > 0) {
                    joueur.score += (int)niveau_struct.temps_restant;
                }

                fin_niveau(resultat_niveau, &joueur);

                niveau_actuel++;

                liberer_niveau(&niveau_struct);

                timer_tir_auto = 0.0f;
                etat = ETAT_FIN;

                if(niveau_actuel > 3) {
                    printf("Partie terminée !\n");
                }

            } else if(resultat_niveau == 0) {

                victoire = 0;

                fin_niveau(resultat_niveau, &joueur);

                etat = ETAT_FIN;

            }

            if(etat != ETAT_JEU) {
                rest(16);
                break;
            }

            /* draw */

            draw_player((int)joueur.x, (int)joueur.y, moving, dir);
            draw_player_hitbox(&joueur);

            if(niveau_struct.boss.pv > 0) {

                boss_dir = (niveau_struct.boss.vitesse > 0) ? 1 : -1;

                boss_moving = 1;

                draw_boss((int)niveau_struct.boss.x, (int)niveau_struct.boss.y, boss_moving, boss_dir);
                draw_boss_hitbox(&niveau_struct.boss);

                draw_boss_vie(niveau_struct.boss.pv);

            }

            for(int i = 0; i < niveau_struct.bulles.nb; i++) {

                draw_bubble(&niveau_struct.bulles.tab[i]);
                draw_bubble_hitbox(&niveau_struct.bulles.tab[i]);

            }

            for(int i = 0; i < niveau_struct.nb_projectiles; i++) {

                if(niveau_struct.projectiles[i].type != 0) {
                    draw_eclair_hitbox(&niveau_struct.projectiles[i]);
                    continue;
                }
                draw_projectile((int)niveau_struct.projectiles[i].x, (int)niveau_struct.projectiles[i].y, niveau_struct.projectiles[i].actif);
                draw_projectile_hitbox(&niveau_struct.projectiles[i]);

            }

            for(int i = 0; i < NB_PROJ_BOSS; i++) {

                draw_projectile_boss((int)proj_boss[i].x, (int)proj_boss[i].y, proj_boss[i].actif);
                draw_projectile_boss_hitbox((int)proj_boss[i].x, (int)proj_boss[i].y, proj_boss[i].actif);

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

                update_menu_fin(frame_dt);
                draw_menu_fin(victoire, niveau_actuel, joueur.score, victoire ? img_victoire : img_defaite);

                if (fin_anim_done) {

                    if (up_pressed && menu_nav_cooldown <= 0.0f) {
                        fin_selection = 0;
                        menu_nav_cooldown = 0.2f;
                    }

                    if (down_pressed && menu_nav_cooldown <= 0.0f) {
                        fin_selection = 1;
                        menu_nav_cooldown = 0.2f;
                    }

                    if (enter_pressed && menu_enter_cooldown <= 0.0f) {
                        menu_enter_cooldown = 0.2f;

                        int choix = fin_selection;

                        if (choix == 0) {
                            if (victoire) {
                                if(niveau_actuel <= 3) {
                                    initialiser_niveau(&niveau_struct, niveau_actuel);
                                    timer_tir_auto = 0.0f;
                                    etat = ETAT_JEU;
                                } else {
                                    sauvegarder_partie(&joueur, niveau_actuel);
                                    fin_selection = 0;
                                    etat = ETAT_MENU;
                                }
                            } else {
                                liberer_niveau(&niveau_struct);
                                joueur.x = SCREEN_W/2;
                                joueur.y = SCREEN_H-170;
                                dir = 1;
                                timer_tir_auto = 0.0f;
                                timer_tir_boss = 0.0f;
                                for(int i = 0; i < NB_PROJ_BOSS; i++){
                                    proj_boss[i].actif = 0;
                                }
                                initialiser_niveau(&niveau_struct, niveau_actuel);
                                etat = ETAT_JEU;
                            }
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

        prev_up = key[KEY_UP];
        prev_down = key[KEY_DOWN];
        prev_enter = key[KEY_ENTER];
    }
}

    destroy_graphics();
    return 0;
}

END_OF_MAIN();

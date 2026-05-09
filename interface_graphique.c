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

void lancer_partie_graphique(Joueur *joueur, Niveau *niveau, int niveau_depart, int *niveau_courant, int *resultat_niveau) {
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

int lancer_niveau_graphique(Niveau *niveau, Joueur *joueur, float dt, float *timer_tir_auto) {
    float cadence_tir = (joueur->arme == 1) ? 0.05f : 0.1f;

    *timer_tir_auto += dt;
    if (*timer_tir_auto > cadence_tir) {
        ajouter_projectile_joueur(niveau, joueur);
        *timer_tir_auto = 0.0f;
    }

    maj_niveau(niveau, joueur, dt);
    return niveau_termine(niveau, joueur);
}

double temps_actuel_secondes(void) {
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

BITMAP *img_eclair[4];

int eclair_x[NB_ECLAIRS];
int eclair_y[NB_ECLAIRS];
int eclair_active[NB_ECLAIRS];
int eclair_anim[NB_ECLAIRS];   /* frame d'animation par éclair */
float eclair_cpt[NB_ECLAIRS];  /* timer d'animation par éclair */
int cpt_spawn_eclair = 0;

BITMAP *img_boss_vie[BOSS_VIE_MAX];
int boss_vie = BOSS_VIE_MAX;  /* commence à vie pleine */

int bulle_x[NB_BULLES];
int bulle_y[NB_BULLES];
int bulle_dx[NB_BULLES]; /* vitesse horizontale */
int bulle_dy[NB_BULLES]; /* vitesse verticale */
int bulle_active[NB_BULLES];
int cpt_spawn_bulle = 0;  /* compteur pour espacer les spawns */

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

Projectile proj_boss[NB_PROJ_BOSS];
float timer_tir_boss = 0.0f;  /* cadence de tir du boss */


int boss_x = 400;
int boss_y = 50;
int boss_dir = 1;
int boss_moving = 1;

/* ============================= */
/* ===== VARIABLES GLOBALES ==== */
/* ============================= */
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

static void draw_text_outline(BITMAP *dest, FONT *f, const char *text, int x, int y, int color, int bg) {
    int outline = makecol(0, 0, 0);
    (void)bg;
    textout_ex(dest, f, text, x - 1, y, outline, -1);
    textout_ex(dest, f, text, x + 1, y, outline, -1);
    textout_ex(dest, f, text, x, y - 1, outline, -1);
    textout_ex(dest, f, text, x, y + 1, outline, -1);
    textout_ex(dest, f, text, x, y, color, -1);
}

void draw_text_centre_outline(BITMAP *dest, FONT *f, const char *text, int x, int y, int color, int bg) {
    int outline = makecol(0, 0, 0);
    (void)bg;
    textout_centre_ex(dest, f, text, x - 1, y, outline, -1);
    textout_centre_ex(dest, f, text, x + 1, y, outline, -1);
    textout_centre_ex(dest, f, text, x, y - 1, outline, -1);
    textout_centre_ex(dest, f, text, x, y + 1, outline, -1);
    textout_centre_ex(dest, f, text, x, y, color, -1);
}

void draw_text_centre_outline_scale(BITMAP *dest, FONT *f, const char *text, int x, int y, int color, int scale) {
    int marge = 4;
    int w = text_length(f, text) + marge * 2;
    int h = text_height(f) + marge * 2;
    int masque = makecol(255, 0, 255);
    BITMAP *tmp = create_bitmap(w, h);

    if(!tmp) return;

    clear_to_color(tmp, masque);
    draw_text_centre_outline(tmp, f, text, w / 2, marge, color, -1);

    masked_stretch_blit(tmp, dest,
                        0, 0, w, h,
                        x - (w * scale) / 2,
                        y - (h * scale) / 2,
                        w * scale, h * scale);

    destroy_bitmap(tmp);
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

    draw_text_centre_outline(buffer, font, "SUPER BULLES",
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
            color = makecol(255, 255, 0); // option sélectionnée en rouge
        else
            color = makecol(255, 255, 255); // blanc

        draw_text_centre_outline(buffer, font, options[i],
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
    int cx = (int)(joueur->x + joueur->tx / 2.0f);
    int cy = (int)(joueur->y + joueur->ty / 2.0f);
    int rayon = (joueur->tx < joueur->ty ? joueur->tx : joueur->ty) / 2;

    // Draw circular hitbox (yellow)
    circle(buffer, cx, cy, rayon, makecol(255, 255, 0));
}

void draw_buff(Buff* buff) {
    if(!buff || !buff->actif) return;

    int x = (int)buff->x;
    int y = (int)buff->y;
    int jaune = makecol(255, 230, 0);
    int orange = makecol(255, 140, 0);

    circlefill(buffer, x, y, 12, jaune);
    triangle(buffer, x, y - 20, x - 6, y - 6, x + 6, y - 6, jaune);
    triangle(buffer, x + 20, y, x + 6, y - 6, x + 6, y + 6, jaune);
    triangle(buffer, x, y + 20, x - 6, y + 6, x + 6, y + 6, jaune);
    triangle(buffer, x - 20, y, x - 6, y - 6, x - 6, y + 6, jaune);
    circle(buffer, x, y, 12, orange);
    textout_centre_ex(buffer, font, ">>", x, y - 4, makecol(255,255,255), -1);
}

void draw_buff_timer(Joueur* joueur) {
    if(!joueur || joueur->buff_tir_timer <= 0.0f) return;

    int largeur_max = joueur->tx;
    int largeur = (int)(largeur_max * (joueur->buff_tir_timer / 5.0f));
    int x = (int)joueur->x;
    int y = (int)(joueur->y + joueur->ty + 6);

    if(largeur < 0) largeur = 0;
    if(largeur > largeur_max) largeur = largeur_max;

    rect(buffer, x, y, x + largeur_max, y + 6, makecol(255,255,255));
    if(largeur > 2) {
        rectfill(buffer, x + 1, y + 1, x + largeur - 1, y + 5, makecol(255,230,0));
    }
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

void update_explosions(float dt) {
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

void update_eclairs(float dt) {
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

void draw_ui(int score, int time_left, const char *pseudo, int niveau) {
    char text[100];
    const int marge = 10;
    const int ligne1 = SCREEN_H - 50;
    const int ligne2 = SCREEN_H - 30;

    sprintf(text, "Score: %d", score);
    draw_text_outline(buffer, font, text, SCREEN_W - marge - text_length(font, text), ligne1, makecol(255,255,255), -1);

    sprintf(text, "Temps: %d", time_left);
    draw_text_outline(buffer, font, text, SCREEN_W - marge - text_length(font, text), ligne2, makecol(255,255,255), -1);

    sprintf(text, "Joueur: %s", pseudo);
    draw_text_outline(buffer, font, text, marge, ligne1, makecol(255,255,255), -1);

    sprintf(text, "Niveau: %d", niveau);
    draw_text_outline(buffer, font, text, marge, ligne2, makecol(255,255,255), -1);
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
        sprintf(msg_score, "SCORE FINAL: %d", score);
        draw_text_centre_outline(buffer, font, msg_score,
                          SCREEN_W/2, 220,
                          makecol(255,255,255), -1);

        const char *choix[4];
        choix[0] = victoire ? ((niveau_actuel <= 3) ? "NIVEAU SUIVANT" : "REVENIR AU MENU") : "RECOMMENCER";
        choix[1] = "SAUVEGARDER";
        choix[2] = "REVENIR AU MENU";
        choix[3] = "QUITTER PROGRAMME";

        for (int i = 0; i < 4; i++) {
            int color = (i == fin_selection)
                ? makecol(255,255,0)
                : makecol(200,200,200);

            draw_text_centre_outline(buffer, font,
                              choix[i],
                              SCREEN_W/2,
                              280 + i * 36,
                              color, -1);
        }
    }

    update_display();
}

void reset_game(Joueur *joueur, Niveau *niveau, int *niveau_actuel) {

    joueur->x = 100;
    joueur->y = SCREEN_H-170;
    joueur->score = 0;
    joueur->arme = 0;
    joueur->buff_tir_timer = 0.0f;
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
    niveau->nb_buffs = 10;
    for (int i = 0; i < niveau->nb_buffs; i++) {
        niveau->buffs[i].actif = 0;
        niveau->buffs[i].type = 0;
        niveau->buffs[i].vitesse = 120.0f;
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

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <allegro.h>
#include "struct.h"

/* ============================= */
/* ===== INITIALISATION ========= */
/* ============================= */

int init_graphics();
void destroy_graphics();

void clear_screen();
void update_display();

/* ============================= */
/* ===== MENU ================== */
/* ============================= */

void draw_menu(int selection);

/* ============================= */
/* ===== JEU =================== */
/* ============================= */

/* On passe uniquement les infos utiles */

void draw_player(int x, int y, int moving, int dir);
void draw_boss(int x, int y, int moving_boss, int dir);
void draw_boss_vie(int vie);

void draw_bubble(Bulle *bulle);

void draw_explosion(int x, int y, int active, int anim);
void draw_eclair(int x, int y, int active, int anim);

void draw_projectile(int x, int y, int active);
void draw_projectile_boss(int x, int y, int active);

void spawn_explosion(int x, int y);
void spawn_eclair(int x, int y);

void draw_menu_fin(int victoire, int niveau_actuel, int score, BITMAP *img_annonce);
void reset_game(Joueur *joueur, Niveau *niveau, int *niveau_actuel);
void update_menu_fin(float dt);

void draw_ui(int score, int time_left, const char *pseudo);

/* ============================= */
/* ===== GLOBAL ================= */
/* ============================= */

void draw_background_level(int level);

void draw_end_screen(int score, int victoire);

#endif
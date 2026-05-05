#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <allegro.h>

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

void draw_bubble(int x, int y, int active);

void draw_projectile(int x, int y, int active);

void draw_ui(int score, int time_left, const char *pseudo);

/* ============================= */
/* ===== GLOBAL ================= */
/* ============================= */

void draw_background_level(int level);

void draw_end_screen(int score, int victoire);

#endif
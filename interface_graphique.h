#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <allegro.h>
#include "struct.h"
#include "ihm.h"

#define NB_OPTIONS 5
#define NB_ECLAIRS 5
#define BOSS_VIE_MAX 6
#define NB_EXPLOSIONS 5
#define NB_PROJ_BOSS 5

extern float fin_scale;


/* ============================= */
/* ===== INITIALISATION ========= */
/* ============================= */

int init_graphics();
void destroy_graphics();

void clear_screen();
void update_display();
double temps_actuel_secondes(void);
void lancer_partie_graphique(Joueur *joueur, Niveau *niveau, int niveau_depart, int *niveau_courant, int *resultat_niveau);
int lancer_niveau_graphique(Niveau *niveau, Joueur *joueur, float dt, float *timer_tir_auto);
void update_explosions(float dt);
void update_eclairs(float dt);
void draw_text_centre_outline(BITMAP *dest, FONT *f, const char *text, int x, int y, int color, int bg);
void draw_text_centre_outline_scale(BITMAP *dest, FONT *f, const char *text, int x, int y, int color, int scale);
void draw_text_outline_public(BITMAP *dest, FONT *f, const char *text, int x, int y, int color, int bg);

/* ============================= */
/* ===== MENU ================== */
/* ============================= */

void draw_menu(int selection);

/* ============================= */
/* ===== JEU =================== */
/* ============================= */

/* On passe uniquement les infos utiles */

void draw_player(int x, int y, int moving, int dir);
void draw_player_stun(int x, int y, int moving, int dir, float stun_timer);
void draw_player2(int x, int y, int moving, int dir);
void draw_player2_stun(int x, int y, int moving, int dir, float stun_timer);
void draw_boss(int x, int y, int moving_boss, int dir);
void draw_boss_vie(int vie);

void draw_bubble(Bulle *bulle);
void draw_bubble_hitbox(Bulle* bulle);
void draw_player_hitbox(Joueur* joueur);
void draw_boss_hitbox(Boss* boss);
void draw_projectile_hitbox(Projectile* proj);
void draw_projectile_boss_hitbox(int x, int y, int active);
void draw_eclair_hitbox(Projectile* proj);
void draw_buff(Buff* buff);
void draw_buff_timer(Joueur* joueur);

void draw_explosion(int x, int y, int active, int anim);
void draw_eclair(int x, int y, int active, int anim);

void draw_projectile(int x, int y, int active);
void draw_projectile_boss(int x, int y, int active);

void spawn_explosion(int x, int y);
void spawn_eclair(int x, int y);

void draw_menu_fin(int victoire, int niveau_actuel, int score, BITMAP *img_annonce);
void reset_game(Joueur *joueur, Niveau *niveau, int *niveau_actuel);
void update_menu_fin(float dt);

void draw_ui(int score, int time_left, const char *pseudo, int niveau);

/* ============================= */
/* ===== GLOBAL ================= */
/* ============================= */

void draw_background_level(int level);

void draw_end_screen(int score, int victoire);

extern int fin_selection;
extern int fin_anim_done;
extern char pseudo[20];
extern int pseudo_index;
extern int eclair_x[NB_ECLAIRS];
extern int eclair_y[NB_ECLAIRS];
extern int eclair_active[NB_ECLAIRS];
extern int eclair_anim[NB_ECLAIRS];
extern int explo_x[NB_EXPLOSIONS];
extern int explo_y[NB_EXPLOSIONS];
extern int explo_active[NB_EXPLOSIONS];
extern int explo_anim[NB_EXPLOSIONS];
extern Projectile proj_boss[NB_PROJ_BOSS];
extern float timer_tir_boss;
extern int boss_dir;
extern int boss_moving;
extern int dir;
extern int moving;
extern int moving_boss;
extern BITMAP *buffer;
extern BITMAP *img_background_menu;
extern BITMAP *img_victoire;
extern BITMAP *img_defaite;

#endif

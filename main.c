#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <allegro.h>
#include "interface_graphique.h"
#include "joueur.h"
#include "niveau.h"
#include "jeu.h"
#include "ihm.h"

#define DUREE_STUN_DUEL 2.0f

static void nettoyer_effets_niveau(void) {
    for(int i = 0; i < NB_ECLAIRS; i++) {
        eclair_active[i] = 0;
        eclair_anim[i] = 0;
    }
    for(int i = 0; i < NB_EXPLOSIONS; i++) {
        explo_active[i] = 0;
        explo_anim[i] = 0;
    }
    for(int i = 0; i < NB_PROJ_BOSS; i++) {
        proj_boss[i].actif = 0;
    }
}

static void ajouter_projectile_duel(Niveau *niveau, Joueur *joueur, int proprietaire) {
    for(int i = 0; i < niveau->nb_projectiles; i++) {
        if(!niveau->projectiles[i].actif) {
            niveau->projectiles[i] = tirer(joueur);
            niveau->projectiles[i].proprietaire = proprietaire;
            return;
        }
    }

    if(niveau->nb_projectiles < 20) {
        niveau->projectiles[niveau->nb_projectiles] = tirer(joueur);
        niveau->projectiles[niveau->nb_projectiles].proprietaire = proprietaire;
        niveau->nb_projectiles++;
    }
}

static void mettre_joueur_en_stun(Joueur *joueur) {
    if(joueur->stun_timer <= 0.0f) {
        spawn_explosion(joueur->x + joueur->tx / 2, joueur->y + joueur->ty / 2);
    }
    joueur->stun_timer = DUREE_STUN_DUEL;
    joueur->arme = 0;
    joueur->buff_tir_timer = 0.0f;
}

static void draw_ui_duel(Joueur *joueur_gauche, Joueur *joueur_droite, int time_left, int niveau) {
    char texte[100];
    const int marge = 10;
    const int y_score = SCREEN_H - 35;
    const int y_centre = SCREEN_H - 50;

    sprintf(texte, "Joueur 1: %d", joueur_gauche->score);
    draw_text_outline_public(buffer, font, texte, marge, y_score, makecol(255,255,255), -1);

    sprintf(texte, "Joueur 2: %d", joueur_droite->score);
    draw_text_outline_public(buffer, font, texte, SCREEN_W - marge - text_length(font, texte), y_score, makecol(255,255,255), -1);

    sprintf(texte, "Temps: %d", time_left);
    draw_text_centre_outline(buffer, font, texte, SCREEN_W / 2, y_centre, makecol(255,255,255), -1);

    sprintf(texte, "Niveau: %d", niveau);
    draw_text_centre_outline(buffer, font, texte, SCREEN_W / 2, y_centre + 20, makecol(255,255,255), -1);
}

static int maj_collisions_duel(Niveau *niveau, Joueur *joueur) {
    int touche = 0;

    if(joueur->stun_timer > 0.0f) return 0;

    for(int i = 0; i < niveau->bulles.nb; i++) {
        if(niveau->bulles.tab[i].actif && collision_bulle_joueur(&niveau->bulles.tab[i], joueur)) {
            touche = 1;
        }
    }

    for(int i = 0; i < niveau->nb_projectiles; i++) {
        if(niveau->projectiles[i].actif && niveau->projectiles[i].type == 1 &&
           collision_projectile_boss_joueur(&niveau->projectiles[i], joueur)) {
            niveau->projectiles[i].actif = 0;
            touche = 1;
        }
    }

    if(touche) {
        mettre_joueur_en_stun(joueur);
    }

    return touche;
}

int main(void) {

    if (!init_graphics()) {
        printf("Erreur initialisation graphique\n");
        return -1;
    }

    ihm_init();

    srand(time(NULL));

    double last_time_sec = temps_actuel_secondes();

    int selection = 0;
    int selection_pause = 0;

    Joueur joueur;
    Joueur joueur2;
    Niveau niveau_struct;
    int niveau_actuel = 0;
    int mode_charger = 0;
    int mode_duel = 0;
    int victoire = 0;
    int gagnant_duel = 0;
    int resultat_niveau = -1;
    float timer_tir_auto = 0.0f;
    float timer_tir_auto_j2 = 0.0f;
    float compte_rebours = 0.0f;
    int quitter_programme = 0;

    SaisiePseudo saisie;
    int saisie_initialisee = 0;

    memset(&joueur, 0, sizeof(Joueur));
    memset(&joueur2, 0, sizeof(Joueur));
    memset(&niveau_struct, 0, sizeof(Niveau));

    reset_game(&joueur, &niveau_struct, &niveau_actuel);

    while (!quitter_programme) {

        ihm_update();

        double now_time_sec = temps_actuel_secondes();
        float frame_dt = (float)(now_time_sec - last_time_sec);
        last_time_sec = now_time_sec;
        if (frame_dt < 0.0f) frame_dt = 0.0f;
        if (frame_dt > 0.1f) frame_dt = 0.1f;

        switch (ihm_get_etat()) {

        /* ================= MENU ================= */
        case ETAT_MENU: {

            saisie_initialisee = 0;
            fin_scale = 0.1f;
            fin_anim_done = 0;
            fin_selection = 0;

            draw_menu(selection);

            int choix = ihm_menu_principal(&selection, NB_OPTIONS);

            if (choix == 1) {
                mode_charger = 0;
                mode_duel = 0;
                saisie_initialisee = 0;
                clear_keybuf();
                ihm_set_etat(ETAT_PSEUDO);
            }
            else if (choix == 2) {
                mode_charger = 1;
                mode_duel = 0;
                saisie_initialisee = 0;
                clear_keybuf();
                ihm_set_etat(ETAT_PSEUDO);
            }
            else if (choix == 3) {
                ihm_set_etat(ETAT_DUO);
            }
            else if (choix == 4) {
                ihm_set_etat(ETAT_REGLES);
            }
            else if (choix == 5) {
                quitter_programme = 1;
            }

            rest(20);
            break;
        }

        /* ================= PSEUDO ================= */
        case ETAT_PSEUDO: {

            if (!saisie_initialisee) {
                ihm_init_saisie_pseudo(&saisie);
                clear_keybuf();
                saisie_initialisee = 1;
            }

            clear_bitmap(buffer);

            if (img_background_menu) {
                stretch_blit(img_background_menu, buffer,
                             0,0,img_background_menu->w,img_background_menu->h,
                             0,0,SCREEN_W,SCREEN_H);
            }

            int bx = SCREEN_W/2 - 200;
            int by = SCREEN_H/2 - 80;
            int bw = SCREEN_W/2 + 200;
            int bh = SCREEN_H/2 + 80;

            rectfill(buffer, bx, by, bw, bh, makecol(0, 0, 0));
            rect(buffer, bx, by, bw, bh, makecol(255, 255, 0));

            draw_text_centre_outline(buffer, font,
                "ENTREZ VOTRE PSEUDO",
                SCREEN_W/2, SCREEN_H/2 - 55,
                makecol(255, 255, 0), -1);

            rectfill(buffer,
                     SCREEN_W/2 - 150, SCREEN_H/2 - 15,
                     SCREEN_W/2 + 150, SCREEN_H/2 + 15,
                     makecol(50, 50, 50));

            rect(buffer,
                 SCREEN_W/2 - 150, SCREEN_H/2 - 15,
                 SCREEN_W/2 + 150, SCREEN_H/2 + 15,
                 makecol(255, 255, 255));

            ihm_update_saisie_pseudo(&saisie);

            draw_text_centre_outline(buffer, font,
                saisie.affichage,
                SCREEN_W/2, SCREEN_H/2 - 5,
                makecol(255, 255, 255), -1);

            draw_text_centre_outline(buffer, font,
                "Appuyez sur ENTREE pour valider",
                SCREEN_W/2, SCREEN_H/2 + 45,
                makecol(200, 200, 200), -1);

            update_display();

            if (saisie.fini && strlen(saisie.buffer) > 0) {
                strncpy(pseudo, saisie.buffer, 19);
                pseudo[19] = '\0';

                if(mode_charger) {
                    int niv = charger_partie(pseudo, &joueur);
                    initialiser_joueur(&joueur, pseudo);
                    niveau_actuel = (niv != -1) ? niv : 0;
                } else {
                    initialiser_joueur(&joueur, pseudo);
                    niveau_actuel = 0;
                }

                lancer_partie_graphique(&joueur, &niveau_struct, niveau_actuel, &niveau_actuel, &resultat_niveau);
                timer_tir_auto = 0.0f;
                timer_tir_auto_j2 = 0.0f;
                joueur.arme = 0;
                joueur.buff_tir_timer = 0.0f;
                joueur.stun_timer = 0.0f;
                nettoyer_effets_niveau();
                compte_rebours = 3.0f;
                saisie_initialisee = 0;
                mode_charger = 0;
                ihm_set_etat(ETAT_JEU);
            }

            break;
        }

        /* ================= REGLES ================= */
        case ETAT_REGLES: {

            clear_bitmap(buffer);

            if (img_background_menu) {
                stretch_blit(img_background_menu, buffer,
                             0,0,img_background_menu->w,img_background_menu->h,
                             0,0,SCREEN_W,SCREEN_H);
            }

            draw_text_centre_outline(buffer, font,
                "REGLES DU JEU",
                SCREEN_W/2, 80, makecol(255, 255, 255), -1);

            draw_text_centre_outline(buffer, font,
                "- Deplacer le joueur avec LEFT / RIGHT",
                SCREEN_W/2, 150, makecol(255, 255, 255), -1);

            draw_text_centre_outline(buffer, font,
                "- Tir automatique des projectiles",
                SCREEN_W/2, 180, makecol(255, 255, 255), -1);

            draw_text_centre_outline(buffer, font,
                "- Detruire les bulles pour gagner des points",
                SCREEN_W/2, 210, makecol(255, 255, 255), -1);

            draw_text_centre_outline(buffer, font,
                "- Eviter les attaques du boss",
                SCREEN_W/2, 240, makecol(255, 255, 255), -1);

            draw_text_centre_outline(buffer, font,
                "Appuie sur une touche pour revenir",
                SCREEN_W/2, 320, makecol(255,255,0), -1);

            update_display();

            ihm_afficher_regles();
            ihm_set_etat(ETAT_MENU);

            break;
        }

        /* ================= DUO ================= */
        case ETAT_DUO: {

            clear_bitmap(buffer);

            if (img_background_menu) {
                stretch_blit(img_background_menu, buffer,
                             0,0,img_background_menu->w,img_background_menu->h,
                             0,0,SCREEN_W,SCREEN_H);
            }

            draw_text_centre_outline(buffer, font,
                "MODE DUO",
                SCREEN_W/2, 160, makecol(255,255,255), -1);

            draw_text_centre_outline(buffer, font,
                "ENTER pour lancer",
                SCREEN_W/2, 260, makecol(255,255,0), -1);

            update_display();

            if (ihm_touche_appuyee(KEY_ENTER)) {
                mode_duel = 1;
                mode_charger = 0;
                initialiser_joueur(&joueur, "Joueur 1");
                initialiser_joueur(&joueur2, "Joueur 2");
                joueur.x = 40;
                joueur2.x = SCREEN_W - joueur2.tx - 40;
                joueur.y = SCREEN_H - 170;
                joueur2.y = SCREEN_H - 170;
                niveau_actuel = 0;
                initialiser_niveau(&niveau_struct, niveau_actuel);
                timer_tir_auto = 0.0f;
                timer_tir_auto_j2 = 0.0f;
                gagnant_duel = 0;
                fin_selection = 0;
                nettoyer_effets_niveau();
                compte_rebours = 3.0f;
                ihm_set_etat(ETAT_JEU);
            }

            if (ihm_touche_appuyee(KEY_ESC) || ihm_quitter_demande()) {
                ihm_set_etat(ETAT_MENU);
            }

            rest(20);
            break;
        }

        /* ================= JEU ================= */
        case ETAT_JEU: {

            clear_screen();
            draw_background_level(niveau_actuel + 1);

            float dt = frame_dt;

            if(joueur.stun_timer > 0.0f) {
                joueur.stun_timer -= dt;
                if(joueur.stun_timer < 0.0f) joueur.stun_timer = 0.0f;
            }
            if(mode_duel && joueur2.stun_timer > 0.0f) {
                joueur2.stun_timer -= dt;
                if(joueur2.stun_timer < 0.0f) joueur2.stun_timer = 0.0f;
            }

            Input input_j1;
            ihm_lire_inputs(&input_j1);

            if(compte_rebours > 0.0f) {
                moving = 0;
            } else if (joueur.stun_timer <= 0.0f && input_j1.gauche) {
                deplacer_joueur(&joueur, 0, dt);
                dir = -1;
                moving = 1;
            } else if (joueur.stun_timer <= 0.0f && input_j1.droite) {
                deplacer_joueur(&joueur, 1, dt);
                dir = 1;
                moving = 1;
            } else {
                moving = 0;
            }

            if (joueur.x < 0) joueur.x = 0;
            if (joueur.x > SCREEN_W - 100) joueur.x = SCREEN_W - 100;

            int moving2 = 0;
            int dir2 = 1;

            if(mode_duel) {
                Input input_j2;
                ihm_lire_inputs_joueur2(&input_j2);

                if(compte_rebours > 0.0f || joueur2.stun_timer > 0.0f) {
                    moving2 = 0;
                } else if(input_j2.gauche) {
                    deplacer_joueur(&joueur2, 0, dt);
                    dir2 = -1;
                    moving2 = 1;
                } else if(input_j2.droite) {
                    deplacer_joueur(&joueur2, 1, dt);
                    dir2 = 1;
                    moving2 = 1;
                }

                if (joueur2.x < 0) joueur2.x = 0;
                if (joueur2.x > SCREEN_W - 100) joueur2.x = SCREEN_W - 100;
            }

            if (input_j1.pause) {
                selection_pause = 0;
                ihm_maj_etats_precedents();
                ihm_set_etat(ETAT_PAUSE);
                rest(16);
                break;
            }

            if(compte_rebours > 0.0f) {
                int valeur_compte_rebours = (int)compte_rebours + 1;
                if(valeur_compte_rebours > 3) valeur_compte_rebours = 3;

                draw_player_stun((int)joueur.x, (int)joueur.y, moving, dir, joueur.stun_timer);
                if(mode_duel) {
                    draw_player2_stun((int)joueur2.x, (int)joueur2.y, moving2, dir2, joueur2.stun_timer);
                }

                if(niveau_struct.boss.pv > 0) {
                    boss_dir = (niveau_struct.boss.vitesse > 0) ? 1 : -1;
                    draw_boss((int)niveau_struct.boss.x, (int)niveau_struct.boss.y, 0, boss_dir);
                    draw_boss_vie(niveau_struct.boss.pv);
                }

                for(int i = 0; i < niveau_struct.bulles.nb; i++) {
                    draw_bubble(&niveau_struct.bulles.tab[i]);
                }

                for(int i = 0; i < niveau_struct.nb_buffs; i++) {
                    draw_buff(&niveau_struct.buffs[i]);
                }

                if(mode_duel) {
                    draw_ui_duel(&joueur, &joueur2, (int)niveau_struct.temps_restant, niveau_actuel + 1);
                } else {
                    draw_ui(joueur.score, (int)niveau_struct.temps_restant, joueur.pseudo, niveau_actuel + 1);
                }

                char texte_compte_rebours[8];
                sprintf(texte_compte_rebours, "%d", valeur_compte_rebours);
                draw_text_centre_outline_scale(buffer, font, texte_compte_rebours,
                                               SCREEN_W/2, SCREEN_H/2 - 20,
                                               makecol(255,255,255), 3);

                compte_rebours -= dt;
                if(compte_rebours < 0.0f) compte_rebours = 0.0f;

                update_display();
                rest(16);
                break;
            }

            if(mode_duel) {
                int touche_projectile_boss = boss_attaque(&niveau_struct.boss, &niveau_struct.bulles, proj_boss, NB_PROJ_BOSS, &joueur, dt, &timer_tir_boss);
                if(touche_projectile_boss) {
                    mettre_joueur_en_stun(&joueur);
                }
                for(int i = 0; i < NB_PROJ_BOSS; i++) {
                    if(proj_boss[i].actif && joueur2.stun_timer <= 0.0f && collision_projectile_boss_joueur(&proj_boss[i], &joueur2)) {
                        proj_boss[i].actif = 0;
                        mettre_joueur_en_stun(&joueur2);
                    }
                }

                timer_tir_auto += dt;
                if(timer_tir_auto > ((joueur.arme == 1) ? 0.04f : 0.1f) && joueur.stun_timer <= 0.0f) {
                    ajouter_projectile_duel(&niveau_struct, &joueur, 1);
                    timer_tir_auto = 0.0f;
                }
                float cadence_j2 = (joueur2.arme == 1) ? 0.04f : 0.1f;
                timer_tir_auto_j2 += dt;
                if(timer_tir_auto_j2 > cadence_j2 && joueur2.stun_timer <= 0.0f) {
                    ajouter_projectile_duel(&niveau_struct, &joueur2, 2);
                    timer_tir_auto_j2 = 0.0f;
                }
                maj_niveau_duel(&niveau_struct, &joueur, &joueur2, dt);
                maj_collisions_duel(&niveau_struct, &joueur);
                maj_collisions_duel(&niveau_struct, &joueur2);
                if(niveau_struct.temps_restant <= 0.0f || (niveau_struct.boss.pv <= 0 && niveau_struct.bulles.nb == 0)) {
                    resultat_niveau = 1;
                } else {
                    resultat_niveau = -1;
                }
            } else {
                int touche_projectile_boss = boss_attaque(&niveau_struct.boss, &niveau_struct.bulles, proj_boss, NB_PROJ_BOSS, &joueur, dt, &timer_tir_boss);
                if(touche_projectile_boss) {
                    resultat_niveau = 0;
                } else {
                    resultat_niveau = lancer_niveau_graphique(&niveau_struct, &joueur, dt, &timer_tir_auto);
                }
            }

            update_explosions(dt);
            update_eclairs(dt);

            if(resultat_niveau == 1) {
                victoire = 1;
                if(niveau_struct.temps_restant > 0) {
                    joueur.score += (int)niveau_struct.temps_restant;
                    if(mode_duel) joueur2.score += (int)niveau_struct.temps_restant;
                }
                if(mode_duel) {
                    gagnant_duel = 0;
                    if(joueur.score > joueur2.score) gagnant_duel = 1;
                    else if(joueur2.score > joueur.score) gagnant_duel = 2;
                }
                fin_niveau(resultat_niveau, &joueur);
                niveau_actuel++;
                liberer_niveau(&niveau_struct);
                timer_tir_auto = 0.0f;
                fin_selection = 0;
                fin_scale = 0.1f;
                fin_anim_done = 0;
                ihm_set_etat(ETAT_FIN);
                if(niveau_actuel > 3) printf("Partie terminée !\n");
            } else if(resultat_niveau == 0) {
                victoire = mode_duel ? 1 : 0;
                fin_niveau(resultat_niveau, &joueur);
                fin_selection = 0;
                fin_scale = 0.1f;
                fin_anim_done = 0;
                ihm_set_etat(ETAT_FIN);
            }

            if(ihm_get_etat() != ETAT_JEU) {
                rest(16);
                break;
            }

            draw_player_stun((int)joueur.x, (int)joueur.y, moving, dir, joueur.stun_timer);
            draw_buff_timer(&joueur);
            if(mode_duel) {
                draw_player2_stun((int)joueur2.x, (int)joueur2.y, moving2, dir2, joueur2.stun_timer);
                draw_buff_timer(&joueur2);
            }

            if(niveau_struct.boss.pv > 0) {
                boss_dir = (niveau_struct.boss.vitesse > 0) ? 1 : -1;
                boss_moving = 1;
                draw_boss((int)niveau_struct.boss.x, (int)niveau_struct.boss.y, boss_moving, boss_dir);
                draw_boss_vie(niveau_struct.boss.pv);
            }

            for(int i = 0; i < niveau_struct.bulles.nb; i++) {
                draw_bubble(&niveau_struct.bulles.tab[i]);
            }

            for(int i = 0; i < niveau_struct.nb_buffs; i++) {
                draw_buff(&niveau_struct.buffs[i]);
            }

            for(int i = 0; i < niveau_struct.nb_projectiles; i++) {
                if(niveau_struct.projectiles[i].type != 0) continue;
                draw_projectile((int)niveau_struct.projectiles[i].x, (int)niveau_struct.projectiles[i].y, niveau_struct.projectiles[i].actif);
            }

            for(int i = 0; i < NB_PROJ_BOSS; i++) {
                draw_projectile_boss((int)proj_boss[i].x, (int)proj_boss[i].y, proj_boss[i].actif);
            }

            for(int i = 0; i < NB_EXPLOSIONS; i++) {
                draw_explosion(explo_x[i], explo_y[i], explo_active[i], explo_anim[i]);
            }

            for(int i = 0; i < NB_ECLAIRS; i++) {
                draw_eclair(eclair_x[i], eclair_y[i], eclair_active[i], eclair_anim[i]);
            }

            if(mode_duel) {
                draw_ui_duel(&joueur, &joueur2, (int)niveau_struct.temps_restant, niveau_actuel + 1);
            } else {
                draw_ui(joueur.score, (int)niveau_struct.temps_restant, joueur.pseudo, niveau_actuel + 1);
            }

            update_display();
            rest(16);
            break;
        }

        /* ================= PAUSE ================= */
                /* ================= PAUSE ================= */
            case ETAT_PAUSE: {

            clear_bitmap(buffer);
            draw_background_level(niveau_actuel + 1);

            set_trans_blender(0, 0, 0, 128);
            drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
            rectfill(buffer, 0, 0, SCREEN_W, SCREEN_H, makecol(0,0,0));
            drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);

            draw_text_centre_outline(buffer, font,
                "PAUSE", SCREEN_W/2, 200, makecol(255,255,0), -1);

            int color_reprendre = (selection_pause == 0) ? makecol(255,255,0) : makecol(255,255,255);
            int color_menu      = (selection_pause == 1) ? makecol(255,255,0) : makecol(255,255,255);

            draw_text_centre_outline(buffer, font,
                "Reprendre", SCREEN_W/2, 260, color_reprendre, -1);

            draw_text_centre_outline(buffer, font,
                "Menu principal", SCREEN_W/2, 300, color_menu, -1);

            update_display();

            int choix_pause = ihm_menu_pause(&selection_pause, 2);

            if (choix_pause == 1) {
                selection_pause = 0;
                ihm_set_etat(ETAT_JEU);
            }
            if (choix_pause == 2) {
                selection_pause = 0;
                reset_game(&joueur, &niveau_struct, &niveau_actuel);
                ihm_set_etat(ETAT_MENU);
            }

            rest(20);
            break;
            }

        /* ================= FIN ================= */
        case ETAT_FIN: {

            if(mode_duel && fin_selection > 2) fin_selection = 0;

            update_menu_fin(frame_dt);

            if(mode_duel) {
                char texte_duel[120];
                int niveau_fond = niveau_actuel;
                if(niveau_fond > 3) niveau_fond = 3;

                draw_background_level(niveau_fond + 1);
                set_trans_blender(0, 0, 0, 128);
                drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
                rectfill(buffer, 0, 0, SCREEN_W, SCREEN_H, makecol(0, 0, 0));
                drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);

                if(gagnant_duel == 1) sprintf(texte_duel, "Joueur 1 gagne !");
                else if(gagnant_duel == 2) sprintf(texte_duel, "Joueur 2 gagne !");
                else sprintf(texte_duel, "Egalite !");
                draw_text_centre_outline(buffer, font, texte_duel, SCREEN_W / 2, 95, makecol(255,255,0), -1);

                sprintf(texte_duel, "J1: %d   J2: %d", joueur.score, joueur2.score);
                draw_text_centre_outline(buffer, font, texte_duel, SCREEN_W / 2, 130, makecol(255,255,255), -1);

                if(gagnant_duel == 1) {
                    draw_player_stun(SCREEN_W / 2 - 40, 165, 0, 1, 0.0f);
                } else if(gagnant_duel == 2) {
                    draw_player2_stun(SCREEN_W / 2 - 40, 165, 0, -1, 0.0f);
                } else {
                    draw_player_stun(SCREEN_W / 2 - 100, 165, 0, 1, 0.0f);
                    draw_player2_stun(SCREEN_W / 2 + 20, 165, 0, -1, 0.0f);
                }

                if(fin_anim_done) {
                    const char *choix[3];
                    choix[0] = (niveau_actuel <= 3) ? "NIVEAU SUIVANT" : "REVENIR AU MENU";
                    choix[1] = "REVENIR AU MENU";
                    choix[2] = "QUITTER PROGRAMME";

                    for(int i = 0; i < 3; i++) {
                        int color = (i == fin_selection)
                            ? makecol(255,255,0)
                            : makecol(200,200,200);
                        draw_text_centre_outline(buffer, font, choix[i],
                                                 SCREEN_W / 2, 340 + i * 42, color, -1);
                    }
                }

                update_display();
            } else {
                draw_menu_fin(victoire, niveau_actuel, joueur.score, victoire ? img_victoire : img_defaite);
            }

            if (fin_anim_done) {

                int up_fin = ihm_touche_appuyee(KEY_UP);
                int down_fin = ihm_touche_appuyee(KEY_DOWN);
                int enter_fin = ihm_touche_appuyee(KEY_ENTER);

                if (up_fin) {
                    int fin_max = mode_duel ? 2 : 3;
                    fin_selection--;
                    if(fin_selection < 0) fin_selection = fin_max;
                }

                if (down_fin) {
                    int fin_max = mode_duel ? 2 : 3;
                    fin_selection++;
                    if(fin_selection > fin_max) fin_selection = 0;
                }

                if (enter_fin) {
                    int choix = fin_selection;

                    if (mode_duel && choix == 2) {
                        quitter_programme = 1;
                    } else if (mode_duel && choix == 1) {
                        fin_selection = 0;
                        ihm_set_etat(ETAT_MENU);
                    } else if (choix == 3) {
                        quitter_programme = 1;
                    } else if (choix == 1) {
                        int dernier_niveau_gagne = niveau_actuel;
                        sauvegarder_partie(&joueur, dernier_niveau_gagne);
                        fin_selection = 0;
                        ihm_set_etat(ETAT_MENU);
                    } else if (choix == 0) {
                        if(victoire && niveau_actuel > 3) {
                            fin_selection = 0;
                            ihm_set_etat(ETAT_MENU);
                        } else {
                            if(!victoire) liberer_niveau(&niveau_struct);
                            initialiser_niveau(&niveau_struct, niveau_actuel);
                            joueur.x = mode_duel ? 40 : SCREEN_W/2;
                            joueur.y = SCREEN_H-170;
                            joueur2.x = SCREEN_W - joueur2.tx - 40;
                            joueur2.y = SCREEN_H-170;
                            if(!victoire) joueur.score = 0;
                            joueur.arme = 0;
                            joueur.buff_tir_timer = 0.0f;
                            joueur.stun_timer = 0.0f;
                            joueur2.arme = 0;
                            joueur2.buff_tir_timer = 0.0f;
                            joueur2.stun_timer = 0.0f;
                            dir = 1;
                            moving = 0;
                            timer_tir_auto = 0.0f;
                            timer_tir_auto_j2 = 0.0f;
                            timer_tir_boss = 0.0f;
                            compte_rebours = 3.0f;
                            nettoyer_effets_niveau();
                            fin_selection = 0;
                            fin_scale = 0.1f;
                            fin_anim_done = 0;
                            ihm_set_etat(ETAT_JEU);
                        }
                    } else {
                        fin_selection = 0;
                        ihm_set_etat(ETAT_MENU);
                    }
                }
            }

            rest(20);
            break;
        }

        default:
            break;
        }

        ihm_maj_etats_precedents();
    }

    destroy_graphics();
    ihm_fermer();
    return 0;
}

END_OF_MAIN();
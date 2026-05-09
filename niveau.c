#include "niveau.h"
#include "interface_graphique.h"

#define BOSS_VITESSE_INITIALE 120.0f
#define BOSS_ACCELERATION 25.0f
#define DELAI_CHARGE_ECLAIR 0.6f
#define BUFF_DUREE_TIR_RAPIDE 5.0f
#define BUFF_PROBA_DROP 5

static float random_unit(void) {
    return (float)rand() / (float)RAND_MAX;
}

static float rayon_joueur(Joueur* joueur) {
    return (joueur->tx < joueur->ty ? joueur->tx : joueur->ty) / 2.0f;
}

void initialiser_niveau(Niveau* niveau, int num_niveau){
    printf("num_niveau = %d\n", num_niveau);

    int tab_tps_niv[]={50, 70, 70, 100};
    int tab_nb_bulles[]={5, 7, 7, 10};

    niveau->projectiles = malloc(sizeof(Projectile) * 20);
    niveau->nb_projectiles = 0;
    niveau->nb_buffs = 10;
    niveau->num_niveau = num_niveau;
    for(int i = 0; i < niveau->nb_buffs; i++){
        niveau->buffs[i].actif = 0;
        niveau->buffs[i].type = 0;
        niveau->buffs[i].vitesse = 120.0f;
    }

    // Bulles
    if(num_niveau < 3){
        niveau->temps_restant = tab_tps_niv[num_niveau];
        niveau->bulles.nb = tab_nb_bulles[num_niveau];

        
        niveau->bulles.capacite = 50;
        niveau->bulles.tab = malloc(sizeof(Bulle) * niveau->bulles.capacite);
        for(int i = 0; i < niveau->bulles.nb; i++){

            niveau->bulles.tab[i].r = 50;
            niveau->bulles.tab[i].x = (float)(niveau->bulles.tab[i].r + rand() % (SCREEN_W - 2 * niveau->bulles.tab[i].r));
            niveau->bulles.tab[i].y = 50;
            if(i % 2 == 0) niveau->bulles.tab[i].vx = 160;
            else niveau->bulles.tab[i].vx = -160;
            niveau->bulles.tab[i].vy = 160;
            niveau->bulles.tab[i].tx = 60;
            niveau->bulles.tab[i].ty = 60;

            if (num_niveau >= 2 && i % 3 == 0) niveau->bulles.tab[i].type = 1;
            else niveau->bulles.tab[i].type = 0;
            niveau->bulles.tab[i].actif = (i == 0) ? 1 : 0;
            niveau->bulles.tab[i].delai_spawn = (i == 0) ? 0.0f : (float)i;
            niveau->bulles.tab[i].nb_splits = 0;
            niveau->bulles.tab[i].charge_eclair = 0;
            niveau->bulles.tab[i].timer_eclair = 0.0f;
        }
        niveau->boss.pv = 0; // pas de boss
    }
  
    // Boss
    else if(num_niveau==3){
        // BOSS LEVEL
        niveau->temps_restant = 120;

        niveau->bulles.nb = 0;
        niveau->bulles.capacite = 50;
        niveau->bulles.tab = malloc(sizeof(Bulle) * 50);

        niveau->boss.x = SCREEN_W/2;
        niveau->boss.y = 50;
        niveau->boss.pv = BOSS_PV_MAX;
        niveau->boss.vitesse = BOSS_VITESSE_INITIALE;
    }

    // Always initialize boss position
    if(num_niveau < 3){
        niveau->boss.x = SCREEN_W/2;
        niveau->boss.y = 50;
        niveau->boss.vitesse = 0;
    }
}
 
static void creer_buff_tir_rapide(Niveau* niveau, float x, float y){
    if(niveau->num_niveau < 1) return;
    if(rand() % 100 >= BUFF_PROBA_DROP) return;

    for(int i = 0; i < niveau->nb_buffs; i++){
        if(!niveau->buffs[i].actif){
            niveau->buffs[i].x = x;
            niveau->buffs[i].y = y;
            niveau->buffs[i].vitesse = 120.0f;
            niveau->buffs[i].type = 0;
            niveau->buffs[i].actif = 1;
            return;
        }
    }
}

static int collision_buff_joueur(Buff* buff, Joueur* joueur){
    float joueur_cx = joueur->x + joueur->tx / 2.0f;
    float joueur_cy = joueur->y + joueur->ty / 2.0f;
    float dx = buff->x - joueur_cx;
    float dy = buff->y - joueur_cy;
    float rayon = rayon_joueur(joueur) + 14.0f;

    return dx * dx + dy * dy <= rayon * rayon;
}

void maj_niveau(Niveau* niveau, Joueur* joueur, float dt){
    if(joueur->buff_tir_timer > 0.0f){
        joueur->buff_tir_timer -= dt;
        if(joueur->buff_tir_timer <= 0.0f){
            joueur->buff_tir_timer = 0.0f;
            joueur->arme = 0;
        }
    }

    // déplacer bulles
    for(int i = 0; i < niveau->bulles.nb; i++){
        if(!niveau->bulles.tab[i].actif){
            niveau->bulles.tab[i].delai_spawn -= dt;
            if(niveau->bulles.tab[i].delai_spawn <= 0.0f){
                niveau->bulles.tab[i].actif = 1;
                niveau->bulles.tab[i].delai_spawn = 0.0f;
            }
        }
        deplacer_bulle(&niveau->bulles.tab[i], dt);
    }

    // déplacer projectiles
    for(int i = 0; i < niveau->nb_projectiles; i++){
        if(niveau->projectiles[i].actif){
            if(niveau->projectiles[i].type == 1){
                niveau->projectiles[i].duree_vie -= dt;
                if(niveau->projectiles[i].delai_activation > 0.0f){
                    niveau->projectiles[i].delai_activation -= dt;
                }
                if(niveau->projectiles[i].duree_vie <= 0.0f){
                    niveau->projectiles[i].actif = 0;
                }
            }
            else niveau->projectiles[i].y -= niveau->projectiles[i].vitesse * dt;
            // suppression si sort de l'écran
            if(niveau->projectiles[i].y < 0 || niveau->projectiles[i].y > SCREEN_H){
                niveau->projectiles[i].actif = 0;
            }
        }
    }

    // collisions projectile / bulle
    for(int i = 0; i < niveau->bulles.nb; i++){
        if(!niveau->bulles.tab[i].actif) continue;
        for(int j = 0; j < niveau->nb_projectiles; j++){
            if(niveau->projectiles[j].actif && niveau->projectiles[j].type == 0 && collision_bulle_projectile(&niveau->bulles.tab[i], &niveau->projectiles[j])){
                float drop_x = niveau->bulles.tab[i].x;
                float drop_y = niveau->bulles.tab[i].y;
                niveau->projectiles[j].actif = 0;
                joueur->score += 5;
                creer_buff_tir_rapide(niveau, drop_x, drop_y);
                diviser_bulle(&niveau->bulles.tab[i], &niveau->bulles);
                printf("Collision bulle !\n");
                break;
            }
        }
        if(niveau->bulles.tab[i].actif == 0){
            niveau->bulles.tab[i] = niveau->bulles.tab[niveau->bulles.nb - 1];
            niveau->bulles.nb--;
            i--;
        }
    }

    // collisions projectile / boss
    if(niveau->boss.pv > 0){
        for(int j = 0; j < niveau->nb_projectiles; j++){
            if(niveau->projectiles[j].actif && niveau->projectiles[j].type == 0 && collision_boss_projectile(&niveau->boss, &niveau->projectiles[j])){
                niveau->projectiles[j].actif = 0;
                niveau->boss.pv--;
                spawn_explosion(niveau->boss.x, niveau->boss.y);
                if(niveau->boss.vitesse < 0) niveau->boss.vitesse -= BOSS_ACCELERATION;
                else niveau->boss.vitesse += BOSS_ACCELERATION;
            }
        }
    }

    // boss
    if(niveau->boss.pv > 0){
        deplacer_boss(&niveau->boss, dt);
    }

    for(int i = 0; i < niveau->nb_buffs; i++){
        if(!niveau->buffs[i].actif) continue;

        niveau->buffs[i].y += niveau->buffs[i].vitesse * dt;

        if(collision_buff_joueur(&niveau->buffs[i], joueur)){
            niveau->buffs[i].actif = 0;
            joueur->arme = 1;
            joueur->buff_tir_timer = BUFF_DUREE_TIR_RAPIDE;
        } else if(niveau->buffs[i].y > SCREEN_H - 80){
            niveau->buffs[i].actif = 0;
        }
    }

    // éclairs des bulles
    for(int i = 0; i < niveau->bulles.nb; i++){
        if(niveau->bulles.tab[i].actif && niveau->bulles.tab[i].type == 1){
            eclair_bulle(&niveau->bulles.tab[i], niveau->projectiles, &niveau->nb_projectiles, dt);
        }
    }

    // temps
    niveau->temps_restant -= dt;
}

void maj_niveau_duel(Niveau* niveau, Joueur* joueur1, Joueur* joueur2, float dt){
    if(joueur1->buff_tir_timer > 0.0f){
        joueur1->buff_tir_timer -= dt;
        if(joueur1->buff_tir_timer <= 0.0f){
            joueur1->buff_tir_timer = 0.0f;
            joueur1->arme = 0;
        }
    }
    if(joueur2->buff_tir_timer > 0.0f){
        joueur2->buff_tir_timer -= dt;
        if(joueur2->buff_tir_timer <= 0.0f){
            joueur2->buff_tir_timer = 0.0f;
            joueur2->arme = 0;
        }
    }

    for(int i = 0; i < niveau->bulles.nb; i++){
        if(!niveau->bulles.tab[i].actif){
            niveau->bulles.tab[i].delai_spawn -= dt;
            if(niveau->bulles.tab[i].delai_spawn <= 0.0f){
                niveau->bulles.tab[i].actif = 1;
                niveau->bulles.tab[i].delai_spawn = 0.0f;
            }
        }
        deplacer_bulle(&niveau->bulles.tab[i], dt);
    }

    for(int i = 0; i < niveau->nb_projectiles; i++){
        if(niveau->projectiles[i].actif){
            if(niveau->projectiles[i].type == 1){
                niveau->projectiles[i].duree_vie -= dt;
                if(niveau->projectiles[i].delai_activation > 0.0f){
                    niveau->projectiles[i].delai_activation -= dt;
                }
                if(niveau->projectiles[i].duree_vie <= 0.0f){
                    niveau->projectiles[i].actif = 0;
                }
            }
            else niveau->projectiles[i].y -= niveau->projectiles[i].vitesse * dt;
            if(niveau->projectiles[i].y < 0 || niveau->projectiles[i].y > SCREEN_H){
                niveau->projectiles[i].actif = 0;
            }
        }
    }

    for(int i = 0; i < niveau->bulles.nb; i++){
        if(!niveau->bulles.tab[i].actif) continue;
        for(int j = 0; j < niveau->nb_projectiles; j++){
            if(niveau->projectiles[j].actif && niveau->projectiles[j].type == 0 && collision_bulle_projectile(&niveau->bulles.tab[i], &niveau->projectiles[j])){
                float drop_x = niveau->bulles.tab[i].x;
                float drop_y = niveau->bulles.tab[i].y;
                int proprietaire = niveau->projectiles[j].proprietaire;
                niveau->projectiles[j].actif = 0;
                if(proprietaire == 2) joueur2->score += 5;
                else joueur1->score += 5;
                creer_buff_tir_rapide(niveau, drop_x, drop_y);
                diviser_bulle(&niveau->bulles.tab[i], &niveau->bulles);
                break;
            }
        }
        if(niveau->bulles.tab[i].actif == 0){
            niveau->bulles.tab[i] = niveau->bulles.tab[niveau->bulles.nb - 1];
            niveau->bulles.nb--;
            i--;
        }
    }

    if(niveau->boss.pv > 0){
        for(int j = 0; j < niveau->nb_projectiles; j++){
            if(niveau->projectiles[j].actif && niveau->projectiles[j].type == 0 && collision_boss_projectile(&niveau->boss, &niveau->projectiles[j])){
                int proprietaire = niveau->projectiles[j].proprietaire;
                niveau->projectiles[j].actif = 0;
                niveau->boss.pv--;
                if(proprietaire == 2) joueur2->score += 5;
                else joueur1->score += 5;
                spawn_explosion(niveau->boss.x, niveau->boss.y);
                if(niveau->boss.vitesse < 0) niveau->boss.vitesse -= BOSS_ACCELERATION;
                else niveau->boss.vitesse += BOSS_ACCELERATION;
            }
        }
        deplacer_boss(&niveau->boss, dt);
    }

    for(int i = 0; i < niveau->nb_buffs; i++){
        if(!niveau->buffs[i].actif) continue;

        niveau->buffs[i].y += niveau->buffs[i].vitesse * dt;

        if(collision_buff_joueur(&niveau->buffs[i], joueur1)){
            niveau->buffs[i].actif = 0;
            joueur1->arme = 1;
            joueur1->buff_tir_timer = BUFF_DUREE_TIR_RAPIDE;
        } else if(collision_buff_joueur(&niveau->buffs[i], joueur2)){
            niveau->buffs[i].actif = 0;
            joueur2->arme = 1;
            joueur2->buff_tir_timer = BUFF_DUREE_TIR_RAPIDE;
        } else if(niveau->buffs[i].y > SCREEN_H - 80){
            niveau->buffs[i].actif = 0;
        }
    }

    for(int i = 0; i < niveau->bulles.nb; i++){
        if(niveau->bulles.tab[i].actif && niveau->bulles.tab[i].type == 1){
            eclair_bulle(&niveau->bulles.tab[i], niveau->projectiles, &niveau->nb_projectiles, dt);
        }
    }

    niveau->temps_restant -= dt;
}

int niveau_termine(Niveau* niveau, Joueur* joueur){
    for(int i = 0; i < niveau->bulles.nb; i++){
        if(!niveau->bulles.tab[i].actif) continue;
        if(collision_bulle_joueur(&niveau->bulles.tab[i], joueur)){
            spawn_explosion(joueur->x, joueur->y);
            return 0; 
        }
    }

    for(int i = 0; i < niveau->nb_projectiles; i++){
        if(niveau->projectiles[i].actif && niveau->projectiles[i].type == 1){
            if(collision_projectile_boss_joueur(&niveau->projectiles[i], joueur)){
                niveau->projectiles[i].actif = 0;
                spawn_explosion(joueur->x + joueur->tx / 2, joueur->y + joueur->ty / 2);
                return 0;
            }
        }
    }

    if (niveau->temps_restant<=0){
        return 0;
    }

    if (niveau->boss.pv > 0){
        return -1;
    }

    if (niveau->bulles.nb==0){
        return 1;
    }

    return -1;
}
 
void deplacer_bulle(Bulle* bulle, float dt){ 
    if(!bulle->actif) return;
    float g = 1.0;
    bulle->vy += g*dt;
    bulle->x += bulle->vx * dt;
    bulle->y += bulle->vy * dt;

    if(bulle->x - bulle->r < 0){
        bulle->x = bulle->r;
        if(bulle->vx < 0) bulle->vx = -bulle->vx;
    }
    else if(bulle->x + bulle->r > SCREEN_W){
        bulle->x = SCREEN_W - bulle->r;
        if(bulle->vx > 0) bulle->vx = -bulle->vx;
    }

    if(bulle->y - bulle->r < 0){
        bulle->y = bulle->r;
        if(bulle->vy < 0) bulle->vy = -bulle->vy * 0.9;
    }
    else if(bulle->y + bulle->r > SCREEN_H-90){
        bulle->y = SCREEN_H - 90 - bulle->r;
        if(bulle->vy > 0) bulle->vy = -bulle->vy * 0.9;
    }

    bulle->xcoin = bulle->x - bulle->r;
    bulle->ycoin = bulle->y - bulle->r;
}

void diviser_bulle(Bulle* bulle, ListeBulles* liste){
    if(bulle->nb_splits >= 2 || bulle->r <= 10){
        bulle->actif = 0;
        spawn_explosion(bulle->x, bulle->y);
        return;
    }
    Bulle nouvelle = *bulle;
    bulle->nb_splits += 1;
    nouvelle.nb_splits = bulle->nb_splits;
    bulle->r *= 0.75;
    nouvelle.r *= 0.75;
    bulle->vx = 160;
    nouvelle.vx = -160;
    bulle->charge_eclair = 0;
    bulle->timer_eclair = 0.0f;
    nouvelle.charge_eclair = 0;
    nouvelle.timer_eclair = 0.0f;
    if(liste->nb < liste->capacite){
        liste->tab[liste->nb] = nouvelle;
        liste->nb++;
    }
}

int collision_bulle_projectile(Bulle* bulle, Projectile* proj){
    float dx=bulle->x - proj->x;
    float dy=bulle->y - proj->y;
    float distance= dx*dx + dy*dy;
    if(distance < (bulle->r)*(bulle->r)){
        return 1;
    }
    return 0;
}

int collision_bulle_joueur(Bulle* bulle, Joueur* joueur){
    // calcule le centre du joueur
    float joueur_cx = joueur->x + joueur->tx / 2.0f;
    float joueur_cy = joueur->y + joueur->ty / 2.0f;
    
    // calcule la distance entre le centre de la bulle et le centre du joueur
    float dx = bulle->x - joueur_cx;
    float dy = bulle->y - joueur_cy;
    float distance = dx*dx + dy*dy;
    
    // somme des rayons
    float player_radius = rayon_joueur(joueur);
    float sum_r = bulle->r + player_radius;
    
    if(distance < sum_r * sum_r){
        return 1;
    }
    return 0;
}

int collision_boss_projectile(Boss* boss, Projectile* proj){
    float dx = boss->x - proj->x;
    float dy = boss->y - proj->y;
    float distance2 = dx*dx + dy*dy;
    float rayon_boss = 40;
    if(distance2 < rayon_boss * rayon_boss){
        return 1;
    }
    return 0;
}

int collision_projectile_boss_joueur(Projectile* proj, Joueur* joueur){
    float joueur_cx = joueur->x + joueur->tx / 2.0f;
    float joueur_cy = joueur->y + joueur->ty / 2.0f;
    float player_radius = rayon_joueur(joueur);

    if(proj->type == 1){
        if(proj->delai_activation > 0.0f) return 0;

        float gauche = proj->x - ECLAIR_HITBOX_LARGEUR / 2.0f;
        float droite = proj->x + ECLAIR_HITBOX_LARGEUR / 2.0f;
        float haut = proj->y;
        float bas = proj->y + ECLAIR_HITBOX_HAUTEUR;
        float closest_x = joueur_cx;
        float closest_y = joueur_cy;

        if(closest_x < gauche) closest_x = gauche;
        if(closest_x > droite) closest_x = droite;
        if(closest_y < haut) closest_y = haut;
        if(closest_y > bas) closest_y = bas;

        float dx = joueur_cx - closest_x;
        float dy = joueur_cy - closest_y;
        float distance = dx*dx + dy*dy;

        if(distance < player_radius * player_radius){
            return 1;
        }
        return 0;
    }

    float dx = proj->x - joueur_cx;
    float dy = proj->y - joueur_cy;
    float distance = dx*dx + dy*dy;
    float sum_r = RAYON_HITBOX_PROJECTILE_BOSS + player_radius;

    if(distance < sum_r * sum_r){
        return 1;
    }
    return 0;
}

void deplacer_boss(Boss* boss, float dt){
    boss->x += boss->vitesse * dt;
    if(boss->x <= 0){
        boss->x = 0;
        if(boss->vitesse < 0) boss->vitesse = -boss->vitesse;
    }
    else if(boss->x >= SCREEN_W){
        boss->x = SCREEN_W;
        if(boss->vitesse > 0) boss->vitesse = -boss->vitesse;
    }
}

int boss_attaque(Boss* boss, ListeBulles* bulles, Projectile* projectiles, int nb_projectiles, Joueur* joueur, float dt, float* timer_tir){
    const float vitesse_projectile = 250.0f;
    const float delai_tir = 0.8f;
    const float frequence_bulle_par_seconde = 0.25f;
    int joueur_touche = 0;

    if(boss->pv <= 0){
        for(int i = 0; i < nb_projectiles; i++){
            projectiles[i].actif = 0;
        }
        *timer_tir = 0.0f;
        return 0;
    }

    for(int i = 0; i < nb_projectiles; i++){
        if(projectiles[i].actif){
            projectiles[i].y += projectiles[i].vitesse * dt;

            if(projectiles[i].y > SCREEN_H){
                projectiles[i].actif = 0;
            }
            else if(collision_projectile_boss_joueur(&projectiles[i], joueur)){
                projectiles[i].actif = 0;
                joueur_touche = 1;
                spawn_explosion(joueur->x + joueur->tx / 2, joueur->y + joueur->ty / 2);
            }
        }
    }

    *timer_tir += dt;
    if(*timer_tir > delai_tir){
        for(int i = 0; i < nb_projectiles; i++){
            if(!projectiles[i].actif){
                projectiles[i].x = boss->x;
                projectiles[i].y = boss->y;
                projectiles[i].vitesse = vitesse_projectile;
                projectiles[i].duree_vie = 0.0f;
                projectiles[i].delai_activation = 0.0f;
                projectiles[i].actif = 1;
                projectiles[i].type = 2;
                *timer_tir = 0.0f;
                break;
            }
        }
    }

    if(random_unit() < frequence_bulle_par_seconde * dt){
        if(bulles->nb < bulles->capacite){
            Bulle b;
            b.x = boss->x;
            b.y = boss->y;
            b.vx = (rand() % 2 == 0) ? -160 : 160;
            b.vy = 160;
            b.r = 50;
            b.tx = 60;
            b.ty = 60;
            b.type = 1;
            b.actif = 1;
            b.delai_spawn = 0.0f;
            b.nb_splits = 0;
            b.charge_eclair = 0;
            b.timer_eclair = 0.0f;
            bulles->tab[bulles->nb] = b;
            bulles->nb++;
        }
    }

    return joueur_touche;
}

void eclair_bulle(Bulle* bulle, Projectile* projectiles, int* nb_projectiles, float dt){
    const float frequence_par_seconde = 0.25f; /* ~1 fois toutes les 2s par bulle */

    if(!bulle->charge_eclair){
        if(random_unit() < frequence_par_seconde * dt){
            bulle->charge_eclair = 1;
            bulle->timer_eclair = DELAI_CHARGE_ECLAIR;
        }
        return;
    }

    bulle->timer_eclair -= dt;
    if(bulle->timer_eclair <= 0.0f){
        int index = -1;

        for(int i = 0; i < *nb_projectiles; i++){
            if(!projectiles[i].actif){
                index = i;
                break;
            }
        }

        if(index == -1 && *nb_projectiles < 20){
            index = *nb_projectiles;
            (*nb_projectiles)++;
        }

        bulle->charge_eclair = 0;
        bulle->timer_eclair = 0.0f;
        if(index == -1) return;

        projectiles[index].x = bulle->x;
        projectiles[index].y = bulle->y + bulle->r;
        projectiles[index].vitesse = 0;
        projectiles[index].duree_vie = ECLAIR_DUREE_VIE;
        projectiles[index].delai_activation = ECLAIR_DELAI_HITBOX;
        projectiles[index].actif = 1;
        projectiles[index].type = 1;
        spawn_eclair(bulle->x, bulle->y);
    }
}

void liberer_niveau(Niveau* niveau){
    free(niveau->bulles.tab);
    free(niveau->projectiles);
}

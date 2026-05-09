#include "niveau.h"
#include "interface_graphique.h"

static float random_unit(void) {
    return (float)rand() / (float)RAND_MAX;
}

void initialiser_niveau(Niveau* niveau, int num_niveau){
    printf("num_niveau = %d\n", num_niveau);

    int tab_tps_niv[]={100, 5, 5, 5};
    int tab_nb_bulles[]={5, 10, 15, 20};

    niveau->projectiles = malloc(sizeof(Projectile) * 20);
    niveau->nb_projectiles = 0;

    // Bulles
    if(num_niveau < 3){
        niveau->temps_restant = tab_tps_niv[num_niveau];
        niveau->bulles.nb = tab_nb_bulles[num_niveau];
        niveau->bulles.capacite = 50;
        niveau->bulles.tab = malloc(sizeof(Bulle) * niveau->bulles.capacite);
        for(int i = 0; i < niveau->bulles.nb; i++){
            niveau->bulles.tab[i].x = SCREEN_W/2;
            niveau->bulles.tab[i].y = 100;
            if(i % 2 == 0) niveau->bulles.tab[i].vx = 200;
            else niveau->bulles.tab[i].vx = -200;
            niveau->bulles.tab[i].vy = 200;
            niveau->bulles.tab[i].r = 50;
            niveau->bulles.tab[i].tx = 60;
            niveau->bulles.tab[i].ty = 60;
            if (num_niveau >= 2 && i % 3 == 0) niveau->bulles.tab[i].type = 1;
            else niveau->bulles.tab[i].type = 0;
            niveau->bulles.tab[i].actif = 1;
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
        niveau->boss.pv = 6;
        niveau->boss.vitesse = 2;
    }

    // Always initialize boss position
    if(num_niveau < 3){
        niveau->boss.x = SCREEN_W/2;
        niveau->boss.y = 50;
        niveau->boss.vitesse = 2;
    }
}
 
void maj_niveau(Niveau* niveau, Joueur* joueur, float dt){
    // déplacer bulles
    for(int i = 0; i < niveau->bulles.nb; i++){
        deplacer_bulle(&niveau->bulles.tab[i], dt);
    }

    // déplacer projectiles
    for(int i = 0; i < niveau->nb_projectiles; i++){
        if(niveau->projectiles[i].actif){
            niveau->projectiles[i].y -= niveau->projectiles[i].vitesse * dt;
            // suppression si sort de l'écran
            if(niveau->projectiles[i].y < 0){
                niveau->projectiles[i].actif = 0;
            }
        }
    }

    // collisions projectile / bulle
    for(int i = 0; i < niveau->bulles.nb; i++){
        if(!niveau->bulles.tab[i].actif) continue;
        for(int j = 0; j < niveau->nb_projectiles; j++){
            if(niveau->projectiles[j].actif && collision_bulle_projectile(&niveau->bulles.tab[i], &niveau->projectiles[j])){
                niveau->projectiles[j].actif = 0;
                diviser_bulle(&niveau->bulles.tab[i], &niveau->bulles);
                printf("Collision bulle !\n");
            }
            if(niveau->projectiles[j].actif && collision_boss_projectile(&niveau->boss, &niveau->projectiles[j])){
                niveau->projectiles[j].actif = 0;
                niveau->boss.pv--;
                spawn_explosion(niveau->boss.x, niveau->boss.y);
                niveau->boss.vitesse += 0.2;
            }
        }
        if(niveau->bulles.tab[i].actif == 0){
            niveau->bulles.tab[i] = niveau->bulles.tab[niveau->bulles.nb - 1];
            niveau->bulles.nb--;
            i--;
        }
    }

    // boss
    if(niveau->boss.pv > 0){
        deplacer_boss(&niveau->boss, dt);
        boss_attaque(&niveau->boss, &niveau->bulles, dt);
    }

    // éclairs des bulles
    for(int i = 0; i < niveau->bulles.nb; i++){
        if(niveau->bulles.tab[i].type == 1){
            eclair_bulle(&niveau->bulles.tab[i], niveau->projectiles, &niveau->nb_projectiles, dt);
        }
    }

    // temps
    niveau->temps_restant -= dt;
}

int niveau_termine(Niveau* niveau, Joueur* joueur){
    for(int i = 0; i < niveau->bulles.nb; i++){
        if(collision_bulle_joueur(&niveau->bulles.tab[i], joueur)){
            spawn_explosion(joueur->x, joueur->y);
            return 0; 
        }
    }
    if (niveau->bulles.nb==0){
        return 1;
    }
    if (niveau->temps_restant<=0){
        return 0;
    }
    if (niveau->boss.pv<=0 && niveau->bulles.nb==0){
        return 1;
    }
    return -1;
}
 
void deplacer_bulle(Bulle* bulle, float dt){ 
    float g = 1.0;
    bulle->vy += g*dt;
    bulle->x += bulle->vx * dt;
    bulle->y += bulle->vy * dt;
    if(bulle->x - bulle->r <= 0 || bulle->x + bulle->r >= SCREEN_W) bulle->vx = -bulle->vx;
    if(bulle->y - bulle->r <= 0 || bulle->y + bulle->r >= SCREEN_H-170) bulle->vy = -bulle->vy * 0.9;
    bulle->xcoin = bulle->x - bulle->r;
    bulle->ycoin = bulle->y - bulle->r;
}

void diviser_bulle(Bulle* bulle, ListeBulles* liste){
    if(bulle->r <= 10){
        bulle->actif = 0;
        spawn_explosion(bulle->x, bulle->y);
        return;
    }
    Bulle nouvelle = *bulle;
    bulle->r /= 2;
    nouvelle.r /= 2;
    bulle->vx = 2;
    nouvelle.vx = -2;
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
    // Calculate player center
    float joueur_cx = joueur->x + joueur->tx / 2.0f;
    float joueur_cy = joueur->y + joueur->ty / 2.0f;
    
    // Calculate distance between bubble center and player center
    float dx = bulle->x - joueur_cx;
    float dy = bulle->y - joueur_cy;
    float distance = dx*dx + dy*dy;
    
    // Sum of radii (bubble radius + estimated player radius)
    float player_radius = (joueur->tx + joueur->ty) / 4.0f;
    float sum_r = bulle->r + player_radius;
    
    if(distance < sum_r * sum_r){
        return 1;
    }
    return 0;
}

int collision_bulle_bulle(Bulle* bulle1, Bulle* bulle2){
    float dx = bulle1->x - bulle2->x;
    float dy = bulle1->y - bulle2->y;
    float distance2 = dx*dx + dy*dy;
    float somme_r = bulle1->r + bulle2->r;
    if(distance2 < somme_r * somme_r){
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

void deplacer_boss(Boss* boss, float dt){
    boss->x += boss->vitesse * dt;
    if(boss->x <= 0 || boss->x >= SCREEN_W){
        boss->vitesse = -boss->vitesse;
    }
}

void boss_attaque(Boss* boss, ListeBulles* bulles, float dt){
    const float frequence_par_seconde = 0.75f; /* ~1 fois toutes les 1.33s */
    if(random_unit() < frequence_par_seconde * dt){
        if(bulles->nb < bulles->capacite){
            Bulle b;
            b.x = boss->x;
            b.y = boss->y;
            b.vx = (rand() % 2 == 0) ? -2 : 2;
            b.vy = 2;
            b.r = 20;
            b.tx = 40;
            b.ty = 40;
            b.type = 1; // bulles dangereuses (éclair possible)
            bulles->tab[bulles->nb] = b;
            bulles->nb++;
        }
    }
}

void eclair_bulle(Bulle* bulle, Projectile* projectiles, int* nb_projectiles, float dt){
    if(*nb_projectiles < 20){
        const float frequence_par_seconde = 0.5f; /* ~1 fois toutes les 2s par bulle */
        if(random_unit() < frequence_par_seconde * dt){
            projectiles[*nb_projectiles].x = bulle->x;
            projectiles[*nb_projectiles].y = bulle->y;
            projectiles[*nb_projectiles].vitesse = 5;
            projectiles[*nb_projectiles].actif = 1;
            (*nb_projectiles)++;
            spawn_eclair(bulle->x, bulle->y);
        }
    }
}

void liberer_niveau(Niveau* niveau){
    free(niveau->bulles.tab);
    free(niveau->projectiles);
}

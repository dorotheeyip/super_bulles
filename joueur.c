#include "joueur.h"

void initialiser_joueur(Joueur* joueur, char* pseudo){ // x,y et tx ty a maj
    strcpy(joueur->pseudo, pseudo);
    joueur->x=SCREEN_W/2;
    joueur->y=SCREEN_H-170;
    joueur->tx=80;
    joueur->ty=80;
    joueur->score=0;
    joueur->arme=0;
    joueur->buff_tir_timer=0.0f;
    joueur->stun_timer=0.0f;
}

void deplacer_joueur(Joueur* joueur, int direction, float dt){
    const float vitesse = 300.0f; /* pixels/seconde */
    const float deplacement = vitesse * dt;

    if(direction==0){
        joueur->x -= deplacement;
    }
    if(direction==1){
        joueur->x += deplacement;
    }
}

Projectile tirer(Joueur* joueur){
    Projectile proj;
    proj.x=joueur->x + joueur->tx / 2.0f;
    proj.y=joueur->y;
    proj.actif=1;
    proj.type=0;
    proj.proprietaire=1;
    proj.duree_vie=0.0f;
    proj.delai_activation=0.0f;
    if(joueur->arme==0){
        proj.vitesse=400;
    }
    else {
        proj.vitesse=800;
    }
    return proj;
}

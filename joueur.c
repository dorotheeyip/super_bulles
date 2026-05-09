#include "joueur.h"

void initialiser_joueur(Joueur* joueur, char* pseudo){ // x,y et tx ty a maj
    strcpy(joueur->pseudo, pseudo);
    joueur->x=SCREEN_W/2;
    joueur->y=SCREEN_H-170;
    joueur->tx=40;
    joueur->ty=40;
    joueur->score=0;
    joueur->arme=0;
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
    proj.x=joueur->x;
    proj.y=joueur->y;
    proj.actif=1;
    if(joueur->arme==0){
        proj.vitesse=5;
    }
    else {
        proj.vitesse=5;
    }
    return proj;
}
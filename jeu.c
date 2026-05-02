#include "jeu.h"

void lancer_partie(char* pseudo){
    Joueur joueur;
    initialiser_joueur(&joueur, pseudo);
    int niveau_courant = 0;
    int resultat = -1;
    while(niveau_courant <= 3 && resultat != 0){
        resultat = lancer_niveau(niveau_courant, &joueur);
        fin_niveau(resultat, &joueur);
        if(resultat == 1){
            niveau_courant++;
        }
    }
    if(niveau_courant == 4){
        printf("Partie terminée !\n");
    }
}

int lancer_niveau(int num_niveau, Joueur* joueur){
    printf("Debut niveau %d\n", num_niveau);
    Niveau niveau;
    initialiser_niveau(&niveau, num_niveau);
    int jeu_en_cours = 1;
    clock_t last = clock();
    float timer_tir = 0;
    while(jeu_en_cours){
        clock_t now = clock();
        float dt = (float)(now - last) / CLOCKS_PER_SEC;
        last = now;
        maj_niveau(&niveau, joueur, dt);

        timer_tir += dt;
        if(timer_tir > 0.5 && niveau.nb_projectiles < 20){ // tir toutes les 0.5s
            niveau.projectiles[niveau.nb_projectiles] = tirer(joueur);
            niveau.nb_projectiles++;
            timer_tir = 0;
        }

        int res = niveau_termine(&niveau, joueur);
        if(res == 1 || res == 0){
            jeu_en_cours = 0;
        }
        printf("Temps restant: %.2f | Bulles: %d | Projectiles: %d\n", niveau.temps_restant, niveau.bulles.nb, niveau.nb_projectiles);
    }
    int resultat = niveau_termine(&niveau, joueur);
    liberer_niveau(&niveau);
    return resultat;
}


void fin_niveau(int resultat, Joueur* joueur){
    if(resultat == 1){
        joueur->score += 100;
        printf("Niveau réussi !\n");
    }
    else if(resultat == 0){
        printf("Perdu...\n");
    }
}

void sauvegarder_partie(Joueur* joueur, int niveau) {
    FILE *pf = fopen("sauvegarde.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!temp) return;
    char pseudo_fichier[50];
    int niveau_fichier;
    int trouve = 0;
    if (pf) {
        while (fscanf(pf, "%s %d", pseudo_fichier, &niveau_fichier) == 2) {
            
            if (strcmp(pseudo_fichier, joueur->pseudo) == 0) {
                fprintf(temp, "%s %d\n", joueur->pseudo, niveau);
                trouve = 1;
            } else {
                fprintf(temp, "%s %d\n", pseudo_fichier, niveau_fichier);
            }
        }
        fclose(pf);
    }
    if (!trouve) {
        fprintf(temp, "%s %d\n", joueur->pseudo, niveau);
    }
    fclose(temp);
    remove("sauvegarde.txt");
    rename("temp.txt", "sauvegarde.txt");
}

int charger_partie(char* pseudo, Joueur* joueur) {
    FILE *pf = fopen("sauvegarde.txt", "r");
    if (!pf) return -1;
    char pseudo_fichier[50];
    int niveau;
    while (fscanf(pf, "%s %d", pseudo_fichier, &niveau) == 2) {
        
        if (strcmp(pseudo_fichier, pseudo) == 0) {
            strcpy(joueur->pseudo, pseudo);
            fclose(pf);
            return niveau;
        }
    }
    fclose(pf);
    return -1; 
}
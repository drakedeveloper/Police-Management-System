#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"

typedef struct Plainte {
    int id;
    char cin[9];
    char prenom[50];
    char nom[50];
    char dateNaissance[20];
    char plainte[255];
    char date[20];
    int priorite;
    char cinSujet[9];
    char nomSujet[50];
    struct Plainte *suivante;
} Plainte;

typedef struct CasierJudiciaire {
    int id;
    char cin[9];
    char prenom[50];
    char nom[50];
    char crime[255];
    char dateNaissance[20];
    struct CasierJudiciaire *suivant;
} CasierJudiciaire;

typedef struct Utilisateur {
    char nomUtilisateur[50];
    char motDePasse[50];
} Utilisateur;

Plainte *debutFile = NULL, *finFile = NULL;
Plainte *sommetPile = NULL;
CasierJudiciaire *debutCasier = NULL;

int prochainId = 1;
int plaintesEnAttente = 0;
int plaintesResolues = 0;
int nombreCasiers = 0;

#define MAX_UTILISATEURS 10
Utilisateur utilisateurs[MAX_UTILISATEURS] = {
    {"admin", "admin123"},
    {"policier", "police456"}
};

int verifierAuthentification(char *nomUtilisateur, char *motDePasse) {
    for (int i = 0; i < MAX_UTILISATEURS; i++) {
        if (strcmp(utilisateurs[i].nomUtilisateur, nomUtilisateur) == 0 &&
            strcmp(utilisateurs[i].motDePasse, motDePasse) == 0) {
            return 1;
        }
    }
    return 0;
}

int verifierCIN(char *cin) {
    if (strlen(cin) != 8) return 0;
    for (int i = 0; i < 8; i++) {
        if (cin[i] < '0' || cin[i] > '9') return 0;
    }
    return 1;
}
void sauvegarderPlaintes() {
    FILE *f = fopen("plaintes.txt", "w");
    if (!f) {
        printf("\033[31mErreur lors de l'ouverture du fichier plaintes.txt pour l'ecriture.\033[0m\n");
        return;
    }
    Plainte *temp = debutFile;
    while (temp) {
        fprintf(f, "%d|%s|%s|%s|%s|%s|%s|%d|%s|%s\n",
                temp->id, temp->cin, temp->prenom, temp->nom, temp->dateNaissance,
                temp->plainte, temp->date, temp->priorite, temp->cinSujet, temp->nomSujet);
        temp = temp->suivante;
    }
    fclose(f);
    printf("\033[32mPlaintes sauvegardees avec succes.\033[0m\n");
}
void sauvegarderCasiers() {
    FILE *f = fopen("dossiers.txt", "w");
    if (!f) {
        printf("\033[31mErreur lors de l'ouverture du fichier dossiers.txt pour l'ecriture.\033[0m\n");
        return;
    }
    CasierJudiciaire *temp = debutCasier;
    while (temp) {
        fprintf(f, "%d|%s|%s|%s|%s|%s\n",
                temp->id, temp->cin, temp->prenom, temp->nom, temp->crime, temp->dateNaissance);
        temp = temp->suivant;
    }
    fclose(f);
    printf("\033[32mDossiers criminels sauvegardes avec succes.\033[0m\n");
}
void chargerPlaintes() {
    FILE *f = fopen("plaintes.txt", "r");
    if (!f) {
        printf("\033[33mAucun fichier plaintes.txt trouvé. Creation d'un nouveaux ficher.\033[0m\n");
        return;
    }
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), f)) {
        Plainte *nouvellePlainte = (Plainte *)malloc(sizeof(Plainte));
        sscanf(buffer, "%d|%8s|%49[^|]|%49[^|]|%19[^|]|%254[^|]|%19[^|]|%d|%8[^|]|%49[^\n]",
               &nouvellePlainte->id, nouvellePlainte->cin, nouvellePlainte->prenom, nouvellePlainte->nom,
               nouvellePlainte->dateNaissance, nouvellePlainte->plainte, nouvellePlainte->date,
               &nouvellePlainte->priorite, nouvellePlainte->cinSujet, nouvellePlainte->nomSujet);
        nouvellePlainte->suivante = NULL;

        if (!finFile) {
            debutFile = finFile = nouvellePlainte;
        } else {
            finFile->suivante = nouvellePlainte;
            finFile = nouvellePlainte;
        }
        plaintesEnAttente++;
        if (nouvellePlainte->id >= prochainId) {
            prochainId = nouvellePlainte->id + 1;
        }
    }
    fclose(f);
    printf("\033[32mPlaintes chargees avec succes.\033[0m\n");
}


void chargerCasiers() {
    FILE *f = fopen("dossiers.txt", "r");
    if (!f) {
        printf("\033[33mAucun fichier dossiers.txt trouve. Creation d'un nouveaux ficher.\033[0m\n");
        return;
    }
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), f)) {
        CasierJudiciaire *nouveauCasier = (CasierJudiciaire *)malloc(sizeof(CasierJudiciaire));
        sscanf(buffer, "%d|%8s|%49[^|]|%49[^|]|%254[^|]|%19[^\n]",
               &nouveauCasier->id, nouveauCasier->cin, nouveauCasier->prenom,
               nouveauCasier->nom, nouveauCasier->crime, nouveauCasier->dateNaissance);
        nouveauCasier->suivant = debutCasier;
        debutCasier = nouveauCasier;
        nombreCasiers++;
        if (nouveauCasier->id >= prochainId) {
            prochainId = nouveauCasier->id + 1;
        }
    }
    fclose(f);
    printf("\033[32mDossiers criminels charges avec succes.\033[0m\n");
}


void ajouterPlainte(char *cin, char *prenom, char *nom, char *dateNaissance, char *textePlainte, char *date, int priorite, char *cinSujet, char *nomSujet) {
    if (!verifierCIN(cin)) {
        printf(RED "CIN invalide. Il doit contenir exactement 8 chiffres.\n" RESET);
        return;
    }
    Plainte *nouvellePlainte = (Plainte *)malloc(sizeof(Plainte));
    nouvellePlainte->id = prochainId++;
    strcpy(nouvellePlainte->cin, cin);
    strcpy(nouvellePlainte->prenom, prenom);
    strcpy(nouvellePlainte->nom, nom);
    strcpy(nouvellePlainte->dateNaissance, dateNaissance);
    strcpy(nouvellePlainte->plainte, textePlainte);
    strcpy(nouvellePlainte->date, date);
    strcpy(nouvellePlainte->cinSujet, cinSujet);
    strcpy(nouvellePlainte->nomSujet, nomSujet);
    nouvellePlainte->priorite = priorite;
    nouvellePlainte->suivante = NULL;

    if (finFile == NULL) {
        debutFile = finFile = nouvellePlainte;
    } else {
        finFile->suivante = nouvellePlainte;
        finFile = nouvellePlainte;
    }

    plaintesEnAttente++;
    if (priorite == 1) {
        printf(GREEN "Notification : Plainte urgente ajoutee.\n" RESET);
    }
    sauvegarderPlaintes();
    printf(GREEN "Plainte ajoutee avec succes ! ID : %d\n" RESET, nouvellePlainte->id);
}

void afficherFile() {
    if (debutFile == NULL) {
        printf(RED "Aucune plainte en attente.\n" RESET);
        return;
    }
    Plainte *temp = debutFile;
    printf(BLUE "--- Plaintes en Attente ---\n" RESET);
    while (temp != NULL) {
        printf("ID : %d | CIN : %s | Nom : %s %s | Plainte : %s | Date : %s | Priorite : %d\n",
               temp->id, temp->cin, temp->prenom, temp->nom, temp->plainte, temp->date, temp->priorite);
        if (strlen(temp->cinSujet) > 0) {
            printf("CIN de l'accuse : %s | Nom de l'accuse : %s\n", temp->cinSujet, temp->nomSujet);
        }
        temp = temp->suivante;
    }
}

void resoudrePlainte() {
    if (debutFile == NULL) {
        printf(RED "Aucune plainte a resoudre.\n" RESET);
        return;
    }

    Plainte *plainteResolue = debutFile;
    debutFile = debutFile->suivante;
    if (debutFile == NULL) {
        finFile = NULL;
    }

    plainteResolue->suivante = sommetPile;
    sommetPile = plainteResolue;

    plaintesResolues++;
    plaintesEnAttente--;

    // Remove resolved complaint from file
    FILE *f = fopen("plaintes.txt", "r");
    if (!f) {
        printf(RED "Erreur lors de l'ouverture du fichier plaintes.txt pour la lecture.\n" RESET);
        return;
    }

    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        printf(RED "Erreur lors de la creation d'un fichier temporaire.\n" RESET);
        fclose(f);
        return;
    }

    char buffer[512];
    while (fgets(buffer, sizeof(buffer), f)) {
        int id;
        sscanf(buffer, "%d|", &id);
        if (id != plainteResolue->id) {
            fputs(buffer, temp);
        }
    }

    fclose(f);
    fclose(temp);
    remove("plaintes.txt");
    rename("temp.txt", "plaintes.txt");

    printf(GREEN "Plainte ID %d resolue.\n" RESET, plainteResolue->id);
}


void afficherPile() {
    if (sommetPile == NULL) {
        printf(RED "Aucune plainte resolue.\n" RESET);
        return;
    }

    Plainte *temp = sommetPile;
    printf(BLUE "--- Plaintes Resolues ---\n" RESET);
    while (temp != NULL) {
        printf("ID : %d | CIN : %s | Nom : %s %s | Plainte : %s | Date : %s | Priorite : %d\n",
               temp->id, temp->cin, temp->prenom, temp->nom, temp->plainte, temp->date, temp->priorite);
        temp = temp->suivante;
    }
}

void ajouterCasier(char *cin, char *prenom, char *nom, char *dateNaissance, char *crime) {
    if (!verifierCIN(cin)) {
        printf(RED "CIN invalide. Il doit contenir exactement 8 chiffres.\n" RESET);
        return;
    }
    CasierJudiciaire *nouveauCasier = (CasierJudiciaire *)malloc(sizeof(CasierJudiciaire));
    nouveauCasier->id = prochainId++;
    strcpy(nouveauCasier->cin, cin);
    strcpy(nouveauCasier->prenom, prenom);
    strcpy(nouveauCasier->nom, nom);
    strcpy(nouveauCasier->dateNaissance, dateNaissance);
    strcpy(nouveauCasier->crime, crime);
    nouveauCasier->suivant = debutCasier;
    debutCasier = nouveauCasier;

    nombreCasiers++;
    sauvegarderCasiers();
    printf(GREEN "Dossier criminel ajoute avec succes ! ID : %d\n" RESET, nouveauCasier->id);
}

void afficherCasiers() {
    if (debutCasier == NULL) {
        printf(RED "Aucun dossier criminel.\n" RESET);
        return;
    }
    CasierJudiciaire *temp = debutCasier;
    printf(BLUE "--- Dossiers Criminels ---\n" RESET);
    while (temp != NULL) {
        printf("ID : %d | CIN : %s | Prenom : %s | Nom : %s | Crime : %s | Date de Naissance : %s\n",
               temp->id, temp->cin, temp->prenom, temp->nom, temp->crime, temp->dateNaissance);
        temp = temp->suivant;
    }
}

void verifierCasier(char *cin) {
    if (!verifierCIN(cin)) {
        printf(RED "CIN invalide. Il doit contenir exactement 8 chiffres.\n" RESET);
        return;
    }

    CasierJudiciaire *temp = debutCasier;
    int trouve = 0;
    printf(CYAN "--- Verification des Dossiers Criminels ---\n" RESET);
    while (temp != NULL) {
        if (strcmp(temp->cin, cin) == 0) {
            printf(GREEN "Dossier Trouve : ID : %d | Prenom : %s | Nom : %s | Crime : %s\n" RESET,
                   temp->id, temp->prenom, temp->nom, temp->crime);
            trouve = 1;
        }
        temp = temp->suivant;
    }

    Plainte *plainte = debutFile;
    while (plainte != NULL) {
        if (strcmp(plainte->cinSujet, cin) == 0) {
            printf(YELLOW "Plainte Contre : ID : %d | Nom : %s %s\n" RESET,
                   plainte->id, plainte->prenom, plainte->nom);
            trouve = 1;
        }
        plainte = plainte->suivante;
    }

    if (!trouve) {
        printf(RED "Aucun dossier trouve pour le CIN : %s\n" RESET, cin);
    }
}

void menu() {
    printf(MAGENTA "--- Menu Principal ---\n" RESET);
    printf("1. Ajouter une plainte\n");
    printf("2. Voir les plaintes en attente\n");
    printf("3. Resoudre une plainte\n");
    printf("4. Voir les plaintes resolues\n");
    printf("5. Ajouter un dossier criminel\n");
    printf("6. Voir les dossier criminels\n");
    printf("7. Verifier un dossier par CIN\n");
    printf("8. Quitter\n");
    printf("Choix : ");
}

int main() {
    char nomUtilisateur[50], motDePasse[50], cin[9], prenom[50], nom[50], dateNaissance[20], plainte[255], date[20], cinSujet[9], nomSujet[50];
    int choix, priorite;
    chargerPlaintes();
    chargerCasiers();
    printf(MAGENTA "Bienvenue ! Veuillez vous authentifier.\n" RESET);
    printf("Nom d'utilisateur : ");
    fgets(nomUtilisateur, sizeof(nomUtilisateur), stdin);
    nomUtilisateur[strcspn(nomUtilisateur, "\n")] = 0;
    printf("Mot de passe : ");
    fgets(motDePasse, sizeof(motDePasse), stdin);
    motDePasse[strcspn(motDePasse, "\n")] = 0;
    while (!verifierAuthentification(nomUtilisateur, motDePasse)) {
        printf(RED "Authentification echouee. Reessayez\n" RESET);
        printf("Nom d'utilisateur : ");
        fgets(nomUtilisateur, sizeof(nomUtilisateur), stdin);
        nomUtilisateur[strcspn(nomUtilisateur, "\n")] = 0;
        printf("Mot de passe : ");
        fgets(motDePasse, sizeof(motDePasse), stdin);
        motDePasse[strcspn(motDePasse, "\n")] = 0;
    }

    while (1) {
        menu();
        scanf("%d", &choix);
        getchar();

        switch (choix) {
            case 1:
                printf("CIN : ");
                fgets(cin, sizeof(cin), stdin);
                cin[strcspn(cin, "\n")] = 0;
                printf("Prenom : ");
                fgets(prenom, sizeof(prenom), stdin);
                prenom[strcspn(prenom, "\n")] = 0;
                printf("Nom : ");
                fgets(nom, sizeof(nom), stdin);
                nom[strcspn(nom, "\n")] = 0;
                printf("Date de Naissance (jj/mm/aaaa) : ");
                fgets(dateNaissance, sizeof(dateNaissance), stdin);
                dateNaissance[strcspn(dateNaissance, "\n")] = 0;
                printf("Plainte : ");
                fgets(plainte, sizeof(plainte), stdin);
                plainte[strcspn(plainte, "\n")] = 0;
                printf("Date (jj/mm/aaaa) : ");
                fgets(date, sizeof(date), stdin);
                date[strcspn(date, "\n")] = 0;
                printf("Est-ce contre quelqu'un ? (1 pour Oui, 0 pour Non) : ");
                scanf("%d", &priorite);
                getchar();
                if (priorite == 1) {
                    printf("CIN de l'accuse : ");
                    fgets(cinSujet, sizeof(cinSujet), stdin);
                    cinSujet[strcspn(cinSujet, "\n")] = 0;
                    printf("Nom de l'accuse : ");
                    fgets(nomSujet, sizeof(nomSujet), stdin);
                    nomSujet[strcspn(nomSujet, "\n")] = 0;
                } else {
                    strcpy(cinSujet, "");
                    strcpy(nomSujet, "");
                }
                priorite = priorite ? 1 : 0;
                ajouterPlainte(cin, prenom, nom, dateNaissance, plainte, date, priorite, cinSujet, nomSujet);
                break;
            case 2:
                afficherFile();
                break;
            case 3:
                resoudrePlainte();
                break;
            case 4:
                afficherPile();
                break;
            case 5:
                printf("CIN : ");
                fgets(cin, sizeof(cin), stdin);
                cin[strcspn(cin, "\n")] = 0;
                printf("Prenom : ");
                fgets(prenom, sizeof(prenom), stdin);
                prenom[strcspn(prenom, "\n")] = 0;
                printf("Nom : ");
                fgets(nom, sizeof(nom), stdin);
                nom[strcspn(nom, "\n")] = 0;
                printf("Date de Naissance (jj/mm/aaaa) : ");
                fgets(dateNaissance, sizeof(dateNaissance), stdin);
                dateNaissance[strcspn(dateNaissance, "\n")] = 0;
                printf("Crime : ");
                fgets(plainte, sizeof(plainte), stdin);
                plainte[strcspn(plainte, "\n")] = 0;
                ajouterCasier(cin, prenom, nom, dateNaissance, plainte);
                break;
            case 6:
                afficherCasiers();
                break;
            case 7:
                printf("Entrez le CIN : ");
                fgets(cin, sizeof(cin), stdin);
                cin[strcspn(cin, "\n")] = 0;
                verifierCasier(cin);
                break;
            case 8:
                printf(YELLOW "Enregistrement des donnees avant de quitter...\n" RESET);
                sauvegarderPlaintes();
                sauvegarderCasiers();
                printf(GREEN "Donnees sauvegardees. Au revoir !\n" RESET);                exit(0);
            default:
                printf(RED "Choix invalide. Reessayez.\n" RESET);
        }
    }

    return 0;
}

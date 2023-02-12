/***   Gestion du temps   ****/

// bibliotheque de convertion de timestamp
#include <DTime.h>

//déclaration de la class dtime
DTime dtime;

// definition des variables de temps
uint32_t timestamp;         // stockage de la timespamp
uint32_t last_millis;       // stockage de millis a un instant T
uint32_t diff_millis;       // temps ecoulé entre 2 enregistrement
int delta_millis = 1000;    // intervalle entre 2 enregistrements soit 1 seconde
int debug_millis = 0;       // temps ecoulé entre 2 enregistrement

void maj_time(){
    // si millis() est revenue a zéro (49.7 jours)
    if(last_millis > millis()){
        diff_millis = (4294967295 - last_millis)+millis();  // on calcul le temps ecoulé
    }
    else{
        diff_millis = millis()-last_millis;
    }
    // si delta_millis est écoulé
    if(diff_millis >= delta_millis){
        last_millis = millis();         // on rafraichie last_millis
        timestamp += diff_millis/1000;  // on ajoute diff_millis en seconde à la timestamp 
        if(DEBUG){
            if (debug_millis >= 60 ){
                dtime.setTimestamp(timestamp);
                Serial.printf("Nous somme le %d/%d/%d il est %d:%d\n",dtime.day,dtime.month,dtime.year,dtime.hour,dtime.minute);
                debug_millis = 0;
            }
            else{
                debug_millis++;
            }
        }
    }
}

/*****************************/

/***   Gestion des relais   ***/

// definition des variables des relais
int pin_relay[4] = {21,19,18,5};    // definition des pin
int mode_relay[4] = {};      // 0 = arret forcé, 1 = marche forcé, 2 = auto
String mode_relay_txt[3] = {"ARRET","MARCHE","AUTO"};// 0 = arret forcé, 1 = marche forcé, 2 = auto
int etat_relay[4] = {};    // 0 = arret, 1 = marche
String cdm_relay[4] = {"BTN_ETAT_P_FILT","BTN_ETAT_P_CHAUD","BTN_ETAT_ECL"};    // nom du bouton de la page html


// fonction d'initialisation des relais 
void init_relay(){
    for(int i=0;i<4;i++){
        pinMode(pin_relay[i], OUTPUT);
        delay(10);
        digitalWrite(pin_relay[i], 0);
    }
}

// on cherche l'index du relais par rapport au texte
int num_relay(String relay){
    int result;
    // on commence par trouver l'index du tableau
    for(int i=0;i<4;i++){
        if (cdm_relay[i] == relay){
            result =  i;
            break;
        }
    }
    return result;
}
// inversion de l'état du relay
void toggle_relay(int index){
    etat_relay[index] = 1-etat_relay[index];    // on inverse l'état du relay 1-0=1, 1-1=0
}

// Changement de mode
void toggle_mode_relay(int index){
    switch (mode_relay[index]){
    case 0: // si le mode actuel est a 0 on passe a 1
        mode_relay[index] = 1;
        etat_relay[index] = 1;
        break;
    case 1: // si le mode actuel est a 1 on passe a 2
        mode_relay[index] = 2;
        etat_relay[index] = 0;
        break;
    case 2: // si le mode actuel est a 2 on reviens a 0
        mode_relay[index] = 0;
        etat_relay[index] = 0;
        break;    
    default:
        break;
    }
}

// changement d'état de relay
void state_relay(){
    for(int i=0;i<4;i++){
        // on effectue l'operation que si il y a un changement d'état
        if (digitalRead(pin_relay[i]) != etat_relay[i]){
            digitalWrite(pin_relay[i], etat_relay[i]);
        }       
    }
}

/*****************************/
/***   Gestion du temps   ****/

// bibliotheque de convertion de timestamp
#include <DTime.h>

// déclaration de la class dtime
DTime dtime;
DTime timestamp_prog;

// variable texte pour la date
String day_txt[7] = {"Lun", "Mar", "Mer", "Jeu", "Ven", "Sam", "Dim"};
String month_txt[12] = {"Janvier", "Févrié", "Mars", "Avril", "Mai", "juin", "juillet", "Aout", "Septembre", "Octobre", "Novembre", "Décembre"};

// definition des variables de temps
uint32_t timestamp;      // stockage de la timespamp
uint32_t last_millis;    // stockage de millis a un instant T
uint32_t diff_millis;    // temps ecoulé entre 2 enregistrement
int delta_millis = 1000; // intervalle entre 2 enregistrements soit 1 seconde
int time_minute = 0;     // temps ecoulé entre 2 enregistrement pour les minutes
bool check_time = false; // Autorisation pour les timer en mode auto

// definition des variables des relais
int pin_relay[4] = {21, 19, 18, 5};                                                              // definition des pin
int mode_relay[4] = {};                                                                          // 0 = arret forcé, 1 = marche forcé, 2 = auto
String mode_relay_txt[3] = {"ARRET", "MARCHE", "AUTO"};                                          // 0 = arret forcé, 1 = marche forcé, 2 = auto
int etat_relay[4] = {};                                                                          // 0 = arret, 1 = marche
String cdm_relay[4] = {"BTN_ETAT_P_FILT", "BTN_ETAT_ECL", "BTN_ETAT_ECL_2", "BTN_ETAT_P_CHAUD"}; // nom du bouton de la page html
String etat_relay_txt[2] = {"OFF", "ON"};

// variable de stockage des datas de programmation
// tableau 2 dimention [index btn][donnée]
int prog_on[2][4];                // heure programmer pour l'activation
int prog_duration[2][4];          // durer d'activation
int prog_week[2][4];              // semaine type 0 = aucune programmation, 1 = 7/7 jours, 2 = samedi au dimanche, 3 = lundi au vendredi
uint32_t time_duration_end[2][4]; // Stockage de la timestamp de fin
int prog_active[3];               // Stockage de la programation active

// pour la pompe eau chaude
int prog_max_temp;
int prog_delta;

// fonction d'ajout du 0 sur la date si inferieur a 10
String decimate(byte b)
{
    return ((b < 10) ? "0" : "") + String(b);
}

// fonction de mise en forme de la date
String date_txt()
{
    dtime.setTimestamp(timestamp);
    String result = day_txt[dtime.weekday - 1] + " " + decimate(dtime.day) + " " + month_txt[dtime.month - 1] + " " + dtime.year + " - " + decimate(dtime.hour) + ":" + decimate(dtime.minute);
    return result;
}

// fonction de mise a jour de l'horloge
void maj_time()
{
    // si millis() est revenue a zéro (49.7 jours)
    if (last_millis > millis())
    {
        diff_millis = (4294967295 - last_millis) + millis(); // on calcul le temps ecoulé
    }
    else
    {
        diff_millis = millis() - last_millis;
    }
    // si delta_millis est écoulé
    if (diff_millis >= delta_millis)
    {
        last_millis = millis();          // on rafraichie last_millis
        timestamp += diff_millis / 1000; // on ajoute diff_millis en seconde à la timestamp
                                         // On autorise la mise à jour des commande auto

        if (time_minute >= 60)
        {
            dtime.setTimestamp(timestamp);
            check_time = true;
            Serial.printf("Nous somme le %d/%d/%d Jour de la semaine : %d et il est %d:%d\n", dtime.day, dtime.month, dtime.year, dtime.weekday, dtime.hour, dtime.minute);
            time_minute = 0;
        }
        else
        {
            time_minute++;
        }
    }
}

/*****************************/

/***   Gestion des relais   ***/

// fonction d'initialisation des relais
void init_relay()
{
    for (int i = 0; i < 4; i++)
    {
        pinMode(pin_relay[i], OUTPUT);
        delay(10);
        digitalWrite(pin_relay[i], 0);
    }
}

// on cherche l'index du relais par rapport au texte
int num_relay(String relay)
{
    int result;
    // on commence par trouver l'index du tableau
    for (int i = 0; i < 4; i++)
    {
        if (cdm_relay[i] == relay)
        {
            result = i;
            break;
        }
    }
    return result;
}

// inversion de l'état du relay
void toggle_relay(int index)
{
    etat_relay[index] = 1 - etat_relay[index]; // on inverse l'état du relay 1-0=1, 1-1=0
}

// Changement de mode
void toggle_mode_relay(int index)
{
    prog_active[index] = 0; // on desactive le blocage de prog
    switch (mode_relay[index])
    {
    case 0: // si le mode actuel est a 0 on passe a 1
        mode_relay[index] = 1;
        etat_relay[index] = 1;
        break;
    case 1: // si le mode actuel est a 1 on passe a 2
        mode_relay[index] = 2;
        etat_relay[index] = 0;
        // on force la prise en compte des programmation
        check_time = true;
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
void state_relay()
{
    for (int i = 0; i < 4; i++)
    {
        // on effectue l'operation que si il y a un changement d'état
        if (digitalRead(pin_relay[i]) != etat_relay[i])
        {
            digitalWrite(pin_relay[i], etat_relay[i]);
        }
    }
}

/*****************************/

/***   gestion des historiques   ***/

// verification de l'existance des fichiers d'historique
void check_histo()
{
    String file_histo;
    // si le fichier n'existe pas encore on crée un fichier
    for (int i = 0; i < 4; i++)
    {
        file_histo = "/histo/" + cdm_relay[i] + ".txt"; // chemin vers le fichier corespondant
        if (!SPIFFS.exists(file_histo))
        {
            // création du fichier
            File create_histo = SPIFFS.open(file_histo, FILE_WRITE);
            String data = date_txt() + " -> Création du fichier d'historique\n";
            create_histo.print(data); // on ouvre les data Json
            delay(50);
            create_histo.close(); // on ferme le fichier
            Serial.print(F("Création du fichier d'historique : "));
            Serial.println(cdm_relay[i]);
            delay(50);
        }
    }
}

// ajout de ligne dans le fichier d'historique
void add_histo(String btn, String txt)
{
    if (btn != "")
    {
        String file_add = "/histo/" + btn + ".txt"; // chemin vers le fichier corespondant
        File add_histo = SPIFFS.open(file_add, FILE_APPEND);
        String data = date_txt() + " -> " + txt + "\n";
        add_histo.print(data); // On ajoute le texte
        delay(50);
        add_histo.close(); // on ferme le fichier
        Serial.print(F("Mise à jour du fichier d'historique : "));
        Serial.println(btn);
        delay(50);
    }
}

/*****************************/

/***   gestion des programmations horaire   ***/

// fonction explode pour extraire les données d'un string par separateur
String explode(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (data.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// mise à jour du fichier de backup de programamation
bool save_prog_spiffs(String btn) // btn nom du bouton de la page html
{
    bool retour = false;
    String file_conf_prog = "/config/" + btn + ".json"; // chemin vers le fichier corespondant
    // Création du fichier
    File prog_backup = SPIFFS.open(file_conf_prog, FILE_WRITE);
    // si le ficher est bien ouvert
    if (prog_backup)
    {
        int crc; // variable de verification du contenue
        String data;
        data = '{';
        crc += data.length();    // on compte le nombre de caractere pour la verificaion d'écriture
        prog_backup.print(data); // on ouvre les data Json
        prog_backup.close();     // on ferme le fichier

        // on ouvre le fichier en mode append (ajouter)
        prog_backup = SPIFFS.open(file_conf_prog, FILE_APPEND);
        // extraction de l'index du btn
        int index = num_relay(btn);
        // si c'est un bouton de prog horraire
        if (index <= 2)
        {
            // on boucle pour l'ecriture des données
            for (int i = 0; i < 4; i++)
            {
                data = "\"on_" + String(i + 1) + "\":\"" + String(prog_on[index][i]) + "\",";
                crc += data.length(); // on compte le nombre de caractere pour la verificaion d'écriture
                prog_backup.print(data);
                data = "\"duration_" + String(i + 1) + "\":\"" + String(prog_duration[index][i]) + "\",";
                crc += data.length(); // on compte le nombre de caractere pour la verificaion d'écriture
                prog_backup.print(data);
                data = "\"semaine_" + String(i + 1) + "\":\"" + String(prog_duration[index][i]) + "\"";
                // si se n'est pas la derniere donnée on place une virgule
                if (i < 3)
                {
                    data += ",";
                }
                crc += data.length(); // on compte le nombre de caractere pour la verificaion d'écriture
                prog_backup.print(data);
            }
            prog_backup.print("}"); // on ouvre les data Json
            crc++;
            prog_backup.close();
        }
        // si c'est le bouton de la pompe eau chaude
        else if (index == 3)
        {
            data = "\"max_temp\":\"" + String(prog_max_temp) + "\",";
            crc += data.length(); // on compte le nombre de caractere pour la verificaion d'écriture
            prog_backup.print(data);
            data = "\"delta\":\"" + String(prog_delta) + "\"}";
            crc += data.length(); // on compte le nombre de caractere pour la verificaion d'écriture
            prog_backup.print(data);
            prog_backup.close();
        }
        delay(10);
        // on verifie que l'écriture est correct
        Serial.print(F("\nCréation de : "));
        Serial.print(file_conf_prog);
        if (SPIFFS.open(file_conf_prog).size() == crc)
        {
            Serial.println(F(" -> SUCCES\n"));
            retour = true;
        }
        else
        {
            Serial.println(F(" -> ERROR\n"));
        }
    }
    return retour;
}

// chargement de la sauvegarde du SPIFFS vers les variables
bool load_prog_spiffs(String btn)
{
    String file_conf_prog = "/config/" + btn + ".json"; // chemin vers le fichier corespondant
    // si le Json n'existe pas encore
    if (!SPIFFS.exists(file_conf_prog))
    {
        Serial.println(F("Fichier innexistant !!!"));
        save_prog_spiffs(btn);
    }
    else // si le fichier existe déjà
    {
        // on extrait les données du fichier
        File prog_backup = SPIFFS.open(file_conf_prog, FILE_READ);
        char car;
        int size = SPIFFS.open(file_conf_prog).size();
        String result = "";
        // on extrait le texte du fichier
        while (prog_backup.available())
        {
            car = prog_backup.read();
            // on echap les caractères suivant { " }
            if (car != 123 && car != 125 && car != 34)
            {
                result += car;
            }
        }
        prog_backup.close();
        // on stock les donnée dans les variables
        int index = num_relay(btn); // index du tableau
        int i = 0;                  // increment de la boucle
        int var = 0;                // type de variable 0 = prog_on, 1 = prog_duration, 2 = prog_week
        int pos = 0;
        String extract;
        // on extrait les donnée pour "BTN_ETAT_P_FILT" || "BTN_ETAT_ECL" || "BTN_ETAT_ECL_2"
        if (index <= 2)
        { // {"on_1":"10","duration_1":"120","semaine_1":"1","on_2":"14","duration_2":"60","semaine_2":"2","on_3":"18","duration_3":"30","semaine_3":"3","on_4":"8","duration_4":"120","semaine_4":"1"}
            for (int i = 0; i < 12; i++)
            {
                extract = explode(result, ',', i);
                if (extract != 0)
                {
                    switch (var)
                    {
                    case 0:
                        prog_on[index][pos] = explode(extract, ':', 1).toInt();
                        var++;
                        break;
                    case 1:
                        prog_duration[index][pos] = explode(extract, ':', 1).toInt();
                        var++;
                        break;
                    case 2:
                        prog_week[index][pos] = explode(extract, ':', 1).toInt();
                        var = 0;
                        pos++;
                        break;

                    default:
                        break;
                    }
                }
            }
        }
        else if (index == 3) // on extrait les donnée pour "BTN_ETAT_P_CHAUD"
        {                    // {"max_temp":"0","delta":"0"}
            extract = explode(result, ',', 0);
            prog_max_temp = explode(extract, ':', 1).toInt();
            extract = explode(result, ',', 1);
            prog_delta = explode(extract, ':', 1).toInt();
        }
    }
    return true;
}

// fonction de cycle de commande en mode auto

void check_cdm_auto()
{
    // si on n'a passé un cycle
    if (check_time)
    {
        // on verifie que la connexion wifi n'est pas perdu
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.print(F("Perte de la connexion wifi reconnexion en cours : "));
            if (!WiFi.reconnect())
            {
                Serial.println(F("[ERROR]\n"));
                return;
            }
            Serial.println(F("[OK]"));
        }
        dtime.setTimestamp(timestamp); // on charge la timestamp
        // on verifie si il y a des commande en automatique pour les 3 relais horaire
        for (int i = 0; i < 3; i++)
        {
            if (mode_relay[i] == 2) // si on a des mode auto activé sur les gestion horaire
            {
                for (int prog = 0; prog < 4; prog++) // on verifie chaque programme de semaine
                {
                    int result_prog_week = prog_week[i][prog];
                    if (result_prog_week != 0) // si il y a une programation de semaine
                    {
                        // on calcule la timestamp de l'heure a 0 min (en cas de reboot ex reboot a 12h22 + 60 min arret a 13h22 au lieu de 13h)
                        timestamp_prog.setDate(dtime.year, dtime.month, dtime.day);
                        timestamp_prog.setTime(prog_on[i][prog], 0, 0);
                        uint32_t timestamp_prog_start = timestamp_prog.timestamp;
                        uint32_t timestamp_prog_end = timestamp_prog.timestamp + (prog_duration[i][prog] * 60);
                        switch (result_prog_week)
                        {
                        case 1:                                                                                                                      // 7/7 jours
                            if ((timestamp >= timestamp_prog_start && timestamp <= timestamp_prog_end) && etat_relay[i] == 0 && prog_active[i] == 0) // si on est dans la tranche horaire du debut et fin et que le relais est a 0 et que aucune prog est en fonction
                            {

                                // on enregistre la timestamp de fin en seconde
                                time_duration_end[i][prog] = timestamp_prog_end;
                                // on change l'état du relais
                                etat_relay[i] = 1;
                                prog_active[i] = prog + 1;
                                add_histo(cdm_relay[i], "Activation du programme : " + String(prog + 1));
                                Serial.print(F("\nActivation du programme : "));
                                Serial.print(prog + 1);
                                Serial.print(F(" pour le relais : "));
                                Serial.println(cdm_relay[i]);
                            }
                            else if (time_duration_end[i][prog] < timestamp && etat_relay[i] == 1 && prog_active[i] == prog + 1) // si la fin du timer est activé
                            {
                                etat_relay[i] = 0;
                                prog_active[i] = 0;
                                add_histo(cdm_relay[i], "Fin du programme : " + String(prog + 1));
                                Serial.print(F("\nFin du programme : "));
                                Serial.print(prog + 1);
                                Serial.print(F(" pour le relais : "));
                                Serial.println(cdm_relay[i]);
                            }
                            break;
                        case 2: // samedi/dimanche
                            if ((timestamp >= timestamp_prog_start && timestamp <= timestamp_prog_end) && etat_relay[i] == 0 && dtime.weekday >= 6 && prog_active[i] == 0)
                            {

                                // on enregistre la timestamp de fin en seconde
                                time_duration_end[i][prog] = timestamp_prog_end;
                                // on change l'état du relais
                                etat_relay[i] = 1;
                                // on verouille la programmation
                                prog_active[i] = prog + 1;
                                add_histo(cdm_relay[i], "Activation du programme : " + (prog + 1));
                                Serial.print(F("\nActivation du programme : "));
                                Serial.print(prog + 1);
                                Serial.print(F(" pour le relais : "));
                                Serial.println(cdm_relay[i]);
                            }
                            else if (time_duration_end[i][prog] < timestamp && etat_relay[i] == 1 && prog_active[i] == prog + 1) // si la fin du timer est activé
                            {
                                etat_relay[i] = 0;
                                prog_active[i] = 0;
                                add_histo(cdm_relay[i], "Fin du programme : " + String(prog + 1));
                                Serial.print(F("\nFin du programme : "));
                                Serial.print(prog + 1);
                                Serial.print(F(" pour le relais : "));
                                Serial.println(cdm_relay[i]);
                            }
                            break;
                        case 3: // du lundi au vendredi
                            if ((timestamp >= timestamp_prog_start && timestamp <= timestamp_prog_end) && etat_relay[i] == 0 && (dtime.weekday >= 1 && dtime.weekday <= 5) && prog_active[i] == 0)
                            {
                                // on enregistre la timestamp de fin en seconde
                                time_duration_end[i][prog] = timestamp_prog_end;
                                // on change l'état du relais
                                etat_relay[i] = 1;
                                prog_active[i] = prog + 1;
                                add_histo(cdm_relay[i], "Activation du programme : " + String(prog + 1));
                                Serial.print(F("\nActivation du programme : "));
                                Serial.print(prog + 1);
                                Serial.print(F(" pour le relais : "));
                                Serial.println(cdm_relay[i]);
                            }
                            else if (time_duration_end[i][prog] < timestamp && etat_relay[i] == 1 && prog_active[i] == prog + 1) // si la fin du timer est activé
                            {
                                etat_relay[i] = 0;
                                prog_active[i] = 0;
                                add_histo(cdm_relay[i], "fin du programme : " + String(prog + 1));
                                Serial.print(F("\nFin du programme : "));
                                Serial.print(prog + 1);
                                Serial.print(F(" pour le relais : "));
                                Serial.println(cdm_relay[i]);
                            }

                            break;

                        default:
                            break;
                        }
                    }
                }
            }
            else if (mode_relay[i] == 3)
            {
                // partie pour la pompe d'eau chaude
            }
        }
        check_time = false;
    }
}

/*****************************/
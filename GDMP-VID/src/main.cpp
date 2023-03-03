/*
███████╗███╗   ███╗██╗  ██╗ ██████╗ ███████╗██╗   ██╗
██╔════╝████╗ ████║██║  ██║██╔═══██╗██╔════╝╚██╗ ██╔╝
███████╗██╔████╔██║███████║██║   ██║███████╗ ╚████╔╝
╚════██║██║╚██╔╝██║██╔══██║██║   ██║╚════██║  ╚██╔╝
███████║██║ ╚═╝ ██║██║  ██║╚██████╔╝███████║   ██║
╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝   ╚═╝
Projet GDMP Gestion De Ma Piscine
crée le :     10/12/2022
par :         David AUVRÉ alias SMHOSY

*** remerciement ***
Icone de l'application https://www.vectorportal.com licence CC BY https://creativecommons.org/licenses/by/4.0/
Fonction explode de Vikas Kandari https://arduino.stackexchange.com/questions/1013/how-do-i-split-an-incoming-string
*/

#include <Arduino.h>

// bibliotheque pour le wifi et serveur web
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

// chargement du fichier de confuration
#include <config.h>

// defifinition du mode de debug
#define DEBUG true
#define Serial \
  if (DEBUG)   \
  Serial

// configuration du port du serveur web
AsyncWebServer server(80);

// chargement du fichier de fonction
#include <fonction.h>

void setup()
{
  // configuration du port serie pour le debug
  Serial.begin(115200);
  // mode debug présentation
  Serial.println(F("Projet GDMP Gestion De Ma Piscine"));
  Serial.print(F("Version : "));
  Serial.println(ver);
  Serial.print(F("Derniere modification : "));
  Serial.println(modif);
  Serial.println(F("DEBUG ACTIVE\n"));
  /*************************************************************/

  // Initialisation de l'espace de fichier SPIFFS
  Serial.print(F("- Initialisation de SPIFFS -> "));
  if (!SPIFFS.begin(true))
  {
    Serial.println(F("[ERROR]\n"));
    return;
  }
  Serial.println(F("[OK]\n"));
  /*************************************************************/

  // connection au wifi
  Serial.print(F("- Initialisation du WIFI -> "));
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println(F("[ERROR]\n"));
    return;
  }
  Serial.println(F("[OK]"));
  Serial.print(F("IP de GDMP : "));
  Serial.println(WiFi.localIP());

  /*************************************************************/

  // on initialise les relais
  init_relay();
  delay(100);
  /*************************************************************/

  //  on charge le fichier de config
  load_prog_spiffs("BTN_ETAT_P_FILT");
  load_prog_spiffs("BTN_ETAT_ECL");
  load_prog_spiffs("BTN_ETAT_P_CHAUD");
  delay(100);
  /*************************************************************/

  // On verifie la presence des fichiers d'historique
  check_histo();
  /*************************************************************/
  // on affiche la page par défaut sur l'ip
  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request)
            {
    request->send(SPIFFS, "/index.html");
    Serial.println("\n*** nouveau client Connecté ***\n"); });

  // on affiche la mise a jour des capteur json
  server.on("/maj.json", HTTP_ANY, [](AsyncWebServerRequest *request)
            {
              
    String reponse_json = "";
    String  json_index = "{";
            json_index += "\"T_EXT\":\"" + String(random(-50, 400) / 10.0, 1) + "\",";
            if (mode_relay[num_relay("BTN_ETAT_ECL")] == 2)
            {
              json_index += "\"ETAT_ECL\":\"" + mode_relay_txt[mode_relay[num_relay("BTN_ETAT_ECL")]] + " " + etat_relay_txt[etat_relay[num_relay("BTN_ETAT_ECL")]] + "\",";
            }
            else
            {
              json_index += "\"ETAT_ECL\":\"" + mode_relay_txt[mode_relay[num_relay("BTN_ETAT_ECL")]] + "\",";
            }
            json_index += "\"T_EAU_H\":\"" + String(random(-50, 400) / 10.0, 1) + "\",";
            json_index += "\"H_EXT\":\"" + String(random(200, 980) / 10.0, 1) + "\",";
            json_index += "\"HPA\":\"" + String(random(950, 1025)) + "\",";
            json_index += "\"PH\":\"" + String(random(40, 140) / 10.0, 1) + "\",";
            json_index += "\"REDOX\":\"" + String(random(250, 780)) + "\",";
            json_index += "\"T_EAU_C\":\"" + String(random(-50, 400) / 10.0, 1) + "\",";
            if (mode_relay[num_relay("BTN_ETAT_P_CHAUD")] == 2)
            {
              json_index += "\"ETAT_P_CHAUD\":\"" + mode_relay_txt[mode_relay[num_relay("BTN_ETAT_P_CHAUD")]] + " " + etat_relay_txt[etat_relay[num_relay("BTN_ETAT_P_CHAUD")]] + "\",";
            }
            else
            {
              json_index += "\"ETAT_P_CHAUD\":\"" + mode_relay_txt[mode_relay[num_relay("BTN_ETAT_P_CHAUD")]] + "\",";
            }

            if (mode_relay[num_relay("BTN_ETAT_P_FILT")] == 2)
            {
              json_index += "\"ETAT_P_FILT\":\"" + mode_relay_txt[mode_relay[num_relay("BTN_ETAT_P_FILT")]] + " " + etat_relay_txt[etat_relay[num_relay("BTN_ETAT_P_FILT")]] + "\",";
            }
            else
            {
              json_index += "\"ETAT_P_FILT\":\"" + mode_relay_txt[mode_relay[num_relay("BTN_ETAT_P_FILT")]] + "\",";
            }
            json_index += "\"T_EAU_B\":\"" + String(random(-50, 400) / 10.0, 1) + "\"";
            json_index += "}";
    //on stock le nombre de parametre  
    uint8_t nb_params = request->params();
    //on verifie la presence de parametre
    if(nb_params){  
      // préparation des variables// variable de reponse web
      String param_name[nb_params];
      String param_value[nb_params];
      Serial.println(F("Présence de parametre GET"));
      // extraction des données
      for(int i=0;i<nb_params;i++){
			  AsyncWebParameter* p = request->getParam(i);
			  param_name[i] = p->name();
			  param_value[i] = p->value();
        Serial.print(param_name[i]);
        Serial.print(F(" : "));
        Serial.println(param_value[i]);
      }

      // si le parametre timestamp est present
      if(param_name[0] == "timestamp" && param_value[0] !=""){
        // on stock la timestamp
        timestamp = param_value[0].toInt();
        Serial.print(F("Récuperation de la timestamp du client : "));
        Serial.println(timestamp);
        // on configure pour decodé la timestamp en date lisible 
        dtime.setTimestamp(timestamp);
        Serial.printf("Nous somme le %d/%d/%d Jour de la semaine : %d et il est %d:%d\n", dtime.day, dtime.month, dtime.year,dtime.weekday, dtime.hour, dtime.minute);
        // on force la reprise des programmation
        check_time=true;
        // on prepare la reponse json
        reponse_json = json_index;
        
      }
      
      // si le parametre cdm est present
      if(param_name[0] == "cdm" && param_value[0] !=""){
        int index_cdm = num_relay(param_value[0]);
        // on change l'état du mode
        toggle_mode_relay(index_cdm);
        add_histo(cdm_relay[index_cdm], "Passage en mode : " + mode_relay_txt[mode_relay[index_cdm]]);
        Serial.print(F("Changement d'état du relais : "));
        Serial.println(param_value[0]);
        Serial.print(F("Passage en mode : "));
        Serial.println(mode_relay_txt[mode_relay[index_cdm]]);
        // on prepare la reponse json
        reponse_json = "{";
        reponse_json += "\"statut\":\"" + mode_relay_txt[mode_relay[index_cdm]] + "\"";
        reponse_json += "}";

      }// si le parametre btn est present
      if(param_name[0] == "btn" && param_value[0] !=""){
        int index_btn = num_relay(param_value[0]);
        Serial.print(F("Chargement de la page cdm_btn : "));
        Serial.println(param_value[0]);
        Serial.println(F("Mise a jour des données"));
        // on prepare la reponse json
        reponse_json = "{";
        reponse_json += "\"statut\":\"" + mode_relay_txt[mode_relay[index_btn]] + "\"";
        reponse_json += "}";
      }
    }
    else{
      reponse_json = json_index;
    }

    request->send(200, "application/json", reponse_json);
    Serial.println("\n*** Mise a jour JSON ***\n"); });

  // ecoute des parametre de la dossier histo
  server.on("/histo/", HTTP_ANY, [](AsyncWebServerRequest *request)
            {
              //on stock le nombre de parametre  
              uint8_t nb_params = request->params();
              String name_btn;
              //on verifie la presence de parametre
              if(nb_params){  
                // préparation des variables
                String param_name[nb_params];
                String param_value[nb_params];
                Serial.println(F("Présence de parametre GET"));
                // extraction des données
                for(int i=0;i<nb_params;i++){
                  AsyncWebParameter* p = request->getParam(i);
                  param_name[i] = p->name();
                  param_value[i] = p->value();
                  Serial.print(param_name[i]);
                  Serial.print(F(" : "));
                  Serial.println(param_value[i]);
                }

                // si le parametre btn est present on supprime le fichier d'historique
                if(param_name[0] == "btn" && param_value[0] !=""){
                  name_btn = param_value[0];
                  // création du fichier
                      String file_histo = "/histo/" + name_btn + ".txt"; // chemin vers le fichier corespondant
                      File create_histo = SPIFFS.open(file_histo, FILE_WRITE);
                      String data = date_txt() + " -> Suppression des historiques\n";
                      create_histo.print(data); 
                      create_histo.close(); // on ferme le fichier
                      Serial.print(F("Suppression du fichier d'historique : "));
                      Serial.println(name_btn);
                }
    }
    // on envoi le fichier a jour  
    request->send(SPIFFS, "/histo/" + name_btn + ".txt"); });

  // ecoute des parametre de la dossier config pour les enregistrements des programmations
  server.on("/config/", HTTP_ANY, [](AsyncWebServerRequest *request)
            {
              //on stock le nombre de parametre  
              uint8_t nb_params = request->params();
              String name_btn;
              //on verifie la presence de parametre
              if(nb_params){  
                // préparation des variable
                String param_name[nb_params];
                String param_value[nb_params];
                Serial.println(F("Présence de parametre GET"));
                // extraction des données
                for(int i=0;i<nb_params;i++){
                  AsyncWebParameter* p = request->getParam(i);
                  param_name[i] = p->name();
                  param_value[i] = p->value();
                  Serial.print(param_name[i]);
                  Serial.print(F(" : "));
                  Serial.println(param_value[i]);
                }

                // si le parametre save_btn est present
                if (param_name[0] == "save_btn" && param_value[0] != "")
                {
                  name_btn = param_value[0];
                  // on enregistre les paramatre dans le fichier
                  String file_conf_prog = "/config/" + name_btn + ".json"; // chemin vers le fichier corespondant
                  // Création du fichier
                  File prog_backup = SPIFFS.open(file_conf_prog, FILE_WRITE);
                  // si le ficher est bien ouvert
                  if (prog_backup)
                  {
                    String data_save;
                    data_save = '{';
                    for (int i = 1; i < nb_params; i++)
                    {
                      data_save += "\"" + param_name[i] + "\":\"" + param_value[i] + "\"";
                      if (i != nb_params-1)
                      {
                        data_save += ",";
                      }
                    }

                    data_save += '}';
                    prog_backup.print(data_save);
                    prog_backup.close();
                    add_histo(name_btn, "Modification de la programmation ");
                    Serial.print(F("Modification de la programmation de : "));
                    Serial.println(name_btn);
                    // on recharge les donnée dans les variable
                    load_prog_spiffs(name_btn);
                    // on vide les fin de programmation et on force la programmation
                    for (int i = 0; i < 4; i++)
                    {
                      time_duration_end[num_relay(name_btn)][i] = 0;
                    }
                    check_time=true;
                  }
                }
    }
    // on envoi le fichier a jour  
    request->send(SPIFFS, "/config/" + name_btn + ".json"); });

  // On renvoie toute les requetes web vers le SPIFFS
  server.serveStatic("/", SPIFFS, "/");

  // On initialise le serveur web
  server.begin();

  /*************************************************************/
}

void loop()
{
  // mise a jour du timestamp
  maj_time();

  // verification des programme auto
  check_cdm_auto();

  // mise a jour de l'état des relais
  state_relay();

  delay(100);
}
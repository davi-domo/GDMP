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
// bibliotheque de convertion de timestamp
#include <DTime.h>
#include "SPIFFS.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BlueDot_BME280.h>
#include <DFRobot_ADS1115.h>
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// atribution du capteur bme28
BlueDot_BME280 bme280 = BlueDot_BME280();
DFRobot_ADS1115 ads(&Wire);

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

  // On configure la precision des sonde ds18b20
  sensors.setResolution(ad_T_EAU_H, 12);
  Serial.print("T_EAU_H Resolution: ");
  Serial.println(sensors.getResolution(ad_T_EAU_H), DEC);
  sensors.setResolution(ad_T_EAU_B, 12);
  Serial.print("T_EAU_B Resolution: ");
  Serial.println(sensors.getResolution(ad_T_EAU_B), DEC);
  sensors.setResolution(ad_T_EAU_C, 12);
  Serial.print("T_EAU_C Resolution: ");
  Serial.println(sensors.getResolution(ad_T_EAU_C), DEC);
  /*************************************************************/

// initialisation du capteur BME280
  bme280.parameter.I2CAddress = 0x76; 
  bme280.parameter.sensorMode = 0b11;
  bme280.parameter.IIRfilter = 0b100;
  bme280.parameter.humidOversampling = 0b101;
  bme280.parameter.tempOversampling = 0b101;
  bme280.parameter.pressOversampling = 0b101;
  bme280.parameter.pressureSeaLevel = 1013.25;
  bme280.parameter.tempOutsideCelsius = 15;

  bme280.init(33,32); // petite modif réalisé dans la librairie d'origine pour selectionner les pin I2C
  delay(100);
  
    ads.setAddr_ADS1115(ADS1115_IIC_ADDRESS1);   // 0x48
    ads.setGain(eGAIN_ONE);   // 2/3x gain
    ads.setMode(eMODE_SINGLE);       // single-shot mode
    ads.setRate(eRATE_128);          // 128SPS (default)
    ads.setOSMode(eOSMODE_SINGLE);   // Set to start a single-conversion
    ads.init();
  delay(1000);

  xTaskCreate(check_ds18b20, "vTask1", 10000, NULL, 3, NULL);
  delay(1000);
  xTaskCreate(check_bme280, "vTask2", 10000, NULL, 2, NULL);
  delay(1000);
  xTaskCreate(check_ads, "vTask3", 10000, NULL, 1, NULL);
  delay(1000);
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
            json_index += "\"T_EXT\":\"" + val_bme280[0] + "\",";
            if (mode_relay[num_index("BTN_ETAT_ECL")] == 2)
            {
              json_index += "\"ETAT_ECL\":\"" + mode_relay_txt[mode_relay[num_index("BTN_ETAT_ECL")]] + " " + etat_relay_txt[etat_relay[num_index("BTN_ETAT_ECL")]] + "\",";
            }
            else
            {
              json_index += "\"ETAT_ECL\":\"" + mode_relay_txt[mode_relay[num_index("BTN_ETAT_ECL")]] + "\",";
            }
            json_index += "\"T_EAU_H\":\"" + temp_eau[0] + "\",";
            json_index += "\"H_EXT\":\"" + val_bme280[1] + "\",";
            json_index += "\"HPA\":\"" + val_bme280[2] + "\",";
            json_index += "\"PH\":\"" + val_ads1115[0] + "\",";
            json_index += "\"REDOX\":\"" + val_ads1115[1] + "\",";
            json_index += "\"T_EAU_C\":\"" + temp_eau[2] + "\",";
            if (mode_relay[num_index("BTN_ETAT_P_CHAUD")] == 2)
            {
              json_index += "\"ETAT_P_CHAUD\":\"" + mode_relay_txt[mode_relay[num_index("BTN_ETAT_P_CHAUD")]] + " " + etat_relay_txt[etat_relay[num_index("BTN_ETAT_P_CHAUD")]] + "\",";
            }
            else
            {
              json_index += "\"ETAT_P_CHAUD\":\"" + mode_relay_txt[mode_relay[num_index("BTN_ETAT_P_CHAUD")]] + "\",";
            }

            if (mode_relay[num_index("BTN_ETAT_P_FILT")] == 2)
            {
              json_index += "\"ETAT_P_FILT\":\"" + mode_relay_txt[mode_relay[num_index("BTN_ETAT_P_FILT")]] + " " + etat_relay_txt[etat_relay[num_index("BTN_ETAT_P_FILT")]] + "\",";
            }
            else
            {
              json_index += "\"ETAT_P_FILT\":\"" + mode_relay_txt[mode_relay[num_index("BTN_ETAT_P_FILT")]] + "\",";
            }
            json_index += "\"T_EAU_B\":\"" + temp_eau[1] + "\"";
            json_index += "}";
    //on stock le nombre de parametre  
    uint8_t nb_params = request->params();
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
        check_cdm_auto=true;
        // on prepare la reponse json
        reponse_json = json_index;
        
      }
      
      // si le parametre cdm est present
      if(param_name[0] == "cdm" && param_value[0] !=""){
        int index_cdm = num_index(param_value[0]);
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
        int index_btn = num_index(param_value[0]);
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
                      String data = date_format() + " -> Suppression des historiques\n";
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
                      time_duration_end[num_index(name_btn)][i] = 0;
                    }
                    check_cdm_auto=true;
                  }
                }
    }
    // on envoi le fichier a jour  
    request->send(SPIFFS, "/config/" + name_btn + ".json"); });

  // ecoute des parametre de la dossier stat pour l'affichage des courbe
  server.on("/stat/", HTTP_ANY, [](AsyncWebServerRequest *request)
            {
    // on stock le nombre de parametre
    uint8_t nb_params = request->params();
    uint8_t index_sonde;
    String reponse_json;
    String value_ds18b20;
    String value_bme280;
    String value_ads1115;
    // on verifie la presence de parametre
    if (nb_params)
    {
      // préparation des variable
      String param_name[nb_params];
      String param_value[nb_params];
      Serial.println(F("Présence de parametre GET"));
      // extraction des données
      for (int i = 0; i < nb_params; i++)
      {
        AsyncWebParameter *p = request->getParam(i);
        param_name[i] = p->name();
        param_value[i] = p->value();
        Serial.print(param_name[i]);
        Serial.print(F(" : "));
        Serial.println(param_value[i]);
      }

      // si le parametre save_btn est present
      if (param_name[0] == "sonde" && param_value[0] != "")
      {
        if(param_value[1] == "DS18B20"){
        index_sonde = num_index(param_value[0],2);
        reponse_json = "[\n";
        value_ds18b20 = (ds18b20_day[index_sonde][0] == NULL) ? "null" : ds18b20_day[index_sonde][0];
        reponse_json += "{\"date\":\"" + date_format("number") + "\",\"stat\":[" + value_ds18b20;
                for (int j = 1; j <= dtime.hour; j++) // on incremente toute les heures
                {
                    value_ds18b20 = (ds18b20_day[index_sonde][j] == NULL) ? "null" : ds18b20_day[index_sonde][j];
                    reponse_json += "," + value_ds18b20;
                }
                reponse_json += "]}\n";
                reponse_json += "]";
        // presense de action = del on supprime les stats
                if (param_value[2] != NULL){
                  String file_del = "/stat/" + param_value[0] + ".json";
                  if( param_value[2] == "del" && SPIFFS.exists(file_del)) {
                    SPIFFS.remove(file_del);
                    Serial.print(F("Suppression du fichier de statistique de la sonde : "));
                    Serial.println(param_value[0]);
                  }
                }
        }
        if(param_value[1] == "BME280"){
        index_sonde = num_index(param_value[0],3);
        reponse_json = "[\n";
        value_bme280 = (bme280_day[index_sonde][0] == NULL) ? "null" : bme280_day[index_sonde][0];
        reponse_json += "{\"date\":\"" + date_format("number") + "\",\"stat\":[" + value_bme280;
                for (int j = 1; j <= dtime.hour; j++) // on incremente toute les heures
                {
                    value_bme280 = (bme280_day[index_sonde][j] == NULL) ? "null" : bme280_day[index_sonde][j];
                    reponse_json += "," + value_bme280;
                }
                reponse_json += "]}\n";
                reponse_json += "]";
        // presense de action = del on supprime les stats
                if (param_value[2] != NULL){
                  String file_del = "/stat/" + param_value[0] + ".json";
                  if( param_value[2] == "del" && SPIFFS.exists(file_del)) {
                    SPIFFS.remove(file_del);
                    Serial.print(F("Suppression du fichier de statistique de la sonde : "));
                    Serial.println(param_value[0]);
                  }
                }
        }
        if(param_value[1] == "ADS1115"){
        index_sonde = num_index(param_value[0],4);
        reponse_json = "[\n";
        value_ads1115 = (ads1115_day[index_sonde][0] == NULL) ? "null" : ads1115_day[index_sonde][0];
        reponse_json += "{\"date\":\"" + date_format("number") + "\",\"stat\":[" + value_ads1115;
                for (int j = 1; j <= dtime.hour; j++) // on incremente toute les heures
                {
                    value_ads1115 = (ads1115_day[index_sonde][j] == NULL) ? "null" : ads1115_day[index_sonde][j];
                    reponse_json += "," + value_ads1115;
                }
                reponse_json += "]}\n";
                reponse_json += "]";
        // presense de action = del on supprime les stats
                if (param_value[2] != NULL){
                  String file_del = "/stat/" + param_value[0] + ".json";
                  if( param_value[2] == "del" && SPIFFS.exists(file_del)) {
                    SPIFFS.remove(file_del);
                    Serial.print(F("Suppression du fichier de statistique de la sonde : "));
                    Serial.println(param_value[0]);
                  }
                }
        }
        
      }
    }
    // on envoi le fichier a jour
    request->send(200, "application/json", reponse_json); });

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
  cdm_auto();

  // mise a jour des statistique
  maj_stat();

  // mise a jour de l'état des relais
  state_relay();

  delay(100);
}
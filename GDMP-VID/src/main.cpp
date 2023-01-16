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

// variable de reponse web
String reponse_json = "";

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

// on affiche la page par défaut sur l'ip
  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html");
    Serial.println("\n*** nouveau client Connecté ***\n");
	});
  // on affiche la mise a jour des capteur json
  server.on("/maj.json", HTTP_ANY, [](AsyncWebServerRequest *request){
    reponse_json = "{";
    reponse_json += "\"T_EXT\":\"" + String(random(-50, 400) / 10.0, 1) + "\",";
    reponse_json += "\"ETAT_ECL\":\"" + ETAT_REL[random(0, 3)] + "\",";
    reponse_json += "\"T_EAU_H\":\"" + String(random(-50, 400) / 10.0, 1) + "\",";
    reponse_json += "\"H_EXT\":\"" + String(random(200, 980) / 10.0, 1) + "\",";
    reponse_json += "\"HPA\":\"" + String(random(950, 1025) ) + "\",";
    reponse_json += "\"PH\":\"" + String(random(40, 140) / 10.0, 1) + "\",";
    reponse_json += "\"REDOX\":\"" + String(random(250, 780) ) + "\",";
    reponse_json += "\"T_EAU_C\":\"" + String(random(-50, 400) / 10.0, 1) + "\",";
    reponse_json += "\"ETAT_P_CHAUD\":\"" + ETAT_REL[random(0, 3)] + "\",";
    reponse_json += "\"ETAT_P_FILT\":\"" + ETAT_REL[random(0, 3)] + "\",";
    reponse_json += "\"T_EAU_B\":\"" + String(random(-50, 400) / 10.0, 1) + "\"";
    reponse_json += "}";

    request->send(200, "application/json", reponse_json);
    Serial.println("\n*** Mise a jour JSON ***\n");
	});
  // On renvoie toute les requetes web vers le SPIFFS
  server.serveStatic("/", SPIFFS, "/");
  // On initialise le serveur web
  server.begin();
  /*************************************************************/
}

void loop()
{
  // put your main code here, to run repeatedly:
}
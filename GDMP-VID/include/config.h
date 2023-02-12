/* fichier de configuration de GDMP */

// detail du projet

const char *ver = "1.1";
const char *modif = "10/02/23";

// configuation du wifi
const char *ssid = "VOTRE_SSID";
const char *password = "VOTRE_MOT_DE_PASSE";

//ip de GDMP
IPAddress ip(192, 168, 0, 101);
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(192, 168, 0, 255);

// variable de simulation de capteur
String ETAT_REL[3] ={"MARCHE","ARRET","AUTO"};
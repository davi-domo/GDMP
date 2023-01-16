#include <DTime.h>

DTime dtime;

uint64_t cycle;

String decimate(byte b) {
  return ((b < 10) ? "0" : "") + String(b);
}

void sendData() {
  String m[12] = {"Janeiro", "Fevereiro", "Mar" + String(char(231)) + "o", "Abril", "Maio", "Junho", "Julho", "Agosto", "Setembro", "Outubro", "Novembro", "Dezembro"};
  String w[7] = {"Domingo", "Segunda", "Ter" + String(char(231)) + "a", "Quarta", "Quinta", "Sexta", "S" + String(char(225)) + "bado"};

  Serial.print(dtime.timestamp);
  Serial.print("    -   ");
  Serial.print(w[dtime.weekday]);

  Serial.print(", ");
  Serial.print(dtime.year);
  Serial.print("-");
  Serial.print(m[dtime.month - 1]);
  Serial.print("-");
  Serial.print(decimate(dtime.day));
  Serial.print("       at    ");
  Serial.print(decimate(dtime.hour));
  Serial.print(":");
  Serial.print(decimate(dtime.minute));
  Serial.print(":");
  Serial.println(decimate(dtime.second));

}

void setup() {
  Serial.begin(9600);
  delay(100);
  cycle = (uint64_t)millis() + 1000;
}

void loop() {
  if ((uint64_t)millis() >=  cycle) {
    cycle += 1000;
    dtime.tick();
    sendData();
  }
}

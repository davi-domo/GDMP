#include "DTime.h"

DTime c, d, e;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  c.setTimestamp(1459468799);
  d.setDate(2019, 1, 18);
  d.setTime(20, 57, 0);
  
  Serial.print(e.weekday);
  Serial.print(", ");
  Serial.print(e.year);
  Serial.print("-");
  Serial.print(e.month);
  Serial.print("-");
  Serial.print(e.day);
  Serial.print("  at  ");
  Serial.print(e.hour);
  Serial.print(":");
  Serial.print(e.minute);
  Serial.print(":");
  Serial.println(e.second);

  Serial.print(c.weekday);
  Serial.print(", ");
  Serial.print(c.year);
  Serial.print("-");
  Serial.print(c.month);
  Serial.print("-");
  Serial.print(c.day);
  Serial.print("  at  ");
  Serial.print(c.hour);
  Serial.print(":");
  Serial.print(c.minute);
  Serial.print(":");
  Serial.println(c.second);

  c.tick();
  Serial.print(c.weekday);
  Serial.print(", ");
  Serial.print(c.year);
  Serial.print("-");
  Serial.print(c.month);
  Serial.print("-");
  Serial.print(c.day);
  Serial.print("  at  ");
  Serial.print(c.hour);
  Serial.print(":");
  Serial.print(c.minute);
  Serial.print(":");
  Serial.println(c.second);

  Serial.println(d.timestamp);

  c.setTime(21, 19, 02);
  Serial.print(c.weekday);
  Serial.print(", ");
  Serial.print(c.year);
  Serial.print("-");
  Serial.print(c.month);
  Serial.print("-");
  Serial.print(c.day);
  Serial.print("  at  ");
  Serial.print(c.hour);
  Serial.print(":");
  Serial.print(c.minute);
  Serial.print(":");
  Serial.println(c.second);

  c.setDate(2019, 1, 17);
  Serial.print(c.weekday);
  Serial.print(", ");
  Serial.print(c.year);
  Serial.print("-");
  Serial.print(c.month);
  Serial.print("-");
  Serial.print(c.day);
  Serial.print("  at  ");
  Serial.print(c.hour);
  Serial.print(":");
  Serial.print(c.minute);
  Serial.print(":");
  Serial.println(c.second);


}

void loop() {
  // put your main code here, to run repeatedly:
}

#include <HCSR04.h>
#include <RH_ASK.h>
#include <CRC16_c.h>
#include "RfData.h"

bool constexpr SerialEnabled = true;

uint32_t currentTime;
uint32_t triggerTime;
RH_ASK driver(2000, 11, 9);

void setup() {
  if (SerialEnabled) Serial.begin(115200);
  triggerTime = 0;

  if (SerialEnabled) Serial.println("hc begin");
  HCSR04.begin(TriggerPin, EchoPins, sizeof(EchoPins), 30000UL, HCSR04Sensor::unlockSkip);
  if (SerialEnabled) Serial.println("hc began");
  if (!driver.init()) {
    if (SerialEnabled) Serial.println("driver failed");
  }
  CRC16_reset();
}

void updateCrc() {
  CRC16_restart();
  auto data = reinterpret_cast<uint8_t const*>(&rfData.crc) + sizeof(rfData.crc);
  auto length = sizeof(rfData) - sizeof(rfData.crc);
  while (length--) {
    CRC16_add(*data++);
  }
  rfData.crc = CRC16_getCRC();
}

void sendRfData() {
  driver.send((uint8_t *)&rfData, sizeof(rfData));
  driver.waitPacketSent();
}

double results[EchoPinCount];

void measureDistances() {
  if (SerialEnabled) Serial.println("start meas");
  auto t1 = millis();
  HCSR04.measureDistanceMm(results);
  t1 = millis() - t1;
  rfData.msgId = 0x20 + EchoPinCount;
  for (size_t i = 0; i < EchoPinCount; ++i) {
    rfData.results[i] = results[i];
  }

  if (SerialEnabled) Serial.print("meas took: "); 
  if (SerialEnabled) Serial.print(t1);
  if (SerialEnabled) Serial.print("ms, D: [");
  for (size_t i = 0; i < EchoPinCount; ++i) {
    if (SerialEnabled) Serial.print(rfData.results[i]);
    if (SerialEnabled) Serial.print(", ");
  }
  if (SerialEnabled) Serial.println("]");
  updateCrc();
  sendRfData();
}

void loop() {
  currentTime = millis();
  if (currentTime - triggerTime > 100) {
    triggerTime = currentTime;
    measureDistances();
  }
}

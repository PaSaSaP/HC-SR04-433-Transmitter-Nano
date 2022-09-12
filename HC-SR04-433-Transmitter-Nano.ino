//#include <kd_HysteresisValue.h>
#include <kd_MeanValue.h>
#include <HCSR04.h>
#include <RH_ASK.h>
#include <CRC16_c.h>

bool constexpr SerialEnabled = true;

#define TriggerPin 4
// uint8_t const EchoPins[] {5, 6, 7, 8};
uint8_t const EchoPins[] {5, 6};
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
}

struct RfData {
  uint16_t begin = 0x1212;
  double results[sizeof(EchoPins) / sizeof(*EchoPins)];
  uint16_t crc;
  uint16_t end = 0xEFEF;
} rfData;

void updateCrc() {
  CRC16_restart();
  auto data = reinterpret_cast<uint8_t const*>(rfData.results);
  auto length = sizeof(rfData.results);
  while (length--) {
    CRC16_add(*data++);
  }
  rfData.crc = CRC16_getCRC();
}

void sendRfData() {
  // driver.send("abcd\n", 6);
  driver.send((uint8_t *)&rfData, sizeof(rfData));
  driver.waitPacketSent();
}

void measureDistances() {
  if (SerialEnabled) Serial.println("start measurement");
  auto t1 = millis();
  HCSR04.measureDistanceCm(rfData.results);
  t1 = millis() - t1;

  if (SerialEnabled) Serial.print("pomiar zajal: "); 
  if (SerialEnabled) Serial.print(t1);
  if (SerialEnabled) Serial.print("ms, D: [");
  if (SerialEnabled) Serial.print(rfData.results[0]);
  if (SerialEnabled) Serial.print(", ");
  if (SerialEnabled) Serial.print(rfData.results[1]);
  // if (SerialEnabled) Serial.print(", ");
  // if (SerialEnabled) Serial.print(rfData.results[2]);
  // if (SerialEnabled) Serial.print(", ");
  // if (SerialEnabled) Serial.print(rfData.results[3]);
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

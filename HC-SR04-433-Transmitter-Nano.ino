#include <HCSR04.h>
#include <RH_ASK.h>
#include <CRC16_c.h>
#include "RfData.h"

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#include <LowPower.h>

bool constexpr SerialEnabled = false;

uint32_t currentTime;
uint32_t triggerTime;
RH_ASK driver(4000, 11, 9);

void setup() {
  CLKPR = 0x80; // (1000 0000) enable change in clock frequency
  CLKPR = 0x01; // (0000 0001) use clock division factor 2 to reduce the frequency from 16 MHz to 8 MHz

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
    // multiple it because of clock is changed to 8MHz
    rfData.results[i] = results[i]*2;
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
  // currentTime = millis();
  // if (currentTime - triggerTime > 100) {
  //   triggerTime = currentTime;
  //   measureDistances();
  // }
  measureDistances();
  
  LowPower.idle(SLEEP_120MS, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, 
              SPI_OFF, USART0_OFF, TWI_ON);
}

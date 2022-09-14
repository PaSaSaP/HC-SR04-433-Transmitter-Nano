#ifndef HEADER_RFDATA_H_
#define HEADER_RFDATA_H_

#define TriggerPin 4
// uint8_t const EchoPins[] {5, 6, 7, 8};
uint8_t const EchoPins[] {5, 6};
size_t constexpr EchoPinCount = sizeof(EchoPins) / sizeof(*EchoPins);

struct RfData {
  uint16_t crc;
  uint16_t results[EchoPinCount];
  uint16_t msgId;
  uint16_t end = 0xEFEF;
} rfData;

#endif /* HEADER_RFDATA_H_ */


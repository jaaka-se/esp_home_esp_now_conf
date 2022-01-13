//
// Author: jaaka.se
//
#ifndef __Custom32_RC_H__
#define __Custom32_RC_H__

#include "esphome.h"
//ref https://esphome.io/components/sensor/custom.html
#ifdef __cplusplus
  extern "C" {
 #endif

  uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

uint8_t temprature_sens_read();

class MyCustomSensorF : public Component, public Sensor {
 public:
  void setup() override {
    // This will be called by App.setup()
  }
  void loop() override {
    // This will be called by App.loop()
  }
};
class MyCustomSensorP : public PollingComponent, public Sensor {
 public:
  // constructor
  MyCustomSensorP() : PollingComponent(15000) {}
 // 15_000 ms hart beat
  float get_setup_priority() const override { return esphome::setup_priority::LATE; }

  void setup() override {
    // This will be called by App.setup()
  }
  void update() override {
    // This will be called every "update_interval" milliseconds.
        publish_state((temprature_sens_read() - 32) / 1.8);
    Serial.printf(":: Cuestom32 temp %d \n",temprature_sens_read());
  }
};

#endif	//__MESH32_RC_H__

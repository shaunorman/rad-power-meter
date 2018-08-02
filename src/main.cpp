/**
 * This software is written to monitor the power usage of my home power meter.
 *
 * My power meter is a Western Power supplied Landis + Gyer E360 3 Phase power
 * meter
 *
 * https://westernpower.com.au/media/1553/three-phase-ami-smartmeter-fact-sheet.pdf
 *
 * Every time the LED flashes a Watt of power is consumed. This is being
 * monitored by a LDR Module and when the light is turned on a pin is powered
 * and then an interrupt is calling adding a timestamp difference from a
 * reference time to a vector.
 *
 * At the moment it publishes to the MQTT server on every watt used so I can
 * display and instant power reading. I might change this to send every 5 or
 * so pules.
 *
 *
 *  PINS:
 * mcu   ->   rld
 * D4         D0
 * VCC        VV
 * G          GND
 * A0         A0
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <RadConfig.h>
#include <RadDevice.h>
#include <RadHelpers.h>
#include <RadWiFi.h>
#include <RadMQTT.h>



//
// DATA STORAGE
//

typedef std::vector<long> vector_time;
vector_time watt_history;

// TODO: logging for the past hour to show watts used in the past 1, 5 and
// 60mins on the screen
// Vector filtering: http://antonym.org/2009/09/stl-filtering.html
// vector_time watt_history_1_min;
// vector_time watt_history_5_min;
// vector_time watt_history_hour;


//
// TIMERS
//

unsigned long watt_last_change = 0;
unsigned long mqtt_last_poll = 0;


//
// Reference points
// This will get updated every 6 hours and the values will be used as
// reference points when the watt recordings are sent to the server.
//

unsigned long ref_updated_every_ms = 21600000;
// The time the last update occured
unsigned long ref_epoch_updated;
// And the time in millis the device was at when it occure, for reference.
unsigned long ref_epoch_updated_millis = 0;


//
// Setup
//

// If the LED is on or not
bool watt_led_on = false;

int LDR_pin = D5;

RadConfig config = RadConfig();

RadWiFi wifi(config);

RadMQTT mqtt(config);

// UTC time (timezones suck)
WiFiUDP ntpUDP;
NTPClient time_client(ntpUDP, "0.au.pool.ntp.org", 0, ref_updated_every_ms);


// This interrupt fires for both when the sensor turns on, and when it turns off.
void interrupt_watt() {
    unsigned long now = millis();
    // Only do something if its been more than 100ms since the last event.
    if (now - watt_last_change > 100) {
        watt_last_change = now;

        // LED is turning on.
        if (false == watt_led_on) {
            watt_led_on = true;
            watt_history.push_back(now - ref_epoch_updated_millis);

        }
        else {
            watt_led_on = false;
            digitalWrite(LED_BUILTIN, HIGH);
        }
    }
}

void setup() {
    pinMode(D4, OUTPUT);
    pinMode(LDR_pin, INPUT);

    config.debug_on();

    Serial.begin(115200);
    delay(2500);

    wifi.connect();

    time_client.begin();

    attachInterrupt(digitalPinToInterrupt(LDR_pin), interrupt_watt, CHANGE);
}

void loop() {
    wifi.loop();
    mqtt.loop();

    // Update time via NTP every "ref_updated_every_ms"
    // But do not update if we have anything in the history log as we are using the last time
    // our time was updated as a reference.
    if (ref_epoch_updated_millis == 0 || millis() - ref_epoch_updated_millis > ref_updated_every_ms) {
        config.debug("Updating Time");
        time_client.update();
        ref_epoch_updated = time_client.getEpochTime();
        ref_epoch_updated_millis = millis();
        config.debug("Time Updated to: " + time_client.getFormattedTime());
    }

    // Lets publish some data if we have some!
    if (watt_history.size() > 0 ) {
        config.debug("Processing watt history to publish");
        String pulses;
        for(const auto &s : watt_history) {
            if(pulses != "") {
                pulses += ",";
            }
            pulses += String(s);
        }

        // Create our own json string instead of including a library
        String request_data = "{\"ref_epoch\": " + String(ref_epoch_updated);
        request_data += ", \"ref_millis\": " + String(ref_epoch_updated_millis);
        request_data += ", \"pulses\": \"" + pulses + "\"}";

        config.debug("MQTT Publish /power-meter");
        config.debug(request_data);
        bool result = mqtt.client->publish("/power-meter", request_data.c_str());

        if (result) {
            watt_history.clear();
            config.debug("Watt history cleared");
        }
        else {
            config.log("[MQTT] failure to send history");
        }
    }
}

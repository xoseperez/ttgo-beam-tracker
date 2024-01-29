/*

Main module

Copyright (C) 2018-2019 by Xose Pérez <xose dot perez at gmail dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "configuration.h"
#include <rom/rtc.h>
#include <CayenneLPP.h>

// Cayenne buffer
CayenneLPP lpp(51);

// Message counter, stored in RTC memory, survives deep sleep
RTC_DATA_ATTR uint32_t count = 0;

// -----------------------------------------------------------------------------
// Application
// -----------------------------------------------------------------------------

void send() {

    /*
    
    Channel definition for trackers:

    CHANNEL 0: counter (digital_out_0)
    CHANNEL 1: gps (gps_1.altitude gps_1.latitude gps_1.longitude)
    CHANNEL 2: temperature (temperature_2)
    CHANNEL 3: accelerometer (accelerometer_3.x accelerometer_3.y accelerometer_3.z)
    CHANNEL 4: battery (analog_out_4) in volts
    CHANNEL 5: sats (digital_out_5)
    CHANNEL 6: hdop (analog_out_4)

    */

    char buffer[40];
    snprintf(buffer, sizeof(buffer), "Latitude: %10.6f\n", gps_latitude());
    screen_print(buffer);
    snprintf(buffer, sizeof(buffer), "Longitude: %10.6f\n", gps_longitude());
    screen_print(buffer);
    snprintf(buffer, sizeof(buffer), "Error: %4.2fm\n", gps_hdop());
    screen_print(buffer);

    lpp.reset();
    lpp.addDigitalOutput(0, count);
    if (gps_lock()) {
        lpp.addGPS(1, gps_latitude(), gps_longitude(), gps_altitude());
    } else {
        screen_print("No GPS lock yet\n");
    }
    //lpp.addAnalogOutput(4, battery());
    lpp.addDigitalOutput(5, gps_sats());
    lpp.addAnalogOutput(6, gps_hdop());

    #if LORAWAN_CONFIRMED_EVERY > 0
        bool confirmed = (count % LORAWAN_CONFIRMED_EVERY == 0);
    #else
        bool confirmed = false;
    #endif

    ttn_cnt(count);
    ttn_send(lpp.getBuffer(), lpp.getSize(), LORAWAN_PORT, confirmed);

    count++;

}

void sleep() {

    #if SLEEP_BETWEEN_MESSAGES

        if (gps_lock()) {

            // Show the going to sleep message on the screen
            char buffer[20];
            snprintf(buffer, sizeof(buffer), "Sleeping in %3.1fs\n", (MESSAGE_TO_SLEEP_DELAY / 1000.0));
            screen_print(buffer);

        }
    
    #endif

    // Wait for MESSAGE_TO_SLEEP_DELAY millis to sleep
    delay(MESSAGE_TO_SLEEP_DELAY);

    #if SLEEP_BETWEEN_MESSAGES

        if (gps_lock()) {
        
            // Turn off screen
            screen_off();

            // Set the user button to wake the board
            sleep_interrupt(BUTTON_PIN, LOW);

            // We sleep for the interval between messages minus the current millis
            // this way we distribute the messages evenly every SEND_INTERVAL millis
            uint32_t sleep_for = (millis() < SEND_INTERVAL) ? SEND_INTERVAL - millis() : SEND_INTERVAL;
            sleep_millis(sleep_for);
        
        }

    #endif

}

void callback(uint8_t message) {

    if (EV_JOINING == message) screen_print("Joining TTN...\n");
    if (EV_JOINED == message) screen_print("TTN joined!\n");
    if (EV_JOIN_FAILED == message) screen_print("TTN join failed\n");
    if (EV_REJOIN_FAILED == message) screen_print("TTN rejoin failed\n");
    if (EV_RESET == message) screen_print("Reset TTN connection\n");
    if (EV_LINK_DEAD == message) screen_print("TTN link dead\n");
    if (EV_ACK == message) screen_print("ACK received\n");
    if (EV_PENDING == message) screen_print("Message discarded\n");
    if (EV_QUEUED == message) screen_print("Message queued\n");

    if (EV_TXCOMPLETE == message) {
        screen_print("Message sent\n");
        sleep();
    }

    if (EV_RESPONSE == message) {

        screen_print("[TTN] Response: ");

        size_t len = ttn_response_len();
        uint8_t data[len];
        ttn_response(data, len);

        char buffer[6];
        for (uint8_t i=0; i<len; i++) {
            snprintf(buffer, sizeof(buffer), "%02X", data[i]);
            screen_print(buffer);
        }

        screen_print("\n");

    }

}

uint32_t get_count() {
    return count;
}

void setup() {

    // Debug
    #ifdef DEBUG_PORT
        DEBUG_PORT.begin(SERIAL_BAUD);
    #endif

    // Buttons & LED
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);

    // Hello
    DEBUG_MSG(APP_NAME " " APP_VERSION "\n");

    // Display
    screen_setup();

    // Init GPS
    gps_setup();

    // Show logo on first boot
    if (0 == count) {
        screen_print(APP_NAME " " APP_VERSION, 0, 0);
        screen_show_logo();
        screen_update();
        delay(LOGO_DELAY);
    }

    // TTN setup
    if (!ttn_setup()) {
        screen_print("[ERR] Radio module not found!\n");
        delay(MESSAGE_TO_SLEEP_DELAY);
        screen_off();
        sleep_forever();
    }

    ttn_register(callback);
    ttn_join();
    ttn_sf(LORAWAN_SF);
    ttn_adr(LORAWAN_ADR);

}

void loop() {

    gps_loop();
    ttn_loop();
    screen_loop();

    // Send every SEND_INTERVAL millis
    static uint32_t last = 0;
    if (0 == last || millis() - last > SEND_INTERVAL) {
        last = millis();
        send();
    }
    
}

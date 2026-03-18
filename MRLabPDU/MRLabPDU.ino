/*
    SPDX-License-Identifier: EUPL-1.2
    Copyright (C) 2025 Daniel Hojka <daniel.hojka@sonible.com>

    Licensed under the EUPL, Version 1.2 or – as soon they will be
    approved by the European Commission – subsequent versions of the EUPL.
    You may not use this work except in compliance with the Licence.
    You may obtain a copy of the Licence at:
    https://joinup.ec.europa.eu/collection/eupl/eupl-text-eupl-12
    
    sonible MR Lab PDU firmware
    
    A simple Arduino based firmware for the "CONTROLLINO MAXI Power" providing 
    some api endpoints and a few macros for power sequencing the mixed reality 
    lab at the TU Vienna.
    
    Hints:

        on:  http://x.x.x.x/digital/24/1
        off: http://x.x.x.x/digital/24/0

    to switch on and off the relay controlled by pin 24 

    To run the power on/off sequence, use:

        on:  http://x.x.x.x/sequence?p=1
        off: http://x.x.x.x/sequence?p=0

    To change the inter-relay-delay after boot up use:

      http://x.x.x.x/delay?p=$VALUE [in milliseconds]

 */

#define RELAY_ON    HIGH
#define RELAY_OFF   LOW
#define RELAY_DELAY 250
#define RELAY0 22
#define RELAY1 23
#define RELAY2 24
#define RELAY3 25
#define RELAY4 26

// Libraries
#include <SPI.h>
#include <Ethernet.h>
#include <aREST.h>
#include <avr/wdt.h>

// Enter a MAC address for your controller below.
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0xFE, 0x41};

// copy define macro into gloabl variable
int interRelayDelay = RELAY_DELAY;

// IP address in case DHCP fails
IPAddress ip(172,16,60,20);

// Ethernet server
EthernetServer server(80);

// Create aREST instance
aREST rest = aREST();

// Declare functions to be exposed to the API
int relaySequence(String command);
int setRelayDelay(String command);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  // Function to be exposed
  rest.function("sequence",relaySequence);
  rest.function("delay",setRelayDelay);

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id("001");
  rest.set_name("sonible_pdu");

  // Start the Ethernet connection and the server
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // configure using the static IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  server.begin();
  Serial.println("sonible PDU Rest API ready ");
  Serial.print("listening on ");
  Serial.println(Ethernet.localIP());

  // Start watchdog
  wdt_enable(WDTO_4S);
}

void loop() {

  // listen for incoming clients
  EthernetClient client = server.available();
  rest.handle(client);
  wdt_reset();

}

// Custom function accessible by the API
int relaySequence(String command) {

  // Get state from command
  int state = command.toInt();
  Serial.print("future power on state: ");
  Serial.println(state);

  // S5 (RELAY4) needs to power on first to provide valid audio clock conditions on power up
  if ( state == 1 ) {
    digitalWrite(RELAY4,RELAY_ON);
    delay(interRelayDelay);
    digitalWrite(RELAY0,RELAY_ON);
    delay(interRelayDelay);
    digitalWrite(RELAY1,RELAY_ON);
    delay(interRelayDelay);
    digitalWrite(RELAY2,RELAY_ON);
    delay(interRelayDelay);
    digitalWrite(RELAY3,RELAY_ON);
    delay(interRelayDelay);
  } else if ( state == 0 ) {
    digitalWrite(RELAY0,RELAY_OFF);
    digitalWrite(RELAY1,RELAY_OFF);
    digitalWrite(RELAY2,RELAY_OFF);
    digitalWrite(RELAY3,RELAY_OFF);
    digitalWrite(RELAY4,RELAY_OFF);
  }
  else {
    // do nothing
  }
 return 0;
}


// function to alter the inter channel delay during runtime
int setRelayDelay(String command) {

  // Get state from command
  int tmp_interRelayDelay = command.toInt();

  if ( tmp_interRelayDelay >= 0 && tmp_interRelayDelay <= 2000 ) {
      // allow delays between 0 and 2000 milliseconds
      Serial.print("set new inter relay delay: ");
      Serial.println(tmp_interRelayDelay);
      interRelayDelay = tmp_interRelayDelay;
  }
  else {
    // do nothing
  }
  return 0;
}


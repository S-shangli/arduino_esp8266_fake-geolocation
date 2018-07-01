/*
 * Fake GeoLocation for Arduino with ESP8266
 * 
 * Based on themarkszabo/FakeBeaconESP8266 (https://github.com/markszabo/FakeBeaconESP8266)
 * More info: http://nomartini-noparty.blogspot.com/2016/07/esp8266-and-beacon-frames.html
 * More about beacon frames: https://mrncciew.com/2014/10/08/802-11-mgmt-beacon-frame/
 * 
 */

#include <ESP8266WiFi.h>

extern "C" {
  #include <user_interface.h>
}

uint8  beacon_interval_ms = 100;

char wifissid[][32] = {
  "SSID1",
  "SSID2",
  "SSID3",
  "SSID4",
  "SSID5"
};
uint8 macaddr[][6] = {
  {0x12,0x34,0x56,0x78,0x9a,0xbc},
  {0x34,0x56,0x78,0x9a,0xbc,0xde},
  {0x56,0x78,0x9a,0xbc,0xde,0xf0},
  {0x78,0x9a,0xbc,0xde,0xf0,0x12},
  {0x9a,0xbc,0xde,0xf0,0x12,0x34}
};
float txpwr[]   = { 100.0 ,  75.0 ,  50.0 ,  25.0 ,   0.1 }; // % 
uint8 channel[] = {   1   ,   3   ,   5   ,   7   ,   9   }; // ch

uint8 entry_num = sizeof(channel)/sizeof(uint8);



void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("mac address entry : ");Serial.print(entry_num);Serial.print("\n");
  for(uint8 i=0; i < entry_num; i++){
    Serial.print("mac");Serial.print(i+1);Serial.print(": ");
    Serial.print(macaddr[i][0],HEX);Serial.print("-"); Serial.print(macaddr[i][1],HEX);Serial.print("-");
    Serial.print(macaddr[i][2],HEX);Serial.print("-"); Serial.print(macaddr[i][3],HEX);Serial.print("-");
    Serial.print(macaddr[i][4],HEX);Serial.print("-"); Serial.print(macaddr[i][5],HEX);
    Serial.print("  pwr:");Serial.print(txpwr[i]);Serial.print("%");
    Serial.print("  ch:");Serial.print(channel[i]);
    Serial.print("  SSID:");Serial.print(wifissid[i]);Serial.print("\n");
  }

  delay(500);

  Serial.print("set STATION_MODE ... ");
  wifi_set_opmode(STATION_MODE);
  Serial.print("done\nset PROMISCUOUS ... ");
  wifi_promiscuous_enable(1); 
  Serial.print("done\nStart disturbing\n");
}


void loop() {

  // send beacon
  for(uint8 i=0; i < entry_num; i++){
    sendBeacon(wifissid[i],channel[i],macaddr[i],txpwr[i]);
  }

  // wait beacon interval (roughly)
  delay( beacon_interval_ms - entry_num*2 );

}

void sendBeacon(char* ssid, uint8 ch, uint8* mac, float pwr) {
    long ts_us = micros();
    wifi_set_channel(ch);

    uint8_t packet[128] = { 
        /* 0*/  0x80, 0x00, //Frame Control 
        /* 2*/  0x00, 0x00, //Duration
        /* 4*/  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //Destination address 
        /*10*/  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //Source address - overwritten later
        /*16*/  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //BSSID          - overwritten later
        /*22*/  0xf0, 0xff,                         //Seq-ctl(dummy num)
        //Frame body starts here
        /*24*/  0x55, 0x55, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00, //timestamp - overwritten later
        /*32*/  0xFF, 0x00,                         //Beacon interval - overwritten later
        /*34*/  0x31, 0x04,                         //Capability info
        /* SSID */
        /*36*/  0x00                                // Declaration SSID tag
    };
    uint8_t postSSID[13] = {
        /* 38+ssidLen */ 0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, //supported rate
        /* 48+ssidLen */ 0x03, 0x01, 0x04           //DSSS (Current Channel) - overwritten later
    };

    // set SSID length
    int ssidLen = strlen(ssid);
    packet[37] = ssidLen;

    // set SSID name
    for(int i = 0; i < ssidLen; i++) {
      packet[38+i] = ssid[i];
    }

    // concat packet[] + postSSID[]
    for(int i = 0; i < 12; i++) {
      packet[38 + ssidLen + i] = postSSID[i];
    }

    // set SRC mac address and BSSID
    packet[10] = packet[16] = mac[0];
    packet[11] = packet[17] = mac[1];
    packet[12] = packet[18] = mac[2];
    packet[13] = packet[19] = mac[3];
    packet[14] = packet[20] = mac[4];
    packet[15] = packet[21] = mac[5];

    // set timestamp
    packet[24] = (ts_us & 0x00000000000000ff) >> (0*8);
    packet[25] = (ts_us & 0x000000000000ff00) >> (1*8);
    packet[26] = (ts_us & 0x0000000000ff0000) >> (2*8);
    packet[27] = (ts_us & 0x00000000ff000000) >> (3*8);
    packet[28] = (ts_us & 0x000000ff00000000) >> (4*8);
    packet[29] = (ts_us & 0x0000ff0000000000) >> (5*8);
    packet[30] = (ts_us & 0x00ff000000000000) >> (6*8);
    packet[31] = (ts_us & 0xff00000000000000) >> (7*8);

    // set beacon interval
    packet[32] = (beacon_interval_ms & 0x00ff) >> 0;
    packet[33] = (beacon_interval_ms & 0xff00) >> 8;

    // set channel
    packet[50 + ssidLen] = ch;

    // set tx-power
    system_phy_set_max_tpw((uint8)(82.0*(pwr/100.0)));

    // get packet length
    int packetSize = 51 + ssidLen;

    // send beacon frame
    wifi_send_pkt_freedom(packet, packetSize, 0);

    // delay for stabilize(we need guard interval?)
    delay(2);
}

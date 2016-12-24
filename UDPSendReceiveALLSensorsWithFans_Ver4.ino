/* This is built to manage fans that eject humidity and gas from our baths 
 *  (and maybe kitchen??)
 *  
 *  Mark Bartok Jan 2017
 * 
 */


#include <SPI.h>                  // needed for Arduino versions later than 0018
#include <Ethernet.h>             // Ethernet card support
#include <EthernetUdp.h>          // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <DHT.h>                  // Hunidity & temp sensor library

// DHT SENSORS ----------------------------------------
  // Global SENSOR declaration
  #define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321

  // Constant for the number of DHT sensors
  #define dhtSensorCount 5

  // connect probe to pin -- Argument for  library constructor below
  #define DHTP0 2      
  #define DHTP1 3
  #define DHTP2 7
  #define DHTP3 5
  #define DHTP4 6

  // Initialize DHT sensors.
  DHT dht0(DHTP0, DHTTYPE);           // Indoor reference 
  DHT dht1(DHTP1, DHTTYPE);           // Master bath
  DHT dht2(DHTP2, DHTTYPE);           // Girls bath
  DHT dht3(DHTP3, DHTTYPE);           // Playroom bath
  DHT dht4(DHTP4, DHTTYPE);           // Attic
  
  // Arrays to hold sensor readings
  float dhtHumidityReading[dhtSensorCount] = { 0 };   // Initialize as ZERO values
  float dhtTempReading[dhtSensorCount] = { 0 };       //

// ---- End DHT Sensors --------------------------------

// Gas Sensors -----------------------------------------
  #define gasSensorCount 3
  int gasReading[gasSensorCount] = { 0 };           // Initialize as ZERO values 

// ----- End Gas Sensors ------------------------------


// FANS -----------------------------------------------
  #define fanRelayCount 5
  
  #define pinFan0 A0                  // MasterBath shower
  #define pinFan1 A1                  // MasterBath toilet
  #define pinFan2 A2                  // Girls Bath
  #define pinFan3 A3                  // Playroom Bath
  #define pinFan4 A4                  // PowderRoom (GAS sensor only)
  
  // Array to hold fan status
  int fanStatus = 0;
  int fanStatusArray[] = {0};         // Reset all to OFF

  // Fan trigger points
  #define hiTrigger 65
  #define loTrigger 45


  
// Initialize status as OFF
  /* void setUpFans(byte fanStatus[]) {
    for (int i = 0; i <=4; i++){
      fanStatus[i] = 0;
      fanStatus[0] = 1;             // debug
      return fanStatus[];
    }
  }
  */
// ----- END FANS  ------------------------------------


/* -----  UDP COMMUNICATIONS SETUP ----------------------------------------------------------------
 UDPSendReceiveString:
 This sketch receives UDP message strings, prints them to the serial port
 and sends an "acknowledge" string back to the sender

 A Processing sketch is included at the end of file that can be used to send
 and received messages for testing with a computer.

 created 21 Aug 2010
 by Michael Margolis

 This code is in the public domain.
 */

 
    // Enter a MAC address and IP address for your controller below.
    // The IP address will be dependent on your local network:
    byte mac[] = {0xDA, 0xAB, 0xBC, 0xED, 0xFE, 0xEF};
    byte gateway[] = {192, 168, 5, 1};
    byte ip[] = {192, 168, 5, 240};
    
    unsigned int localPort = 8888;      // local port to listen on
    
    // buffers for receiving and sending data
    char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
    char  ReplyBuffer[] = "acknowledged";       // a string to send back
    // char repBuffer[] = "";                      // data out placeholder
    
    // An EthernetUDP instance to let us send and receive packets over UDP
    EthernetUDP Udp;
    
    // Make it more friendly?
    String stillhereReply = "Hello / Response#: ";  // Does This need to be a String?
    
    
    // ----------------------------------------------------------------------

    
void setup() {
  // Configure Analog Fan pins for output
  pinMode (A0, OUTPUT);
  pinMode (A1, OUTPUT);
  pinMode (A2, OUTPUT);
  pinMode (A3, OUTPUT);
  pinMode (A4, OUTPUT);
  
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip, gateway);
  Udp.begin(localPort);

  Serial.begin(9600);

  Serial.print("IP = ");
  Serial.println(Ethernet.localIP());

  // Here are the Sensor routines

      Serial.println("NEW Humidity Test!");
      //  takeAction (-1,0);  //MAKE SURE that the damper Servo is SHUT

      // Placeholder for the FAN routines...  use the LED
      // initialize digital pin 13 as an output.
      pinMode(13, OUTPUT);

      
}
// ------------------------------------------------------------------------

void loop() {
  // Poll the sensors and build variables 
     // Read the Temperature and Humidity
      dhtHumidityReading[0] = dht0.readHumidity();
      dhtHumidityReading[1] = dht1.readHumidity();
      dhtHumidityReading[2] = dht2.readHumidity();
      dhtHumidityReading[3] = dht3.readHumidity();
      dhtHumidityReading[4] = dht4.readHumidity();      
      
     // Read temperature as Fahrenheit (isFahrenheit = true); (blank) for Celcius is default
      dhtTempReading[0] = dht0.readTemperature(true);
      dhtTempReading[1] = dht1.readTemperature(true);  
      dhtTempReading[2] = dht2.readTemperature(true);
      dhtTempReading[3] = dht3.readTemperature(true);
      dhtTempReading[4] = dht4.readTemperature(true);
  
     // BUILD OUTPUT STREAM


     // TEMPS
     String outputT = "";                       // outputT is the string of temps to report
     
     for (int i=0; i< dhtSensorCount; i++) { 
        outputT = outputT + "t" +=i;
        outputT = outputT + ":"; 
        outputT = outputT +=dhtTempReading[i];
        outputT = outputT + ":"; 
                
      }  // --- End TEMP string builder

     
     //Now Humidity
     String outputH = "";                       // outputH is the string of humidity readings to report
     
     for (int i=0; i< dhtSensorCount; i++) { 
        outputH = outputH + "h" +=i;
        outputH = outputH + ":"; 
        outputH = outputH +=dhtHumidityReading[i];
        outputH = outputH + ":"; 
     }   // --- End HUMIDITY string builder

  // ----  OUTPUT STRING CONSTRUCTION  T:temp; H:humidity; G:gas
  char repBufferT[48];
  outputT.toCharArray(repBufferT,48);
  char repBufferH[48];
  outputH.toCharArray(repBufferH,48);
  // -----  END CONSTRUCTOR ------------------------------------

  // Send something to connecting PC
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(repBufferT);
  Udp.write(":");
  Udp.write(repBufferH);
  
  // Udp.write(intCounter);
  Udp.endPacket();

  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i = 0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    // send a reply to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
  } // ------------ End packetSize

  
  delay(5000);
    //Serial.print ("Fan0 stat: ");
    //Serial.println (fanStatusArray[0]);
    printFanPower(dhtHumidityReading);
    Serial.println ("------------");
   
/*
      for (int i = 0; i <=4; i++){
        fanStatus = fanStatusArray[i];       
        Serial.println(fanStatus);
      }
    //switchFanPower(fanStatus);
*/
  
} // -------------------  End Of LOOP

  // FUNCTIONS  
    
    int printFanPower(float dhtHumidityReading[]) { 
      // Loop thru all readings

      int fanStatus = 0;

      for (int i=0; i <dhtSensorCount ; i++) {
      // First compare value to hiTrigger 
        if (dhtHumidityReading[i] > hiTrigger) {          // if hiTrigger exceeded
          fanStatus = 1;                                  // turn the Fan on
        }
        else if ((fanStatusArray[i] == 1) &&              // if Fan on
                (dhtHumidityReading[i] > loTrigger)) {    // and above cutoff 
          fanStatus = 1;                                  // leave it on
        } 
          // default to OFF
        else {
          fanStatus = 0;          
        } // If end
        fanStatusArray[i] = fanStatus;

        // Debug printing
        // fanStatus = fanStatusArray[i];
        Serial.print("Fan");
        Serial.print (i);
        Serial.print (": ");
        Serial.println (fanStatus);

      }   // -- FOR end

    }     // -----  End of FAN CALCs -----
       
     
     


    
  
  

/*
 
 WiFi Web Server for Servo Control
 AP name     : Laser Lab / defined
 Webpage     : IP address of Redbear
 Password    : 00000000
*/

#ifndef __CC3200R1M1RGC__
// Do not include SPI for CC3200 LaunchPad
#include <SPI.h>
#endif
#include <WiFi.h>
#include <Servo.h>
//#include <IPAddress.h>
#include "utility/simplelink.h"




#define rightServo 6
#define leftServo 5
#define upLED 7
#define leftLED 9
#define rightLED 12  
#define downLED 13

#define HOME_AP "Tiensoon"
#define HOME_PW "0000000000"
#define JX_AP  "Winnie_2G"
#define JX_PW "tiantiankaixin"
#define PHONE_AP "Tien Long"
#define PHONE_PW "bendanben"
#define NUS_AP "NUS"
#define PORT 80




WiFiServer server(PORT); // port 80..

Servo leftDrive;  // create servo object to control a servo
Servo rightDrive; //another servo object for the left side


unsigned int n;
unsigned int ret;



void wifi_Setup()
{
    int networkAvailable = 0;
    Serial.begin(115200);
    //int status = WL_IDLE_STATUS; // Init WiFi as Idle state (not connected to network)
    WiFi.init(); // initialise WiFi
   
    networkAvailable = scanNetwork();
    
    // Attempt to connect to specified network
    if (connectNetwork(networkAvailable) == 0)
    {
        // unable to find the specified network
        ap_Setup(); // Setup own access point.
    } 
    
    server.begin();                           // start the web server at port (80)

}

// return numbers of network available
int scanNetwork()
{
    return WiFi.scanNetworks();
}

// return 1 if connected to network, 0 if unable to find suitable network to connect
char connectNetwork(int networkAvailable)
{
   for (int network = 0; network < networkAvailable; network++)
   {
       if(strstr(WiFi.SSID(network),  PHONE_AP))
       {
           connect_AP(WiFi.SSID(network), PHONE_PW);
           return 1;
       }
       
       if(strstr(WiFi.SSID(network),  HOME_AP))
       {
          connect_AP(WiFi.SSID(network), HOME_PW);
          return 1;
       }
       
    
       if(strstr(WiFi.SSID(network),  JX_AP))
       {
          connect_AP(WiFi.SSID(network), JX_PW);
          return 1;
       }
       
       
       /*
       if(strstr(WiFi.SSID(network),  NUS_AP))
       {
          connect_AP(WiFi.SSID(network), NUS_PW);
          return 1;
       }
       */
   }
   
   return 0;
}


// setup redbear as access point for others to connect.
void ap_Setup()
{
    char int_ssid[] = "Laser Robot";  // Network name for WiFi AP
    char int_password[] = "00000000"; // Password for WiFi AP
    
   
    WiFi.beginNetwork(int_ssid, int_password);  // Start WiFi in (AP mode) with WPA network 
    
    Serial.println("\n\nSeting up redbear as access point");
  
    while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
    }  
  
    printWiFiStatus();
  
}


void connect_Home_AP()
{
    char ext_ssid[] = "Tiensoon";
    char ext_password[] = "0000000000"; // Password for WiFi AP
    
    connect_AP(ext_ssid, ext_password);
}

// Connect to home access point.
void connect_AP(char* ext_ssid, char* ext_password) 
{
     WiFi.begin(ext_ssid, ext_password); // connect to external AP
    
    // attempt to connect to Wifi network:
    Serial.print("\n\nAttempting to connect to Network named: ");
    // print the network name (SSID);
    Serial.println(ext_ssid); 
    
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.begin(ext_ssid, ext_password);
    
    while ( WiFi.status() != WL_CONNECTED) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
    }
    
    Serial.println("\nYou're connected to the network");
    Serial.println("Waiting for an ip address");
  
    while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
    }  
    
    printWiFiStatus();
}

void printWiFiStatus()
{
    Serial.println("\n\n----- WiFi Status -----");
    
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}


// setup mDNS (multicast DNS) to advertise 
void mDNS_Setup()
{
    // setup mdns service    
    int retVal = 0;
   
    const signed char serviceName[] = "laser-robot._http._tcp.local"	;
    unsigned char nameLen = 29;
    const signed char serviceText[] = "project=FYP"	;
    unsigned char textLen = 12;
    uint16_t port = PORT;  // HTTP Port
    uint32_t TTL = 1800; // Update every 30 min
    uint32_t OPTION = 0x01; // Service should be unique.
    unsigned char retLen = 12;
    unsigned char retText[12] = "laser-robot";
    
  
    sl_NetAppSet (SL_NET_APP_DEVICE_CONFIG_ID, NETAPP_SET_GET_DEV_CONF_OPT_DEVICE_URN, retLen, retText); // set device name as laser-robot => laser-robot.local
    
    retVal = sl_NetAppStart(SL_NET_APP_MDNS_ID); // start MDNS 
    
    Serial.print("ret value is :");
    Serial.println(retVal);
    
    sl_NetAppMDNSUnRegisterService(serviceName, nameLen);   // Unregister previous services first to overwrite..             
    retVal = sl_NetAppMDNSRegisterService(serviceName, nameLen, serviceText, textLen, port, TTL, OPTION); // register service.
  
    Serial.print("ret value is :");
    Serial.println(retVal);   
}


void setup() 
{
  
    wifi_Setup();
    mDNS_Setup();
        
    stop(); // stop from moving
}


void loop()
{
  int i = 0;
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) 
  {                                         // if there is a client,
    char buffer[150] = {0};                 // make a buffer to hold incoming data
    while (client.connected()) 
    {                                       // loop while the client's connected
      if (client.available()) 
      {                                    
          char c = client.read();           // read a byte from connected client..
          
          Serial.print(c);
          
          if (c == '\n') 
          {                                // if the byte is a newline character
                                          // if the current line is blank, it signify the end of the client HTTP request, so send a response:
          if (strlen(buffer) == 0) 
          {
            
            displayWebpage(client); // send HTML code for client browser to display
            Serial.println("Web displayed");
 
            // break out of the while loop:
            break;
          }
          else 
          {                              // if you got a newline, then clear the buffer:
              memset(buffer, 0, 150);
              i = 0;
          }
        }
        else if (c != '\r')
        {                                // if you got anything else but a carriage return character,
            buffer[i++] = c;             // add it to the end of the currentLine
        }

        // Check to see client request
        if (endsWith(buffer, "GET /U")) 
        {  
                     Serial.println("Forw displayed");
 
          driveForward();
        }
        if (endsWith(buffer, "GET /D")) 
        {  
          driveBackward();
        }
        if (endsWith(buffer, "GET /L"))
        {  
          turnLeft();
        }
        if (endsWith(buffer, "GET /R")) 
        {  
          turnRight();
        }
        if (endsWith(buffer, "GET /S")) 
        {  
          stop();
        }
        
      }
    }
    
    // close the connection:
    client.stop();
  }
}

void displayWebpage(WiFiClient client)
{
  
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK ");
            client.println("Content-type:text/html");
            client.println();
                        
//          client.println("<html><head><title>LASER</title></head><body>LAB</body></html>");

            client.println("<html><head><title>Laser Robot Control</title></head><body align=center>");
            client.println("<h1 align=center><font color=\"red\">Laser MicroRobot WiFi Test</font></h1>");
            
           
            client.println("<p align=center><font size =\"5 px\"><font color=\"blue\">Servo Control</font></p>");
            
            client.println("<b1 style=\"margin-left: 4px\"><button onclick=\"location.href='/U'\">UP</button></b1><br><br>");
            client.print("<button onclick=\"location.href='/L'\">LEFT</button>");
            client.print("<b1 style=\"margin-left: 50px\"><button onclick=\"location.href='/S'\">STOP</button>");
         
            client.println("<b1 style=\"margin-left: 50px\"><button onclick=\"location.href='/R'\">RIGHT</button></b1><br><br>");
            client.println("<b1 style=\"margin-left: 3px\"><button onclick=\"location.href='/D'\">DOWN</b1></button>");
            


            
            // The HTTP response ends with 2 blank line:
            client.println();
            client.println();
}

//String compare 
boolean endsWith(char* inString, char* compString) 
{
  int compLength = strlen(compString);
  int strLength = strlen(inString);
  
  //compare the last "compLength" values of the inString
  int i;
  for (i = 0; i < compLength; i++) 
  {
    char a = inString[(strLength - 1) - i];
    char b = compString[(compLength - 1) - i];
    if (a != b) 
    {
        return false;
    }
  }
  
  return true;
}



//turns left about 90 degrees
void turnLeft()
{
  start();
  leftDrive.write(0);
  rightDrive.write(0);
}

//turns right about 90 degrees
void turnRight()
{
  start();
  leftDrive.write(180);
  rightDrive.write(180);
}


//turns left about 90 degrees
void stop()
{
  leftDrive.detach();
  rightDrive.detach();
}



//drives straight for 1 second
void driveForward()
{
  start();
  leftDrive.write(180);
  rightDrive.write(0);
}

//drives straight backward for 1 second
void driveBackward()
{
  start();
  leftDrive.write(0);
  rightDrive.write(180);
}


void start()
{
    leftDrive.attach(leftServo);  // attach left servo
    rightDrive.attach(rightServo);  // attach right servo
}


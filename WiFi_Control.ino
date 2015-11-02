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



#define INIT 0
#define LEFT    1
#define RIGHT   2
#define UP      3
#define DOWN    4
#define STOP    5



#define rightServo 6
#define leftServo 5
#define upLED 7
#define leftLED 9
#define rightLED 12  
#define downLED 13

#define HOME_AP "Tiensoon"
#define HOME_PW "0000000000"
#define EXT_AP  "Toh"
#define EXT_PW "0967237482"
#define PHONE_AP "Tien Long"
#define PHONE_PW "bendanben"
#define NUS_AP "NUS"
#define PORT 80


#define KEY_PRESSED 0
#define INITIAL 1




WiFiServer server(PORT); // port 80..

Servo leftDrive;  // create servo object to control a servo
Servo rightDrive; //another servo object for the left side
volatile uint8_t browserState = INITIAL; // determine if it is a new browser..
uint8_t direction = INIT; // determine the direction of the robot
uint8_t prevDirection = INIT; // determine the direction of the robot


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
       
    
       if(strstr(WiFi.SSID(network),  EXT_AP))
       {
          connect_AP(WiFi.SSID(network), EXT_PW);
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
    
    if(retVal != -6)
    {
      Serial.println("Mdns Failed to Start");
      return;
    }
    
    sl_NetAppMDNSUnRegisterService(serviceName, nameLen);   // Unregister previous services first to overwrite..             
    retVal = sl_NetAppMDNSRegisterService(serviceName, nameLen, serviceText, textLen, port, TTL, OPTION); // register service.
  
    
    if(retVal == 0)
    {
      Serial.println("Mdns Started Successfully");
    }
    else
    {
      Serial.println("Mdns Registration Failed");
    }
    
    
    
   // unsigned char Len = 60;
   // unsigned char Text[60] = "laser-robot";
    
    
   //Serial.print("Web: ");

   //sl_NetAppGet(SL_NET_APP_DEVICE_CONFIG_ID, NETAPP_SET_GET_DEV_CONF_OPT_DOMAIN_NAME, &Len, Text);
   //Serial.println((char*)Text);
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
          
          if (c == '\n') 
          {                                // if the byte is a newline character
                                          // if the current line is blank, it signify the end of the client HTTP request, so send a response:
          if (strlen(buffer) == 0) 
          {
            if(browserState == INITIAL)
            {
              displayWebpage(client); // send HTML code for client browser to display
              //Serial.println("Web displayed");
              // break out of the while loop:
              break;
            }
            else
            {
              remainAtWebpage(client); // send HTML response 204 (Remain at web)
              //Serial.println("Remain @ Web");
              browserState = INITIAL;
              break; // break out of the while loop
            }
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
          browserState = KEY_PRESSED;
          driveForward();
        }
        
        if (endsWith(buffer, "GET /D")) 
        {  
          browserState = KEY_PRESSED;
          driveBackward();
        }
        
        if (endsWith(buffer, "GET /L"))
        {  
          browserState = KEY_PRESSED;
          turnLeft();
        }
        
        if (endsWith(buffer, "GET /R")) 
        {  
          browserState = KEY_PRESSED;
          turnRight();
        }
        
        if (endsWith(buffer, "GET /S")) 
        {  
          browserState = KEY_PRESSED;
          stop();
        }
        
        
      }
    }
    
    // close the connection:
    client.stop();
  }
}


void remainAtWebpage(WiFiClient client)
{
           client.println("HTTP/1.1 204 OK ");
           // The HTTP response ends with 2 blank line:
           client.println();
           client.println();  
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
            client.print("<b1 style=\"margin-left: 60px\"><button onclick=\"location.href='/S'\">STOP</button>");
         
            client.println("<b1 style=\"margin-left: 50px\"><button onclick=\"location.href='/R'\">RIGHT</button></b1><br><br>");
            client.println("<b1 style=\"margin-left: 3px\"><button onclick=\"location.href='/D'\">DOWN</b1></button>");
            
            printScript(client);

            
            // The HTTP response ends with 2 blank line:
            client.println();
            client.println();
}

void printScript(WiFiClient client)
{
  client.println("<script>");
  

  
  // Start KEY DOWN EVEN
  client.println("document.onkeydown = function(e) {");
  client.println("switch (e.keyCode) {");
  
  client.println("case 37:");
  //client.println("alert('left');");
  client.println("window.location.href ='/L'");
  client.println("break;");

  client.println("case 38:");
  //client.println("alert('up');");
  client.println("window.location.href ='/U'");
  client.println("break;");

  client.println("case 39:");
  //client.println("alert('right');");
  client.println("window.location.href ='/R'");
  client.println("break;");

  client.println("case 40:");
  //client.println("alert('down');");
  client.println("window.location.href ='/D'");
  client.println("break;");
  
  client.println("}");
  client.println("};");
  // END KEY DOWN EVENT


 // Start KEY UP EVEN
  client.println("document.onkeyup = function(f) {");
  client.println("switch (f.keyCode) {");
  
  client.println("case 37:");
  client.println("window.location.href ='/S'");
  client.println("break;");
  
  client.println("case 38:");
  client.println("window.location.href ='/S'");
  client.println("break;");
  
  
  client.println("case 39:");
  client.println("window.location.href ='/S'");
  client.println("break;");
  
  client.println("case 40:");
  client.println("window.location.href ='/S'");
  client.println("break;");
  
  client.println("}");
  client.println("};");
  // END KEY UP EVENT

  

  client.println("</script>");
  client.println("</html>");

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
  if(direction != LEFT)
  {
    start();
    leftDrive.write(0);
    rightDrive.write(0);
    direction = LEFT;
    
    Serial.println("Left");
  }

}

//turns right about 90 degrees
void turnRight()
{
  if(direction != RIGHT)
  {
    start();
    leftDrive.write(180);
    rightDrive.write(180);
    direction = RIGHT;
    
    Serial.println("Right");
  }
}


//turns left about 90 degrees
void stop()
{
  if(direction != STOP)
  {
    leftDrive.detach();
    rightDrive.detach();
    direction = STOP;
    
    Serial.println("Stop");
  }
}



//drives straight for 1 second
void driveForward()
{
  if(direction != UP)
  {
    start();
    leftDrive.write(180);
    rightDrive.write(0);
    direction = UP;
    
    Serial.println("Forward");
  }
}

//drives straight backward for 1 second
void driveBackward()
{
  if(direction != DOWN)
  {
    start();
    leftDrive.write(0);
    rightDrive.write(180);
    direction = DOWN;
    
    
    Serial.println("Backward");
  }
}


void start()
{
    leftDrive.attach(leftServo);  // attach left servo
    rightDrive.attach(rightServo);  // attach right servo
}


// not tested... try at nus?
int beginWPAEnterprise(char* ssid, char *passphrase, char *username)
{

    //
    // Set IP address configuration to DHCP if needed
    //
    bool init_success = WiFiClass::init();
    if (!init_success) {
        return WL_CONNECT_FAILED;
    }

    WiFi.setIpDefaults();

    //
    //initialize the simplelink driver and make sure it was a success
    //
    sl_WlanPolicySet(SL_POLICY_CONNECTION , SL_CONNECTION_POLICY(1,1,0,0,0), 0, 0);

    //
    //get name length and set security type to WPA
    //add passphrase and keylength to security parameters
    //
    int NameLen = strlen(ssid);
    SlSecParams_t SecParams = {0};
    SecParams.Type = SL_SEC_TYPE_WPA_ENT;
    SecParams.Key = (signed char *)passphrase;
    SecParams.KeyLen = strlen(passphrase);
    
    SlSecParamsExt_t eapParams = {0};
    eapParams.EapMethod = SL_ENT_EAP_METHOD_PEAP0_MSCHAPv2;
    eapParams.User = (signed char *)username;
    eapParams.UserLen = strlen(username);
    eapParams.AnonUserLen = 0;
    
    //
    //connect to the access point (non enterprise, so 5th argument is NULL)
    //also mac address parameters set as null (3rd argument)
    //
    int iRet = sl_WlanConnect((signed char *)ssid, NameLen, NULL, &SecParams, &eapParams);

    //
    //return appropriate status as described by arduino wifi library
    //the WiFiClass:WiFi_status is handled by the WlanEvenHandler
    //in SimpleLinkCallbacks.cpp. However, if iRet < 0, there was an error
    //
    if (iRet == 0) {
        sl_WlanProfileAdd((signed char *)ssid, NameLen, 0, &SecParams, &eapParams, 6, 0);
        return WiFi.status();
    } else {
        return WL_CONNECT_FAILED;
    }
}




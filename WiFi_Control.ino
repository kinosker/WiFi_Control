

/*
 
 WiFi Web Server for Servo Control
 WiFi name   : RedBearLab CC3200
 Webpage     : IP address of Redbear
 Password     : 00000000
*/

#ifndef __CC3200R1M1RGC__
// Do not include SPI for CC3200 LaunchPad
#include <SPI.h>
#endif
#include <WiFi.h>
#include <Servo.h>
//#include <IPAddress.h>
//#include "utility/simplelink.h"


WiFiServer server(80); // port 80..

Servo leftDrive;  // create servo object to control a servo
Servo rightDrive; //another servo object for the left side


unsigned int n;
unsigned int ret;


#define rightServo 6
#define leftServo 5
#define upLED 7
#define leftLED 9
#define rightLED 12  
#define downLED 13
//#define SERVICE_NAME "AAA._uart._tcp.local"

void wifi_Setup()
{
    Serial.begin(115200);
    //int status = WL_IDLE_STATUS; // Init WiFi as Idle state (not connected to network)
    
    
    char ssid[] = "Laser Robot";  // Network name for WiFi AP
    char password[] = "00000000"; // Password for WiFi AP
    IPAddress homeIP(192, 168, 1, 1); // IP address for home page
  
  
    WiFi.config(homeIP);
    WiFi.beginNetwork(ssid, password);  // Start WiFi in AP mode with WPA network

    while (WiFi.localIP() == INADDR_NONE); // wait for IP address to setup
    
    server.begin();                           // start the web server at port (80)

    Serial.print("Ip is :");
    Serial.println(WiFi.localIP());
    
    
}

void mDNS_Setup()
{
    // setup mdns service 
    
    int retVal = 0;
   
    const signed char serviceName[] = "Robot._http._tcp.local"	;
    unsigned char nameLen = 23;
    const signed char serviceText[] = "NA"	;
    unsigned char textLen = 3;
    uint16_t port = 80;  // HTTP Port
    uint32_t TTL = 1800; // Update every 30 min
    uint32_t OPTION = 0x01; // Service should be unique.
   
    
    retVal = sl_NetAppStart(SL_NET_APP_MDNS_ID);
    
    Serial.print("ret value is :");
    Serial.println(retVal);
    
    sl_NetAppMDNSUnRegisterService(serviceName, nameLen);                
    retVal = sl_NetAppMDNSRegisterService(serviceName, nameLen, serviceText, textLen, port, TTL, OPTION);
  
    Serial.print("ret value is :");
    Serial.println(retVal);
    
}
void setup() 
{
  
    wifi_Setup();
    mDNS_Setup();
    
    pinMode(upLED, OUTPUT);      // set the LED pin mode
    pinMode(downLED, OUTPUT);      // set the LED pin mode
    pinMode(leftLED, OUTPUT);      // set the LED pin mode
    pinMode(rightLED, OUTPUT);      // set the LED pin mode

    digitalWrite(upLED, LOW);  
    digitalWrite(rightLED, LOW);
    digitalWrite(leftLED, LOW);
    digitalWrite(downLED, LOW);
    
    pinMode(leftServo, OUTPUT);      // set the LED pin mode
    pinMode(rightServo, OUTPUT);      // set the LED pin mode
    digitalWrite(leftServo, LOW);
    digitalWrite(rightServo, LOW);
    
    //start();
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
                                          // if th  e current line is blank, it signify the end of the client HTTP request, so send a response:
          if (strlen(buffer) == 0) 
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK ");
            client.println("Content-type:text/html");
            client.println();
            
//            // the content of the HTTP response follows the header:
//            
              client.println("<html><head><title>LASER</title></head><body>LAB</body></html>");
//            client.println("<h1 align=center><font color=\"red\">Laser MicroRobot WiFi Test</font></h1>");
//            




//           
//            client.println("<p align=center><font size =\"5 px\"><font color=\"blue\">Servo Control</font></p>");
//            
//            client.println("<b1 style=\"margin-left: 4px\"><button onclick=\"location.href='/U'\">UP</button></b1><br><br>");
//            client.print("<button onclick=\"location.href='/L'\">LEFT</button>");
//            client.print("<b1 style=\"margin-left: 50px\"><button onclick=\"location.href='/S'\">STOP</button>");
//         
//            client.println("<b1 style=\"margin-left: 50px\"><button onclick=\"location.href='/R'\">RIGHT</button></b1><br><br>");
//            client.println("<b1 style=\"margin-left: 3px\"><button onclick=\"location.href='/D'\">DOWN</b1></button>");
//            
//            client.println();            
            
            // The HTTP response ends with another blank line:
            client.println();
            client.println();
  
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
          digitalWrite(upLED, HIGH);
          digitalWrite(rightLED, LOW);
          digitalWrite(leftLED, LOW);
          digitalWrite(downLED, LOW);
          driveForward();
        }
        if (endsWith(buffer, "GET /D")) 
        {  
          digitalWrite(upLED, LOW);
          digitalWrite(rightLED, LOW);
          digitalWrite(leftLED, LOW);
          digitalWrite(downLED, HIGH);
          driveBackward();
        }
        if (endsWith(buffer, "GET /L"))
        {  
          digitalWrite(upLED, LOW);
          digitalWrite(rightLED, LOW);
          digitalWrite(leftLED, HIGH);
          digitalWrite(downLED, LOW);
          turnLeft();
        }
        if (endsWith(buffer, "GET /R")) 
        {  
          digitalWrite(upLED, LOW);
          digitalWrite(rightLED, HIGH);
          digitalWrite(leftLED, LOW);
          digitalWrite(downLED, LOW);
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
//  delay(600);
//  stop();
}

//turns right about 90 degrees
void turnRight()
{
  start();
  leftDrive.write(180);
  rightDrive.write(180);
//  delay(600);
 // stop();
}


//turns left about 90 degrees
void stop()
{
  leftDrive.detach();
  rightDrive.detach();
  
  //leftDrive.write(90);
  //rightDrive.write(90);
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


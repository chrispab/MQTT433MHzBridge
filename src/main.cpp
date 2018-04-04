//work only using wifiesp lib as connection to net

//#include <SPI.h>
#include <WiFiEsp.h>
#include <NewRemoteTransmitter.h>

#include <PubSubClient.h>

//forward decs
void printWifiStatus();
boolean processRequest(String &getLine);
void sendResponse(WiFiEspClient client);
void listenForClients(void);
void LEDBlink(int LEDPin, int repeatNum);
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();

char ssid[] = "notwork";                              // your network SSID (name)
char pass[] = "a new router can solve many problems"; // your network password
//int keyIndex = 0;                                     // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

#include "SoftwareSerial.h"
SoftwareSerial ESPSerial(7, 6); // RX, TX

//WiFiEspServer server(80);

IPAddress mqttserver(192, 168, 0, 200);

//WiFiEspClient client = server.available();
//EthernetClient ethClient;

NewRemoteTransmitter transmitter(282830, 4);
byte socket = 3;
bool state = false;

uint8_t socketNumber = 0;

#define LEDPIN 5
//Supported baud rates are 300, 600, 1200, 2400, 4800, 9600, 14400,
//19200, 28800, 31250, 38400, 57600, and 115200.
#define ESP_BAUD 9600
#define CR Serial.println()

//#define subscribeTopic "Outside_Sensor/#"

#define subscribeTopic "433Bridge/cmnd/#"

WiFiEspClient WiFiEClient;
PubSubClient psclient(mqttserver, 1883, callback, WiFiEClient);

void setup()
{ //Initialize serial monitor port to PC and wait for port to open:
    Serial.begin(115200);
    //Serial.begin(9600);

    pinMode(LEDPIN, OUTPUT); // set the LED pin mode

    // initialize serial for ESP module
    // TODO - try setting esp-01 baud rate higher for better response
    //find fastets baud rate soft serial can manage confortably
    //try with 31250 baud 8mhz xtal
    //try 57600 with 16mhz xtal

    //  To then change the esp-01 from default baud rate of 115200
    // to 57600 for current session
    // AT+UART_CUR=57600,8,1,0,0 – N0TE NO FLOW CTL – DIFF TO EXSPREIF EXAMPLE
    // AT+UART_CUR=57600,8,1,0,0
    // reopen serial terminal at 57600 and test
    // AT
    // AT+GMR

    //open at power up default BAUD rate - currently set to 9600 to
    // allow use of software serial to ESP-01 module
    ESPSerial.begin(ESP_BAUD);
    //get current baud for this session
    delay(40);

    Serial.println("Get startup BAUD rate ...");
    ESPSerial.println("AT+UART_CUR?");
    delay(40); //this delay exactly here appears to be reqd to read back info from esp
    while (ESPSerial.available())
    {
        String inData = ESPSerial.readStringUntil('\n');
        Serial.println("Got response from ESP-01: " + inData);
    }

    //set working baud for this session
    // Serial.println("Sending an AT command...");
    // ESPSerial.println("AT+UART_CUR?"); //ADD SET COMMAND
    // delay(30);
    // while (ESPSerial.available())
    // {
    //     String inData = ESPSerial.readStringUntil('\n');
    //     Serial.println("Got response from ESP-01: " + inData);
    // }
    //query current settings
    //Serial.println(ESPSerial.println("AT+UART_CUR?"));

    //////////////////////////////////////////////////////
    // ESPSerial.println("AT+UART_CUR=57600,8,1,0,0");
    //     ESPSerial.println("AT+UART_CUR=14400,8,1,0,0");
    // delay(300);
    // ESPSerial.end();
    // delay(300);
    // ESPSerial.begin(14400);
    // delay(3000);
    // Serial.println("ask AT-");
    // ESPSerial.println("AT");
    // delay(30);
    // while (ESPSerial.available())
    // {
    //     String inData = ESPSerial.readStringUntil('\n');
    //     Serial.println("atrst response: " + inData);
    // }

    //////////////////////////////////////////////////

    //show new working baud
    // Serial.println("ask ESP-01 for it's current baud rate---------");
    // ESPSerial.println("AT+UART_CUR?");
    // delay(30);
    // while (ESPSerial.available())
    // {
    //     String inData = ESPSerial.readStringUntil('\n');
    //     Serial.println("current working BAUD response from ESP-01: " + inData);
    // }
    // delay(1000);
    //set baud rate for this session to 57600 - not saved in flash
    // AT+UART_CUR=115200,8,1,0,3

    // initialize ESP module
    WiFi.init(&ESPSerial);
    // check for the presence of the shield:
    if (WiFi.status() == WL_NO_SHIELD)
    {
        Serial.println("WiFi shield not present");
        // don't continue:
        while (true)
            ;
    }

    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);

        // wait 10 seconds for connection:
        delay(5000);
    }
    //server.begin();
    // you're connected now, so print out the status:
    printWifiStatus();
    CR;
    //delay(5000);
    reconnect();
    // if (psclient.connect("ESP8266Client"))
    // {
    //     psclient.publish("outTopic", "hello world");
    //     //psclient.subscribe("Outside_Sensor/#");
    //     psclient.subscribe(subscribeTopic);
    //     //psclient.subscribe("");
    // }
}

void loop()
{
    if (!psclient.connected())
    {
        reconnect();
    }
    psclient.loop();
    // listenForClients();
}

void callback(char *topic, byte *payload, unsigned int length)
{
    // Power<x> 		Show current power state of relay<x> as On or Off
    // Power<x> 	0 / off 	Turn relay<x> power Off
    // Power<x> 	1 / on 	Turn relay<x> power On
    // handle message arrived mqtt pubsub

    // Serial.println("Rxed a mesage from broker : ");

    payload[length] = '\0';
    //String s = String((char *)payload);
    // //float f = s.toFloat();
    // Serial.print(s);
    // Serial.println("--EOP");

    CR;
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (uint8_t i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    CR;
    Serial.println();
    //search for a match to the incoming topic
    //form of Message "sonoff_FR/stat/POWER<n>" OFF
    // where <n> is the socket number 1-16
    //e.g topic = "433Bridge/cmnd/Power1", and payload = 1 or 0
    // either match whole topic string or trim off last 1or 2 chars and convert to a number
    //convert last 1-2 chars to socket number
    //get last char
    char lastChar = topic[strlen(topic) - 1]; //lst char will always be a digit char
    //see if last but 1 is also a digit char - ie number has two digits - 10 to 16
    char lastButOneChar = topic[strlen(topic) - 2];

    if ((lastButOneChar >= '0') && (lastButOneChar <= '9'))
    {                                                                    // is it a 2 digit number
        socketNumber = ((lastButOneChar - '0') * 10) + (lastChar - '0'); // calc actual int
    }
    else
    {
        socketNumber = (lastChar - '0');
    }

    socketNumber--;       // convert from 1-16 range to 0-15 range sendUnit uses
    uint8_t newState = 0; // default to off
    if ((payload[0] - '1') == 0)
    {
        newState = 1;
    }
    transmitter.sendUnit(socketNumber, newState);
    LEDBlink(LEDPIN, socketNumber); // blink socketNumber times

    digitalWrite(LEDPIN, newState);
    // if (!strcmp(topic, "433Bridge/cmnd/Power5"))
    // {
    //     digitalWrite(LEDPIN, 1); // GET /H turns the LED on
    //     delay(100);
    //     digitalWrite(LEDPIN, 0); // GET /H turns the LED on
    // }
    // last char
}

void reconnect()
{
    // Loop until we're reconnected
    while (!psclient.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (psclient.connect("ESP8266Client"))
        {
            Serial.println("connected to MQTT server");
            // Once connected, publish an announcement...
            psclient.publish("outTopic", "hello world");
            // ... and resubscribe
            //psclient.subscribe("inTopic");
            psclient.subscribe(subscribeTopic);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(psclient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void LEDBlink(int LPin, int repeatNum)
{
    for (int i = 0; i < repeatNum; i++)
    {
        digitalWrite(LPin, 1); // GET /H turns the LED on
        delay(50);
        digitalWrite(LPin, 0); // GET /H turns the LED on
        delay(50);
    }
}
//  void listenForClients(void)
//  {
//     // listen for incoming clients
//     WiFiEspClient client = server.available();

//     if (client) // keep checking for new client connection coming in
//     {
//         Serial.println("new client!!");
//         String currentLine = ""; // make a String to hold incoming data from the client
//         String request = "";     //storage for get request line
//         char c;
//         while (client.connected())
//         {
//             if (client.available())
//             {
//                 c = client.read(); //get next char rxed
//                 //think about storing line rather thatmn printing each char after rx
//                 Serial.write(c); //echo toserial monitor
//                 if (c == '\n')   //could be 1st-end of line  or 2nd - end of request \n in request
//                 {
//                     // if the current line is empty, means this is 2nd \n char in a row.
//                     // therefore that's the end of the client HTTP request
//                     if (currentLine.length() == 0)
//                     {
//                         //request has been rxed, see if we can process it
//                         processRequest(request);
//                         //send a response
//                         sendResponse(client);
//                         // break out of the while loop to terminate connection
//                         break;
//                     }
//                     else                                        // 1st \n rxed - end of current line - and currentLine is not empty
//                     {                                           //look to see if current line has  a GET request in it
//                         if (currentLine.indexOf("GET /") != -1) //last line was a get line
//                         {
//                             request = currentLine; // store current line in request as it is the GET line
//                         }
//                         currentLine = ""; // empty current line ready for next line rxed
//                     }
//                 }
//                 else if (c != '\r') // ignore if \r
//                 {
//                     // you've gotten a character on the current line - other than \n or \r
//                     currentLine += c; // add current rxed char it to the end of the currentLine
//                 }
//             }
//         }
//         // give the web browser time to receive the data
//         delay(20);

//         // close the connection:
//         client.stop();
//         Serial.println("client disconnected!!");
//     }
// }

void printWifiStatus()
{
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}

// boolean processRequest(String &getLine)
// {
//     char *command;
//     char *param;
//     char *value;
//     char myString[50];
//     const char s[] = " /?=&"; // delimeter chars for strtok
//     char *token;
//     char *ptr;
//     //long ret;
//     bool newState = false;

//     //now chck if a GET was received previously
//     //vars for sockt and state should be present
//     //if so then do socket control command
//     //poss url format
//     ///test/demo_form.php?name1=value1&name2=value2

//     //current call used is like this
//     //Switch TEST_433_Control "TEST_433_Control"  <switch> {http=">[ON:GET:http://192.168.0.230/45=ZON:on] >[OFF:GET:http://192.168.0.230/45=ZOFF:off]" }
//     //http://192.168.0.230/45=ZON

//     // format of expected url http://192.168.0.230/<command|device>/<id>/<state>
//     // e.g http://192.168.0.230/socket/12/1  - socket 12 (of 0-15), ON

//     // extract socket num and on or off param
//     if (getLine.indexOf("GET /") != -1)
//     //if (1)
//     { // get command detected
//         Serial.println("GET / - found in response");

//         strcpy(myString, getLine.c_str());

//         //string will come in form "GET /<command>?<param>=<value>
//         /* get the first token which is GET*/
//         //1st call of strtok inserts NULLs where any chars match chars in array s[]
//         token = strtok(myString, s); // get up to before '=' pointer
//         Serial.print("1st  token GET found : ");
//         Serial.println(token);
//         //get command or page to submit to from "/"command"?"
//         command = strtok(NULL, s);
//         Serial.print("2nd  token COMMAND found command : ");
//         Serial.println(command);
//         //get desired state, 'ON' or 'OFF'
//         param = strtok(NULL, s);
//         Serial.print("3rd token PARAM found: ");
//         Serial.println(param);

//         //get value of param
//         value = strtok(NULL, s);
//         Serial.print("4th token VALUE found desired state: ");
//         Serial.println(value);
//     }

//     //now send command to socket

//     //check for type of command received
//     if (!strcmp("switch", command))
//     {
//         //get the integer val of param char array - param - the socket number
//         int socketNum = strtol(param, &ptr, 10);
//         //Serial.print("The number(ul int) is : ");
//         //Serial.println(socketNum);

//         //calc if command is true or false - converst state string to int 0, or 1

//         if (!strcmp(value, "ON"))
//         { //mustbe ON command
//             newState = true;
//         } //else nmust be OFF
//         else
//         {
//             newState = false;
//         }
//         Serial.print(">>>>> Send command to socket : ");
//         Serial.print(socketNum);
//         Serial.print(", new  State : ");
//         Serial.println(newState);

//         digitalWrite(LEDPIN, newState); // GET /H turns the LED on
//         transmitter.sendUnit(socketNum, newState);
//         return true;
//     }
//     else
//     {
//         Serial.println("Request not recognised");
//         return false;
//     }
// }

// void sendResponse(WiFiEspClient client)
// {
//     // send a standard http response header
//     // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
//     // and a content-type so the client knows what's coming, then a blank line:
//     client.println("HTTP/1.1 200 OK");
//     // client.println("Content-Type: text/html");
//     // client.println();
//     // // the content of the HTTP response follows the header:
//     // client.println("<!DOCTYPE HTML>");

//     // for (int i = 0; i < 16; i++)
//     // {
//     // client.print("<a href=/");
//     // client.print(i);
//     // client.print("=ON> Turn ON Socket : ");
//     // client.print(i);
//     // client.print("</a><br>");
//     // client.println();

//     // client.print("<a href=/");
//     // client.print(i);
//     // client.print("=OFF> Turn OFF Socket : ");
//     // client.print(i);
//     // client.print("</a><br>");
//     //  client.println();
//     // }

//     //client.println();
// }

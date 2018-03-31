// create new bboard to test
//work only using wifiesp lib as connection to net


//#include <SPI.h>
#include <WiFiEsp.h>
#include <NewRemoteTransmitter.h>

#include <PubSubClient.h>


char ssid[] = "notwork";                              // your network SSID (name)
char pass[] = "a new router can solve many problems"; // your network password
//int keyIndex = 0;                                     // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

#include "SoftwareSerial.h"
SoftwareSerial ESPSerial(7, 6); // RX, TX

WiFiEspServer server(80);


IPAddress mqttserver(172, 16, 0, 2);


//WiFiEspClient client = server.available();
//EthernetClient ethClient;

NewRemoteTransmitter transmitter(282830, 4);
byte socket = 3;
bool state = false;
#define LEDPIN 5
//Supported baud rates are 300, 600, 1200, 2400, 4800, 9600, 14400, 
//19200, 28800, 31250, 38400, 57600, and 115200. 
#define ESP_BAUD 9600


void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived mqtt pubsub

}


WiFiEspClient WiFiEClient;
PubSubClient client(mqttserver, 1883, callback, WiFiEClient);




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

boolean processRequest(String &getLine)
{
    char *command;
    char *param;
    char *value;
    char myString[50];
    const char s[] = " /?=&"; // delimeter chars for strtok
    char *token;
    char *ptr;
    //long ret;
    bool newState = false;

    //now chck if a GET was received previously
    //vars for sockt and state should be present
    //if so then do socket control command
    //poss url format
    ///test/demo_form.php?name1=value1&name2=value2

    //current call used is like this
    //Switch TEST_433_Control "TEST_433_Control"  <switch> {http=">[ON:GET:http://192.168.0.230/45=ZON:on] >[OFF:GET:http://192.168.0.230/45=ZOFF:off]" }
    //http://192.168.0.230/45=ZON

    // format of expected url http://192.168.0.230/<command|device>/<id>/<state>
    // e.g http://192.168.0.230/socket/12/1  - socket 12 (of 0-15), ON

    // extract socket num and on or off param
    if (getLine.indexOf("GET /") != -1)
    //if (1)
    { // get command detected
        Serial.println("GET / - found in response");

        strcpy(myString, getLine.c_str());

        //string will come in form "GET /<command>?<param>=<value>
        /* get the first token which is GET*/
        //1st call of strtok inserts NULLs where any chars match chars in array s[]
        token = strtok(myString, s); // get up to before '=' pointer
        Serial.print("1st  token GET found : ");
        Serial.println(token);
        //get command or page to submit to from "/"command"?"
        command = strtok(NULL, s);
        Serial.print("2nd  token COMMAND found command : ");
        Serial.println(command);
        //get desired state, 'ON' or 'OFF'
        param = strtok(NULL, s);
        Serial.print("3rd token PARAM found: ");
        Serial.println(param);

        //get value of param
        value = strtok(NULL, s);
        Serial.print("4th token VALUE found desired state: ");
        Serial.println(value);
    }

    //now send command to socket

    //check for type of command received
    if (!strcmp("switch", command))
    {
        //get the integer val of param char array - param - the socket number
        int socketNum = strtol(param, &ptr, 10);
        //Serial.print("The number(ul int) is : ");
        //Serial.println(socketNum);

        //calc if command is true or false - converst state string to int 0, or 1

        if (!strcmp(value, "ON"))
        { //mustbe ON command
            newState = true;
        } //else nmust be OFF
        else
        {
            newState = false;
        }
        Serial.print(">>>>> Send command to socket : ");
        Serial.print(socketNum);
        Serial.print(", new  State : ");
        Serial.println(newState);

        digitalWrite(LEDPIN, newState); // GET /H turns the LED on
        transmitter.sendUnit(socketNum, newState);
        return true;
    }
    else
    {
        Serial.println("Request not recognised");
        return false;
    }
}

void sendResponse(WiFiEspClient client)
{
    // send a standard http response header
    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
    // and a content-type so the client knows what's coming, then a blank line:
    client.println("HTTP/1.1 200 OK");
    // client.println("Content-Type: text/html");
    // client.println();
    // // the content of the HTTP response follows the header:
    // client.println("<!DOCTYPE HTML>");

    // for (int i = 0; i < 16; i++)
    // {
    // client.print("<a href=/");
    // client.print(i);
    // client.print("=ON> Turn ON Socket : ");
    // client.print(i);
    // client.print("</a><br>");
    // client.println();

    // client.print("<a href=/");
    // client.print(i);
    // client.print("=OFF> Turn OFF Socket : ");
    // client.print(i);
    // client.print("</a><br>");
    //  client.println();
    // }

    //client.println();
}

void setup()
{ //Initialize serial monitor port and wait for port to open:
    Serial.begin(115200);
    //Serial.begin(9600);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for Leonardo only
    }
    pinMode(LEDPIN, OUTPUT); // set the LED pin mode

    // initialize serial for ESP module
    // TODO - try setting esp-01 baud rate higher for better response
    //find fastets baud rate soft serial can manage confortably
    //try with 31250 baud 8mhz xtal
    //try 57600 with 16mhz xtal
    ESPSerial.begin(ESP_BAUD);
    Serial.println("Sending an AT command...");
    ESPSerial.println("AT+UART_CUR?");
    delay(30);
    while (ESPSerial.available())
    {
        String inData = ESPSerial.readStringUntil('\n');
        Serial.println("Got reponse from ESP8266: " + inData);
    }
    //query current settings
    //Serial.println(ESPSerial.println("AT+UART_CUR?"));

    //////////////////////////////////////////////////////
    //  ESPSerial.println("AT+UART_CUR=14400,8,1,0,0");
    //  delay(300);
    //  ESPSerial.end();
    // // delay(300);
    //  ESPSerial.begin(14400);
    //////////////////////////////////////////////////


    ESPSerial.println("AT+UART_CUR?");
    delay(30);
    while (ESPSerial.available())
    {
        String inData = ESPSerial.readStringUntil('\n');
        Serial.println("Got NEW reponse from ESP8266: " + inData);
    }
    delay(300);
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
        delay(3000);
    }
    server.begin();
    // you're connected now, so print out the status:
    printWifiStatus();

      if (client.connect("arduinoClient", "testuser", "testpass")) {
    client.publish("outTopic","hello world");
    client.subscribe("inTopic");
  }
}

void loop()
{
      client.loop();

    // listen for incoming clients
    WiFiEspClient client = server.available();

    if (client) // keep checking for new client connection coming in
    {
        Serial.println("new client");
        String currentLine = ""; // make a String to hold incoming data from the client
        String request = "";     //storage for get request line
        char c;
        while (client.connected())
        {
            if (client.available())
            {
                c = client.read(); //get next char rxed
                Serial.write(c);   //echo toserial monitor
                if (c == '\n')     //could be 1st-end of line  or 2nd - end of request \n in request
                {
                    // if the current line is empty, means this is 2nd \n char in a row.
                    // therefore that's the end of the client HTTP request
                    if (currentLine.length() == 0)
                    {
                        //request has been rxed, see if we can process it
                        processRequest(request);
                        //send a response
                        sendResponse(client);
                        // break out of the while loop to terminate connection
                        break;
                    }
                    else                                        // 1st \n rxed - end of current line - and currentLine is not empty
                    {                                           //look to see if current line has  a GET request in it
                        if (currentLine.indexOf("GET /") != -1) //last line was a get line
                        {
                            request = currentLine; // store current line in request as it is the GET line
                        }
                        currentLine = ""; // empty current line ready for next line rxed
                    }
                }
                else if (c != '\r') // ignore if \r
                {
                    // you've gotten a character on the current line - other than \n or \r
                    currentLine += c; // add current rxed char it to the end of the currentLine
                }
            }
        }
        // give the web browser time to receive the data
        //delay(1);

        // close the connection:
        client.stop();
        Serial.println("client disonnected");
    }
}

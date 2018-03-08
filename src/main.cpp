//#include <SPI.h>
#include <WiFiEsp.h>
#include <NewRemoteTransmitter.h>

char ssid[] = "notwork";                              // your network SSID (name)
char pass[] = "a new router can solve many problems"; // your network password
//int keyIndex = 0;                                     // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

#include "SoftwareSerial.h"
SoftwareSerial Serial1(7, 6); // RX, TX

WiFiEspServer server(80);
NewRemoteTransmitter transmitter(282830, 4);
byte socket = 3;
bool state = false;

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

void processGetString(String &getLine)
{
    char *command;
    char *param;
    char *value;
    char myString[50];
    const char s[] = " /?=&"; // delimeter chars for strtok
    char *token;
    char *ptr;
    long ret;
    bool newState = false;

    //poss url format
    ///test/demo_form.php?name1=value1&name2=value2

    //current call used is like this
    //Switch TEST_433_Control "TEST_433_Control"  <switch> {http=">[ON:GET:http://192.168.0.230/45=ZON:on] >[OFF:GET:http://192.168.0.230/45=ZOFF:off]" }
    //http://192.168.0.230/45=ZON

    // extract socket num and on or off param
    if (getLine.indexOf("GET /") != -1)
    { // get command detected
        Serial.println("GET / - found in response");

        strcpy(myString, getLine.c_str());

        //string will come in form "GET /<command>?<param>=<value>
        /* get the first token which is GET*/
        //1st call of strtok inserts NULLs where any chars match chars in array s[]
        token = strtok(myString, s); // get up to before '=' pointer
        Serial.print("1st  token GET found : ");
        Serial.println(token);
        //token = strtok(myString, s);
        //get socket number - "0" to "15"
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

    

    //Serial.print("String part is : ");
    //Serial.println(ptr);

    //check for type of command received
    if (!strcmp("switch", command))
    {
    //get the integer val of param char array - param - the socket number
        int socketNum = strtol(param, &ptr, 10);
        //Serial.print("The number(ul int) is : ");
        Serial.println(ret); 
        
        //calc if command is true or false - converst state string to int 0, or 1
        
        if (!strcmp(value,"ON") )
        { //mustbe ON command
            newState = true;
        } //else nmust be OFF
        else
        {
            newState = false;
        }
        Serial.print("++++ Send command to socket : ");
        Serial.print(socketNum);
        Serial.print(", new  State : ");
        Serial.println(newState);

        digitalWrite(5, newState); // GET /H turns the LED on
        transmitter.sendUnit(socketNum, newState);
    }
    else{
        Serial.println("Request not recognised");
    }
}

void setup()
{ //Initialize serial and wait for port to open:
    //Serial.begin(115200);
    Serial.begin(9600);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for Leonardo only
    }
    pinMode(5, OUTPUT); // set the LED pin mode

    // initialize serial for ESP module
    // TODO - try setting esp-01 baud rate higher for better response
    //find fastets baud rate soft serial can manage confortably
    Serial1.begin(9600);
    // initialize ESP module
    WiFi.init(&Serial1);

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
}

void loop()
{
    // listen for incoming clients
    WiFiEspClient client = server.available();

    if (client)
    {
        Serial.println("new client");
        String currentLine = ""; // make a String to hold incoming data from the client
        String GetRequest = "";  //storage for get request line
        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();
                Serial.write(c);
                if (c == '\n')
                {

                    // if the current line is blank, and you have got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    //send a response then process the command from the get line if there was a GET?
                    { // send a standard http response header
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        client.println();
                        // the content of the HTTP response follows the header:
                        client.println("<!DOCTYPE HTML>");

                        for (int i = 0; i < 16; i++)
                        {
                            client.print("<a href=/");
                            client.print(i);
                            client.print("=ON> Turn ON Socket : ");
                            client.print(i);
                            client.print("</a><br>");
                            client.println();

                            client.print("<a href=/");
                            client.print(i);
                            client.print("=OFF> Turn OFF Socket : ");
                            client.print(i);
                            client.print("</a><br>");
                            client.println();
                        }

                        client.println();

                        //now chck if a GET was received previously
                        //vars for sockt and state should be present
                        //if so then do socket control command
                        if (GetRequest.indexOf("GET /") != -1)
                        {
                            processGetString(GetRequest);
                        }

                        // break out of the while loop:
                        break;
                    }
                    else
                    { // \n rxed and currentLine is not empty so process it
                        // Check to see what the client request was and process
                        //look to see if there was a GET request on the current line
                        if (currentLine.indexOf("GET /") != -1) //last line was a get request with params
                        {
                            GetRequest = currentLine;
                        }
                        currentLine = "";
                    }
                }
                else if (c != '\r')
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

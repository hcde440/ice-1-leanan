/* Leana Nakkour
 * HCDE 440
 * ICE #1 
 * April 5, 2019
 * 
 * A sketch to get the ESP8266 on the network and connect to some open services via HTTP to
 * get our external IP address and (approximate) geolocative information in the getGeo()
 * function. To do this we will connect to http://freegeoip.net/json/, an endpoint which
 * requires our external IP address after the last slash in the endpoint to return location
 * data, thus http://freegeoip.net/json/XXX.XXX.XXX.XXX
 * 
 * This sketch also introduces the flexible type definition struct, which allows us to define
 * more complex data structures to make receiving larger data sets a bit cleaner/clearer.
 * 
 * jeg 2017
 * 
 * updated to new API format for Geolocation data from ipistack.com
 * brc 2019
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h> //provides the ability to parse and construct JSON objects

const char* ssid = "hcde440";                          // Constant variable for SSID 
const char* pass = "yoloswag";                         // Constant variable for password for SSID
const char* key = "";                                  // INSERT Personal API key for accurate Geolocation tracking of IP addresses

/*  here we create a new data type definition, a box to hold other data types
 *  for each name:value pair coming in from the service, we will create a slot
 *  in our structure to hold our data
 */
 
typedef struct { 
  String ip;  // Variable to store IP address as a string    
  String cc;  // Variable to store Continent Code as a string
  String cn;  // Variable to store Continent Name as a string
  String rc;  // Variable to store Region Code as a string
  String rn;  // Variable to store Region Name as a string
  String cy;  // Variable to store City as a string
  String ln;  // Variable to store Longitude as a string
  String lt;  // Variable to store Latitude as a string
    
} GeoData;     //then we give our new data structure a name so we can use it in our code

GeoData location; //we have created a GeoData type, but not an instance of that type,
                  //so we create the variable 'location' of type GeoData

typedef struct {
  String tp;  // Variable to store Temperature as a string
  String pr;  // Variable to store Pressure as a string
  String hd;  // Variable to store Humidity as a string
  String ws;  // Variable to store Wind Speed as a string
  String wd;  // Variable to store Weather Description as a string
  
} MetData;

MetData conditions; // Creates the instance of a MetData type with the variable "conditions"

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.print("Connecting to "); Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) { // While the Wifi status reads that it is not connected
    delay(500);                           // Delay for half a second
    Serial.print(".");                    // Prints a "."
  }

  Serial.println(); Serial.println("WiFi connected"); Serial.println();
  Serial.print("Your ESP has been assigned the internal IP address ");          // IP address 192.168.43.64
  Serial.println(WiFi.localIP());

  getGeo(); 

/*  The next five lines prints out sentences that use the 
 *  data stored in the variables that are used in the getGeo and getIP
 *  methods so that they can be read and understood in context.
 */
 
  Serial.println("Your external IP address is " + location.ip);
  Serial.print("Your ESP is currently in " + location.cn + " (" + location.cc + "),");
  Serial.println(" in or near " + location.cy + ", " + location.rc + ".");
  Serial.println("and located at (roughly) ");
  Serial.println(location.lt + " latitude by " + location.ln + " longitude.");

  getMet(); // tp, pr, hd, ws, wd

/*  The next five lines prints out sentences that use the 
 *  data stored in the variables that are used in the getMet
 *  methods so that they can be read and understood in context.
 */

  Serial.println();
  Serial.println("With the temperature in " + location.cy + ", " + location.rc);
  Serial.println("is " + conditions.tp + "F, with a humidity of " + conditions.hd + "%. The winds are blowing");
  Serial.println(conditions.wd + " at " + conditions.ws + " miles per hour, and the ");
  Serial.println("barometric pressure is at " + conditions.pr + " millibars.");

}

void loop() {
  //if we put getIP() here, it would ping the endpoint over and over . . . DOS attack?
}

String getIP() {
  HTTPClient theClient;
  String ipAddress;

  theClient.begin("http://api.ipify.org/?format=json");
  int httpCode = theClient.GET();

  // Conditional to check if the code is greater than zero (ex: 200 = working, 404 = not working)
  if (httpCode > 0) {
    
    // Conditional checks if the code is equal to 200 which means everything works as expected
    if (httpCode == 200) {

      DynamicJsonBuffer jsonBuffer;

      String payload = theClient.getString();
      JsonObject& root = jsonBuffer.parse(payload);
      ipAddress = root["ip"].as<String>();

    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
      return "error";
    }
  }
  return ipAddress;
}

void getGeo() {
  HTTPClient theClient;
  Serial.println("Making HTTP request");
  theClient.begin("http://api.ipstack.com/" + getIP() + "?access_key=" + key); //return IP as .json object
  int httpCode = theClient.GET();

  // Conditional to check if the code is greater than zero (ex: 200 = working, 404 = not working)
  if (httpCode > 0) {
    
    // Conditional checks if the code is equal to 200 which means everything works as expected
    if (httpCode == 200) {
      Serial.println("Received HTTP payload.");
      DynamicJsonBuffer jsonBuffer;
      String payload = theClient.getString();
      Serial.println("Parsing...");
      JsonObject& root = jsonBuffer.parse(payload);

      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed");
        Serial.println(payload);
        return;
      }

      //Some debugging lines below:
      //      Serial.println(payload);
      //      root.printTo(Serial);

      //Using .dot syntax, we refer to the variable "location" which is of
      //type GeoData, and place our data into the data structure.

      // The following lines create a Name:Value pair in the instance of "location" for each variable as a String
      location.ip = root["ip"].as<String>();            //we cast the values as Strings b/c
      location.cc = root["country_code"].as<String>();  //the 'slots' in GeoData are Strings
      location.cn = root["country_name"].as<String>();
      location.rc = root["region_code"].as<String>();
      location.rn = root["region_name"].as<String>();
      location.cy = root["city"].as<String>();
      location.lt = root["latitude"].as<String>();
      location.ln = root["longitude"].as<String>();

    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
    }
  }
}

/* Wrap your HTTP request in a function called getMet(), which will be similar to getIP()and getGeo(), 
 *  and print out the data as a message to the Serial Monitor using Imperial units. 
 *  You should report temperature, humidity, windspeed, wind direction, and cloud conditions. 
 *  Call the API by city name.
 */

void getMet() {

  HTTPClient theClient;                                                         // Creates HTTPClient variable called theClient
  String apiCall = "http://api.openweathermap.org/data/2.5/weather?q=Seattle";  // Sets up API url to be used to call and access API
  apiCall += "&units=imperial&appid=";
  apiCall += "";                                                                // INSERT Personal Weather API key here
  theClient.begin(apiCall);                                                     // Calls API
  int httpCode = theClient.GET();

  // Conditional to check if the code is greater than zero (ex: 200 = working, 404 = not working)
  if (httpCode > 0) {

    // Conditional checks if the code is equal to 200 which means everything works as expected
    if (httpCode == HTTP_CODE_OK) {
      String payload = theClient.getString();
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(payload);

      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed in getMet().");
        return;
      }

      // The following five lines creates a Name:Value pair in the instance of "conditions" for each variable as a String
      conditions.tp = root["main"]["temp"].as<String>();                      
      conditions.pr = root["main"]["pressure"].as<String>();
      conditions.hd = root["main"]["humidity"].as<String>();
      conditions.wd = root["weather"][0]["description"].as<String>();
      conditions.ws = root["wind"]["speed"].as<String>();
      
    }
  }
  else {
    Serial.printf("Something went wrong with connecting to the endpoint in getMet().");
  }
}

    

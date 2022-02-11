#include "SensorNivel.h"



//Define Firebase Data object
FirebaseData stream;
// FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

int count = 0;

volatile bool dataChanged = false;

//Tempo que deve esperar para considerar como desconectado
// #define MAX_DISCONNECTED_TIME 10000 //10 seg

//Tempo que deve esperar para cada ligação
// #define MAX_CALL_TIME_INTERVAL 1800000 //30 min

//Tempo em que conectou à internet pela última vez
// uint32_t lastTimeConnected = 0;

//Tempo em que fez a última ligação. Colocamos como -MAX_CALL_TIME_INTERVAL para ligar imediatamente
//da primeira vez que cair
// uint32_t lastTimeCalledPhone = -MAX_CALL_TIME_INTERVAL;


bool hasInternet()
{
  // WiFiClient client;
  // //Endreço IP do Google 172.217.3.110
  // IPAddress adr = IPAddress(172, 217, 30, 110);
  // //Tempo limite para conexão
  // client.setTimeout(50);
  // //Tenta conectar
  // bool connected = client.connect(adr, 80);
  // //Fecha a conexão
  // // Serial.println(client.timedPeek());
  // client.stop();
  // Serial.println(adr);
  // Serial.println(client.peek());
  // Serial.println(connected);
  // //Retorna true se está conectado ou false se está desconectado

  // return connected;

  // Serial.printf("httpConnected: %d\n", !stream.httpConnected());
  // Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
  // Serial.println("Timeout: " + stream.streamTimeout());
  // Serial.println("isStream: " + stream.isStream());
  // // Serial.println(stream.getWiFiClient().c_str());
  // Serial.println("ETag: " + stream.ETag());
  // Serial.println("++++++++++++++++++++++++++++++++++++++++++++++++");

// stream.tcpClient;

  if(stream.httpConnected())
    return true;
  else
    return false;
}

void streamCallback(FirebaseStream data)
{
  Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); //see addons/RTDBHelper.h
  Serial.println("---------------------------------------------");
  Serial.println();

  //This is the size of stream payload received (current and max value)
  //Max payload size is the payload size under the stream path since the stream connected
  //and read once and will not update until stream reconnection takes place.
  //This max value will be zero as no payload received in case of ESP8266 which
  //BearSSL reserved Rx buffer size is less than the actual stream payload.
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());

  //Due to limited of stack memory, do not perform any task that used large memory here especially starting connect to server.
  //Just set this flag and check it status later.
  dataChanged = true;
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout){
    Serial.println("stream timed out, resuming...\n");
    Serial.println("*****************************************");
  }

  if (!stream.httpConnected()){
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
    Serial.println("++++++++++++++++++++++++++++++++++++++++++++++++");
  }
}

void printStatusConfig();

#define PATH F("PMNP")    //Projeto Medição de Nível Protótipo
#define MAC_DEVICE F("E02134FAD0")
// #define PATH_FULL F("/"+PATH+"/"+MAC_DEVICE)

String pathFull(){
  String temp = "/";
  temp += PATH;
  temp += "/";
  temp += MAC_DEVICE;
  return  temp;
}

void setup()
{

  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  //Or use legacy authenticate method
  //config.database_url = DATABASE_URL;
  //config.signer.tokens.legacy_token = "<database secret>";



  Firebase.begin(&config, &auth);


  // Firebase.RTDB.setReadTimeout(&stream, 2000);
  // Firebase.RTDB.setMaxRetry(&stream, 2);
  // Firebase.RTDB.setMaxErrorQueue(&stream, 2);


  Firebase.reconnectWiFi(true);


// //Recommend for ESP8266 stream, adjust the buffer size to match your stream data size
// #if defined(ESP8266)
//   stream.setBSSLBufferSize(2048 /* Rx in bytes, 512 - 16384 */, 512 /* Tx in bytes, 512 - 16384 */);
// #endif

//   if (!Firebase.RTDB.beginStream(&stream, "/test/stream/data"))
//     Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());

//   Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);


// //WiFi reconnect timeout (interval) in ms (10 sec - 5 min) when WiFi disconnected.
  config.timeout.wifiReconnect = 30 * 1000;

//   //Socket begin connection timeout (ESP32) or data transfer timeout (ESP8266) in ms (1 sec - 1 min).
//   config.timeout.socketConnection = 1 * 1000;

//   //ESP32 SSL handshake in ms (1 sec - 2 min). This option doesn't allow in ESP8266 core library.
//   config.timeout.sslHandshake = 1 * 1000;

//   //Server response read timeout in ms (1 sec - 1 min).
//   config.timeout.serverResponse = 1 * 1000;

//   //RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 5 * 1000;

//   //RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.
//   config.timeout.rtdbStreamReconnect = 1 * 1000;

//   //RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the readStream
//   //will return false (error) when it called repeatedly in loop.
//   config.timeout.rtdbStreamError = 3 * 1000;




//Recommend for ESP8266 stream, adjust the buffer size to match your stream data size
#if defined(ESP8266)
  stream.setBSSLBufferSize(2048 /* Rx in bytes, 512 - 16384 */, 512 /* Tx in bytes, 512 - 16384 */);
#endif



// if (!Firebase.RTDB.beginStream(&stream, "/test/stream/data"))
// if (!Firebase.RTDB.beginStream(&stream, PATH_FULL))
if (!Firebase.RTDB.beginStream(&stream, pathFull()))
  Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());

Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);





  /** Timeout options, below is default config.

  //WiFi reconnect timeout (interval) in ms (10 sec - 5 min) when WiFi disconnected.
  config.timeout.wifiReconnect = 10 * 1000;

  //Socket begin connection timeout (ESP32) or data transfer timeout (ESP8266) in ms (1 sec - 1 min).
  config.timeout.socketConnection = 30 * 1000;

  //ESP32 SSL handshake in ms (1 sec - 2 min). This option doesn't allow in ESP8266 core library.
  config.timeout.sslHandshake = 2 * 60 * 1000;

  //Server response read timeout in ms (1 sec - 1 min).
  config.timeout.serverResponse = 10 * 1000;

  //RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 45 * 1000;

  //RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.
  config.timeout.rtdbStreamReconnect = 1 * 1000;

  //RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the readStream
  //will return false (error) when it called repeatedly in loop.
  config.timeout.rtdbStreamError = 3 * 1000;

  */
 }

 void printStatusConfig(){
    Serial.println();
    Serial.print("wifiReconnect: ");
    Serial.println(config.timeout.wifiReconnect);
    Serial.print("socketConnection: ");
    Serial.println(config.timeout.socketConnection);
    Serial.print("sslHandshake: ");
    Serial.println(config.timeout.sslHandshake);
    Serial.print("serverResponse: ");
    Serial.println(config.timeout.serverResponse);
    Serial.print("rtdbKeepAlive: ");
    Serial.println(config.timeout.rtdbKeepAlive);
    Serial.print("rtdbStreamReconnect: ");
    Serial.println(config.timeout.rtdbStreamReconnect);
    Serial.print("rtdbStreamError: ");
    Serial.println(config.timeout.rtdbStreamError);
    Serial.println();
 }

unsigned long delayLedSwap = 0;
unsigned long delayConnection = 0;
unsigned long delayPrintConfig = 0;
bool statusConectionOld = false;

String addPathMain(String value){
  String temp = pathFull();
  temp += "/";
  temp += value;
  return temp;
}

void loop()
{
  //Flash string (PROGMEM and FPSTR), Arduino String, C++ string, const char, char array, string literal are supported
  //in all Firebase and FirebaseJson functions, unless F() macro is not supported.

  // if (Firebase.ready() && (millis() - sendDataPrevMillis > 3000 || sendDataPrevMillis == 0))
  if ((millis() - sendDataPrevMillis) > 3000 || sendDataPrevMillis == 0)
  {
    sendDataPrevMillis = millis();
    count++;
    FirebaseJson json;
    json.add("value", count);
    // json.add("num", count);
    json.set("sample/[0]", count);
    json.set("sample/[1]", count+1);
    json.set("sample/[2]", count+2);
    json.set("sample/[3]", count+3);
    json.set("sample/[4]", count+4);
    json.set("sample/[5]", count+5);
    json.set("sample/[6]", count+6);
    // Serial.printf("Set json... %s\n\n", Firebase.RTDB.setJSON(&fbdo, "/test/stream/data/json", &json) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set json... %s\n\n", Firebase.RTDB.setJSON(&stream, addPathMain(String(count)), &json) ? "ok" : stream.errorReason().c_str());
    // Serial.println("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$");
  }

  if(millis() > delayLedSwap+100){
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delayLedSwap = millis();
  }

  if(millis() > delayPrintConfig+5000){
    // printStatusConfig();
    delayPrintConfig = millis();
  }

  if (dataChanged)
  {
    dataChanged = false;
    //When stream data is available, do anything here...
  }

  if(millis() > delayConnection+2000)
  {
    delayConnection = millis();
    // WiFi.printDiag(Serial);
    // Serial.println();
    // Serial.println();


    bool statusConection = hasInternet();
    if(statusConection != statusConectionOld){
      statusConectionOld = statusConection;
      if(statusConection){
      //Mostra no display que está conectado
      // Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
        // Firebase.RTDB.beginStream(&stream, "/test/stream/data");
        Serial.println("Connected");
      }else{
        //Mostra no display que está desconectado
        statusConectionOld = statusConection;
        Serial.println("Disconnected");
        // Firebase.RTDB.removeStreamCallback(&stream);
        // Firebase.RTDB.endStream(&stream);
      }
    }
  }
}

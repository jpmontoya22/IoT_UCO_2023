#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "xxxxxxxxx";
const char* password =  "xxxxxxxxx";
const char* mqttServer = "broker.mqtt-dashboard.com";
const int mqttPort = 1883;
const char* TopicInput = "input";
const char* TopicOutput = "output";
const char* TopicAlive = "alive";
const char* TopicStatusRequest = "StatusRequest";
const char* TopicJsonStatus = "JsonStatus";

const String api= "http://worldtimeapi.org/api/timezone/";
String zonaHoraria;

WiFiClient espClient;
PubSubClient client(espClient);
WiFiClient clientHttp;

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print(F("Connecting to ")) ;
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


String obtenerDiaLetra(int numero){
  switch (numero) {
  case 0:
    return "Domingo";
    break;
  case 1:
    return "Lunes";
    break;
  case 2:
    return "Martes";
    break;
  case 3:
    return "Miercoles";
    break;
  case 4:
    return "Jueves";
    break;
  case 5:
    return "Viernes";
    break;
  case 6:
    return "Sabado";
    break;
  default:
    return "Dia";
    break;
}
}
String obtenerMesLetra(String numero){
  if(numero=="01"){
    return "Enero";
  }else if (numero=="02"){
    return "Febrero";
  }else if (numero=="03"){
    return "Marzo";
  }else if (numero=="04"){
    return "Abril";
  }else if (numero=="05"){
    return "Mayo";
  }else if (numero=="06"){
    return "Junio";
  }else if (numero=="07"){
    return "Julio";
  }else if (numero=="08"){
    return "Agosto";
  }else if (numero=="09"){
    return "Septiembre";
  }else if (numero=="10"){
    return "Octubre";
  }else if (numero=="11"){
    return "Noviembre";
  }else if (numero=="12"){
    return "Diciembre";
  }else{
    return "Mes";
  }
}

String obtenerA(String cadena){
   char str[cadena.length()+1];
   unsigned int i=0;
   for (i=0;i<cadena.length();i++) {
      
      if((char)cadena[i] != '-'){
        str[i]=(char)cadena[i];
      }else {
        break;
      }

    }
    str[i] = 0; // Terminación nula
    Serial.println(str);
    return str;
}

String obtenerMes(String cadena){
  return cadena.substring(5,7);;
}

String obtenerDiaNumero(String cadena){
  return cadena.substring(8,10);
}

String obtenerHora(String cadena){
  return cadena.substring(11,16);
}

void obtenerOutput(String diaLetra, String a, String diaNumero, String mes, String hora){

String datos;

  datos = diaLetra + ", " + diaNumero + " de " + mes + " de " + a + " -- " + hora;
  
  Serial.println(datos);
  client.publish(TopicOutput, datos.c_str());
  
}

void consumirApi(String api, String zonaHoraria){

    HTTPClient http;
    if (http.begin(clientHttp, api+zonaHoraria)) { 
    
    int httpCode = http.GET();  //Realizamos la petición
    String codigoStr = String(httpCode);
    
         
        if (httpCode > 0) { //código de retorno

           Serial.println(httpCode); // esperamos que sea 200
           if(httpCode==200){
            client.publish(TopicStatusRequest,"HTTP Funcional");
           }
           
           String data = http.getString();
            client.publish(TopicJsonStatus,"JSON Funcional");
           
            
            Serial.println(data);
    
            StaticJsonDocument <256> doc;
            deserializeJson(doc,data);
                
            const char* datetime = doc["datetime"];
            int day_of_week = doc["day_of_week"];
          
            Serial.print("datetime: ");
            Serial.println(datetime);
            
            Serial.println("day_of_week: "+day_of_week);
            obtenerOutput(obtenerDiaLetra(day_of_week),obtenerA(datetime),obtenerDiaNumero(datetime),obtenerMesLetra(obtenerMes(datetime)),obtenerHora(datetime));
      
            
            
          }
          else {
            Serial.println("Error en la petición HTTP");
          }
 
    http.end(); //cerramos conexión y liberamos recursos
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  
    Serial.println("callback se esta ejecutando: ");
    
    char str[length+1];
    Serial.print("topico: ");
    Serial.println(topic);
    Serial.print("mensaje: ");
    unsigned int i=0;
    for (i=0;i<length;i++) {
      Serial.print((char)payload[i]);
      str[i]=(char)payload[i];
    }
    Serial.println();
   
   
    str[i] = 0; // Null termination
    zonaHoraria = str;
    Serial.println("zona horaria: "+zonaHoraria);
    consumirApi(api,zonaHoraria);

}

void setup() {
 
  //Start Serial Communication
  Serial.begin(115200);
  
  //Connect to WiFi
  setup_wifi();

  //Connect to MQTT Broker
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  //MQTT Connection Validation
  while (!client.connected()) {
    Serial.println("Connecting to MQTT");
 
    if (client.connect("PabloM")) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
  
  //Publicar Mensaje de solicitud
  client.publish(TopicAlive, "Tu fecha y hora es:");
  client.subscribe(TopicInput);
 
}

 
void loop() {
  //MQTT client loop
  client.loop();
}

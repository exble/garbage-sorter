#include <WiFi.h>
#include <ESP32Servo.h>

WiFiClient client;
WiFiServer server(80);
Servo topServo, lowServo;

enum class Direction{
  A = 0,
  B = 1,
  C = 2,
  D = 3
};

enum class TrashType{
  plastic,
  glass,
  metal,
  other
};

const int TOP = 0, BOT = 1;
int active;
const int weightPin = 33;
const int lightPin = 32;
const int metalPin = 35;
int avgweight = 0, avglight = 0, avgmetal = 0;
int weight, metal, light;
int backGoundLight = -1, caliWeight = -1;
int cnt = 0;
const int countDown = 1000;
int start_time_point;
int d = 0;
static const int lowPin = 18;             /*Connect the servo motor to GPIO18*/
static const int topPin = 19;
static int prevTopAngle = 90, prevLowAngle = 90;

const char* ssid = "sadnet";         /*Enter Your SSID*/
const char* password = "happypolla"; /*Enter Your Password*/

String _readString, lowAngle, topAngle, button;

void html_page(){

client.println("HTTP/1.1 200 OK");
client.println("Content-type:text/html");
client.println("Connection: close");
client.println();

client.println("<!DOCTYPE html>");
client.println("<html>");
client.println("<head>");
client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
client.println("<link rel=\"ico\" href=\"data:,\">");
client.println("<style>");
client.println("body {text-align:center;font-family:\"Roboto\";margin-left:auto;margin-right:auto;}");
client.println(".slidecontainer {width: 400px; margin:0 auto;}");
client.println(".slider {");
  client.println("-webkit-appearance: none;  width: 100%;  height: 25px; background: #d3d3d3;  outline: none;  ");
  client.println("opacity: 0.7;  -webkit-transition: .2s;  transition: opacity .2s;}");

client.println(".slider:hover { opacity: 1;}");

client.println(".slider::-webkit-slider-thumb {");
  client.println("-webkit-appearance: none;  appearance: none;  width: 25px; height: 25px;  background: #04AA6D;  cursor: pointer;}");

client.println(".slider::-moz-range-thumb {");
  client.println("width: 25px;  height: 25px; background: #04AA6D;  cursor: pointer;}");
client.println("</style>");

client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>");

client.println("</head><body>");
client.println("<h1>Servo Motor With ESP32</h1>");

client.println("<p>Top angle.</p>");

client.println("<div class=\"slidecontainer\">");
  client.println("<input type=\"range\" min=\"0\" max=\"180\" value=\"90\" class=\"slider\" id=\"TopRange\" onchange=\"servoMotor(0, this.value)\" >");
  client.println("<p>Angle: <span id=\"Topdemo\"></span>&deg</p>");
client.println("</div>");

client.println("<p>Low angle.</p>");

client.println("<div class=\"slidecontainer\">");
  client.println("<input type=\"range\" min=\"0\" max=\"180\" value=\"90\" class=\"slider\" id=\"LowRange\" onchange=\"servoMotor(1, this.value)\" >");
  client.println("<p>Angle: <span id=\"Lowdemo\"></span>&deg</p>");
client.println("</div>");

client.println("<div class=\"buttons\">");

  client.println("<button onclick=\"buttonHandler(0)\">A</button>");
  client.println("<button onclick=\"buttonHandler(1)\">B</button>");
  client.println("<button onclick=\"buttonHandler(2)\">C</button>");
  client.println("<button onclick=\"buttonHandler(3)\">D</button>");

client.println("</div>");

client.println("<div class=\"activate\">");
  client.println("<button onclick=\"Activate()\">Activate</button>");
client.println("</div>");

client.println("<script>");
client.println("var Topslider = document.getElementById(\"TopRange\");");
client.println("var Lowslider = document.getElementById(\"LowRange\");");
client.println("var Topoutput = document.getElementById(\"Topdemo\");");
client.println("var Lowoutput = document.getElementById(\"Lowdemo\");");
client.println("Topoutput.innerHTML = Topslider.value;");
client.println("Lowoutput.innerHTML = Lowslider.value;");

client.println("Topslider.oninput = function() {Topslider.value = this.value; Topoutput.innerHTML = this.value;}");
client.println("Lowslider.oninput = function() {Lowslider.value = this.value; Lowoutput.innerHTML = this.value;}");

  client.println("$.ajaxSetup({timeout:1000});");

  client.println("function servoMotor(type, angle) { ");
  client.println("if(type == 0){ " );
    client.println("$.get(\"/@topposition$\"+angle + \"*\");");
  client.println("}else{");
    client.println("$.get(\"/@lowposition$\"+angle + \"*\");");
  client.println("}");
    client.println("{Connection: close};");
  client.println("}");
  client.println("function buttonHandler(num) { ");
    client.println("$.get(\"/@button$\"+num + \"*\");");
    client.println("{Connection: close};");
  client.println("}");
  client.println("function Activate() { ");
    client.println("$.get(\"/@Activate$\");");
    client.println("{Connection: close};");
  client.println("}");

client.println("</script></body></html>");

}

void setup() {
  Serial.begin(115200);             /*Set the baud rate to 115200*/
  pinMode(weightPin, INPUT);
  pinMode(lightPin, INPUT);
  pinMode(metalPin, INPUT);
  topServo.attach(topPin);
  lowServo.attach(lowPin);
  topServo.write(prevTopAngle);
  lowServo.write(prevLowAngle);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {/*Wait while connecting to WiFi*/
    delay(100);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());       /*Print the local ip address on the serial window*/
  server.begin();                       /*Start Server*/
}

void turnServo(int spec, int target){
  if(spec == BOT){
    for(int i = 1; i <= abs(target - prevLowAngle); i++){
      int j = target > prevLowAngle ? 1 : -1;
      lowServo.write(prevLowAngle + j*i);
      delay(15);
    }
    prevLowAngle = target;
    Serial.print("low Position = ");
    Serial.println(target);     
  }
  else {
    for(int i = 1; i <= abs(target - prevTopAngle); i++){
        int j = target > prevTopAngle ? 1 : -1;
        topServo.write(prevTopAngle + j*i);
        delay(15);
    }
    prevTopAngle = target;
    Serial.print("top Position = ");
    Serial.println(target);          /*Print the Updated Angle in Degree on Serial Monitor*/
  }
}

void httpServerResponse(){
  client = server.available();
  if (client) {
    Serial.println("New Client Connected"); 
    String currentString = "";   
    while (client.connected()) {
      if (client.available()) { 
        char _readCharacter = client.read();      /*Read the Input data*/
        _readString += _readCharacter;
        if (_readCharacter == '\n') {             /*Read the Input data until newline comein*/
          if (currentString.length() == 0) {
            html_page();                          /*Call the HTML Page Function*/
            if(_readString.indexOf("GET /@lowposition$")>=0) {
              lowAngle = _readString.substring(_readString.indexOf('$')+1, _readString.indexOf('*'));  /*Parse the data between $ to * */
              int target = lowAngle.toInt();
              turnServo(BOT, target);
            }
            else if(_readString.indexOf("GET /@topposition$")>=0) {
              topAngle = _readString.substring(_readString.indexOf('$')+1, _readString.indexOf('*'));  /*Parse the data between $ to * */
              int target = topAngle.toInt();
              turnServo(TOP, target);
              
            }
            else if(_readString.indexOf("GET /@button$")>=0) {
              button = _readString.substring(_readString.indexOf('$')+1, _readString.indexOf('*'));  /*Parse the data between $ to * */
              int but = button.toInt();
              dump(Direction(but));
              Serial.print("dump to = ");
              Serial.println(char(but + 'A'));          /*Print the Updated Angle in Degree on Serial Monitor*/
            }
            else if(_readString.indexOf("GET /@Activate$")>=0) {
              activate();
            }
            client.println();
            break;                               /*Break the while loop*/
          }
          else {currentString = "";}
        }
        else if (_readCharacter != '\r') {currentString += _readCharacter;}
      }
    }

    _readString = ""; /*clear the string*/
    client.stop();    /*Close the Connection*/
    Serial.println("Client disconnected.");
  }
}

void resetServo(){
  turnServo(TOP, 90);
  delay(200);
  turnServo(BOT, 90);
}

void dump(Direction d){
  switch(d){
    case Direction::A:
      turnServo(BOT, 0);
      delay(200);
      turnServo(TOP, 150);
      Serial.print("Dump to:");
      Serial.println("A");
      break;
    case Direction::B:
      turnServo(BOT, 90);
      delay(200);
      turnServo(TOP, 30);
      Serial.print("Dump to:");
      Serial.println("B");
      break;
    case Direction::C:
      turnServo(BOT, 0);
      delay(200);
      turnServo(TOP, 30);
      Serial.print("Dump to:");
      Serial.println("C");
      break;
    case Direction::D:
      turnServo(BOT, 90);
      delay(200);
      turnServo(TOP, 150);
      Serial.print("Dump to:");
      Serial.println("D");
      break;
  }
  delay(500);
  resetServo();
}

void activate(){
  active = 1;
}

void loop(){
  int t_weight = analogRead(weightPin);
  int t_light = analogRead(lightPin);
  int t_metal = analogRead(metalPin);
  avgweight += t_weight;
  avglight += t_light;
  avgmetal += t_metal;
  cnt++;
  if(millis()-500 >= d){
    Serial.print("IP address: ");
    Serial.print(WiFi.localIP()); 
    light = avglight / cnt;
    metal = avgmetal / cnt;
    weight = avgweight / cnt;
    if(backGoundLight == -1){
      backGoundLight = light;
    } 
    if(caliWeight == -1){
      caliWeight = weight;
    }
    light = light - backGoundLight + 4095; 
    Serial.print(", Light: ");
    Serial.print(light);
    Serial.print(", Weight: ");
    Serial.print(weight);
    Serial.print(", Metal: ");
    Serial.println(metal);
    avgweight = 0;
    avglight = 0;
    avgmetal = 0;

    cnt = 0;
    d = millis();
  }
  if(active){
    if(start_time_point == 0){
      start_time_point = millis();
    }
    else if(millis() - start_time_point >= countDown){
      start_time_point = 0;
      active = 0;
      TrashType type;
      if(metal > 0){
        type = TrashType::metal;
        Serial.println("Type: metal");
        dump(Direction::A);
      }
      //else if(weight > 10){ // weight sensor is broken
      //  type = TrashType::glass;
      //  Serial.println("Type: glass");
      //  dump(Direction::B);
      //}
      else if(light > 3000){
        type = TrashType::plastic;
        Serial.println("Type: plastic");
        dump(Direction::C);
      }
      else {
        type = TrashType::other;
        Serial.println("Type: other");
        dump(Direction::D);
      }
    }
  }
  httpServerResponse();
}
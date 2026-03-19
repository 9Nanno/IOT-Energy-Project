#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PZEM004Tv30.h>

// --- Configuration Pins ---
#define RELAY1 25
#define RELAY2 27
#define BTN1 32
#define BTN2 33
#define BTN3 34 

#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
PZEM004Tv30 pzem(Serial2, PZEM_RX_PIN, PZEM_TX_PIN);

const char* ssid = "hum";
const char* password = "12345678";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WebServer server(80);

bool state1 = false;
bool state2 = false;
float pricePerUnit = 4.20;
float costTHB = 0;
float voltage, current, power, energy, frequency, pf;

unsigned long lastBtn1 = 0, lastBtn2 = 0, lastBtn3 = 0;
const int debounceTime = 250; 
unsigned long lastPZEMRead = 0;

void updateRelays() {
  digitalWrite(RELAY1, state1 ? LOW : HIGH);
  digitalWrite(RELAY2, state2 ? LOW : HIGH);
}

void updatePZEMData() {
  if (millis() - lastPZEMRead > 2000) {
    voltage = pzem.voltage();
    current = pzem.current();
    power = pzem.power();
    energy = pzem.energy();
    frequency = pzem.frequency();
    pf = pzem.pf();
    if (!isnan(energy)) costTHB = energy * pricePerUnit;
    lastPZEMRead = millis();
  }
}

// --- ฟังก์ชันส่งข้อมูล JSON สำหรับ Real-time Update ---
void handleGetData() {
  updatePZEMData();
  String json = "{";
  json += "\"v\":" + String(isnan(voltage)?0:voltage) + ",";
  json += "\"i\":" + String(isnan(current)?0:current) + ",";
  json += "\"p\":" + String(isnan(power)?0:power) + ",";
  json += "\"e\":" + String(isnan(energy)?0:energy, 3) + ",";
  json += "\"c\":" + String(costTHB, 2) + ",";
  json += "\"s1\":" + String(state1 ? 1 : 0) + ",";
  json += "\"s2\":" + String(state2 ? 1 : 0);
  json += "}";
  server.send(200, "application/json", json);
}

// --- ฟังก์ชันรับคำสั่งเปิดปิดแบบไม่ต้อง Refresh หน้า ---
void handleToggle() {
  if (server.hasArg("r")) {
    int r = server.arg("r").toInt();
    if (r == 1) state1 = !state1;
    if (r == 2) state2 = !state2;
    if (r == 0) { state1 = false; state2 = false; }
    updateRelays();
    server.send(200, "text/plain", "OK");
  }
}

void handleRoot() {
  String html = "<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family:sans-serif; text-align:center; background:#f4f4f4; color:#333;} ";
  html += ".card{background:white; padding:20px; margin:10px auto; max-width:400px; border-radius:15px; box-shadow: 0 4px 6px rgba(0,0,0,0.1);} ";
  html += ".btn{display:inline-block; padding:15px 25px; margin:5px; color:white; text-decoration:none; border-radius:8px; font-weight:bold; border:none; cursor:pointer; min-width:100px;} ";
  html += ".on{background:#28a745;} .off{background:#dc3545;} .alloff{background:#343a40; width:80%;}</style>";
  
  // JavaScript หัวใจหลักของ Real-time
  html += "<script>";
  html += "function update(){ fetch('/data').then(r=>r.json()).then(d=>{";
  html += "document.getElementById('v').innerText=d.v; document.getElementById('i').innerText=d.i;";
  html += "document.getElementById('p').innerText=d.p; document.getElementById('e').innerText=d.e;";
  html += "document.getElementById('c').innerText=d.c;";
  html += "document.getElementById('btn1').className = d.s1 ? 'btn off' : 'btn on';";
  html += "document.getElementById('btn1').innerText = 'R1 ' + (d.s1 ? 'OFF' : 'ON');";
  html += "document.getElementById('btn2').className = d.s2 ? 'btn off' : 'btn on';";
  html += "document.getElementById('btn2').innerText = 'R2 ' + (d.s2 ? 'OFF' : 'ON');";
  html += "}); }";
  html += "function toggle(r){ fetch('/toggle?r='+r).then(()=>update()); }";
  html += "setInterval(update, 2000);"; // อัปเดตทุก 2 วินาที
  html += "</script></head><body>";
  
  html += "<h1>Energy Monitor</h1>";
  html += "<div class='card'><h3>Power Real-time</h3>";
  html += "<p>Voltage: <b id='v'>--</b> V</p>";
  html += "<p>Current: <b id='i'>--</b> A</p>";
  html += "<p>Power: <b id='p'>--</b> W</p></div>";
  
  html += "<div class='card'><h3>Usage & Cost</h3>";
  html += "<p>Energy: <b id='e'>--</b> kWh</p>";
  html += "<p style='font-size:20px;'>Cost: <b id='c' style='color:#28a745;'>--</b> THB</p></div>";

  html += "<div class='card'><h3>Control</h3>";
  html += "<button id='btn1' class='btn' onclick='toggle(1)'>R1</button>";
  html += "<button id='btn2' class='btn' onclick='toggle(2)'>R2</button><br><br>";
  html += "<button class='btn alloff' onclick='toggle(0)'>TURN ALL OFF</button></div>";
  
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.printf("R1:%s  R2:%s", state1 ? "ON" : "OFF", state2 ? "ON" : "OFF");
  display.setCursor(0, 10);
  display.printf("IP: %s", WiFi.localIP().toString().c_str());
  display.setCursor(0, 20);
  display.println("--------------------");
  display.setCursor(0, 32);
  if (isnan(voltage)) {
    display.println("PZEM: Not Found!");
  } else {
    display.printf("Volt: %.1f V\nCurr: %.2f A\nPowr: %.1f W\nCost: %.2f THB", voltage, current, power, costTHB);
  }
  display.display();
}

void checkButtons() {
  unsigned long cur = millis();
  if (digitalRead(BTN1) == LOW && (cur - lastBtn1 > debounceTime)) { state1 = !state1; updateRelays(); lastBtn1 = cur; }
  if (digitalRead(BTN2) == LOW && (cur - lastBtn2 > debounceTime)) { state2 = !state2; updateRelays(); lastBtn2 = cur; }
  if (digitalRead(BTN3) == LOW && (cur - lastBtn3 > debounceTime)) { state1 = false; state2 = false; updateRelays(); lastBtn3 = cur; }
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY1, OUTPUT); pinMode(RELAY2, OUTPUT);
  updateRelays();
  pinMode(BTN1, INPUT_PULLUP); pinMode(BTN2, INPUT_PULLUP); pinMode(BTN3, INPUT_PULLUP);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println("OLED error");
  display.clearDisplay();
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
  server.on("/", handleRoot);
  server.on("/data", handleGetData);
  server.on("/toggle", handleToggle);
  server.begin();
}

void loop() {
  server.handleClient();
  checkButtons();
  updatePZEMData();
  updateDisplay();
}

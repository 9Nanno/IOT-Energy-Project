#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// WiFi credentials
const char* ssid = "A54NA";
const char* password = "09876543";

// PZEM Serial pins
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Initialize PZEM sensor
PZEM004Tv30 pzem(Serial2, PZEM_RX_PIN, PZEM_TX_PIN);

// Global variables to store sensor data
float voltage = 0.0;
float current = 0.0;
float power = 0.0;
float energy = 0.0;
float frequency = 0.0;
float pf = 0.0;

// ค่าไฟต่อหน่วย
float price_per_kWh = 4.20; 
float monthly_cost = 0.0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP32 Power Monitor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css">
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #ffffff; min-height: 100vh; padding: 20px; color: #333; }
    .container { max-width: 1200px; margin: 0 auto; }
    .header { text-align: center; margin-bottom: 40px; padding: 20px 0; border-bottom: 2px solid #f0f0f0; }
    .header h1 { color: #2c3e50; font-size: 2.5rem; font-weight: 700; margin-bottom: 10px; }
    .header .subtitle { color: #7f8c8d; font-size: 1.1rem; font-weight: 400; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(350px, 1fr)); gap: 25px; margin-bottom: 30px; }
    .card { background: #ffffff; border-radius: 12px; padding: 30px; box-shadow: 0 2px 10px rgba(0,0,0,0.08); border: 1px solid #e8e8e8; display: flex; align-items: center; text-align: left; transition: all 0.3s ease; position: relative; }
    .card:hover { transform: translateY(-2px); box-shadow: 0 8px 25px rgba(0,0,0,0.12); }
    .card::before { content: ''; position: absolute; top: 0; left: 0; right: 0; height: 4px; background: var(--card-color); border-radius: 12px 12px 0 0; }
    .icon { font-size: 45px; margin-right: 25px; min-width: 70px; text-align: center; color: var(--card-color); }
    .content { display: flex; flex-direction: column; flex: 1; }
    .label { font-size: 14px; color: #95a5a6; margin-bottom: 8px; font-weight: 600; text-transform: uppercase; letter-spacing: 1px; }
    .value { font-size: 32px; color: var(--card-color); display: flex; align-items: baseline; font-weight: 700; }
    .unit { font-size: 20px; color: #bdc3c7; margin-left: 8px; font-weight: 500; }
    .card:nth-child(1) { --card-color: #e74c3c; } /* Voltage */
    .card:nth-child(2) { --card-color: #3498db; } /* Current */
    .card:nth-child(3) { --card-color: #f39c12; } /* Power */
    .card:nth-child(4) { --card-color: #2ecc71; } /* Energy */
    .card:nth-child(5) { --card-color: #9b59b6; } /* Frequency */
    .card:nth-child(6) { --card-color: #e67e22; } /* Power Factor */
    .card:nth-child(7) { --card-color: #16a085; } /* Monthly Cost */
    .footer { text-align: center; color: #95a5a6; font-size: 14px; margin-top: 30px; padding-top: 20px; border-top: 1px solid #ecf0f1; }
    .status-dot { display: inline-block; width: 8px; height: 8px; background: #2ecc71; border-radius: 50%; margin-left: 8px; animation: pulse 2s infinite; }
    @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.5; } 100% { opacity: 1; } }
    .full-width {grid-column: 1 / -1;text-align: center;justify-content: center;}
    .full-width .content {align-items: center;}
    .filter-bar {display: flex;justify-content: center;align-items: center;gap: 15px;margin-bottom: 25px;padding: 15px;background: #f9f9f9;border-radius: 10px;box-shadow: 0 2px 8px rgba(0,0,0,0.08);}
    .filter-bar label { font-weight: 600; color: #2c3e50; }
    .filter-bar input, .filter-bar button { padding: 8px 12px; border-radius: 6px; border: 1px solid #ccc; font-size: 14px; }
    .filter-bar button { background: #3498db; color: white; border: none; cursor: pointer; transition: 0.3s; }
    .filter-bar button:hover { background: #2980b9; }
  </style>
  <script>
    function updateData() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var data = JSON.parse(this.responseText);
          document.getElementById('voltage').innerHTML = data.voltage + '<span class="unit">V</span>';
          document.getElementById('current').innerHTML = data.current + '<span class="unit">A</span>';
          document.getElementById('power').innerHTML = data.power + '<span class="unit">W</span>';
          document.getElementById('energy').innerHTML = data.energy + '<span class="unit">kWh</span>';
          document.getElementById('frequency').innerHTML = data.frequency + '<span class="unit">Hz</span>';
          document.getElementById('pf').innerHTML = data.pf;
          document.getElementById('monthly_cost').innerHTML = data.monthly_cost + '<span class="unit">baht</span>';
          document.getElementById('timestamp').textContent = new Date().toLocaleString();
        }
      };
      xhttp.open("GET", "/data", true);
      xhttp.send();
    }
    function resetData() {
      var xhttp = new XMLHttpRequest();
      xhttp.open("GET", "/reset", true);
      xhttp.send();
      alert("รีเซ็ตข้อมูลเรียบร้อยแล้ว (ยกเว้น Monthly Cost)");
    }
    setInterval(updateData, 2000);
    window.onload = updateData;
  </script>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1><i class="fas fa-bolt"></i> IOT Energy Management</h1>
      <p class="subtitle">ระบบตรวจสอบไฟฟ้าแบบเรียลไทม์โดย M6/2 <span class="status-dot"></span></p>
    </div>
    <div class="filter-bar">
      <label for="date">เลือกวันที่:</label>
      <input type="date" id="date">
      <label for="time">เลือกเวลา:</label>
      <input type="time" id="time">
      <button onclick="alert('00.')">ดูบิลค่าไฟฟ้าที่ผ่านมา.</button>
      <button onclick="resetData()">รีเซ็ตข้อมูล</button>
    </div>
    <div class="grid">
      <div class="card"><i class="fas fa-bolt icon"></i><div class="content"><div class="label">Voltage แรงดันไฟฟ้า</div><div class="value" id="voltage">%VOLTAGE%<span class="unit">V</span></div></div></div>
      <div class="card"><i class="fas fa-exchange-alt icon"></i><div class="content"><div class="label">Current กระแส</div><div class="value" id="current">%CURRENT%<span class="unit">A</span></div></div></div>
      <div class="card"><i class="fas fa-plug icon"></i><div class="content"><div class="label">Power พลังงาน</div><div class="value" id="power">%POWER%<span class="unit">W</span></div></div></div>
      <div class="card"><i class="fas fa-chart-line icon"></i><div class="content"><div class="label">Energy พลังงาน</div><div class="value" id="energy">%ENERGY%<span class="unit">kWh</span></div></div></div>
      <div class="card"><i class="fas fa-wave-square icon"></i><div class="content"><div class="label">Frequency ความถี่</div><div class="value" id="frequency">%FREQUENCY%<span class="unit">Hz</span></div></div></div>
      <div class="card"><i class="fas fa-percent icon"></i><div class="content"><div class="label">Power Factor ตัวประกอบกำลังไฟฟ้า</div><div class="value" id="pf">%PF%</div></div></div>
      <div class="card full-width"><i class="fas fa-money-bill-wave icon"></i><div class="content"><div class="label">Monthly Cost ค่าใช้จ่ายราย เดือน</div><div class="value" id="monthly_cost">%MONTHLY_COST%<span class="unit">baht</span></div></div></div>
    </div>
    <div class="footer"><p>ESP32 Power Monitor Dashboard | เวลาล่าสุดที่มีการแก้ไข: <span id="timestamp"></span></p></div>
  </div>
  <script>
  function resetData() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/reset", true);
    xhttp.send();
    alert("รีเซ็ตข้อมูลเรียบร้อยแล้ว");
  }

  // รีเฟรชหน้าเว็บทุก ๆ 3 วินาที
  function refreshPage() {
    location.reload();
  }
  setInterval(refreshPage, 3000);
</script>

</body>
</html>
)rawliteral";

String processor(const String& var) {
  if(var == "VOLTAGE") return isnan(voltage) ? "Error" : String(voltage, 1);
  else if(var == "CURRENT") return isnan(current) ? "Error" : String(current, 2);
  else if(var == "POWER") return isnan(power) ? "Error" : String(power, 1);
  else if(var == "ENERGY") return isnan(energy) ? "Error" : String(energy, 3);
  else if(var == "FREQUENCY") return isnan(frequency) ? "Error" : String(frequency, 1);
  else if(var == "PF") return isnan(pf) ? "Error" : String(pf, 2);
  else if(var == "MONTHLY_COST") return isnan(monthly_cost) ? "Error" : String(monthly_cost, 2);
  return String();
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{";
    json += "\"voltage\":\"" + String(isnan(voltage) ? "Error" : String(voltage, 1)) + "\",";
    json += "\"current\":\"" + String(isnan(current) ? "Error" : String(current, 2)) + "\",";
    json += "\"power\":\"" + String(isnan(power) ? "Error" : String(power, 1)) + "\",";
    json += "\"energy\":\"" + String(isnan(energy) ? "Error" : String(energy, 3)) + "\",";
    json += "\"frequency\":\"" + String(isnan(frequency) ? "Error" : String(frequency, 1)) + "\",";
    json += "\"pf\":\"" + String(isnan(pf) ? "Error" : String(pf, 2)) + "\",";
    json += "\"monthly_cost\":\"" + String(isnan(monthly_cost) ? "Error" : String(monthly_cost, 2)) + "\"";
    json += "}";
    request->send(200, "application/json", json);
  });

  // ✅ Reset endpoint
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    voltage = 0.0;
    current = 0.0;
    power = 0.0;
    energy = 0.0;
    frequency = 0.0;
    pf = 0.0;
    pzem.resetEnergy();  // รีเซ็ตค่าที่เก็บใน PZEM ด้วย
    request->send(200, "text/plain", "Data reset successful (except Monthly Cost)");
  });

  server.begin();
}

void loop() {
  float v = pzem.voltage();
  float c = pzem.current();
  float p = pzem.power();
  float e = pzem.energy();
  float f = pzem.frequency();
  float pf_val = pzem.pf();

  voltage = v;
  current = c;
  power = p;
  energy = e;
  frequency = f;
  pf = pf_val;

  // คำนวณค่าไฟ (สะสมต่อเนื่อง แม้ reset)
  monthly_cost = energy * price_per_kWh;

  // Serial output
  Serial.print("Voltage: "); Serial.print(voltage); Serial.println("V");
  Serial.print("Current: "); Serial.print(current); Serial.println("A");
  Serial.print("Power: "); Serial.print(power); Serial.println("W");
  Serial.print("Energy: "); Serial.print(energy,3); Serial.println("kWh");
  Serial.print("Frequency: "); Serial.print(frequency); Serial.println("Hz");
  Serial.print("PF: "); Serial.println(pf);
  Serial.print("Monthly Cost: "); Serial.print(monthly_cost, 2); Serial.println(" baht");
  Serial.println("-------------------");

  delay(2000);
}

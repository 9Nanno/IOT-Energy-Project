ESP32 Real-time Energy Monitor & Control
Arduino IDE (ESP32 Framework)

2. คุณสมบัติ (Features)
-วัดค่าแรงดันไฟฟ้า (Voltage), กระแส (Current), กำลังไฟฟ้า (Power) และพลังงาน (Energy)
-คำนวณค่าไฟอัตโนมัติ (บาท)
-ควบคุม Relay 2 ช่องผ่านหน้าเว็บ (AJAX - ไม่ต้องรีเฟรชหน้าจอ)
-แสดงผลผ่านจอ OLED 0.96"
-มีปุ่มกด Manual บนตัวเครื่องพร้อมระบบ Debounce

3. รายการอุปกรณ์ (Hardware Required)
-ESP32 Dev Kit
-PZEM-004T v3.0 (Energy Meter)
-OLED Display (SSD1306 128x64)
-Relay Module (2 Channels)
-Push Buttons (3 units)

4. การต่อวงจร (Wiring Diagram)
-PZEM-004T: RX -> GPIO17, TX -> GPIO16
-OLED: SDA -> GPIO21, SCL -> GPIO22
-Relays: R1 -> GPIO25, R2 -> GPIO27
-Buttons: BTN1 -> GPIO32, BTN2 -> GPIO33, BTN3 -> GPIO34

5. Library ที่ต้องใช้ (Dependencies)
-Adafruit_SSD1306 & Adafruit_GFX
-PZEM004Tv30
-WiFi & WebServer (Built-in ESP32)

6. วิธีการใช้งาน (Usage)
-แก้ไขค่า ssid และ password ในโค้ดให้ตรงกับ WiFi ของคุณ
-อัปโหลดโค้ดลง ESP32
-เปิด Serial Monitor เพื่อดู IP Address ที่ได้รับ
-เข้าหน้าเว็บผ่าน Browser ตาม IP นั้น เพื่อดูค่าพลังงานแบบ Real-time

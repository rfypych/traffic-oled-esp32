#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>
#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <WiFiManager.h>

// Konfigurasi layar OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Deklarasi objek display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool displayStarted = false;  // Flag untuk mengecek status display

// Inisialisasi WiFiManager
WiFiManager wifiManager;

// Konfigurasi NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "id.pool.ntp.org", 25200, 60000);  // GMT+7 (7*3600 = 25200)

// Variabel untuk status koneksi
bool wifiConnected = false;
unsigned long wifiConnectStart = 0;
const unsigned long WIFI_CONNECT_TIMEOUT = 30000; // 30 detik timeout

// Deklarasi fungsi yang akan digunakan
void setupGPIO();
void setLightState(int laneIndex, char color);
void updateWeather();
void updateDisplay();

// Fungsi untuk menampilkan status WiFi di OLED
void updateDisplay() {
  if (!displayStarted) return;  // Skip jika display tidak terdeteksi
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  
  // Update waktu dari NTP
  timeClient.update();
  String waktu = timeClient.getFormattedTime();
  
  // Tampilkan header
  display.println("LAMPU LALIN KOTA");
  display.print("Waktu: ");
  display.println(waktu);
  display.print("Status: ");
  display.println(wifiConnected ? "WiFi OK" : "No WiFi");
  
  // Garis pemisah
  display.println("----------------");
  
  // Tampilkan status lampu
  display.print("Mode: ");
  display.println(isAutoMode ? "Otomatis" : "Manual");
  
  display.display();
}

// Fungsi untuk menampilkan status WiFi di OLED
void displayWiFiStatus() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  if (WiFi.status() == WL_CONNECTED) {
    display.println(F("WiFi Terhubung"));
    display.print(F("SSID: "));
    display.println(WiFi.SSID());
    display.print(F("IP: "));
    display.println(WiFi.localIP());
  } else {
    display.println(F("Mode AP Aktif"));
    display.println(F("SSID: LAMPULALIN_AP"));
    display.println(F("IP: 192.168.4.1"));
    display.println(F("Buka browser dan"));
    display.println(F("kunjungi 192.168.4.1"));
  }
  
  display.display();
}

// Konfigurasi Pin GPIO
// Format: {R, Y, G} untuk setiap jalur
const int PIN_CONFIG[4][3] = {
  {4, 25, 26},  // Jalur 0: R=4, Y=25, G=26
  {17, 27, 32}, // Jalur 1: R=17, Y=27, G=32 (GPIO 22 dipindah ke 32 untuk menghindari konflik I2C)
  {16, 5, 18},  // Jalur 2: R=16, Y=5, G=18
  {19, 23, 33}  // Jalur 3: R=19, Y=23, G=33 (GPIO 21 dipindah ke 33 untuk menghindari konflik I2C)
};

const char* LANE_NAMES[] = {
  "HIJAU: UTARA",
  "HIJAU: TIMUR",
  "HIJAU: SELATAN",
  "HIJAU: BARAT"
};

// Durasi Waktu (dalam milidetik)
const unsigned long GREEN_DURATION = 20000;  // 20 detik
const unsigned long YELLOW_DURATION = 5000;  // 5 detik

// Variabel status
int currentLaneIndex = 0;
char currentLightState = 'G';  // 'G' untuk hijau, 'Y' untuk kuning
unsigned long lastLightChangeTime = 0;
unsigned long lastWeatherUpdateTime = 0;
const unsigned long WEATHER_UPDATE_INTERVAL = 300000;  // 5 menit

// Variabel cuaca
struct Weather {
  String icon;
  int temp;
  String label;
} currentWeather;

const Weather WEATHER_CONDITIONS[] = {
  {"☀️", random(28, 35), "Cerah"},
  {"☁️", random(24, 29), "Berawan"},
  {"🌧️", random(22, 26), "Hujan"}
};

void setup() {
  // Inisialisasi Serial
  Serial.begin(115200);
  
  // Inisialisasi I2C
  Wire.begin(21, 22); // SDA, SCL - Pastikan pin sesuai dengan koneksi
  
  // Inisialisasi display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    displayStarted = false;
  } else {
    displayStarted = true;
    display.clearDisplay();
    display.display();
    delay(1000);
    
    // Inisialisasi NTP
    timeClient.begin();
    timeClient.update();
  }
  
  // Setel properti tampilan
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();
  
  // Tampilkan status WiFi
  displayWiFiStatus();
  
  // Konfigurasi WiFi Manager
  wifiManager.setConfigPortalTimeout(180); // Timeout 3 menit
  
  // Coba koneksi WiFi
  bool res = wifiManager.autoConnect("LAMPULALIN_AP", "akucahgrisa");
  
  if(!res) {
    Serial.println("Gagal terhubung ke WiFi");
  } else {
    Serial.println("Terhubung ke WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    wifiConnected = true;
  }
  
  // Inisialisasi EEPROM
  EEPROM.begin(512);
  
  // Inisialisasi GPIO
  setupGPIO();
  
  // Atur tampilan awal
  display.clearDisplay();
  display.display();
  
  // Setel cuaca awal
  updateWeather();
  
  // Nyalakan lampu merah untuk semua jalur kecuali jalur pertama
  for (int i = 0; i < 4; i++) {
    setLightState(i, (i == currentLaneIndex) ? 'G' : 'R');
  }
  
  // Mulai web server
  setupWebServer();
  
  Serial.println("Sistem lampu lalu lintas ESP32 berjalan");
}

// Web Server
WebServer server(80);

// Halaman HTML untuk konfigurasi
const char* html = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Konfigurasi Lampu Lalu Lintas</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin: 0 auto; padding: 20px; }
    .container { max-width: 500px; margin: 0 auto; }
    .form-group { margin: 15px 0; }
    input { padding: 8px; width: 80%; margin: 5px 0; }
    button { background: #4CAF50; color: white; border: none; padding: 10px 20px; cursor: pointer; }
    button:hover { background: #45a049; }
  </style>
</head>
<body>
  <div class="container">
    <h2>Konfigurasi WiFi</h2>
    <form action="/save" method="POST">
      <div class="form-group">
        <input type="text" name="ssid" placeholder="Nama WiFi (SSID)" required>
      </div>
      <div class="form-group">
        <input type="password" name="password" placeholder="Password WiFi">
      </div>
      <button type="submit">Simpan & Restart</button>
    </form>
  </div>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", html);
}

void handleSave() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  
  // Simpan ke EEPROM (contoh sederhana)
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(100, password);
  EEPROM.commit();
  
  String message = "<h2>Konfigurasi Disimpan!</h2><p>Menyambungkan ke " + ssid + "...</p>";
  server.send(200, "text/html", message);
  
  // Coba koneksi ke WiFi baru
  WiFi.begin(ssid.c_str(), password.c_str());
  
  // Tunggu koneksi atau timeout
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nTerhubung ke WiFi baru!");
    wifiConnected = true;
  } else {
    Serial.println("\nGagal terhubung ke WiFi baru");
  }
  
  // Restart ESP
  delay(2000);
  ESP.restart();
}

void setupWebServer() {
  // Setup halaman web
  server.on("/", HTTP_GET, []() {
    String html = "<html><head><title>ESP32 Lampu Lalu Lintas</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family: Arial; margin: 20px;}</style>";
    html += "</head><body>";
    html += "<h1>Kontrol Lampu Lalu Lintas</h1>";
    html += "<h2>Status: " + String(wifiConnected ? "Terhubung ke " + WiFi.SSID() : "Mode AP") + "</h2>";
    html += "<p>IP Address: " + WiFi.localIP().toString() + "</p>";
    
    // Form konfigurasi WiFi
    html += "<h3>Konfigurasi WiFi</h3>";
    html += "<form method='post' action='/wifi'>";
    html += "SSID: <input type='text' name='ssid'><br>";
    html += "Password: <input type='password' name='pass'><br>";
    html += "<input type='submit' value='Simpan'>";
    html += "</form>";
    
    // Status lampu
    html += "<h3>Status Lampu</h3>";
    html += "<p>Jalur Aktif: " + String(LANE_NAMES[currentLaneIndex]) + "</p>";
    html += "<p>Status: " + String(currentLightState == 'G' ? "Hijau" : "Kuning") + "</p>";
    
    html += "</body></html>";
    server.send(200, "text/html", html);
  });

  // Handle penyimpanan konfigurasi WiFi
  server.on("/wifi", HTTP_POST, []() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    
    // Simpan ke EEPROM
    EEPROM.writeString(0, ssid);
    EEPROM.writeString(64, pass);
    EEPROM.commit();
    
    String message = "<h1>Konfigurasi Disimpan</h1>";
    message += "<p>Menyimpan konfigurasi dan menghubungkan ke jaringan...</p>";
    message += "<p>SSID: " + ssid + "</p>";
    server.send(200, "text/html", message);
    
    // Coba hubungkan ke WiFi baru
    WiFi.begin(ssid.c_str(), pass.c_str());
    delay(3000);
    ESP.restart();
  });
  
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  // Handle web server requests
  if (wifiConnected) {
    server.handleClient();
  } else {
    // Mode AP aktif, tangani permintaan konfigurasi
    wifiManager.process();
  }
  
  unsigned long currentMillis = millis();
  
  // 1. Logika lampu lalu lintas
  unsigned long duration = (currentLightState == 'G') ? GREEN_DURATION : YELLOW_DURATION;
  
  if (currentMillis - lastLightChangeTime >= duration) {
    lastLightChangeTime = currentMillis;
    
    if (currentLightState == 'G') {
      // Transisi dari Hijau ke Kuning
      currentLightState = 'Y';
      setLightState(currentLaneIndex, 'Y');
      Serial.print(LANE_NAMES[currentLaneIndex]);
      Serial.println(": KUNING (5 detik)");
    } else {
      // Transisi dari Kuning ke jalur berikutnya
      setLightState(currentLaneIndex, 'R');  // Matikan jalur lama
      
      currentLaneIndex = (currentLaneIndex + 1) % 4;
      currentLightState = 'G';
      
      setLightState(currentLaneIndex, 'G');  // Nyalakan jalur baru
      Serial.print(LANE_NAMES[currentLaneIndex]);
      Serial.println(": HIJAU (20 detik)");
    }
  }
  
  // 2. Perbarui cuaca secara berkala
  if (currentMillis - lastWeatherUpdateTime >= WEATHER_UPDATE_INTERVAL) {
    updateWeather();
    lastWeatherUpdateTime = currentMillis;
    
    Serial.print("Cuaca diperbarui: ");
    Serial.print(currentWeather.label);
    Serial.print(F(" ("));
    Serial.print(currentWeather.temp);
    Serial.println("°C)");
  }
  
  // 3. Update tampilan
  updateDisplay();
  
  delay(100);  // Beri waktu untuk perangkat lain
}

void setupGPIO() {
  // Setel semua pin sebagai OUTPUT dan matikan
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      pinMode(PIN_CONFIG[i][j], OUTPUT);
      digitalWrite(PIN_CONFIG[i][j], LOW);
    }
  }
}

void setLightState(int laneIndex, char color) {
  // Matikan semua lampu di jalur ini
  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_CONFIG[laneIndex][i], LOW);
  }
  
  // Nyalakan lampu yang sesuai
  if (color == 'R') {
    digitalWrite(PIN_CONFIG[laneIndex][0], HIGH);  // Merah
  } else if (color == 'Y') {
    digitalWrite(PIN_CONFIG[laneIndex][1], HIGH);  // Kuning
  } else if (color == 'G') {
    digitalWrite(PIN_CONFIG[laneIndex][2], HIGH);  // Hijau
  }
}

void updateWeather() {
  // Pilih kondisi cuaca acak yang berbeda dari sebelumnya
  int newIndex = random(3);
  while (WEATHER_CONDITIONS[newIndex].label == currentWeather.label) {
    newIndex = random(3);
  }
  
  currentWeather.icon = WEATHER_CONDITIONS[newIndex].icon;
  currentWeather.temp = WEATHER_CONDITIONS[newIndex].temp;
  currentWeather.label = WEATHER_CONDITIONS[newIndex].label;
}

void updateDisplay() {
  display.clearDisplay();
  
  // Atur warna teks (putih)
  display.setTextColor(SSD1306_WHITE);
  
  // 1. Baris Atas: Cuaca (kiri) dan Tanggal (kanan)
  display.setFont();  // Gunakan font default kecil
  String weatherText = currentWeather.icon + " " + String(currentWeather.temp) + "C";
  display.setCursor(0, 0);
  display.print(weatherText);
  
  // Tampilkan tanggal
  String dateText = "18/09/25";  // Format: DD/MM/YY
  display.setCursor(SCREEN_WIDTH - 6 * dateText.length(), 0);
  display.print(dateText);
  
  // 2. Baris Tengah: Waktu
  display.setFont(&FreeSans9pt7b);
  String timeText = "12:00";  // Format: HH:MM
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(timeText, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT - h) / 2 + h);
  display.print(timeText);
  
  // 3. Baris Bawah: Jalur Aktif
  display.setFont();  // Kembali ke font default
  String laneText = "JALUR " + String(LANE_NAMES[currentLaneIndex]);
  display.setCursor(0, SCREEN_HEIGHT - 10);
  display.print(laneText);
  
  // Tampilkan semuanya
  display.display();
}

# Lampu Lalu Lintas ESP32

Proyek ini mengimplementasikan sistem lampu lalu lintas cerdas menggunakan ESP32 DevKit V1 dengan tampilan OLED dan antarmuka web untuk konfigurasi.

## Spesifikasi Hardware

### Komponen yang Dibutuhkan
- 1x ESP32 DevKit V1
- 1x Modul OLED SSD1306 (128x64, I2C)
- 12x LED (4x Merah, 4x Kuning, 4x Hijau)
- 12x Resistor 220Ω
- Kabel jumper
- Breadboard

### Pinout ESP32 DevKit V1

#### Konfigurasi Pin Lampu Lalu Lintas
Setiap jalur menggunakan 3 pin (R, Y, G):

| Jalur | Merah (R) | Kuning (Y) | Hijau (G) |
|-------|-----------|------------|-----------|
| Utara | GPIO 4    | GPIO 25    | GPIO 26   |
| Timur | GPIO 17   | GPIO 27    | GPIO 32   |
| Selatan | GPIO 16  | GPIO 5     | GPIO 18   |
| Barat | GPIO 19   | GPIO 23    | GPIO 33   |

#### Koneksi I2C OLED
| OLED | ESP32 |
|------|-------|
| VCC  | 3.3V  |
| GND  | GND   |
| SCL  | GPIO 22 |
| SDA  | GPIO 21 |

> **Catatan**: Pin I2C default untuk ESP32 adalah SDA=21, SCL=22. Pastikan tidak ada konflik dengan pin lain.

## Instalasi

1. Pasang library yang diperlukan melalui Library Manager Arduino IDE:
   - WiFiManager by tzapu
   - Adafruit SSD1306
   - Adafruit GFX Library
   - Adafruit BusIO

## Flashing Firmware

### File Binary yang Dibutuhkan
Anda hanya memerlukan file binary utama:
- `lampu_lalin_esp32.ino.bin` (alamat flash: 0x10000)

### Langkah Flashing Sederhana
1. Buka [ESP Web Tools](https://espressif.github.io/esptool-js/)
2. Hubungkan ESP32 ke komputer via USB
3. Klik "Connect" dan pilih port COM yang sesuai
4. Klik "Choose Files" dan pilih file `lampu_lalin_esp32.ino.bin`
5. Pastikan alamat flash diatur ke `0x10000`
6. Klik "Program" untuk memulai proses flashing
7. Tunggu hingga proses selesai dan ESP32 akan me-restart otomatis

### Catatan Penting
- Pastikan memilih chip ESP32 di pengaturan flash
- Gunakan mode flash DIO
- Kecepatan flash disarankan 40MHz
- Jika gagal, coba langkah berikut:
  1. Tekan dan tahan tombol BOOT di ESP32
  2. Tekan tombol RESET sambil tetap menahan BOOT
  3. Lepaskan RESET terlebih dahulu, lalu lepaskan BOOT
  4. Coba flashing ulang

## Konfigurasi WiFi

### Konfigurasi Awal
1. Setelah upload, ESP32 akan membuat access point dengan:
   - **SSID**: `LAMPULALIN_AP`
   - **Password**: `akucahgrisa`
   - **IP**: `192.168.4.1`

2. Hubungkan perangkat (HP/Laptop) ke jaringan `LAMPULALIN_AP`

3. Buka browser dan akses `http://192.168.4.1`

4. Masukkan kredensial WiFi yang diinginkan:
   - Pilih SSID dari daftar atau ketik manual
   - Masukkan password WiFi
   - Klik "Simpan"

5. ESP32 akan me-restart dan mencoba terhubung ke jaringan WiFi yang baru dikonfigurasi

### Mengganti Jaringan WiFi
Ada dua cara untuk mengubah jaringan WiFi:

**Cara 1: Reset Manual**
1. Tekan dan tahan tombol **BOOT** selama 10 detik
2. ESP32 akan me-reset pengaturan WiFi
3. Kembali ke mode AP (`LAMPULALIN_AP`)
4. Ikuti langkah konfigurasi awal

**Cara 2: Melalui Web Interface**
1. Buka alamat IP ESP32 di browser
2. Akan muncul halaman konfigurasi WiFi
3. Masukkan SSID dan password baru
4. Klik "Simpan" untuk menerapkan pengaturan

### Catatan Penting
- Password WiFi disimpan di EEPROM dan tidak akan hilang meskipun daya mati
- Jika gagal terhubung ke jaringan, ESP32 akan kembali ke mode AP setelah 30 detik
- Pastikan sinyal WiFi cukup kuat di lokasi pemasangan
- Untuk keamanan, gunakan password yang kuat untuk jaringan WiFi

## Fitur

- Kontrol lampu lalu lintas otomatis dengan durasi yang dapat dikonfigurasi
- Antarmuka web untuk monitoring dan konfigurasi
- Tampilan status di layar OLED
- Mode Access Point untuk konfigurasi awal
- Penyimpanan pengaturan WiFi di EEPROM

## Struktur Kode

- `setup()`: Inisialisasi hardware dan koneksi WiFi
- `loop()`: Logika utama lampu lalu lintas
- `setupWebServer()`: Konfigurasi web server
- `setLightState()`: Mengontrol nyala/mati lampu
- `updateDisplay()`: Memperbarui tampilan OLED
- `updateWeather()`: Simulasi data cuaca

## Troubleshooting

1. **Lampu tidak menyala**:
   - Periksa koneksi kabel
   - Pastikan resistor terpasang dengan benar
   - Verifikasi konfigurasi pin di kode

2. **OLED tidak menampilkan apa-apa**:
   - Periksa koneksi I2C
   - Pastikan alamat I2C benar (biasanya 0x3C)
   - Cek koneksi power (3.3V)

3. **Tidak bisa terhubung ke WiFi**:
   - Reset ESP32 dan coba lagi
   - Pastikan SSID dan password benar
   - Periksa apakah jaringan WiFi tersedia

## Lisensi

Proyek ini dilisensikan di bawah [MIT License](LICENSE).

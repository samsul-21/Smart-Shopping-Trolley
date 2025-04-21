#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Konstanta pin
#define bt_RX 2
#define bt_TX 3
#define bt_Vin 4
#define led 13  // LED indikator

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Dua OLED, misalnya 0x3C dan 0x3D
Adafruit_SSD1306 oled1(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Alamat 0x3C
Adafruit_SSD1306 oled2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Alamat 0x3D

SoftwareSerial BTSerial(bt_RX, bt_TX);  // Untuk komunikasi dengan HC-06

// Struktur data untuk barang
struct Item {
  const char* code;
  const char* name;
  float weight; // gram
  int price;    // rupiah
};

// Daftar item
Item items[] = {
  {"ITEM001", "Sabun Mandi", 200, 5000},
  {"ITEM002", "Sampo", 120, 10000},
  {"ITEM003", "Mie Instan", 85, 3500},
};

int itemCount = sizeof(items) / sizeof(items[0]);

// Variabel total
int totalHarga = 0;
float totalBerat = 0;

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);

  pinMode(bt_RX, INPUT);
  pinMode(bt_TX, OUTPUT);
  pinMode(bt_Vin, OUTPUT);
  pinMode(led, OUTPUT);

  digitalWrite(bt_Vin, HIGH);  // Aktifkan HC-06

  // Inisialisasi OLED
  if (!oled1.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ OLED1 gagal");
    while (1);
  }
  if (!oled2.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
    Serial.println("❌ OLED2 gagal");
    while (1);
  }

  // Tampilan awal
  oled1.clearDisplay();
  oled1.setTextColor(SSD1306_WHITE);
  oled1.setTextSize(1);
  oled1.setCursor(0, 0);
  oled1.println("📦 Scan Barcode...");
  oled1.display();

  oled2.clearDisplay();
  oled2.setTextColor(SSD1306_WHITE);
  oled2.setTextSize(1);
  oled2.setCursor(0, 0);
  oled2.println("Total:");
  oled2.print("Rp "); oled2.println(totalHarga);
  oled2.print("Berat: "); oled2.print(totalBerat); oled2.println(" g");
  oled2.display();

  Serial.println("📦 Silakan input kode barcode (contoh: ITEM001)");
}

void loop() {
  static String input = "";

  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      input.trim();
      prosesBarcode(input);
      input = "";
    } else {
      input += c;
    }
  }
}

// Fungsi untuk mencari item dan menampilkan hasil
void prosesBarcode(String code) {
  bool found = false;

  for (int i = 0; i < itemCount; i++) {
    if (code == items[i].code) {
      totalHarga += items[i].price;
      totalBerat += items[i].weight;

      Serial.println("✅ Barang Ditambahkan:");
      Serial.print("Nama  : "); Serial.println(items[i].name);
      Serial.print("Harga : Rp "); Serial.println(items[i].price);
      Serial.print("Berat : "); Serial.print(items[i].weight); Serial.println(" gram");
      Serial.println("-----------------------------");
      Serial.print("Total Harga : Rp "); Serial.println(totalHarga);
      Serial.print("Total Berat : "); Serial.print(totalBerat); Serial.println(" gram\n");

      // Kirim via Bluetooth
      BTSerial.print("Nama: "); BTSerial.println(items[i].name);
      BTSerial.print("Harga: "); BTSerial.println(items[i].price);
      BTSerial.print("Total Harga: "); BTSerial.println(totalHarga);
      BTSerial.print("Total Berat: "); BTSerial.println(totalBerat);
      BTSerial.println("------------------------");

      // Tampilkan di OLED1: Info barang
      oled1.clearDisplay();
      oled1.setCursor(0, 0);
      oled1.setTextSize(1);
      oled1.println("Barang Ditambahkan:");
      oled1.print("> "); oled1.println(items[i].name);
      oled1.print("Rp "); oled1.println(items[i].price);
      oled1.print("Berat: "); oled1.print(items[i].weight); oled1.println("g");
      oled1.display();

      // Tampilkan di OLED2: Total
      oled2.clearDisplay();
      oled2.setCursor(0, 0);
      oled2.setTextSize(1);
      oled2.println("Total:");
      oled2.print("Rp "); oled2.println(totalHarga);
      oled2.print("Berat: "); oled2.print(totalBerat); oled2.println(" g");
      oled2.display();

      // LED indikator
      digitalWrite(led, HIGH);
      delay(200);
      digitalWrite(led, LOW);

      found = true;
      break;
    }
  }

  if (!found) {
    Serial.println("❌ Kode tidak ditemukan. Coba lagi.");
    BTSerial.println("Kode tidak ditemukan.");

    // Tampilkan error ke OLED1
    oled1.clearDisplay();
    oled1.setCursor(0, 0);
    oled1.setTextSize(1);
    oled1.println("❌ Kode tidak dikenal");
    oled1.println(code);
    oled1.display();
  }
}

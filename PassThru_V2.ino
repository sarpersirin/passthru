#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// RFID Pinleri (Donanımsal SPI)
#define SS_PIN 10
#define RST_PIN 9

// TFT Ekran Pinleri (Yazılımsal SPI)
#define TFT_CS     8
#define TFT_RST    7
#define TFT_DC     6
#define TFT_MOSI   5 
#define TFT_SCK    4 

#define BUZZER_PIN 2

MFRC522 rfid(SS_PIN, RST_PIN);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);

enum Mode { MAIN_MENU, LOAD_BALANCE, WAIT_FOR_CARD_LOAD, CHECK_BALANCE };
Mode currentMode = MAIN_MENU;

String loadAmount = "";
int fareAmount = 40; 

void setup() {
  Serial.begin(9600);
  
  SPI.begin();
  rfid.PCD_Init();
  
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1); 
  tft.fillScreen(ST7735_BLACK);
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  drawMainMenu();
}

void loop() {
  static String serialBuffer = "";
  char key = 0;

  if (Serial.available()) {
    char incoming = Serial.read();
    if (incoming == '\n' || incoming == '\r') {
      key = 'E';
    } else {
      if (currentMode == MAIN_MENU && (incoming == '#' || incoming == '0')) {
        key = incoming;
        serialBuffer = "";
      } else {
        serialBuffer += incoming; 
      }
    }
  }

  switch (currentMode) {
    case MAIN_MENU:
      if (key == '#') {
        tft.fillScreen(ST7735_BLACK);
        tft.setCursor(10, 20);
        tft.setTextColor(ST7735_CYAN);
        tft.setTextSize(2);
        tft.print("BAKIYE YUKLE");
        
        tft.drawFastHLine(0, 45, 160, ST7735_WHITE);
        loadAmount = "";
        currentMode = LOAD_BALANCE;
      } else if (key == '0') {
        tft.fillScreen(ST7735_BLACK);
        
        tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
        tft.setTextSize(3);
        tft.setCursor(26, 15);
        tft.print("BAKIYE");
        tft.setCursor(17, 50);
        tft.print("SORGULA");
        
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setTextSize(2);
        tft.setCursor(26, 95);
        tft.print("KART OKUT");
        
        currentMode = CHECK_BALANCE;
      } else {
        if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
          processCardUsage();
          rfid.PICC_HaltA();
          rfid.PCD_StopCrypto1();
        }
      }
      break;

    case LOAD_BALANCE:
      if (serialBuffer.length() > 0 && isDigit(serialBuffer[serialBuffer.length() - 1])) {
        loadAmount = serialBuffer;
        
        tft.fillRect(0, 60, 160, 40, ST7735_BLACK);
        
        tft.setCursor(15, 65);
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setTextSize(4); 
        tft.print(loadAmount);
        tft.setTextSize(2);
        tft.print("TL");
      }

      if (key == 'E') {
        loadAmount = serialBuffer;
        serialBuffer = "";
        
        tft.fillScreen(ST7735_BLUE);
        tft.setTextColor(ST7735_WHITE, ST7735_BLUE);
        tft.setTextSize(3);
        
        tft.setCursor(44, 35);
        tft.print("KART");
        
        tft.setCursor(44, 70);
        tft.print("OKUT");
        
        Serial.println("System: Scan card...");
        currentMode = WAIT_FOR_CARD_LOAD;
      }
      break;

    case WAIT_FOR_CARD_LOAD:
      if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        writeBalanceToCard(loadAmount.toInt());
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
        
        tft.fillScreen(ST7735_GREEN);
        tft.setTextColor(ST7735_BLACK, ST7735_GREEN);
        tft.setTextSize(3);
        
        tft.setCursor(8, 35);
        tft.print("BASARILI");
        
        tft.setCursor(71, 70);
        tft.print("!");
        
        tone(BUZZER_PIN, 2500); delay(100); noTone(BUZZER_PIN);
        delay(2000);
        drawMainMenu();
        currentMode = MAIN_MENU;
      }
      break;

    case CHECK_BALANCE:
      if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        int balance = readBalance();
        String uid = getUID();
        
        tft.fillScreen(ST7735_BLACK);
        tft.setCursor(10, 20);
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setTextSize(2);
        tft.print("Mevcut Bakiye:");
        
        tft.setCursor(15, 60);
        tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
        tft.setTextSize(4);
        tft.print(balance);
        tft.setTextSize(2);
        tft.print("TL");
        
        Serial.println("UID:" + uid + ",BAL:" + String(balance) + ",STATUS:CHECK");
        
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
        delay(3000);
        drawMainMenu();
        currentMode = MAIN_MENU;
      }
      break;
  }
}

// ANA MENÜ TASARIMI - İKON VE YAZILAR BURADA!
void drawMainMenu() {
  tft.fillScreen(ST7735_BLACK);
  
  // ========================================================
  // 1. ADIM: ÖNCE İKON ÇİZİLİR (Maskelerin yazıyı silmemesi için)
  // ========================================================
  int cx = 90; // İkonun başlangıç X koordinatı
  int cy = 70; // İkonun başlangıç Y koordinatı
  
  // 4 adet kalın çember çiziyoruz (Dalgalar)
  for(int r = 3;  r <= 5;  r++) tft.drawCircle(cx, cy, r, ST7735_WHITE);
  for(int r = 12; r <= 14; r++) tft.drawCircle(cx, cy, r, ST7735_WHITE);
  for(int r = 21; r <= 23; r++) tft.drawCircle(cx, cy, r, ST7735_WHITE);
  for(int r = 30; r <= 32; r++) tft.drawCircle(cx, cy, r, ST7735_WHITE);
  
  // Maskeleme 1: Sol tarafı tamamen siyah bir dikdörtgenle sil (Sadece sağa açılan yay kalsın)
  tft.fillRect(cx - 35, cy - 35, 35, 70, ST7735_BLACK);
  
  // Maskeleme 2: Üst ve alt kısımları üçgenlerle 45 derece açıyla sil
  // Bu işlem tam olarak attığın görseldeki o "huni şeklinde yayılan" dalga şeklini verecek!
  tft.fillTriangle(cx, cy, cx + 35, cy - 35, cx, cy - 35, ST7735_BLACK); // Üst köşeyi sil
  tft.fillTriangle(cx, cy, cx + 35, cy + 35, cx, cy + 35, ST7735_BLACK); // Alt köşeyi sil
  
  // En içteki küçük dalgayı daha dolgun bir noktaya çevir
  tft.fillCircle(cx + 4, cy, 3, ST7735_WHITE);
  
  // ========================================================
  // 2. ADIM: ŞİMDİ YAZILAR VE ÇİZGİLER ÇİZİLİR (Üste binecek)
  // ========================================================
  
  // Başlık ve İmza
  tft.setCursor(15, 5);
  tft.setTextColor(ST7735_CYAN, ST7735_BLACK);
  tft.setTextSize(2);
  tft.print("PassThru V2");
  
  tft.setCursor(55, 25);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  tft.setTextSize(1);
  tft.print("by Sarper");
  
  // Üst Çizgi (Artık ikon maskesinden sonra çizildiği için asla kesilmez)
  tft.drawFastHLine(0, 40, 160, ST7735_WHITE);
  
  // SCAN CARD Yazısı (Full sola, X=5'e yaslandı)
  tft.setCursor(5, 48);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  tft.setTextSize(3); 
  tft.print("SCAN");
  
  tft.setCursor(5, 74);
  tft.print("CARD");
  
  // Alt Durum Çizgisi
  tft.setCursor(8, 105);
  tft.setTextSize(2);
  tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
  tft.print("Status:Ready");
}

String getUID() {
  String uidStr = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uidStr += "0";
    uidStr += String(rfid.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();
  return uidStr;
}

void writeBalanceToCard(int amount) {
  byte block = 4;
  byte buffer[16] = {0};
  
  buffer[0] = (amount >> 8) & 0xFF; 
  buffer[1] = amount & 0xFF;        

  MFRC522::StatusCode status;
  MFRC522::MIFARE_Key key;

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) return;

  rfid.MIFARE_Write(block, buffer, 16);
}

int readBalance() {
  byte block = 4;
  byte buffer[18];
  byte size = 18;
  MFRC522::StatusCode status;
  MFRC522::MIFARE_Key key;

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) return -1;

  status = rfid.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK) return -1;

  int balance = (buffer[0] << 8) | buffer[1];
  return balance;
}

void processCardUsage() {
  String uid = getUID();
  int currentBalance = readBalance();
  
  if (currentBalance < 0) {
    tft.fillScreen(ST7735_RED);
    tft.setCursor(44, 30);
    tft.setTextColor(ST7735_WHITE, ST7735_RED);
    tft.setTextSize(3);
    tft.print("READ");
    tft.setCursor(53, 65);
    tft.print("ERR");
    
    Serial.println("UID:" + uid + ",BAL:0,STATUS:TAMPERED");
    delay(2000);
    drawMainMenu();
    return;
  }

  if (currentBalance >= fareAmount) {
    int newBalance = currentBalance - fareAmount;
    
    byte newBuffer[16] = {0};
    newBuffer[0] = (newBalance >> 8) & 0xFF;
    newBuffer[1] = newBalance & 0xFF;

    MFRC522::StatusCode status;
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

    status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(rfid.uid));
    status = rfid.MIFARE_Write(4, newBuffer, 16);

    if (status == MFRC522::STATUS_OK) {
      tft.fillScreen(ST7735_GREEN);
      tft.setCursor(17, 10);
      tft.setTextColor(ST7735_BLACK, ST7735_GREEN); 
      tft.setTextSize(3);
      tft.print("GRANTED");
      
      tft.setCursor(15, 60);
      tft.setTextSize(2);
      tft.print("Bakiye:");
      
      tft.setCursor(15, 85);
      tft.setTextSize(3);
      tft.print(newBalance);
      tft.setTextSize(2);
      tft.print(" TL");
      
      Serial.println("UID:" + uid + ",BAL:" + String(newBalance) + ",STATUS:GRANTED");

      tone(BUZZER_PIN, 2500); delay(150); noTone(BUZZER_PIN);
      delay(50);
      tone(BUZZER_PIN, 2500); delay(150); noTone(BUZZER_PIN);
    } else {
      tft.fillScreen(ST7735_RED);
      tft.setCursor(44, 30);
      tft.setTextColor(ST7735_WHITE, ST7735_RED);
      tft.setTextSize(3);
      tft.print("WRITE");
      tft.setCursor(53, 65);
      tft.print("ERR");
      Serial.println("UID:" + uid + ",BAL:" + String(currentBalance) + ",STATUS:ERROR");
    }
  } else {
    tft.fillScreen(ST7735_RED);
    tft.setCursor(17, 20);
    tft.setTextColor(ST7735_WHITE, ST7735_RED);
    tft.setTextSize(3);
    tft.print("DENIED!");
    
    tft.setCursor(5, 70);
    tft.setTextSize(2);
    tft.print("Yetersiz");
    tft.setCursor(5, 95);
    tft.print("Bakiye!");
    
    Serial.println("UID:" + uid + ",BAL:" + String(currentBalance) + ",STATUS:DENIED");

    tone(BUZZER_PIN, 2500); delay(600); noTone(BUZZER_PIN);
  }

  delay(2500);
  drawMainMenu();
  currentMode = MAIN_MENU;
}
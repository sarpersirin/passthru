#include <Wire.h>

#include <LiquidCrystal_I2C.h>

#include <SPI.h>

#include <MFRC522.h>

 

LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C LCD

 

#define SS_PIN 10

#define RST_PIN 9

#define BUZZER_PIN 2

 

MFRC522 rfid(SS_PIN, RST_PIN);

 

enum Mode { MAIN_MENU, LOAD_BALANCE, WAIT_FOR_CARD_LOAD, CHECK_BALANCE };

Mode currentMode = MAIN_MENU;

 

String loadAmount = "";

int fareAmount = 1;

 

void setup() {

  Serial.begin(9600);

 

  Wire.begin();

  lcd.init();

  lcd.backlight();

  lcd.setCursor(0, 0);

  lcd.print("PassThru V1");

 

  SPI.begin();

  rfid.PCD_Init();

 

  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);

}

 

void loop() {

  static String serialBuffer = "";

  char key = 0;

  

 

  // Seri porttan veri okuyalım

  if (Serial.available()) {

    char incoming = Serial.read();

 

    // ENTER tuşu (onay)

    if (incoming == '\n' || incoming == '\r') {

      key = 'E';

    }

    else {

      serialBuffer += incoming;

 

      // Menü komutları tek karakter olduğu için hemen işle

      if (incoming == '#' || incoming == '0') {

        key = incoming;

        serialBuffer = "";

      }

    }

  }

 

  switch (currentMode) {

 

    case MAIN_MENU:

      if (key == '#') {

        lcd.clear();

        lcd.print("Load Balance:");

        loadAmount = "";

        currentMode = LOAD_BALANCE;

      }

      else if (key == '0') {

        lcd.clear();

        lcd.print("Check Balance");

        currentMode = CHECK_BALANCE;

      }

      else {

        if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {

          processCardUsage();

          rfid.PICC_HaltA();

          rfid.PCD_StopCrypto1();

        }

      }

      break;

 

    case LOAD_BALANCE:

      // Rakamlar geldikçe miktarı güncelle

      if (serialBuffer.length() > 0 && isDigit(serialBuffer[serialBuffer.length() - 1])) {

        loadAmount = serialBuffer;

        lcd.setCursor(0, 1);

        lcd.print("Amount: " + loadAmount + " ");

      }

 

      // ENTER → kart okutma

      if (key == 'E') {

 

        loadAmount = serialBuffer; // <-- 0 SONLU SAYILAR İÇİN KRİTİK DÜZELTME

        serialBuffer = "";

 

        lcd.clear();

        lcd.print("Scan card...");

        Serial.println("Scan card...");

        currentMode = WAIT_FOR_CARD_LOAD;

      }

      break;

 

    case WAIT_FOR_CARD_LOAD:
    

      if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {

        writeBalanceToCard(loadAmount.toInt());

        rfid.PICC_HaltA();

        rfid.PCD_StopCrypto1();

        lcd.clear();
      
        lcd.print("Load successful!");
        Serial.println("Load successful!");
        delay(100);
tone(BUZZER_PIN, 2500); delay(100); noTone(BUZZER_PIN);
        delay(3000);
        
        

        lcd.clear();

        lcd.print("PassThru V1");

        currentMode = MAIN_MENU;

      }

      break;

 

    case CHECK_BALANCE:

      if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {

        int balance = readBalance();

        lcd.clear();

        lcd.print("Your balance:");

        lcd.setCursor(0, 1);

        lcd.print(balance);
        Serial.println(balance);

        rfid.PICC_HaltA();

        rfid.PCD_StopCrypto1();

        delay(3000);

        lcd.clear();

        lcd.print("PassThru V1");

        currentMode = MAIN_MENU;

      }

      break;

  }

}

 

void writeBalanceToCard(int amount) {

  byte block = 4;

  byte buffer[16] = {0};

  buffer[0] = (byte)amount;

 

  MFRC522::StatusCode status;

  MFRC522::MIFARE_Key key;

 

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

 

  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(rfid.uid));

  if (status != MFRC522::STATUS_OK) {

    lcd.clear();

    lcd.print("Auth error!");

    return;

  }

 

  status = rfid.MIFARE_Write(block, buffer, 16);

  if (status != MFRC522::STATUS_OK) {

    lcd.clear();

    lcd.print("Write error!");
    Serial.println("Write Error!");

  }

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

 

  return buffer[0];

}

 

void processCardUsage() {

  int currentBalance = readBalance();

  if (currentBalance < 0) {

    lcd.clear();

    lcd.print("Card read error!");
    Serial.println("Card read error!");

    return;

  }

 

  if (currentBalance >= fareAmount) {

    int newBalance = currentBalance - fareAmount;

    byte newBuffer[16] = {0};

    newBuffer[0] = (byte)newBalance;

 

    MFRC522::StatusCode status;

    MFRC522::MIFARE_Key key;

    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

 

    status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(rfid.uid));

    status = rfid.MIFARE_Write(4, newBuffer, 16);

 

    if (status == MFRC522::STATUS_OK) {

      lcd.clear();

      lcd.setCursor(0, 0);

      lcd.print("Access Granted!");

      lcd.setCursor(0, 2);

      lcd.print("Remaining Balance:");

      lcd.print(newBalance);
    
      Serial.println("Access Granted!");

      Serial.println("Remaining Balance: ");
      Serial.println(newBalance);
    

      // bip bip

      tone(BUZZER_PIN, 2500); delay(150); noTone(BUZZER_PIN);

      delay(50);

      tone(BUZZER_PIN, 2500); delay(150); noTone(BUZZER_PIN);

    } else {

      lcd.clear();

      lcd.print("Write Error!");
      Serial.println("Write Error");

    }

  } 

  else {

    lcd.clear();

    lcd.print("Insufficient Balance");
    Serial.println("Insufficient Balance");

 

    // uzun beep

    tone(BUZZER_PIN, 2500); delay(600); noTone(BUZZER_PIN);

  }

 

  delay(3000);

  lcd.clear();

  lcd.print("PassThru V1");

  currentMode = MAIN_MENU;

}

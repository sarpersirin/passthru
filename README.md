# RFID PassThru V2 Payment & Access System

This project is an advanced, Arduino-based RFID payment and access control system. 
It allows RFID cards to store a 16-bit balance (up to 30,000 TL), load credit and check balances via a Local Web Dashboard or Serial Monitor, deduct fares automatically, and display real-time feedback using a 1.8" RGB TFT display and a buzzer.

## Features

- RFID card authentication using MFRC522
- High-capacity 16-bit balance storage directly on RFID card memory
- Python/Flask-based Local Web Dashboard for remote administration
- Real-time access logging and CSV export via the Web UI
- 1.8" RGB TFT Display (ST7735S) with custom vector icons and dynamic UI
- Optimized Software SPI to resolve hardware SPI conflicts
- Balance loading and checking modes
- Automatic fare deduction on passage
- Audio-visual feedback using buzzer and colored screens
- Menu-based system logic (state machine)

## Hardware Used

- Arduino Uno
- MFRC522 RFID Reader Module
- RFID Cards / Tags
- 1.8" 128x160 RGB TFT Display (ST7735S)
- Active Buzzer
- Jumper Wires

## System Modes

The system operates using a simple state machine.

### MAIN_MENU
Default mode. Displays custom UI. Waits for RFID card or Web/Serial command.

### LOAD_BALANCE
Enter balance amount via Web Dashboard or Serial Monitor.

### WAIT_FOR_CARD_LOAD
Writes the 16-bit entered balance to the scanned RFID card.

### CHECK_BALANCE
Reads and displays the current balance from the scanned RFID card. Sends data to the Web Dashboard.

## Control Commands

| Command | Action |
|--------|--------|
| `#` | Enter balance loading mode (Serial) |
| `0` | Check balance mode (Serial) |
| `ENTER` | Confirm entered balance and wait for card |
| `Web UI`| Directly sends commands and amounts via Python Server |

### Example Usage (Web Dashboard)

1. Open the Local Web Dashboard (`http://localhost:5000`)
2. Type an amount in the Remote Reader Controls (e.g. `500`)
3. Click **Load**
4. Scan RFID card → Green Screen appears, balance is written

## How It Works

1. System starts and connects to the Python Server.
2. An RFID card is scanned.
3. If no command is active:
   - Fare amount (default: 40 TL) is deducted automatically.
4. If balance is sufficient:
   - Access is granted.
   - Screen turns **GREEN** with "GRANTED" message.
   - Remaining balance is written back to the card using bitwise shifting.
   - Buzzer provides positive feedback.
5. If balance is insufficient:
   - Access is denied.
   - Screen turns **RED** with "DENIED" message.
   - Long buzzer sound is triggered.

## Balance Storage Logic

- Balance is stored in **Block 4** of the RFID card.
- The first **TWO bytes** of the block are used (16-bit integer logic) to bypass the 255-byte limit.
- Max supported balance is dynamically increased up to 32,767 TL.
- Default MIFARE key (`FF FF FF FF FF FF`) is used for authentication.

## Audio & Visual Feedback

- Two short beeps + **Green Screen** → Access granted / Successful load
- One long beep + **Red Screen** → Insufficient balance / Error
- Custom drawn "Wave/RFID" vector icon on the main menu.

## Known Limitations

- Balance is not encrypted.
- No EEPROM backup (card-based storage only).
- Requires an active serial connection to a PC for the Web Dashboard to function.

## Possible Improvements

- ESP32 / WiFi integration for standalone dashboard hosting
- Encrypted RFID authentication
- Mobile application support
- Database-backed user profiles

## AI Usage Disclosure

I used AI (Gemini) for:
- Refactoring UI/UX coordinates and generating mathematical geometry (custom vector icons) using Adafruit_GFX.
- Developing the Python Flask server and real-time JavaScript interactions.
- Implementing bitwise operations (shifting) to store and retrieve 16-bit integers across MIFARE memory blocks.

## Author

Developed by **Sarper**

# RFID PassThru Payment System (Arduino)

This project is an Arduino-based RFID payment and access control system.
It allows RFID cards to store a balance, load credit via the Serial Monitor,
deduct balance automatically during passage, and display real-time feedback
using an I2C LCD and a buzzer.

## Features

- RFID card authentication using MFRC522
- Balance storage directly on RFID card memory
- Serial Monitor–based control system
- Balance loading mode
- Balance checking mode
- Automatic fare deduction on passage
- I2C LCD (20x4) real-time status display
- Audio feedback using buzzer
- Menu-based system logic (state machine)

## Hardware Used

- Arduino Uno
- MFRC522 RFID Reader
- RFID Cards / Tags
- I2C LCD Display (20x4)
- Active Buzzer
- Breadboard
- Jumper Wires

## System Modes

The system operates using a simple state machine.

### MAIN_MENU
Default mode. Waits for RFID card or Serial command.

### LOAD_BALANCE
Enter balance amount via Serial Monitor.

### WAIT_FOR_CARD_LOAD
Writes entered balance to scanned RFID card.

### CHECK_BALANCE
Reads and displays balance from scanned RFID card.


## Serial Monitor Commands

| Command | Action |
|--------|--------|
| `#` | Enter balance loading mode |
| `0` | Check balance mode |
| `ENTER` | Confirm entered balance and wait for card |

### Example Usage

1. Send `#`
2. Type an amount (e.g. `25`)
3. Press **ENTER**
4. Scan RFID card → balance is written
   
## How It Works

1. System starts.
2. An RFID card is scanned.
3. If no command is active:
   - Fare amount (default: 1) is deducted automatically.
4. If balance is sufficient:
   - Access is granted
   - Remaining balance is written back to the card
   - LCD and buzzer provide feedback
5. If balance is insufficient:
   - Access is denied
   - Warning message and long buzzer sound are triggered

## Balance Storage Logic

- Balance is stored in **Block 4** of the RFID card.
- Only the first byte of the block is used.
- Default MIFARE key (`FF FF FF FF FF FF`) is used for authentication.

## Audio Feedback

- Two short beeps → Access granted
- One long beep → Insufficient balance
- One short beep → Successful balance load

## Demo Video

A full demonstration of the system, including RFID scanning,
balance loading, balance checking, and automatic fare deduction,
is available here:

https://vimeo.com/1164426846

## Known Limitations

- Balance is stored in a single byte (0–255 range)
- Balance is not encrypted
- No EEPROM backup (card-based storage only)

## Possible Improvements

- EEPROM or database-backed balance storage
- Encrypted RFID authentication
- ESP32 / WiFi integration
- Web-based admin interface
- Mobile application support

## AI Usage Disclosure

I used AI (ChatGpt) For:
- RFID Writing Functions.
- Translating The Words From Turkish To English.

## Author

Developed by **Sarper**  

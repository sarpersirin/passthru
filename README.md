# RFID PassThru V1 Payment & Access System

This project is a foundational, Arduino-based RFID payment and access control system. It allows RFID cards to store an 8-bit balance (up to 255 units), load credit and check balances via the Arduino Serial Monitor, deduct fares automatically, and display real-time feedback using a 20x4 I2C LCD display and a buzzer.

## Features

*   RFID card authentication using MFRC522
*   8-bit balance storage directly on RFID card memory (Max balance: 255)
*   Serial Monitor integration for administrative commands and balance loading
*   Real-time UI feedback via 20x4 I2C LCD Display
*   Balance loading and checking modes
*   Automatic fare deduction on passage (Default fare: 1 unit)
*   Audio-visual feedback using an active buzzer and LCD text
*   Menu-based system logic (state machine)

## Hardware Used

*   Arduino Uno / Nano
*   MFRC522 RFID Reader Module
*   RFID Cards / Tags
*   20x4 I2C LCD Display (Address: `0x27`)
*   Active Buzzer
*   Jumper Wires

## System Modes

The system operates using a simple state machine controlled via the Serial Monitor.

### MAIN_MENU
Default mode. Displays "PassThru V1". Waits for an RFID card scan to deduct fare or a Serial command to change modes.

### LOAD_BALANCE
Enter the balance amount you want to load via the Serial Monitor. The LCD updates dynamically as you type.

### WAIT_FOR_CARD_LOAD
Triggered after pressing ENTER. The system waits for an RFID card to be scanned to write the entered 8-bit balance.

### CHECK_BALANCE
Reads and displays the current balance from the scanned RFID card on the LCD and Serial Monitor.

## Control Commands

| Command | Action |
| :--- | :--- |
| `#` | Enter balance loading mode (via Serial Monitor) |
| `0` | Enter check balance mode (via Serial Monitor) |
| `ENTER` | Confirm the entered balance and switch to "Scan card" mode |

### Example Usage (Serial Monitor)

1. Open the Arduino Serial Monitor (Baud rate: `9600`).
2. Type `#` to enter Load Balance mode.
3. Type an amount (e.g., `50`).
4. Press **ENTER**. The LCD will say "Scan card...".
5. Scan your RFID card. The LCD will show "Load successful!" and the buzzer will beep.

## How It Works

1. The system starts and initializes the I2C LCD and RFID reader.
2. An RFID card is scanned in the `MAIN_MENU`.
3. The system checks the current balance.
   *   **If balance is sufficient (≥ 1):** Access is granted. The fare is deducted, the new balance is written to the card, the LCD displays "Access Granted!" with the remaining balance, and the buzzer beeps twice fast.
   *   **If balance is insufficient:** Access is denied. The LCD displays "Insufficient Balance" and the buzzer emits one long beep.

## Balance Storage Logic

*   Balance is stored in **Block 4** of the RFID card.
*   Only the **first byte** (`buffer[0]`) of the block is used, limiting the maximum balance to **255** (8-bit integer limit).
*   Default MIFARE key (`FF FF FF FF FF FF`) is used for authentication.

## Audio & Visual Feedback

*   **Two short beeps + "Access Granted!"** → Passage successful / Fare deducted.
*   **One short beep + "Load successful!"** → Balance successfully written to the card.
*   **One long beep + "Insufficient Balance"** → Access denied / Error reading card.

## Known Limitations

*   Balance is capped at 255 units due to 8-bit storage.
*   Balance is stored in plain text (unencrypted).
*   No EEPROM backup or external database (card-based storage only).
*   Requires a PC and Serial Monitor for adding funds.

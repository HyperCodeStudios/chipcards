// #include <Arduino.h>
// #include <Wire.h>
// #include <Adafruit_PN532.h>
// #include <NeoPixelBus.h>
// #include <avr/wdt.h>
// #include <global.h>

// #define ERR_IND_ON HIGH
// #define ERR_IND_OFF LOW

// //// Constants
// // General
// const char *FIRMWARE_SIGNATURE = "0.1";

// // Neopixel
// const uint8_t PIXEL_PIN = 4;
// const uint8_t PIXEL_COUNT = 1;

// // NFC I2C
// const uint8_t SDA_PIN = 2;
// const uint8_t SCL_PIN = 3;
// const uint8_t IRQ_PIN = 5;
// const uint8_t RESET_PIN = 7;

// // Peripherals
// const uint8_t SPEAKER_PIN = 9;
// const uint8_t ERROR_INDICATOR_PIN = LED_BUILTIN;

// //// Runtime
// // Control instances
// NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> pixels(PIXEL_COUNT, PIXEL_PIN);
// Adafruit_PN532 nfc(IRQ_PIN, RESET_PIN);

// // Tag scanning
// String lastTagId = "";
// unsigned long lastTagReadTime = 0;

// // Timing
// const uint64_t TICK_TIME_THRESHOLD = 15; // 15ms -> 66.66Hz (ticks per second)
// uint64_t elapsed = 0;
// uint64_t elapsedSinceTick = 0;
// uint8_t tick = 0;

// //// Indicate Logic
// // General indicate control
// bool indicateInit = true; // True for the first loop call
// uint64_t indicateTargetTime = 0;
// uint8_t indicateMode = 0; // 0 = Off, 1 = Solid, 2 = Blink, 3 = Fade, 4 = Siren
// bool indicateActive = false;
// uint8_t indicateColor[3] = {0, 0, 0};

// // Inidicate fade mode
// const uint8_t INDICATE_FADE_OFF_TARGET[3] = {0, 0, 0};
// const float INDICATE_STEP_COUNT = 200; // Double to avoid strange float/int arithmetic problems

// uint8_t indicateStepsDone = 0;
// bool indicateFadeDirection = false; // true = increasing, false = decreasing

// float indicateFadeBuffer[3] = {};
// float indicateIncrementors[3] = {};

// //// Function definition for stupid idiot compiler
// void loopSerialCommand();
// void loopTagRead();
// void loopIndicate();

// /**
//  * Why? Exactly! Fucking stream wrapper should have this built-in!
//  * ... I shouldn't complain; this functuin is Copilot AI slop anyways... mostly
//  */
// uint32_t readUint32_t(Stream &serial)
// {
//     uint32_t value = 0;
//     for (int i = 0; i < 4; i++)
//     {
//         int byteRead = serial.read();
//         if (byteRead == -1)
//         {
//             Serial.println("Error reading uint32_t from stream!");
//             return 0; // Error handling
//         }
//         value <<= 8;
//         value |= byteRead & 0xFF; // Ensure we only take the last 8 bits
//     }
//     return value;
// }

// /**
//  * Logic used more than once, so here is the function. ReDuNdAnZvErBoT!
//  * My software engineering prof would kiss me for this! But I would hate it. He's an idiot.
//  */
// bool readTerminator(Stream &serial)
// {
//     return (serial.read() == '\n');
// }

// /**
//  * This function feels wrong... Something triggers me about it. But meh
//  */
// bool isByteCommand(char cmd)
// {
//     return cmd == 'p' || cmd == 's' || cmd == 'i';
// }

// void panic()
// {
//     Serial.println("FIRMWARE PANIC! Execution halted. Reset via WDT in T-500ms...");
//     digitalWrite(ERROR_INDICATOR_PIN, ERR_IND_ON);
//     wdt_enable(WDTO_500MS); // Activate the watchdog—because we’ve clearly lost control of the wheel.
//     while (1)
//         ; // Waiting for the watchdog to bring an end to the suffering...
//           // And yes, this is the correct way to handle errors...
// }

// void setup()
// {
//     //// Debugging
//     // Open virtual USB serial for debugging
//     Serial.begin(115200);
//     Serial.println();
//     delay(500);

//     // Print identification header
//     Serial.print("LZ13-Doorslave; Firmware Version ");
//     Serial.println(FIRMWARE_SIGNATURE);
//     Serial.println("developed by Robin and Jonas\n----------\n\n");

//     //// GPIO
//     // Setup pins
//     Serial.println("+ Initialize GPIO");
//     pinMode(ERROR_INDICATOR_PIN, OUTPUT);
//     digitalWrite(ERROR_INDICATOR_PIN, ERR_IND_OFF);

//     pinMode(SPEAKER_PIN, OUTPUT);
//     tone(SPEAKER_PIN, 100, 100);

//     //// UART
//     // Open hardware serial to master
//     Serial.println("+ Initialize UART with master");
//     Serial1.begin(115200, SERIAL_8N1);

//     //// LED
//     // Initialize NeoPixel
//     Serial.println("+ Initialize LED");
//     pixels.Begin();
//     pixels.ClearTo(RgbColor(0, 0, 0));
//     pixels.Show();

//     // Test LED
//     Serial.println("  - Testing LED");
//     pixels.ClearTo(RgbColor(255, 0, 0));
//     pixels.Show();
//     delay(50);
//     pixels.ClearTo(RgbColor(0, 255, 0));
//     pixels.Show();
//     delay(50);
//     pixels.ClearTo(RgbColor(0, 0, 255));
//     pixels.Show();
//     delay(50);
//     pixels.ClearTo(RgbColor(0, 0, 0));
//     pixels.Show();
//     delay(50);

//     //// NFC
//     // Initialize PN532 NFC Reader
//     Serial.print("+ Initialize PN532 module...");
//     if (!nfc.begin())
//     {
//         Serial.println("Failed!");
//         panic();
//     }
//     Serial.println("success!");

//     // Try to identify the PN532 chip
//     Serial.print("+ Verify correct PN532 communication...");
//     uint32_t versiondata = nfc.getFirmwareVersion();
//     if (!versiondata)
//     {
//         Serial.println("Failed!");
//         panic();
//     }
//     Serial.println("success!");

//     // Show firmware version
//     Serial.print("  - Found chip PN5");
//     Serial.println((versiondata >> 24) & 0xFF, HEX);
//     Serial.print("    Firmware version: ");
//     Serial.print((versiondata >> 16) & 0xFF, DEC);
//     Serial.print('.');
//     Serial.println((versiondata >> 8) & 0xFF, DEC);

//     // SAM
//     Serial.println("+ Configuring NFC SAM");
//     nfc.SAMConfig();

//     Serial.println("+ Setup completed successfully!");
// }

// void loop()
// {
//     uint64_t startTime = millis(); // Timer start

//     // Ensures that all time reliant logic can run with a time dependant progression instead of
//     // a shitty CPU cycle dependent progression when using the "tick" variable
//     // especially useful for visual stuff like... an LED...
//     elapsedSinceTick += elapsed;
//     if (elapsedSinceTick > TICK_TIME_THRESHOLD)
//     {                                                                // If at least one tick has to be done
//         uint64_t ticksToDo = elapsedSinceTick / TICK_TIME_THRESHOLD; // Calculate ticks to be done
//         elapsedSinceTick = elapsedSinceTick % TICK_TIME_THRESHOLD;   // Write back the left over time elapsed

//         for (uint64_t i = 0; i < ticksToDo; i++)
//         {
//             tick++;

//             // Do time based computing
//             loopIndicate();
//         }
//     }

//     loopSerialCommand();
//     loopTagRead();

//     elapsed = millis() - startTime; // Timer stop
// }

// void loopIndicate()
// {
//     // Shortcut return if indication is not active
//     if (!indicateActive)
//         return;

//     // Deactivate indication after time is up; avoid reactivation on timer flip
//     if (indicateTargetTime != 0 && millis() > indicateTargetTime)
//     {
//         indicateActive = false;
//         indicateMode = 0; // Reset mode to Off
//     }

//     switch (indicateMode)
//     {
//     case 2: // Blink
//         if (tick % 16 == 0)
//         {
//             pixels.ClearTo(RgbColor(indicateColor[0], indicateColor[1], indicateColor[2]));
//         }
//         else if (tick % 8 == 0)
//         {
//             pixels.ClearTo(RgbColor(0, 0, 0));
//         }
//         break;

//     case 3: // Fade
//     {
//         // precalc
//         if (indicateInit)
//         {
//             // Initialize fade buffer
//             indicateFadeBuffer[0] = indicateColor[0];
//             indicateFadeBuffer[1] = indicateColor[1];
//             indicateFadeBuffer[2] = indicateColor[2];

//             // Calculate incrementors
//             indicateIncrementors[0] = indicateFadeBuffer[0] / INDICATE_STEP_COUNT;
//             indicateIncrementors[1] = indicateFadeBuffer[1] / INDICATE_STEP_COUNT;
//             indicateIncrementors[2] = indicateFadeBuffer[2] / INDICATE_STEP_COUNT;
//         }

//         // Step
//         if (indicateFadeDirection)
//         {
//             indicateFadeBuffer[0] += indicateIncrementors[0];
//             indicateFadeBuffer[1] += indicateIncrementors[1];
//             indicateFadeBuffer[2] += indicateIncrementors[2];
//         }
//         else
//         {
//             indicateFadeBuffer[0] -= indicateIncrementors[0];
//             indicateFadeBuffer[1] -= indicateIncrementors[1];
//             indicateFadeBuffer[2] -= indicateIncrementors[2];
//         }
        
//         // Flip direction
//         if (indicateStepsDone >= INDICATE_STEP_COUNT)
//         {
//             if(indicateFadeDirection) {
//                 // Reset fade buffer to full color
//                 indicateFadeBuffer[0] = indicateColor[0];
//                 indicateFadeBuffer[1] = indicateColor[1];
//                 indicateFadeBuffer[2] = indicateColor[2];
//             } else {
//                 // Reset fade buffer to off color
//                 indicateFadeBuffer[0] = INDICATE_FADE_OFF_TARGET[0];
//                 indicateFadeBuffer[1] = INDICATE_FADE_OFF_TARGET[1];
//                 indicateFadeBuffer[2] = INDICATE_FADE_OFF_TARGET[2];
//             }

//             indicateFadeDirection = !indicateFadeDirection;
//             indicateStepsDone = 0;
//         }

//         // Bake
//         pixels.ClearTo(RgbColor(
//             round(indicateFadeBuffer[0]), 
//             round(indicateFadeBuffer[1]), 
//             round(indicateFadeBuffer[2])));

//         indicateStepsDone++;
        
//         break;
//     }

//     case 1: // Solid
//         pixels.ClearTo(RgbColor(indicateColor[0], indicateColor[1], indicateColor[2]));
//         break;

//     case 4: // Siren
//     {
//         uint8_t fastTick = tick % 128; // Fast tick for siren
//         if (fastTick < 64)
//         {
//             if (fastTick % 15 == 0)
//             {
//                 pixels.ClearTo(RgbColor(indicateColor[0], indicateColor[1], indicateColor[2]));
//             }
//             else if (fastTick % 10 == 0)
//             {
//                 pixels.ClearTo(RgbColor(0, 0, 0));
//             }
//         }
//         else
//         {
//             if (fastTick % 10 == 0)
//             {
//                 pixels.ClearTo(RgbColor(indicateColor[0], indicateColor[1], indicateColor[2]));
//             }
//             else if (fastTick % 5 == 0)
//             {
//                 pixels.ClearTo(RgbColor(0, 0, 0));
//             }
//         }
//         break;
//     }

//     case 0: // Off
//         pixels.ClearTo(RgbColor(0, 0, 0));
//         break;

//     default:
//         break;
//     }
//     pixels.Show();
//     indicateInit = false;
// }

// /**
//  * Command structure:
//  * "indicate" Command: i<uint_8t:mode><uint_8t:R><uint_8t:G><uint_8t:B><uint32_t:time>
//  * - <mode> uint8_t
//  *  0 = Off
//  *  1 = solid
//  *  2 = blink
//  *  3 = fade
//  * - <uint_8t:R><uint_8t:G><uint_8t:B>
//  *  hex color code in the format RRGGBB
//  * - <time> uint32_t
//  *  time in milliseconds (0 = no timeout)
//  *
//  * "sound" Command: s<uint16_t:freq><uint32_t:duration>
//  * - <freq> uint16_t
//  * frequency in Hz
//  * - <duration> uint32_t
//  * duration in milliseconds (0 = no sound at all)
//  *
//  * "ping" Command: p
//  * - No parameters
//  * 
//  * "write" Command: w<uint8_t[2]:cryptokey><uint8_t[16]:datablock><uint8_t:datablock-checksum>
//  * - <cryptokey> uint8_t[2]
//  * 16 bit cryptographic key to use for writing the NFC tag.
//  * - <datablock> uint8_t[16]
//  * 16 byte data block to write to the NFC tag
//  * - <datablock-checksum> uint8_t
//  * Checksum of the data block, calculated as follows:
//  *  checksum = (data[0] + data[1] + ... + data[15]) % 256
//  * 
//  * "read" Command: r<uint8_t[2]:cryptokey>
//  * - <cryptokey> uint8_t[2]
//  * 16 bit cryptographic key to use for reading the NFC tag.
//  * Will be used either as ntag password, or CRYPTO1 key for Mifare Classic.
//  */
// void loopSerialCommand()
// {
//     // Return if no command received
//     if (Serial1.available() == 0)
//         return;

//     char command = Serial1.read(); // Read first byte to determine command
//     Serial.println("Command received: " + String(command));

//     // Handle command
//     switch (command)
//     {
//     case 'p': // PING
//         if (readTerminator(Serial1))
//         {
//             Serial.println("PING command received successfully!");
//         }
//         else
//         {
//             Serial.println("Failed to read PING command termination!");
//             return;
//         }

//         Serial1.write("p\n");
//         Serial1.flush();

//         // Debug output
//         Serial.println("PING");
//         Serial.println();
//         break;

//     case 'i': // INDICATE
//     {
//         Serial.print("Reading INDICATE command...");

//         // Read mode
//         if (!Serial1.available())
//         { // Return if no byte available
//             Serial.println("Failed to read mode!");
//             return;
//         }
//         uint8_t mode;
//         mode = Serial1.read();

//         // Read color
//         if (Serial1.available() < 3)
//         { // Return if not enough bytes available
//             Serial.println("Failed to read color!");
//             return;
//         }

//         int colorBuffer[3];
//         for (int i = 0; i < 3; i++)
//         {
//             colorBuffer[i] = Serial1.read();
//         }

//         // Read time
//         if (Serial1.available() < 4)
//         { // Return if not enough bytes available
//             Serial.println("Failed to read time!");
//             return;
//         }
//         uint32_t time = readUint32_t(Serial1);

//         // Read terminator
//         if (!readTerminator(Serial1))
//         {
//             Serial.println("Expecteded command termination failed!");
//             return;
//         }
//         Serial.println("Success:");

//         // Debug output
//         Serial.println("INDICATE");
//         Serial.print(mode);
//         Serial.println(" (mode)");
//         Serial.print(String(colorBuffer[0]) + "R " + String(colorBuffer[1]) + "G " + String(colorBuffer[2]) + "B");
//         Serial.println(" (color)");
//         Serial.print(time);
//         Serial.println(" (time)");
//         Serial.println();

//         // Set runtime variables
//         indicateMode = mode;
//         indicateActive = true;
//         indicateInit = true;
//         if (time == 0)
//             indicateTargetTime = 0;
//         else
//             indicateTargetTime = millis() + time; // Set target time for indication

//         // Extract RGB values from hex color String; ignore case
//         indicateColor[0] = colorBuffer[0]; // Red
//         indicateColor[1] = colorBuffer[1]; // Green
//         indicateColor[2] = colorBuffer[2]; // Blue

//         break;
//     }

//     case 's': // SOUNG
//     {
//         // Serial1.read(); // Pop 's' from buffer
//         Serial.print("Reading SOUND command...");

//         // Read frequency
//         if (Serial1.available() < 2)
//         {
//             Serial.println("Failed to read frequency!");
//             return;
//         }

//         uint16_t freq;
//         freq = (Serial1.read() << 8) | Serial1.read();

//         // Read duration
//         if (Serial1.available() < 4)
//         {
//             Serial.println("Failed to read duration!");
//             return;
//         }
//         uint32_t duration = readUint32_t(Serial1);

//         // Read terminator
//         if (!readTerminator(Serial1))
//         {
//             Serial.println("Expecteded command termination failed!");
//             return;
//         }
//         Serial.println("Success:");

//         // Debug output
//         Serial.println("SOUND");
//         Serial.print(freq);
//         Serial.println(" (frequency)");
//         Serial.print(duration);
//         Serial.println(" (duration)");
//         Serial.println();

//         // Handle sound command
//         tone(SPEAKER_PIN, freq, duration);
//         break;
//     }

//     default:
//         Serial.println("Received unknown command");
//         Serial.println();
//         break;
//     }

//     // Clean up buffer if bytes left
//     if (Serial1.available())
//     {
//         Serial.print("Cleaning buffer ");
//         delay(10);

//         // Read all remaining bytes to void
//         while (Serial1.available() > 0 && !isByteCommand(Serial1.peek()))
//         {
//             Serial.print("x");
//             Serial1.read();
//         }

//         Serial.println(" done");
//     }
// }

// void loopTagRead()
// {
//     uint8_t uid[7]; // UID max. 7 Bytes
//     uint8_t uidLength;

//     if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50))
//     {
//         String uidString = "";
//         for (uint8_t i = 0; i < uidLength; i++)
//         {
//             uidString += String(uid[i], HEX);
//             if (i < uidLength - 1)
//             {
//                 uidString += ":";
//             }
//         }

//         // Shortcut return if same uid was scanned within 5 seconds
//         if (uidString == lastTagId && (millis() - lastTagReadTime) < 5000)
//             return;

//         // Update last tag read time and ID
//         lastTagId = uidString;
//         lastTagReadTime = millis();

//         // Debug
//         Serial.print("Read card with UID: ");
//         Serial.println(uidString.c_str());

//         // Send UID via UART
//         Serial1.print(uidString + '\n');
//     }
// }
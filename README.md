# Coin Sorter with LED Matrix Display

A sophisticated coin sorting and counting system with real-time LED matrix display, motor control, and firework animations. This project automatically detects and counts US coins (pennies, nickels, dimes, quarters) while providing visual feedback through a 32x5 LED matrix display.

## Features

### 🪙 Coin Detection
- **4 Independent Sensors**: Separate analog sensors for each coin type
- **Real-time Detection**: Instant coin recognition with visual feedback
- **No Missed Coins**: Optimized detection algorithm ensures every coin is counted
- **Non-blocking Operation**: Coin detection never blocks other system functions

### 💰 Display System
- **32x5 LED Matrix**: High-resolution display showing dollar amounts
- **Money Format**: Displays totals in XX.XX format (e.g., $12.34)
- **Color-coded Feedback**: Different colors for different operations
- **Non-blocking Animations**: Firework celebrations for milestones

### ⚙️ Motor Control
- **Pulsing Operation**: Motor runs in controlled pulses for precise movement
- **Bidirectional Control**: Forward and backward sequences
- **Configurable Sequences**: Adjustable pulse timing and direction changes
- **H-bridge Control**: Full forward/backward control with PWM speed control

### 🎮 User Controls
- **Dollar Adjustment**: Add/subtract $1.00 with dedicated buttons
- **Motor Control**: Manual motor activation button
- **Reset Function**: Clear total amount (currently disabled to prevent blocking)
- **Debug Mode**: Comprehensive logging and sensor monitoring

### 💾 Data Persistence
- **EEPROM Storage**: Total amount persists across power cycles
- **Data Integrity**: Magic number verification ensures valid saved data
- **Automatic Loading**: System loads previous total on startup

## Hardware Requirements

### Core Components
- **Arduino Uno** (or compatible board)
- **32x5 LED Matrix** (160 NeoPixels)
- **4x Analog Sensors** (for coin detection)
- **DC Motor** with H-bridge driver
- **3x Push Buttons** (dollar add/subtract, motor control)

### Pin Configuration
```
LED Matrix: Pin 7
Penny Sensor: A2
Nickel Sensor: A1  
Dime Sensor: A3
Quarter Sensor: A0
Reset Button: Pin 6
Add Dollar: Pin 2
Subtract Dollar: Pin 3
Motor Forward: Pin 9
Motor Backward: Pin 10
Motor Button: Pin 8
Motor Enable: Pin 11 (PWM)
```

## Installation & Setup

### 1. Hardware Assembly
1. Connect the LED matrix to pin 7
2. Wire each coin sensor to its designated analog pin
3. Connect motor driver to pins 9, 10, 11 (H-bridge configuration)
4. Wire push buttons to their respective pins
5. Ensure proper power supply for motor and sensors

### 2. Software Setup
1. Install required libraries:
   ```bash
   # Arduino IDE Library Manager
   Adafruit NeoPixel
   ```
2. Upload `coin_sorter.ino` to your Arduino
3. Open Serial Monitor (9600 baud) for debug output

### 3. Configuration
Adjust these settings in the code as needed:
```cpp
#define DEBUG_MODE 1                    // Enable debug output
#define DEBOUNCE_DELAY 10              // Coin detection debounce (ms)
#define DISPLAY_DELAY 25               // Display update frequency (ms)
#define COIN_THRESHOLD 640             // Analog threshold for coin detection
#define FIREWORK_INTERVAL 1000         // Firework every $10.00 (1000 cents)
```

## Usage

### Basic Operation
1. **Power On**: System initializes and loads previous total from EEPROM
2. **Insert Coins**: Drop coins into the sorter - they're automatically detected and counted
3. **View Total**: Current total displays in real-time on LED matrix
4. **Celebrations**: Firework animations trigger at $10.00 milestones

### Button Functions
- **Add Dollar (+$1.00)**: Cyan flash, adds $1.00 to total
- **Subtract Dollar (-$1.00)**: Orange flash, subtracts $1.00 (prevents negative)
- **Motor Control**: Activates motor pulsing sequence
- **Reset**: Clears total (currently disabled to prevent blocking)

### Debug Features
When `DEBUG_MODE` is enabled:
- Real-time sensor value monitoring
- Coin detection logging with sensor values
- Motor operation status and sequence tracking
- EEPROM save/load confirmation

## Technical Details

### Coin Detection Algorithm
```cpp
// Threshold-based detection with debouncing
bool coinDetected = (analogValue <= COIN_THRESHOLD);

// Non-blocking state machine
if (coinDetected && !lastState && timeSinceLastDetection > debounceDelay) {
    // Register coin detection
    return coinValue;
}
```

### Motor Control System
- **Pulse Timing**: 100ms on, 100ms off
- **Sequence Pattern**: 4 forward pulses, 1 backward pulse per sequence
- **Total Sequences**: 2 complete sequences per activation
- **H-bridge Control**: Pin 9 (forward), Pin 10 (backward), Pin 11 (PWM enable)
- **Direction Logic**: Forward = Pin 9 HIGH, Pin 10 LOW; Backward = Pin 9 LOW, Pin 10 HIGH

### Display System
- **Resolution**: 32x5 pixels (160 total)
- **Format**: XX.XX dollar display
- **Colors**: Green (normal), Yellow (coin added), Red (error), Cyan (add dollar), Orange (subtract dollar)
- **Animation**: Non-blocking firework effects with multiple bursts

### Memory Management
- **EEPROM Storage**: Persistent total amount across power cycles
- **Magic Number**: Data integrity verification (0xAA55)
- **Automatic Loading**: System loads saved total on startup
- **Error Handling**: Graceful fallback to $0.00 if EEPROM data is invalid

## Code Organization

### Well-Documented Structure
The code includes comprehensive comments explaining:
- **Pin Functions**: What each pin controls and how
- **Configuration Values**: What each constant controls and units
- **Variable States**: Boolean logic and state machine explanations
- **Function Purpose**: What each function does and why
- **Complex Logic**: Motor control algorithms and animation systems

### Key Sections
```cpp
// Pin Definitions with clear explanations
#define MOTOR_FORWARD_PIN 9   // High to drive forward
#define MOTOR_BACKWARD_PIN 10 // High to drive backward
#define MOTOR_ENA_PIN 11      // PWM pin for motor speed control

// Configuration with units and purpose
#define DEBOUNCE_DELAY 10     // Milliseconds for button/coin debouncing
#define COIN_THRESHOLD 640    // Analog threshold for coin detection

// Variable explanations
bool motorDirection = true;    // true = forward, false = backward
bool pulseState = false;      // true = motor on, false = motor off
```

## Troubleshooting

### Common Issues

**Coins Not Detected:**
- Check sensor wiring and connections
- Verify analog pin assignments (A0-A3)
- Test sensor values in debug mode
- Adjust `COIN_THRESHOLD` if needed (default: 640)

**Motor Not Running:**
- Verify H-bridge connections (pins 9, 10, 11)
- Check power supply for motor
- Test motor pins with simple on/off code
- Ensure proper H-bridge configuration

**Display Issues:**
- Verify LED matrix connections to pin 7
- Check power supply for NeoPixels
- Test individual LED segments
- Verify 32x5 matrix configuration

**EEPROM Issues:**
- Check if total persists across power cycles
- Verify magic number in EEPROM
- Test EEPROM read/write functions
- Check for memory corruption

**False Coin Detections:**
- Check for electrical interference
- Adjust debounce timing (`DEBOUNCE_DELAY`)
- Verify sensor mounting and alignment
- Test individual sensor isolation

### Debug Commands
```cpp
// Enable comprehensive debug output
#define DEBUG_MODE 1

// Monitor sensor values in real-time
// Debug output shows: sensor index, analog value, coin value

// Motor operation tracking
// Debug shows: pulse count, sequence progress, direction changes
```

## Customization

### Adjusting Coin Detection
```cpp
// Modify detection threshold
#define COIN_THRESHOLD 640  // Adjust based on sensor calibration

// Change debounce timing
#define DEBOUNCE_DELAY 10   // Milliseconds
```

### Motor Configuration
```cpp
// Adjust pulse timing
#define PULSE_ON_TIME 100   // Milliseconds motor stays on
#define PULSE_OFF_TIME 100  // Milliseconds motor stays off

// Modify sequence pattern
#define FORWARD_PULSES 4    // Number of forward pulses per sequence
#define BACKWARD_PULSES 1   // Number of backward pulses per sequence
#define TOTAL_SEQUENCES 2   // Number of complete sequences to run
```

### Display Customization
```cpp
// Change LED brightness
strip.setBrightness(50);  // 0-255

// Modify firework frequency
#define FIREWORK_INTERVAL 1000  // Every $10.00 (1000 cents)
```

### EEPROM Configuration
```cpp
// Change EEPROM storage address
#define EEPROM_TOTAL_ADDRESS 0  // Address to store total amount

// Modify magic number for data validation
#define EEPROM_MAGIC_NUMBER 0xAA55  // Magic number to verify data is valid
```

## Safety & Maintenance

### Electrical Safety
- Use appropriate power supplies for motor and sensors
- Ensure proper grounding
- Add capacitors for motor noise suppression
- Use separate power for sensitive analog circuits

### Mechanical Maintenance
- Clean coin sensors regularly
- Lubricate motor components as needed
- Check for coin jams in sorting mechanism
- Inspect wiring connections periodically

### Software Updates
- Backup EEPROM data before updates
- Test new configurations thoroughly
- Monitor sensor values after changes
- Verify motor operation with new settings

## Recent Updates

### Code Improvements (Latest)
- **Restored Comprehensive Comments**: All pin functions, configuration values, and complex logic are now well-documented
- **EEPROM Loading on Startup**: System automatically loads previous total when powered on
- **Improved Code Organization**: Better structured with logical grouping of constants and variables
- **Enhanced Debug Output**: More detailed logging for troubleshooting
- **Non-blocking Operation**: All functions designed to prevent blocking coin detection

### Key Changes
- Added `loadTotalFromEEPROM()` call in `setup()` for automatic data restoration
- Restored detailed comments for all pin definitions and configuration values
- Improved variable documentation explaining state machines and boolean logic
- Enhanced motor control comments explaining H-bridge operation
- Better function documentation explaining purpose and behavior

## License

This project is open source. Feel free to modify and distribute according to your needs.

## Contributing

Contributions are welcome! Please:
1. Test thoroughly before submitting
2. Document any hardware changes
3. Include debug output for issues
4. Follow existing code style and comment standards

## Support

For issues or questions:
1. Check the troubleshooting section
2. Enable debug mode for detailed logging
3. Verify hardware connections
4. Test individual components
5. Check EEPROM data integrity

---

**Happy Coin Sorting! 🪙💰**

# Coin Sorter with LED Matrix Display

A sophisticated coin sorting and counting system with real-time LED matrix display, motor control, and firework animations. This project automatically detects and counts US coins (pennies, nickels, dimes, quarters) while providing visual feedback through a 32x5 LED matrix display.

## Features

### 🪙 Coin Detection
- **4 Independent Sensors**: Separate analog sensors for each coin type
- **Real-time Detection**: Instant coin recognition with visual feedback
- **No Missed Coins**: Optimized detection algorithm ensures every coin is counted
- **Motor Noise Filtering**: Advanced filtering to prevent false readings during motor operation

### 💰 Display System
- **32x5 LED Matrix**: High-resolution display showing dollar amounts
- **Money Format**: Displays totals in XX.XX format (e.g., $12.34)
- **Color-coded Feedback**: Different colors for different operations
- **Non-blocking Animations**: Firework celebrations for milestones

### ⚙️ Motor Control
- **Pulsing Operation**: Motor runs in controlled pulses for precise movement
- **Bidirectional Control**: Forward and backward sequences
- **Configurable Sequences**: Adjustable pulse timing and direction changes
- **Noise Reduction**: Motor-aware analog reading to prevent interference

### 🎮 User Controls
- **Dollar Adjustment**: Add/subtract $1.00 with dedicated buttons
- **Motor Control**: Manual motor activation button
- **Reset Function**: Clear total amount (currently disabled to prevent blocking)
- **Debug Mode**: Comprehensive logging and sensor monitoring

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
Motor Enable: Pin 11
```

## Installation & Setup

### 1. Hardware Assembly
1. Connect the LED matrix to pin 7
2. Wire each coin sensor to its designated analog pin
3. Connect motor driver to pins 9, 10, 11
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
#define MONITOR_QUARTER_SENSOR 0        // Monitor quarter sensor
#define DEBOUNCE_DELAY 10              // Coin detection debounce
#define DISPLAY_DELAY 25               // Display update frequency
```

## Usage

### Basic Operation
1. **Power On**: System initializes with $0.00 display
2. **Insert Coins**: Drop coins into the sorter - they're automatically detected and counted
3. **View Total**: Current total displays in real-time on LED matrix
4. **Celebrations**: Firework animations trigger at $10.00 milestones

### Button Functions
- **Add Dollar (+$1.00)**: Cyan flash, adds $1.00 to total
- **Subtract Dollar (-$1.00)**: Orange flash, subtracts $1.00 (prevents negative)
- **Motor Control**: Activates motor pulsing sequence
- **Reset**: Clears total (currently disabled)

### Debug Features
When `DEBUG_MODE` is enabled:
- Real-time sensor value monitoring
- Coin detection logging
- Motor operation status
- EEPROM save/load confirmation

## Technical Details

### Coin Detection Algorithm
```cpp
// Threshold-based detection
bool coinDetected = (analogValue <= 640);

// Debounced state machine
if (coinDetected && !lastState && timeSinceLastDetection > debounceDelay) {
    // Register coin detection
}
```

### Motor Control System
- **Pulse Timing**: 100ms on, 100ms off
- **Sequence Pattern**: 4 forward pulses, 1 backward pulse per sequence
- **Total Sequences**: 2 complete sequences per activation
- **Noise Filtering**: Reduced analog reading frequency during motor operation

### Display System
- **Resolution**: 32x5 pixels (160 total)
- **Format**: XX.XX dollar display
- **Colors**: Green (normal), Yellow (coin added), Red (error), etc.
- **Animation**: Non-blocking firework effects

### Memory Management
- **EEPROM Storage**: Persistent total amount across power cycles
- **Magic Number**: Data integrity verification
- **Circular Buffer**: Efficient analog reading averaging

## Troubleshooting

### Common Issues

**Coins Not Detected:**
- Check sensor wiring and connections
- Verify analog pin assignments
- Test sensor values in debug mode
- Adjust detection threshold if needed

**Motor Not Running:**
- Verify motor driver connections
- Check power supply for motor
- Test motor pins with simple on/off code
- Ensure proper H-bridge configuration

**Display Issues:**
- Verify LED matrix connections
- Check power supply for NeoPixels
- Test individual LED segments
- Verify pin 7 connection

**False Coin Detections:**
- Enable motor noise filtering
- Check for electrical interference
- Adjust debounce timing
- Verify sensor mounting and alignment

### Debug Commands
```cpp
// Enable sensor monitoring
#define MONITOR_QUARTER_SENSOR 1

// Enable comprehensive debug output
#define DEBUG_MODE 1

// Test motor operation
testMotor();

// Monitor all sensors
debugSensors();
```

## Customization

### Adjusting Coin Detection
```cpp
// Modify detection threshold
bool coinDetected = (currentValue <= 640);  // Adjust 640 as needed

// Change debounce timing
#define DEBOUNCE_DELAY 10  // Milliseconds
```

### Motor Configuration
```cpp
// Adjust pulse timing
const unsigned long PULSE_ON_TIME = 100;   // Milliseconds
const unsigned long PULSE_OFF_TIME = 100;  // Milliseconds

// Modify sequence pattern
const int FORWARD_PULSES = 4;   // Pulses per sequence
const int BACKWARD_PULSES = 1;  // Pulses per sequence
const int TOTAL_SEQUENCES = 2;  // Complete sequences
```

### Display Customization
```cpp
// Change LED brightness
strip.setBrightness(50);  // 0-255

// Modify firework frequency
#define FIREWORK_INTERVAL 1000  // Every $1.00
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

## License

This project is open source. Feel free to modify and distribute according to your needs.

## Contributing

Contributions are welcome! Please:
1. Test thoroughly before submitting
2. Document any hardware changes
3. Include debug output for issues
4. Follow existing code style

## Support

For issues or questions:
1. Check the troubleshooting section
2. Enable debug mode for detailed logging
3. Verify hardware connections
4. Test individual components

---

**Happy Coin Sorting! 🪙💰**

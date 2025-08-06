/*
 * Coin Counter with LED Matrix Display
 * Displays running total in money format (xx.xx)
 * Uses 4 separate sensors for penny, nickel, dime, quarter
 */

#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Pin Definitions
#define LED_PIN           7
#define PENNY_SENSOR_PIN  A2
#define NICKEL_SENSOR_PIN A1
#define DIME_SENSOR_PIN   A3
#define QUARTER_SENSOR_PIN A0
#define RESET_BUTTON_PIN  6
#define ADD_DOLLAR_PIN    2  // Button to add $1.00
#define SUBTRACT_DOLLAR_PIN 3  // Button to subtract $1.00
#define MOTOR_FORWARD_PIN 9   // High to drive forward
#define MOTOR_BACKWARD_PIN 10 // High to drive backward
#define MOTOR_BUTTON_PIN  8
#define MOTOR_ENA_PIN     11  // PWM pin for motor speed control (not used in pulsing mode)

// LED Matrix Configuration
#define LED_COUNT         160  // 32x5 matrix
#define MATRIX_WIDTH      32
#define MATRIX_HEIGHT     5
#define DIGIT_WIDTH       3
#define DIGIT_HEIGHT      5
#define DIGIT_SPACING     1
#define DECIMAL_WIDTH     1

// Coin Values (in cents)
#define PENNY_VALUE       1
#define NICKEL_VALUE      5
#define DIME_VALUE        10
#define QUARTER_VALUE     25

// Debug Configuration
#define DEBUG_MODE        1  // Set to 1 to enable debug features
#define MONITOR_QUARTER_SENSOR 0  // Set to 1 to continuously monitor quarter sensor values

// Firework Configuration
#define FIREWORK_INTERVAL 1000  // Firework every $1.00 (100 cents)
#define FIREWORK_DELAY    500

// Timing Constants
#define DEBOUNCE_DELAY    10 //20 // Reduced for faster coin detection
#define DISPLAY_DELAY     25  //50 // Reduced for more responsive detection
#define FLASH_DELAY       200 //200
#define RESET_DELAY       1000
#define MOTOR_RUN_TIME    2000  // Motor runs for 2 seconds

// EEPROM Configuration
#define EEPROM_TOTAL_ADDRESS 0  // Address to store total amount
#define EEPROM_MAGIC_NUMBER  0xAA55  // Magic number to verify data is valid

// Global Variables
long totalAmount = 0;
unsigned long lastSensorTimes[4] = {0, 0, 0, 0};
bool lastSensorStates[4] = {false, false, false, false}; // false = no coin detected
long lastFireworkMilestone = 0;

// Animation state variables
bool animationPlaying = false;
unsigned long animationStartTime = 0;
int animationStep = 0;
int fireworkBurst = 0;
int fireworkCenterX = 0;
int fireworkCenterY = 0;
uint32_t fireworkColor = 0;
int sparkleStep = 0;
int flashStep = 0;

// Motor control variables
bool motorRunning = false;
bool motorDirection = true; // true = forward, false = backward
unsigned long motorStartTime = 0;
bool lastMotorButtonState = HIGH;
int motorSpeed = 255; // Full speed (0-255)

// Motor pulsing variables
unsigned long lastPulseTime = 0;
bool pulseState = false; // true = motor on, false = motor off
const unsigned long PULSE_ON_TIME = 100;   // 50ms on
const unsigned long PULSE_OFF_TIME = 100;   // 100ms off

// Motor sequence variables
int pulseCount = 0;
int sequenceCount = 0; // Which sequence we're in (0 = first, 1 = second, etc.)
const int FORWARD_PULSES = 4;  // Number of forward pulses per sequence
const int BACKWARD_PULSES = 1; // Number of backward pulses per sequence
const int TOTAL_SEQUENCES = 2; // Number of complete sequences to run
int pulsesInCurrentSequence = 0; // Pulses completed in current sequence

// Dollar adjustment button variables
bool lastAddButtonState = HIGH;
bool lastSubtractButtonState = HIGH;
unsigned long lastAddButtonTime = 0;
unsigned long lastSubtractButtonTime = 0;

// NeoPixel strip object
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Digit patterns for 0-9 (3x5 grid)
const bool DIGIT_PATTERNS[10][5][3] = {
  {{1,1,1}, {1,0,1}, {1,0,1}, {1,0,1}, {1,1,1}}, // 0
  {{0,1,0}, {1,1,0}, {0,1,0}, {0,1,0}, {1,1,1}}, // 1
  {{1,1,1}, {0,0,1}, {1,1,1}, {1,0,0}, {1,1,1}}, // 2
  {{1,1,1}, {0,0,1}, {1,1,1}, {0,0,1}, {1,1,1}}, // 3
  {{1,0,1}, {1,0,1}, {1,1,1}, {0,0,1}, {0,0,1}}, // 4
  {{1,1,1}, {1,0,0}, {1,1,1}, {0,0,1}, {1,1,1}}, // 5
  {{1,1,1}, {1,0,0}, {1,1,1}, {1,0,1}, {1,1,1}}, // 6
  {{1,1,1}, {0,0,1}, {0,0,1}, {0,0,1}, {0,0,1}}, // 7
  {{1,1,1}, {1,0,1}, {1,1,1}, {1,0,1}, {1,1,1}}, // 8
  {{1,1,1}, {1,0,1}, {1,1,1}, {0,0,1}, {1,1,1}}  // 9
};

// Decimal point pattern
const bool DECIMAL_PATTERN[5][1] = {{0}, {0}, {0}, {0}, {1}};

// Sensor configuration
const int SENSOR_PINS[4] = {PENNY_SENSOR_PIN, NICKEL_SENSOR_PIN, DIME_SENSOR_PIN, QUARTER_SENSOR_PIN};
const int COIN_VALUES[4] = {PENNY_VALUE, NICKEL_VALUE, DIME_VALUE, QUARTER_VALUE};

void setup() {
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif

  // Initialize pins
  for (int i = 0; i < 4; i++) {
    pinMode(SENSOR_PINS[i], INPUT); // Analog pins don't need pullup
  }
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MOTOR_BUTTON_PIN, INPUT_PULLUP);
  pinMode(ADD_DOLLAR_PIN, INPUT_PULLUP);
  pinMode(SUBTRACT_DOLLAR_PIN, INPUT_PULLUP);
  
  // Initialize motor pins
  pinMode(MOTOR_FORWARD_PIN, OUTPUT);
  pinMode(MOTOR_BACKWARD_PIN, OUTPUT);
  pinMode(MOTOR_ENA_PIN, OUTPUT);
  digitalWrite(MOTOR_FORWARD_PIN, LOW);
  digitalWrite(MOTOR_BACKWARD_PIN, LOW);
  analogWrite(MOTOR_ENA_PIN, 255); // Full speed for pulsing
  
  // Test motor briefly on startup
  digitalWrite(MOTOR_FORWARD_PIN, HIGH);
  delay(100);
  digitalWrite(MOTOR_FORWARD_PIN, LOW);
  
  #if DEBUG_MODE
    // Initialize Serial for debug output
    Serial.begin(9600);
    Serial.println("Coin Sorter Debug Mode Enabled");
  #endif

  #if MONITOR_QUARTER_SENSOR
    // Initialize Serial for quarter sensor monitoring
    Serial.begin(9600);
    Serial.println("Quarter Sensor Monitoring Enabled");
    Serial.println("Format: Raw_Value | Detected | Threshold");
  #endif

  // Initialize LED matrix
  strip.begin();
  strip.show();
  strip.setBrightness(50);

  // Load saved total from EEPROM
  //loadTotalFromEEPROM();
  
  // Display initial amount
  updateDisplay(totalAmount, strip.Color(0, 255, 0));
  
  #if DEBUG_MODE
    Serial.print("Loaded total from EEPROM: $");
    Serial.println(totalAmount / 100.0, 2);
    // Test motor on startup (debug mode)
    // testMotor();
    // debugSensors();
  #endif
}

void loop() {
  // Check reset button (commented out to prevent blocking)
  // if (digitalRead(RESET_BUTTON_PIN) == LOW) {
  //   resetTotal();
  //   delay(RESET_DELAY);
  // }

  // Check for coin detection (ALWAYS FIRST - highest priority)
  int coinValue = detectCoin();
  if (coinValue > 0) {
    addCoin(coinValue);
  }

  // // Check dollar adjustment buttons
  // checkDollarButtons();

  // // Check motor button
  // checkMotorButton();

  // // Handle animation or normal display
  // if (animationPlaying) {
  //   updateAnimation();
  // } else {
  //   updateDisplay(totalAmount, strip.Color(0, 255, 0));
  // }
  
  // // Monitor quarter sensor values
  // #if MONITOR_QUARTER_SENSOR
  //   monitorQuarterSensor();
  // #endif
  
  // delay(DISPLAY_DELAY);
}

// Detect which coin was inserted
int detectCoin() {
  unsigned long currentTime = millis();
  
  for (int i = 0; i < 4; i++) {
    int currentValue = analogRead(SENSOR_PINS[i]);
    bool coinDetected = (currentValue <= 640);
    
    if (coinDetected && !lastSensorStates[i] && 
        (currentTime - lastSensorTimes[i]) > DEBOUNCE_DELAY) {
      lastSensorTimes[i] = currentTime;
      lastSensorStates[i] = true;
      #if DEBUG_MODE
        Serial.print("Coin detected on sensor ");
        Serial.print(i);
        Serial.print(" (Value: ");
        Serial.print(currentValue);
        Serial.print(") - Coin value: ");
        Serial.println(COIN_VALUES[i]);
      #endif
      return COIN_VALUES[i];
    }
    
    if (!coinDetected && lastSensorStates[i]) {
      lastSensorStates[i] = false;
    }
  }
  
  return 0;
}

// Add coin to total with visual feedback
void addCoin(int value) {
  long previousAmount = totalAmount;
  totalAmount += value;
  
  // Flash display (brief) - non-blocking
  updateDisplay(totalAmount, strip.Color(255, 255, 0));
    
  // Check if we reached a new firework milestone
  long currentMilestone = totalAmount / FIREWORK_INTERVAL;
  long previousMilestone = previousAmount / FIREWORK_INTERVAL;
  
  if (currentMilestone > previousMilestone) {
    startFireworkAnimation();
  }
}

// Update the LED matrix display
void updateDisplay(long amount, uint32_t color) {
  strip.clear();
  drawMoney(amount, color);
  strip.show();
}

// Draw money amount in xx.xx format
void drawMoney(long amount, uint32_t color) {
  long dollars = amount / 100;
  long cents = amount % 100;
  
  String dollarStr = String(dollars);
  String centStr = (cents < 10) ? "0" + String(cents) : String(cents);
  
  // Calculate display width and position
  int maxDollarDigits = (MATRIX_WIDTH - 10) / 4;
  if (dollarStr.length() > maxDollarDigits) {
    dollarStr = dollarStr.substring(dollarStr.length() - maxDollarDigits);
  }
  
  int totalWidth = (dollarStr.length() * 4) + 2 + 8;
  int startX = MATRIX_WIDTH - totalWidth;
  
  // Draw dollars
  int currentX = startX;
  for (int i = 0; i < dollarStr.length(); i++) {
    drawDigit(dollarStr.charAt(i) - '0', currentX, 0, color);
    currentX += DIGIT_WIDTH + DIGIT_SPACING;
  }
  
  // Draw decimal point
  drawDecimalPoint(currentX, 0, color);
  currentX += DECIMAL_WIDTH + DIGIT_SPACING;
  
  // Draw cents
  for (int i = 0; i < centStr.length(); i++) {
    drawDigit(centStr.charAt(i) - '0', currentX, 0, color);
    currentX += DIGIT_WIDTH + DIGIT_SPACING;
  }
}

// Draw a single digit
void drawDigit(int digit, int startX, int startY, uint32_t color) {
  if (digit < 0 || digit > 9) return;
  
  for (int y = 0; y < DIGIT_HEIGHT; y++) {
    for (int x = 0; x < DIGIT_WIDTH; x++) {
      if (DIGIT_PATTERNS[digit][y][x]) {
        setMatrixPixel(startX + x, startY + y, color);
      }
    }
  }
}

// Draw decimal point
void drawDecimalPoint(int startX, int startY, uint32_t color) {
  for (int y = 0; y < DIGIT_HEIGHT; y++) {
    if (DECIMAL_PATTERN[y][0]) {
      setMatrixPixel(startX, startY + y, color);
    }
  }
}

// Set pixel color on the LED matrix
void setMatrixPixel(int x, int y, uint32_t color) {
  if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) return;
  
  int flippedY = MATRIX_HEIGHT - 1 - y;
  int flippedX = MATRIX_WIDTH - 1 - x;
  
  int pixelIndex;
  if (flippedY % 2 == 0) {
    pixelIndex = flippedY * MATRIX_WIDTH + flippedX;
  } else {
    pixelIndex = flippedY * MATRIX_WIDTH + (MATRIX_WIDTH - 1 - flippedX);
  }
  
  strip.setPixelColor(pixelIndex, color);
}

// Non-blocking firework animation functions
void startFireworkAnimation() {
  animationPlaying = true;
  animationStartTime = millis();
  animationStep = 0;
  fireworkBurst = 0;
  sparkleStep = 0;
  flashStep = 0;
  
  // Initialize first firework
  fireworkCenterX = random(8, MATRIX_WIDTH - 8);
  fireworkCenterY = random(1, MATRIX_HEIGHT - 1);
  uint32_t colors[] = {strip.Color(255, 0, 0), strip.Color(0, 255, 0), 
                       strip.Color(0, 0, 255), strip.Color(255, 255, 0),
                       strip.Color(255, 0, 255), strip.Color(0, 255, 255)};
  fireworkColor = colors[random(6)];
}

void updateAnimation() {
  unsigned long currentTime = millis();
  
  // Handle different animation phases
  if (fireworkBurst < 3) {
    // Firework burst phase
    if (animationStep < 8) {
      // Expanding circle
      strip.clear();
      for (int angle = 0; angle < 360; angle += 10) {
        int x = fireworkCenterX + animationStep * cos(angle * PI / 180);
        int y = fireworkCenterY + animationStep * sin(angle * PI / 180);
        
        if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
          setMatrixPixel(x, y, fireworkColor);
        }
      }
      strip.show();
      animationStep++;
    } else {
      // Move to next burst or sparkle phase
      fireworkBurst++;
      if (fireworkBurst < 3) {
        // Start next firework
        fireworkCenterX = random(8, MATRIX_WIDTH - 8);
        fireworkCenterY = random(1, MATRIX_HEIGHT - 1);
        uint32_t colors[] = {strip.Color(255, 0, 0), strip.Color(0, 255, 0), 
                             strip.Color(0, 0, 255), strip.Color(255, 255, 0),
                             strip.Color(255, 0, 255), strip.Color(0, 255, 255)};
        fireworkColor = colors[random(6)];
        animationStep = 0;
        sparkleStep = 0;
      }
    }
  } else if (flashStep < 6) {
    // Final celebration flash
    if (flashStep % 2 == 0) {
      strip.fill(strip.Color(255, 255, 255));
    } else {
      strip.clear();
    }
    strip.show();
    flashStep++;
  } else {
    // Animation complete
    animationPlaying = false;
    strip.clear();
    strip.show();
  }
}

// Legacy blocking functions (kept for reference)
void createFirework(int centerX, int centerY, uint32_t color) {
  // Create expanding circle effect
  for (int radius = 0; radius < 8; radius++) {
    strip.clear();
    
    // Draw expanding circle
    for (int angle = 0; angle < 360; angle += 10) {
      int x = centerX + radius * cos(angle * PI / 180);
      int y = centerY + radius * sin(angle * PI / 180);
      
      if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
        setMatrixPixel(x, y, color);
      }
    }
    
    strip.show();
    delay(FIREWORK_DELAY);
  }
}

void sparkleEffect() {
  // Random sparkles across the matrix
  for (int i = 0; i < 20; i++) {
    int x = random(MATRIX_WIDTH);
    int y = random(MATRIX_HEIGHT);
    uint32_t colors[] = {strip.Color(255, 0, 0), strip.Color(0, 255, 0), 
                         strip.Color(0, 0, 255), strip.Color(255, 255, 0),
                         strip.Color(255, 0, 255), strip.Color(0, 255, 255)};
    setMatrixPixel(x, y, colors[random(6)]);
  }
}

void fireworkAnimation() {
  // Multiple firework bursts
  for (int burst = 0; burst < 3; burst++) {
    int centerX = random(8, MATRIX_WIDTH - 8);
    int centerY = random(1, MATRIX_HEIGHT - 1);
    
    uint32_t colors[] = {strip.Color(255, 0, 0), strip.Color(0, 255, 0), 
                         strip.Color(0, 0, 255), strip.Color(255, 255, 0),
                         strip.Color(255, 0, 255), strip.Color(0, 255, 255)};
    
    createFirework(centerX, centerY, colors[random(6)]);
    
    // Sparkle effect between bursts
    if (burst < 2) {
      for (int sparkle = 0; sparkle < 5; sparkle++) {
        sparkleEffect();
        strip.show();
        delay(100);
        strip.clear();
        delay(50);
      }
    }
  }
  
  // Final celebration flash
  for (int flash = 0; flash < 3; flash++) {
    strip.fill(strip.Color(255, 255, 255));
    strip.show();
    delay(200);
    strip.clear();
    strip.show();
    delay(200);
  }
}

// Motor control functions
void checkMotorButton() {
  bool currentButtonState = digitalRead(MOTOR_BUTTON_PIN);
  unsigned long currentTime = millis();
  
  // Check for button press with debouncing
  if (currentButtonState == LOW && lastMotorButtonState == HIGH && 
      (currentTime - motorStartTime) > DEBOUNCE_DELAY) {
    
    if (!motorRunning) {
      // Start motor
      startMotor();
      #if DEBUG_MODE
        // Debug: Flash display to show button was detected - non-blocking
        updateDisplay(totalAmount, strip.Color(255, 0, 255)); // Purple flash
        Serial.println("Motor button pressed");
      #endif
    }
  }
  
  // Handle motor pulsing if running
  if (motorRunning) {
    updateMotorPulse(currentTime);
  }
  
  lastMotorButtonState = currentButtonState;
}

void startMotor() {
  motorRunning = true;
  motorStartTime = millis();
  lastPulseTime = millis();
  pulseState = true; // Start with motor on
  pulseCount = 0; // Reset pulse counter
  sequenceCount = 0; // Reset sequence counter
  pulsesInCurrentSequence = 0; // Reset sequence pulse counter
  
  // Always start with forward direction
  motorDirection = true;
  
  // Set direction pins for forward
  digitalWrite(MOTOR_FORWARD_PIN, HIGH);
  digitalWrite(MOTOR_BACKWARD_PIN, LOW);
  
  #if DEBUG_MODE
    Serial.print("Motor: Starting ");
    Serial.print(TOTAL_SEQUENCES);
    Serial.print(" sequences (");
    Serial.print(FORWARD_PULSES);
    Serial.print(" forward, ");
    Serial.print(BACKWARD_PULSES);
    Serial.println(" backward each)");
  #endif
  
  // Turn motor on to start
  digitalWrite(MOTOR_FORWARD_PIN, HIGH);
}

void stopMotor() {
  motorRunning = false;
  digitalWrite(MOTOR_FORWARD_PIN, LOW);
  digitalWrite(MOTOR_BACKWARD_PIN, LOW);
  pulseState = false;
  pulseCount = 0;
  sequenceCount = 0;
  pulsesInCurrentSequence = 0;
  
  // Save to EEPROM
  saveTotalToEEPROM();
}

// Dollar adjustment button functions
void checkDollarButtons() {
  unsigned long currentTime = millis();
  
  // Check add dollar button
  bool currentAddState = digitalRead(ADD_DOLLAR_PIN);
  if (currentAddState == LOW && lastAddButtonState == HIGH && 
      (currentTime - lastAddButtonTime) > DEBOUNCE_DELAY) {
    addDollar();
    lastAddButtonTime = currentTime;
  }
  lastAddButtonState = currentAddState;
  
  // Check subtract dollar button
  bool currentSubtractState = digitalRead(SUBTRACT_DOLLAR_PIN);
  if (currentSubtractState == LOW && lastSubtractButtonState == HIGH && 
      (currentTime - lastSubtractButtonTime) > DEBOUNCE_DELAY) {
    subtractDollar();
    lastSubtractButtonTime = currentTime;
  }
  lastSubtractButtonState = currentSubtractState;
}

void addDollar() {
  long previousAmount = totalAmount;
  totalAmount += 100; // Add $1.00 (100 cents)
  
  #if DEBUG_MODE
    Serial.print("Added $1.00 - New total: $");
    Serial.println(totalAmount / 100.0, 2);
  #endif
  
  // Save to EEPROM
  saveTotalToEEPROM();

  // Flash display to show adjustment - non-blocking
  updateDisplay(totalAmount, strip.Color(0, 255, 255)); // Cyan flash
    
  // Check if we reached a new firework milestone
  long currentMilestone = totalAmount / FIREWORK_INTERVAL;
  long previousMilestone = previousAmount / FIREWORK_INTERVAL;
  
  if (currentMilestone > previousMilestone) {
    startFireworkAnimation();
  }
}

void subtractDollar() {
  if (totalAmount >= 100) { // Prevent negative amounts
    totalAmount -= 100; // Subtract $1.00 (100 cents)
    
    #if DEBUG_MODE
      Serial.print("Subtracted $1.00 - New total: $");
      Serial.println(totalAmount / 100.0, 2);
    #endif
    
    // Save to EEPROM
    saveTotalToEEPROM();

    // Flash display to show adjustment - non-blocking
    updateDisplay(totalAmount, strip.Color(255, 165, 0)); // Orange flash
    
  } else {
    #if DEBUG_MODE
      Serial.println("Cannot subtract $1.00 - total would be negative");
    #endif
    
    // Flash red to indicate error - non-blocking
    updateDisplay(totalAmount, strip.Color(255, 0, 0)); // Red flash
  }
}

// EEPROM functions
void saveTotalToEEPROM() {
  // Save magic number and total amount
  EEPROM.put(EEPROM_TOTAL_ADDRESS, EEPROM_MAGIC_NUMBER);
  EEPROM.put(EEPROM_TOTAL_ADDRESS + 2, totalAmount);
  
  #if DEBUG_MODE
    Serial.print("Saved total to EEPROM: $");
    Serial.println(totalAmount / 100.0, 2);
  #endif
}

void loadTotalFromEEPROM() {
  // Check if valid data exists
  unsigned int magicNumber;
  EEPROM.get(EEPROM_TOTAL_ADDRESS, magicNumber);
  
  if (magicNumber == EEPROM_MAGIC_NUMBER) {
    // Load saved total
    EEPROM.get(EEPROM_TOTAL_ADDRESS + 2, totalAmount);
    
    #if DEBUG_MODE
      Serial.println("Valid EEPROM data found");
    #endif
  } else {
    // No valid data, start with zero
    totalAmount = 0;
    
    #if DEBUG_MODE
      Serial.println("No valid EEPROM data, starting with $0.00");
    #endif
  }
}

void clearEEPROM() {
  // Clear EEPROM by writing invalid magic number
  EEPROM.put(EEPROM_TOTAL_ADDRESS, 0x0000);
  
  #if DEBUG_MODE
    Serial.println("EEPROM cleared");
  #endif
}

// Motor pulsing function
void updateMotorPulse(unsigned long currentTime) {
  if (pulseState) {
    // Motor is currently on, check if it should turn off
    if ((currentTime - lastPulseTime) >= PULSE_ON_TIME) {
      // Turn motor OFF by setting both pins LOW
      digitalWrite(MOTOR_FORWARD_PIN, LOW);
      digitalWrite(MOTOR_BACKWARD_PIN, LOW);
      
      pulseState = false;
      lastPulseTime = currentTime;
      
      #if DEBUG_MODE
        Serial.println("Motor: OFF");
      #endif
    }
  } else {
    // Motor is currently off, check if it should turn on
    if ((currentTime - lastPulseTime) >= PULSE_OFF_TIME) {
      pulseCount++; // Increment total pulse counter
      pulsesInCurrentSequence++; // Increment sequence pulse counter
      
      // Check if we need to change direction within current sequence
      if (pulsesInCurrentSequence >= FORWARD_PULSES && motorDirection) {
        // Switch to backward direction
        motorDirection = false;
        
        #if DEBUG_MODE
          Serial.println("Motor: Switching to BACKWARD");
        #endif
      }
      
      // Check if current sequence is complete
      if (pulsesInCurrentSequence >= (FORWARD_PULSES + BACKWARD_PULSES)) {
        sequenceCount++;
        pulsesInCurrentSequence = 0;
        motorDirection = true; // Reset to forward for next sequence
        
        #if DEBUG_MODE
          Serial.print("Motor: Completed sequence ");
          Serial.print(sequenceCount);
          Serial.print("/");
          Serial.println(TOTAL_SEQUENCES);
        #endif
        
        // Check if all sequences are complete
        if (sequenceCount >= TOTAL_SEQUENCES) {
          stopMotor();
          return;
        }
      }
      
      // Turn motor ON in current direction
      if (motorDirection) {
        // Forward: Pin 9 HIGH, Pin 10 LOW
        digitalWrite(MOTOR_FORWARD_PIN, HIGH);
        digitalWrite(MOTOR_BACKWARD_PIN, LOW);
      } else {
        // Backward: Pin 9 LOW, Pin 10 HIGH
        digitalWrite(MOTOR_FORWARD_PIN, LOW);
        digitalWrite(MOTOR_BACKWARD_PIN, HIGH);
      }
      
      pulseState = true;
      lastPulseTime = currentTime;
      
      #if DEBUG_MODE
        Serial.print("Motor: ON (Pulse ");
        Serial.print(pulseCount);
        Serial.print(", Sequence ");
        Serial.print(sequenceCount + 1);
        Serial.print("/");
        Serial.print(TOTAL_SEQUENCES);
        Serial.print(", Sequence Pulse ");
        Serial.print(pulsesInCurrentSequence);
        Serial.print("/");
        Serial.print(FORWARD_PULSES + BACKWARD_PULSES);
        Serial.print(" - ");
        Serial.print(motorDirection ? "FORWARD" : "BACKWARD");
        Serial.println(")");
      #endif
    }
  }
}

#if DEBUG_MODE
// Debug function to display analog sensor values
void debugSensors() {
  Serial.begin(9600);
  while (true) {
    Serial.print("Penny: ");
    Serial.print(analogRead(PENNY_SENSOR_PIN));
    Serial.print(" | Nickel: ");
    Serial.print(analogRead(NICKEL_SENSOR_PIN));
    Serial.print(" | Dime: ");
    Serial.print(analogRead(DIME_SENSOR_PIN));
    Serial.print(" | Quarter: ");
    Serial.println(analogRead(QUARTER_SENSOR_PIN));
    delay(500);
  }
}
#endif

// Test motor function - call this to test if motor works
void testMotor() {
  // Test forward with pulsing
  digitalWrite(MOTOR_FORWARD_PIN, HIGH);
  digitalWrite(MOTOR_BACKWARD_PIN, LOW);
  
  // Run for 1 second with pulsing
  unsigned long startTime = millis();
  while ((millis() - startTime) < 1000) {
    updateMotorPulse(millis());
    delay(10); // Small delay for stability
  }
  
  // Stop
  digitalWrite(MOTOR_FORWARD_PIN, LOW);
  digitalWrite(MOTOR_BACKWARD_PIN, LOW);
  delay(500);
  
  // Test backward with pulsing
  digitalWrite(MOTOR_FORWARD_PIN, LOW);
  digitalWrite(MOTOR_BACKWARD_PIN, HIGH);
  
  // Run for 1 second with pulsing
  startTime = millis();
  while ((millis() - startTime) < 1000) {
    updateMotorPulse(millis());
    delay(10); // Small delay for stability
  }
  
  // Stop
  digitalWrite(MOTOR_FORWARD_PIN, LOW);
  digitalWrite(MOTOR_BACKWARD_PIN, LOW);
}


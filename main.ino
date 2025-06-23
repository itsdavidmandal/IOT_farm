#include <Servo.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 

// === Pin Definitions === 
#define DHTPIN 2 
#define DHTTYPE DHT11 
#define MOISTURE_PIN A0 
#define RELAY_PIN 7 
#define TRIG_PIN 8 
#define ECHO_PIN 9 
#define SERVO_PIN 10 
#define BUZZER_PIN 13 
#define TEMP_LED_PIN 12 
#define DOOR_RED_PIN 3 
#define DOOR_GREEN_PIN 5 

// === Constants === 

const int MOISTURE_THRESHOLD = 20; 
const int DETECTION_DISTANCE_CM = 5; 
const int DOOR_OPEN_ANGLE = 90; 
const int DOOR_CLOSED_ANGLE = 0; 
unsigned long previousSerialMillis = 0; 
const unsigned long serialInterval = 2000; 

// === Objects === 
Servo doorServo; 
DHT dht(DHTPIN, DHTTYPE); 
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// === Globals === 

long duration; 
int distanceCm; 
unsigned long previousLcdMillis = 0; 
const unsigned long lcdInterval = 2500; 

void setup() { 
    Serial.begin(9600); 
    pinMode(MOISTURE_PIN, INPUT); 
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(TEMP_LED_PIN, OUTPUT);
    pinMode(DOOR_RED_PIN, OUTPUT);
    pinMode(DOOR_GREEN_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(TEMP_LED_PIN, LOW);
    digitalWrite(DOOR_RED_PIN, LOW);
    digitalWrite(DOOR_GREEN_PIN, LOW);
    doorServo.attach(SERVO_PIN);
    doorServo.write(DOOR_CLOSED_ANGLE);
    digitalWrite(RELAY_PIN, HIGH); 
    
    dht.begin(); 
    lcd.init(); 
    lcd.backlight(); 
    lcd.setCursor(0, 0); 
    lcd.print("System Init..."); 
    delay(2000); 
    lcd.clear(); 
}

void loop() { 
    // ====== Door Logic ====== 
    digitalWrite(TRIG_PIN, LOW); 
    delayMicroseconds(2); 
    digitalWrite(TRIG_PIN, HIGH); 
    delayMicroseconds(10); 
    digitalWrite(TRIG_PIN, LOW); 
    duration = pulseIn(ECHO_PIN, HIGH); 
    distanceCm = duration * 0.034 / 2; 
    if (distanceCm < DETECTION_DISTANCE_CM) { 
        doorServo.write(DOOR_OPEN_ANGLE); 
        digitalWrite(DOOR_GREEN_PIN, HIGH); 
        digitalWrite(DOOR_RED_PIN, LOW); 
    } else { 
        doorServo.write(DOOR_CLOSED_ANGLE); 
        digitalWrite(DOOR_GREEN_PIN, LOW); 
        digitalWrite(DOOR_RED_PIN, HIGH);
} 
// ====== Soil Moisture ====== 
int raw_moisture = analogRead(MOISTURE_PIN); 
int moisture_percent = map(raw_moisture, 1023, 500, 0, 100); 
moisture_percent = constrain(moisture_percent, 0, 100); 
digitalWrite(RELAY_PIN, (moisture_percent < MOISTURE_THRESHOLD) ? LOW : HIGH); 

// ====== DHT Sensor ====== 
float humidity = dht.readHumidity(); 
float temperature = dht.readTemperature(); 

// ====== Temperature-based buzzer and LED ====== 
if (temperature > 30.0) { 
    digitalWrite(BUZZER_PIN, HIGH); 
    digitalWrite(TEMP_LED_PIN, HIGH); 
} else { 
    digitalWrite(BUZZER_PIN, LOW); 
    digitalWrite(TEMP_LED_PIN, LOW); 
} 

unsigned long currentMillis = millis(); 

// ====== Serial Output for Debugging ====== 

if (currentMillis - previousSerialMillis >= serialInterval) { 
    previousSerialMillis = currentMillis; 
    Serial.print("Distance: "); 
    Serial.print(distanceCm); 
    Serial.print(" cm, Temperature: "); 
    Serial.print(temperature); 
    Serial.print(" C, Moisture: "); 
    Serial.print(moisture_percent); 
    Serial.print(" %, Pump: "); 
    Serial.print(moisture_percent < MOISTURE_THRESHOLD ? "ON" : "OFF"); 
    Serial.print(", Door: "); 
    Serial.println(distanceCm < DETECTION_DISTANCE_CM ? "OPEN" : "CLOSED"); 
} 

// ====== LCD Display ====== 
// unsigned long currentMillis = millis(); 

if (currentMillis - previousLcdMillis >= lcdInterval) { 
    previousLcdMillis = currentMillis; 
    lcd.clear(); 
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print((int)temperature);
    lcd.print(" P:");
    lcd.print(moisture_percent);
    lcd.print("%");
    lcd.setCursor(0, 1);
    lcd.print("D:");
    lcd.print((distanceCm < DETECTION_DISTANCE_CM) ? "Open " : "Close");
    lcd.print(" Pu:");
    lcd.print(moisture_percent < MOISTURE_THRESHOLD ? "ON " : "OFF");
}
delay(100);
}
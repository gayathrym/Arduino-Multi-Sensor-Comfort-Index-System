#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <NewPing.h>
#include <Wire.h>
#include <MPU6050.h>      // For IMU (vibration/ride smoothness)

#define DHTPIN 2
#define DHTTYPE DHT11
#define MQ135PIN A0
#define SOUND_SENSOR_PIN A1
#define TRIGGER_PIN 7
#define ECHO_PIN 6
#define MAX_DIST 200

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DIST);
MPU6050 mpu;

const int SAMPLES = 6;
const float ALPHA = 0.1;  // Exponential smoothing

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 1000;

float smoothGas = 0, smoothHum = 0, smoothTemp = 0, smoothCrowd = 0, smoothNoise = 0, smoothVib = 0;

float expSmooth(float newVal, float oldVal) {
  return ALPHA * newVal + (1 - ALPHA) * oldVal;
}

float normalize(float val, float minVal, float maxVal) {
  return constrain((val - minVal) / (maxVal - minVal), 0, 1);
}

float readAvgAnalog(uint8_t pin) {
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  return sum / float(SAMPLES);
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  dht.begin();
  mpu.initialize();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  smoothGas = readAvgAnalog(MQ135PIN);
  smoothTemp = dht.readTemperature();
  smoothHum = dht.readHumidity();
  smoothCrowd = float(MAX_DIST);
  smoothNoise = readAvgAnalog(SOUND_SENSOR_PIN);
  smoothVib = 1.0;
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdate >= updateInterval) {
    lastUpdate = currentMillis;

    // Read sensors
    float gasReading = readAvgAnalog(MQ135PIN);
    float tempReading = dht.readTemperature();
    float humReading = dht.readHumidity();
    float crowdDist = sonar.ping_cm();
    float noiseReading = readAvgAnalog(SOUND_SENSOR_PIN);

    if (crowdDist == 0) crowdDist = MAX_DIST;

    // Read MPU6050 for ride vibration (use acceleration magnitude)
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);
    float accG = sqrt(ax * ax + ay * ay + az * az) / 16384.0; // ~"g" units

    // Exponential smoothing
    smoothGas = expSmooth(gasReading, smoothGas);
    smoothTemp = expSmooth(tempReading, smoothTemp);
    smoothHum = expSmooth(humReading, smoothHum);
    smoothCrowd = expSmooth(crowdDist, smoothCrowd);
    smoothNoise = expSmooth(noiseReading, smoothNoise);
    smoothVib = expSmooth(accG, smoothVib);

    // Normalize all sensor values (adjust based on your environment)
    float nAir = normalize(smoothGas, 350, 700);
    float nTemp = normalize(smoothTemp, 18, 36);
    float nHum = normalize(smoothHum, 30, 80);
    float nCrowd = 1 - normalize(smoothCrowd, 10, 200);
    float nNoise = normalize(smoothNoise, 200, 800);         // Calibrate as needed
    float nVib = normalize(smoothVib, 1.0, 2.0);            // 1g=stationary, higher=vibrations

    // Comfort Index weights (sum to 1.0)
    float comfortIndex = 100 * (1 - (0.25 * nAir + 0.18 * nTemp + 0.12 * nHum + 
                                     0.15 * nCrowd + 0.15 * nNoise + 0.15 * nVib));
    
    // Display results
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.print("Air: "); display.println(smoothGas, 1);
    display.print("Temp: "); display.print(smoothTemp, 1); display.println(" C");
    display.print("Hum: "); display.print(smoothHum, 1); display.println(" %");
    display.print("Crowd: "); display.print(smoothCrowd, 1); display.println(" cm");
    display.print("Noise: "); display.print(smoothNoise, 1);
    display.print("Vibe: "); display.print(smoothVib, 2);
    display.print("CCI: "); display.println(comfortIndex, 1);

    display.display();
  }
}

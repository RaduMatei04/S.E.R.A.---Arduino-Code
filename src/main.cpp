#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>

const uint8_t I2C_SDA = 21, I2C_SCL = 22;
const uint8_t BME_ADDR = 0x76;
const uint8_t SOIL1_PIN = 32, SOIL2_PIN = 33;
const int SOIL_DRY_RAW = 3000, SOIL_WET_RAW = 1200;
const unsigned long SAMPLE_INTERVAL_MS = 1000;

Adafruit_BME280 bme;
BH1750 lightMeter;
bool bmeOk = false, lightOk = false;
unsigned long lastSampleTime = 0;

struct SensorData {
    float temperature = NAN, humidity = NAN, pressure = NAN, lux = NAN;
    int soil1Raw = 0, soil2Raw = 0;
};

int soilPercent(int raw) {
    return constrain(map(raw, SOIL_DRY_RAW, SOIL_WET_RAW, 0, 100), 0, 100);
}

const char *soilLabel(int percent) {
    if (percent < 20) return "USCAT - uda planta";
    if (percent < 40) return "Umiditate scazuta";
    if (percent < 70) return "Optim";
    return "Umed - nu mai uda";
}

void initSensors() {
    pinMode(SOIL1_PIN, INPUT);
    pinMode(SOIL2_PIN, INPUT);
    bmeOk = bme.begin(BME_ADDR, &Wire);
    lightOk = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);
    if (!bmeOk) Serial.println("BME280 negasit!");
    if (!lightOk) Serial.println("BH1750 negasit!");
}

void readBME(SensorData &d) {
    if (!bmeOk) return;
    d.temperature = bme.readTemperature();
    d.humidity = bme.readHumidity();
    d.pressure = bme.readPressure() / 100.0F;
}

void readLight(SensorData &d) {
    if (lightOk) d.lux = lightMeter.readLightLevel();
}

void readSoil(SensorData &d) {
    d.soil1Raw = analogRead(SOIL1_PIN);
    d.soil2Raw = analogRead(SOIL2_PIN);
}

void printAllReadings(const SensorData &d, unsigned long now) {
    Serial.printf("\n=== t=%.1fs ===\n", now / 1000.0);

    if (bmeOk) Serial.printf("BME280   : %.1fC  %.1f%%  %.1fhPa\n", d.temperature, d.humidity, d.pressure);
    else Serial.println("BME280   : indisponibil");

    if (lightOk) Serial.printf("BH1750   : %.1f lux\n", d.lux);
    else Serial.println("BH1750   : indisponibil");

    int p1 = soilPercent(d.soil1Raw), p2 = soilPercent(d.soil2Raw);
    Serial.printf("Sol P1   : %d%% [%s] (raw %d)\n", p1, soilLabel(p1), d.soil1Raw);
    Serial.printf("Sol P2   : %d%% [%s] (raw %d)\n", p2, soilLabel(p2), d.soil2Raw);
}

void setup() {
    Serial.begin(115200);
    delay(200);
    Wire.begin(I2C_SDA, I2C_SCL);
    initSensors();
    Serial.println(" Statie senzori sera - pornita");
}

void loop() {
    unsigned long now = millis();
    if (now - lastSampleTime < SAMPLE_INTERVAL_MS) return;
    lastSampleTime = now;

    SensorData data;
    readBME(data);
    readLight(data);
    readSoil(data);

    printAllReadings(data, now);
}

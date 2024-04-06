#include <Elegoo_GFX.h>      // Core graphics library
#include <Elegoo_TFTLCD.h>   // Hardware-specific library
#include <SoftwareSerial.h>  // Explanatory
#include "ELMduino.h"        // OBD reader library

#define LCD_CS A3  // Chip Select goes to Analog 3
#define LCD_CD A2  // Command/Data goes to Analog 2
#define LCD_WR A1  // LCD Write goes to Analog 1
#define LCD_RD A0  // LCD Read goes to Analog 0

#define LCD_RESET A4  // Can alternately just connect to Arduino's reset pin

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

ELM327 myELM327;

SoftwareSerial mySerial(10, 11);  // RX, TX
#define ELM_PORT mySerial

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

String error_data;

long int start_time;
long int end_time;

float mph;
float rpm;
float vvt_timing;
float oil_temperature;
float fuel_pressure;
float coolant_temperature;
float battery_voltage;
float intake_temperature;
float fuel_level;


void setup(void) {
  delay(6000);  // Waiting for BT connect

  Serial.begin(9600);
  ELM_PORT.begin(38400);

  Serial.println("[LOG] Car Monitor Start");

  Serial.println("[LOG] Attempting to connect to ELM327...");

  if (!myELM327.begin(ELM_PORT, false, 2000)) {
    Serial.println("[ERROR] Couldn't connect to OBD scanner");
    while (1)
      ;
  }

  Serial.println("[LOG] Connected to ELM327");

  tft.reset();

  tft.begin(0x9341);  // Code (0x0000) is for this display brand
}

void loop(void) {
  tft.setRotation(1);  // Making the screen landscape mode

  start_time = millis();

  float temp_mph = myELM327.mph();
  if (myELM327.nb_rx_state == ELM_SUCCESS) { mph = (uint32_t)temp_mph; }

  float temp_rpm = myELM327.rpm();
  if (myELM327.nb_rx_state == ELM_SUCCESS) { rpm = (uint32_t)temp_rpm; }

  float temp_vvt_timing = myELM327.timingAdvance();
  if (myELM327.nb_rx_state == ELM_SUCCESS) { vvt_timing = (uint32_t)temp_vvt_timing; }

  float temp_oil_temperature = myELM327.oilTemp();
  if (myELM327.nb_rx_state == ELM_SUCCESS) { oil_temperature = (uint32_t)temp_oil_temperature; }

  float temp_fuel_pressure = myELM327.fuelPressure();
  if (myELM327.nb_rx_state == ELM_SUCCESS) { fuel_pressure = (uint32_t)temp_fuel_pressure; }

  float temp_coolant_temperature = myELM327.engineCoolantTemp();
  if (myELM327.nb_rx_state == ELM_SUCCESS) { coolant_temperature = (uint32_t)temp_coolant_temperature; }

  float temp_battery_voltage = myELM327.batteryVoltage();
  if (myELM327.nb_rx_state == ELM_SUCCESS) { battery_voltage = (uint32_t)temp_battery_voltage; }

  float temp_intake_temperature = myELM327.intakeAirTemp();
  if (myELM327.nb_rx_state == ELM_SUCCESS) { intake_temperature = (uint32_t)temp_intake_temperature; }

  float temp_fuel_level = myELM327.fuelLevel();
  if (myELM327.nb_rx_state == ELM_SUCCESS) { fuel_level = (uint32_t)temp_fuel_level; }

  end_time = millis();


  if (error_data != "") {
    displayError();
  }

  checkForProblems();

  renderMainView();

  log();
}

void log() {
  Serial.println("OIL TEMPERATURE: " + String((int)oil_temperature));
  Serial.println("FUEL PRESSURE: " + String((int)fuel_pressure));
  Serial.println("COOLANT TEMPERATURE: " + String((int)coolant_temperature));
  Serial.println("BATTERY VOLTAGE: " + String((int)battery_voltage));
  Serial.println("INTAKE TEMPERATURE: " + String((int)intake_temperature));
  Serial.println("FUEL LEVEL: " + String((int)fuel_level));
  Serial.println("TOTAL CALL TIME: " + String((int)end_time - (int)start_time));
  Serial.println("--------------");
}

void checkForProblems() {
  // Just googled these values, they may need to be changed

  if (oil_temperature < 35.0) {
    error_data = error_data + "OIL TEMP LOW \n";
  }

  if (fuel_pressure < 20.0) {
    error_data = error_data + " " + "FUEL PRESS LOW \n";
  }

  if (coolant_temperature > 100.0) {
    error_data = error_data + " " + "COOLANT TEMP HIGH \n";
  }

  if (battery_voltage < 12.3) {
    error_data = error_data + " " + "BATTERY BAD \n";
  }

  if (intake_temperature < 2.0) {
    error_data = error_data + " " + "LOW INTAKE TEMP \n";
  }

  if (fuel_level < 0.3) {
    error_data = error_data + " " + "LOW FUEL \n";
  }
}

void renderMainView() {
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);

  // RPM Display
  tft.setCursor(50, 10);
  tft.setTextSize(2);
  tft.println("RPM");
  tft.setCursor(10, 30);
  tft.setTextSize(5);
  tft.println((int)rpm);

  // MPH Display
  tft.setCursor(267, 10);
  tft.setTextSize(2);
  tft.println("MPH");
  tft.setCursor(255, 30);
  tft.setTextSize(5);
  tft.println((int)mph);

  // Bottom text
  tft.setCursor(45, 220);
  tft.setTextSize(2);
  tft.println(String((int)oil_temperature) + "C   " + String((int)fuel_pressure) + "PSI   " + String((int)vvt_timing) + "DEG");

  delay(1000);
}

void displayError() {
  tft.setCursor(10, 10);

  tft.fillScreen(RED);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.println(error_data);

  error_data = "";  // Clearing the errors because they were (hopefully) read

  tft.setCursor(110, 230);
  tft.setTextSize(1);
  tft.println("(WAIT 5 SECONDS)");

  delay(5000);  // Waiting so error can be read and render queue can be cycled
}

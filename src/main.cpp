#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET LED_BUILTIN // Reset pin # (or -1 if sharing Arduino reset pin)

HX711 scale; // HX711 Load Cell Sensor
TwoWire Wire2; // Second I2C bus
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire2, OLED_RESET); // I2C Display

const int LOADCELL_DOUT_PIN = D7;
const int LOADCELL_SCK_PIN = D6;
const float WEIGHT_MULTIPLIER = 0.00263;

float last_weight = 0.0F;
boolean weight_stable = false;

void update_oled_screen(long reading) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2); 
  display.setTextColor(SSD1306_WHITE);
  display.print(reading);
  display.print("g");
  display.display();
}

void update_oled_screen(const char* text, int text_size = 2) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(text_size); 
  display.setTextColor(SSD1306_WHITE);
  display.print(text);
  display.display();
}

float get_weight() {
  float reading = scale.get_units(50);
  return reading;
}

void setup() {
  Serial.begin(9600);

  pinMode(D3, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  update_oled_screen("Initialising...", 1);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.tare(20);
  scale.set_scale(380.2281F);

  update_oled_screen("Ready");

  delay(2000);
}

void loop() {

  if (scale.is_ready()) {

    auto weight = get_weight();
    auto target_weight = last_weight * 0.9F;

    Serial.print("Last weight: ");
    Serial.println(last_weight);

    Serial.print("Target Weight: ");
    Serial.println(target_weight);

    update_oled_screen(weight);

    Serial.print("HX711 average: ");
    Serial.println(weight);

    Serial.print("Weight stable: ");
    Serial.println(weight_stable);

    if (weight > 1.0F && weight > target_weight && weight_stable) {
      digitalWrite(D3, HIGH);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
      digitalWrite(D3, LOW);
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
    }

    if ((int)last_weight == (int)weight && weight_stable == false && weight > 1.0F) {
      weight_stable = true;
    }

    last_weight = weight;

    if (weight_stable) {
      Serial.print("Waiting for 5 minutes");
      delay(300000);
    } else {
      delay(500);
    }

  } else {
    Serial.println("HX711 not found.");
    delay(500);
  }

  Serial.println("HX711 not found.");
}
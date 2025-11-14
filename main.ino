#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Arduino.h>
#include <Bounce2.h>
#include <DHT.h>
#include <RTClib.h>
#include <Wire.h>

// --- Configurazione Hardware ---
// diaplay OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define OLED_RESET -1
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Sensore DHT
#define DHTTYPE DHT11
#define TEMPERATURE_SENSOR = 15;
DHT dht(TEMPERATURE_SENSOR, DHTTYPE);

// Pin
#define PIN_SDA 21
#define PIN_SCL 22
#define BACK_BUTTON 13
#define NEXT_BUTTON 14
#define CONFIRM_BUTTON 27
#define INCREMENT_BUTTON 26
#define DECREMENT_BUTTON 25
#define BUZZER_PIN 2

// --- Costanti Utilizzate ---
const int DEBOUNCER_INTERVAL = 25;
const int SHOW_TEXT_MILLIS = 1500;
const int BUZZER_FREQUENCY = 1000;
const int MENU_SCREEN_COUNT = 5;

// --- Componenti ---
RTC_DS3231 rtc;
Bounce next_debouncer;
Bounce confirm_debouncer;
Bounce increment_debouncer;
Bounce decrement_debouncer;
Bounce back_debouncer;

// --- Variabili di Stato del Sistema ---
enum ScreenState {
  MENU,
  INFO,
  SET_ALARM,
  ONOFF_ALARM,
  SET_TIME,
  ALARM_RINGING,
  SHOW_MESSAGE
};
ScreenState currentScreen = MENU;
ScreenState screenToReturnTo = MENU;

// Variabili per l'orario
int seconds = 0, minutes = 0, hours = 12;
int newMinutes = minutes, newHours = hours; // Per la modifica
float currentTemperature = 0.0;

// Variabili per la sveglia
int alarmHour = 12, alarmMinute = 0;
bool alarmSet = false;
bool alarmTriggered = false;
int newAlarmHour = alarmHour, newAlarmMinute = alarmMinute; // Per la modifica

// Variabili per Messaggi
unsigned long messageStartTime = 0;
String messageToShow = "";

// --- Setup ---
void setup() {
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);

  Serial.begin(115200);
  delay(100);
  Serial.println("Setup avviato...");

  // Setup Pin Pulsanti
  pinMode(NEXT_BUTTON, INPUT_PULLUP);
  pinMode(CONFIRM_BUTTON, INPUT_PULLUP);
  pinMode(INCREMENT_BUTTON, INPUT_PULLUP);
  pinMode(BACK_BUTTON, INPUT_PULLUP);
  pinMode(DECREMENT_BUTTON, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Setup Debouncers
  next_debouncer.attach(NEXT_BUTTON);
  confirm_debouncer.attach(CONFIRM_BUTTON);
  increment_debouncer.attach(INCREMENT_BUTTON);
  back_debouncer.attach(BACK_BUTTON);
  decrement_debouncer.attach(DECREMENT_BUTTON);

  next_debouncer.interval(DEBOUNCER_INTERVAL);
  confirm_debouncer.interval(DEBOUNCER_INTERVAL);
  increment_debouncer.interval(DEBOUNCER_INTERVAL);
  back_debouncer.interval(DEBOUNCER_INTERVAL);
  decrement_debouncer.interval(DEBOUNCER_INTERVAL);

  // Setup Display
  display.begin(OLED_ADDR, true);
  display.clearDisplay();
  display.display();

  // Setup Sensori
  dht.begin();
  if (!rtc.begin()) {
    Serial.println("RTC non trovato");
  } else {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println("RTC in esecuzione.");
  }

  // Inizializza i valori temporanei
  updateData();
  newHours = hours;
  newMinutes = minutes;
  newAlarmHour = alarmHour;
  newAlarmMinute = alarmMinute;

  Serial.println("Setup completato.");
}

// --- Loop Principale ---
void loop() {
  updateData();
  updateButtons();
  if (currentScreen == SHOW_MESSAGE) {
    handleTemporaryMessage();
  } else {
    handleInput();
    updateLogic();
  }
  updateDisplay();
  updateBuzzer();
}

void updateData() {
  DateTime now = rtc.now();
  hours = now.hour();
  minutes = now.minute();
  seconds = now.second();

  float t = dht.readTemperature();
  if (!isnan(t)) {
    currentTemperature = t;
  }
}

void updateButtons() {
  next_debouncer.update();
  confirm_debouncer.update();
  increment_debouncer.update();
  decrement_debouncer.update();
  back_debouncer.update();
}

void updateBuzzer() {
  if (alarmTriggered) {
    tone(BUZZER_PIN, BUZZER_FREQUENCY);
  } else {
    noTone(BUZZER_PIN);
  }
}

void handleInput() {
  if (next_debouncer.fell()) {
    if (currentScreen == SET_ALARM)
      resetTempAlarm();
    if (currentScreen == SET_TIME)
      resetTempTime();

    currentScreen = (ScreenState)((currentScreen + 1) % MENU_SCREEN_COUNT);
  }

  if (back_debouncer.fell()) {
    if (currentScreen == SET_ALARM)
      resetTempAlarm();
    if (currentScreen == SET_TIME)
      resetTempTime();

    int prevScreen = (currentScreen - 1);
    if (prevScreen < 0) {
      prevScreen = MENU_SCREEN_COUNT - 1;
    }
    currentScreen = (ScreenState)prevScreen;
  }

  switch (currentScreen) {
  case SET_ALARM:
    if (increment_debouncer.fell()) {
      newAlarmMinute += 5;
      if (newAlarmMinute >= 60) {
        newAlarmMinute = 0;
        newAlarmHour = (newAlarmHour + 1) % 24;
      }
    }
    if (decrement_debouncer.fell()) {
      newAlarmMinute -= 5;
      if (newAlarmMinute < 0) {
        newAlarmMinute = 55;
        newAlarmHour = (newAlarmHour + 23) % 24;
      }
    }
    if (confirm_debouncer.fell()) {
      alarmSet = true;
      alarmTriggered = false;
      alarmHour = newAlarmHour;
      alarmMinute = newAlarmMinute;
      showTemporaryMessage("Sveglia impostata", MENU);
    }
    break;

  case ONOFF_ALARM:
    if (confirm_debouncer.fell()) {
      alarmSet = !alarmSet;
      if (alarmSet) {
        showTemporaryMessage("Sveglia attivata", MENU);
      } else {
        showTemporaryMessage("Sveglia disattivata", MENU);
        alarmTriggered = false;
      }
    }
    break;

  case SET_TIME:
    if (decrement_debouncer.fell()) {
      newHours = (newHours + 1) % 24;
    }
    if (increment_debouncer.fell()) {
      newMinutes = (newMinutes + 1) % 60;
    }
    if (confirm_debouncer.fell()) {
      hours = newHours;
      minutes = newMinutes;
      setNewClock();
      showTemporaryMessage("Orario impostato", MENU);
    }
    break;

  case ALARM_RINGING:
    if (confirm_debouncer.fell() || back_debouncer.fell()) {
      alarmTriggered = false;
      showTemporaryMessage("Sveglia Fermata", MENU);
    }
    break;

  case MENU:
    break;
  case INFO:
    break;
  case SHOW_MESSAGE:
    break;
  }
}

void updateLogic() {
  if (alarmSet && !alarmTriggered && hours == alarmHour &&
      minutes == alarmMinute && seconds == 0) {
    alarmTriggered = true;
    currentScreen = ALARM_RINGING;
  }
}

void handleTemporaryMessage() {
  if (millis() - messageStartTime > SHOW_TEXT_MILLIS) {
    currentScreen = screenToReturnTo;
  }
}

void setNewClock() {
  DateTime now = rtc.now();
  rtc.adjust(DateTime(now.year(), now.month(), now.day(), hours, minutes, 0));
}

void showTemporaryMessage(String text, ScreenState returnTo) {
  messageToShow = text;
  screenToReturnTo = returnTo;
  messageStartTime = millis();
  currentScreen = SHOW_MESSAGE;
}

void resetTempAlarm() {
  newAlarmHour = alarmHour;
  newAlarmMinute = alarmMinute;
}

void resetTempTime() {
  newHours = hours;
  newMinutes = minutes;
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  switch (currentScreen) {
  case MENU:
    drawMenu();
    break;
  case INFO:
    drawInfo();
    break;
  case SET_ALARM:
    drawSetAlarm();
    break;
  case ONOFF_ALARM:
    drawOnOffAlarm();
    break;
  case SET_TIME:
    drawSetTime();
    break;
  case ALARM_RINGING:
    drawAlarmRinging();
    break;
  case SHOW_MESSAGE:
    drawTemporaryMessage();
    break;
  }

  display.display();
}

void drawMenu() {
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(currentTemperature, 1);
  display.print("C");
  display.setCursor(110, 0);
  display.print(alarmSet ? "ON" : "OFF");

  drawClock();
}

void drawClock() {
  char timeStr[9];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hours, minutes, seconds);
  display.setTextSize(2);
  display.setCursor(20, 20);
  display.println(timeStr);
}

void drawInfo() {
  display.setCursor(10, 5);
  display.println("Pagine disponibili");
  display.setCursor(0, 16);
  display.println("Principale");
  display.println("> Info (Sei qui)");
  display.println("Imposta Sveglia");
  display.println("ON / OFF Sveglia");
  display.println("Imposta orario");
}

void drawSetAlarm() {
  display.setCursor(0, 8);
  display.println("IMPOSTA SVEGLIA");
  display.setTextSize(2);
  display.setCursor(20, 30);

  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", newAlarmHour, newAlarmMinute);
  display.println(buf);
}

void drawOnOffAlarm() {
  display.setCursor(6, 10);
  display.println("Gestisci sveglia:");
  display.setCursor(6, 25);
  display.println(alarmSet ? "Attivata" : "Disattivata");
  display.setCursor(6, 40);
  display.print("Orario: ");
  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", alarmHour, alarmMinute);
  display.println(buf);
}

void drawSetTime() {
  display.setCursor(0, 0);
  display.println("IMPOSTA ORARIO");

  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", newHours, newMinutes);
  display.setTextSize(2);
  display.setCursor(26, 24);
  display.println(buf);
  display.drawLine(26, 44, 26 + 24, 44, SH110X_WHITE);
  display.drawLine(26 + 36, 44, 26 + 60, 44, SH110X_WHITE);
}

void drawAlarmRinging() {
  display.setTextSize(2);
  display.setCursor(9, 20);
  display.println("SVEGLIA!");
}

void drawTemporaryMessage() {
  display.setTextSize(1);
  display.setCursor(9, 20);
  display.println(messageToShow);
}
#include <ArduinoBLE.h>

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214");

BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

const int ledPin = LED_BUILTIN;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(ledPin, OUTPUT);

  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  BLE.setLocalName("LED");
  BLE.setAdvertisedService(ledService);

  ledService.addCharacteristic(switchCharacteristic);

  BLE.addService(ledService);

  switchCharacteristic.writeValue(0);

  BLE.advertise();

  Serial.println("BLE LED Peripheral");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");

    Serial.println(central.address());

    while (central.connected()) {
      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value()) {
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);
        } else {
          Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);
        }
      }
    }

    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}

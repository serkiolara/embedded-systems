// Lab 1 — NTC Thermistor Reader
// ESP32 ADC on GPIO34
// Andrea Venegas & Sergio Lara

void setup() {
  Serial.begin(115200);
  pinMode(34, INPUT);
}

void loop() {
  // 1. Read sensor (0-4095)
  int valorADC = analogRead(34);

  // 2. Convert to voltage (0-3300 mV)
  float voltaje = (valorADC * 3300.0) / 4095.0;

  // 3. Calculate NTC resistance
  float resistenciaNTC = 10000.0 * ((3300.0 - voltaje) / voltaje);

  // 4. Calculate temperature (simplified formula)
  float temperatura = 25.0 + ((resistenciaNTC - 10000.0) / -300.0);

  // 5. Print results
  Serial.print("ADC: ");
  Serial.print(valorADC);
  Serial.print(" | Voltage: ");
  Serial.print(voltaje);
  Serial.print(" mV | Temp: ");
  Serial.print(temperatura);
  Serial.println(" C");

  delay(2000);
}

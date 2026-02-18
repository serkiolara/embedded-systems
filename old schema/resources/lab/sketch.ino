void setup() {
  Serial.begin(115200);
  pinMode(34, INPUT);
}

void loop() {
  // 1. Leer el sensor (0-4095)
  int valorADC = analogRead(34);
  
  // 2. Convertir a voltaje (0-3300 mV)
  float voltaje = (valorADC * 3300.0) / 4095.0;
  
  // 3. Calcular resistencia del NTC
  float resistenciaNTC = 10000.0 * ((3300.0 - voltaje) / voltaje);
  
  // 4. Calcular temperatura (fórmula simplificada)
  float temperatura = 25.0 + ((resistenciaNTC - 10000.0) / -300.0);
  
  // 5. Mostrar resultados
  Serial.print("ADC: ");
  Serial.print(valorADC);
  Serial.print(" | Voltaje: ");
  Serial.print(voltaje);
  Serial.print(" mV | Temp: ");
  Serial.print(temperatura);
  Serial.println(" °C");
  
  delay(2000); // Esperar 2 segundos
}
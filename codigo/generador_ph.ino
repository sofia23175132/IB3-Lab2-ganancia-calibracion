/*
  Laboratorio N.º 2 - Instrumentación Biomédica III
  Simulador de electrodo de pH con el DAC interno del ESP32

  Ecuación:  Vin = Vbias - S * (pH - 7)
    Vbias = 1.65 V       (mitad del rango 0-3.3 V del DAC)
    S     = 0.05916 V/pH (pendiente de Nernst a 25 °C)

  Uso:
    1. Monitor Serie a 115200 baudios, con "Nueva línea" activado.
    2. Escribir un valor de pH entre 0 y 14 y presionar Enter.
    3. El DAC actualiza la salida en GPIO25 y se imprime el resultado.

  Conexión: GPIO25 (DAC1) -> pin 3 del TL084 (entrada no inversora del buffer)
            GND del ESP32 -> GND común del circuito
*/

const int   PIN_DAC  = 25;      // DAC1 del ESP32 (el DAC2 es GPIO26)
const float VBIAS    = 1.65;    // V
const float S        = 0.05916; // V/pH
const float VREF_DAC = 3.3;     // V, fondo de escala del DAC
const int   DAC_MAX  = 255;     // DAC de 8 bits: códigos 0 a 255

void generarPH(float ph) {
  // Voltaje ideal según la ecuación de Nernst centrada en pH 7
  float vinIdeal = VBIAS - S * (ph - 7.0);

  // Conversión a código del DAC (8 bits) y protección de límites
  int codigo = (int)roundf(vinIdeal / VREF_DAC * DAC_MAX);
  codigo = constrain(codigo, 0, DAC_MAX);

  dacWrite(PIN_DAC, codigo);

  // Voltaje que el DAC realmente genera (cuantizado a pasos de ~12.9 mV)
  float vinReal = codigo * VREF_DAC / DAC_MAX;

  Serial.printf("pH = %.2f | Vin ideal = %.4f V | codigo DAC = %d | Vin generado = %.4f V\n",
                ph, vinIdeal, codigo, vinReal);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("=== Simulador de pH (Lab 2) ===");
  Serial.println("Ingrese un valor de pH entre 0 y 14 y presione Enter.");

  generarPH(7.0);  // Estado inicial: pH neutro (Vin = 1.65 V)
}

void loop() {
  if (Serial.available() > 0) {
    String linea = Serial.readStringUntil('\n');
    linea.trim();
    linea.replace(',', '.');   // acepta "6,5" además de "6.5"

    if (linea.length() == 0) return;

    // Conversión con verificación: rechaza texto que no sea un número
    char *fin;
    float ph = strtof(linea.c_str(), &fin);

    if (*fin != '\0') {
      Serial.println("Error: ingrese solo un numero (ej. 7 o 6.5).");
      return;
    }

    // Validación del rango (la forma negada también rechaza NaN)
    if (!(ph >= 0.0 && ph <= 14.0)) {
      Serial.println("Error: pH fuera de rango. Debe estar entre 0 y 14.");
      return;
    }

    generarPH(ph);
  }
}

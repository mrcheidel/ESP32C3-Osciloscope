# Ejemplos Prácticos - Osciloscopio ESP32-C3

## 📚 Casos de Uso Comunes

### 1. Medir Señal PWM de un Arduino

**Escenario**: Quieres verificar que tu Arduino está generando un PWM de 1kHz al 50% de duty cycle.

**Conexiones**:
```
Arduino UNO          ESP32-C3
Pin 9 (PWM) ────────→ GPIO2
GND         ────────→ GND
```

**Configuración del Osciloscopio**:
- Base de tiempo: **100 µs** (para ver ~10 ciclos)
- Trigger: **2.5V**, flanco ascendente (↑)
- Modo: Continuo

**Código Arduino de Prueba**:
```cpp
void setup() {
  pinMode(9, OUTPUT);
}

void loop() {
  analogWrite(9, 128); // 50% duty cycle
}
```

**Verificación**:
- El período debe ser ~1ms (1kHz)
- El ancho del pulso alto debe ser ~500µs (50%)
- Vpp debe ser ~5V (¡CUIDADO! Necesitas divisor de voltaje)

---

### 2. Analizar Señal de un Sensor Analógico

**Escenario**: Tienes un sensor de temperatura LM35 y quieres ver su salida.

**Conexiones**:
```
LM35              ESP32-C3
Vout  ───────────→ GPIO2
GND   ───────────→ GND
+5V   ───────────→ (fuente externa, NO al ESP32)
```

**⚠️ IMPORTANTE**: El LM35 puede dar hasta 1.5V (150°C), que está dentro del rango del ESP32-C3.

**Configuración del Osciloscopio**:
- Base de tiempo: **1000 µs** (1ms)
- Trigger: Desactivado
- Modo: Continuo

**Interpretación**:
- Vavg muestra el voltaje promedio
- 10mV = 1°C
- Ejemplo: 0.25V = 25°C

---

### 3. Depurar Comunicación Serial (UART)

**Escenario**: Verificar que un módulo está enviando datos a 9600 baud.

**Conexiones**:
```
Módulo TX  ──[10kΩ]──→ GPIO2 (con divisor si es 5V)
Módulo GND ──────────→ GND
```

**Configuración del Osciloscopio**:
- Base de tiempo: **10 µs**
- Trigger: **1.65V**, flanco descendente (↓)
- Modo: Único

**Verificación a 9600 baud**:
- Cada bit debe durar ~104 µs (1/9600)
- Start bit = 0V
- Stop bit = 3.3V
- 8 bits de datos entre medio

---

### 4. Medir Frecuencia de un Oscilador

**Escenario**: Verificar un cristal de 32.768 kHz de un RTC.

**Conexiones**:
```
RTC Output ──→ GPIO2
RTC GND    ──→ GND
```

**Configuración del Osciloscopio**:
- Base de tiempo: **5 µs**
- Trigger: **1.65V**, flanco ascendente
- Modo: Continuo

**Verificación**:
- Período ≈ 30.5 µs
- Frecuencia mostrada ≈ 32.768 kHz
- Forma de onda: cuadrada o senoidal

---

### 5. Detectar Rebotes en un Pulsador

**Escenario**: Ver los rebotes mecánicos al presionar un botón.

**Circuito**:
```
3.3V ──┬── GPIO2
       │
    [10kΩ]
       │
       ├─── Pulsador ─── GND
```

**Configuración del Osciloscopio**:
- Base de tiempo: **100 µs**
- Trigger: **2.0V**, flanco descendente
- Modo: Único

**Observación**:
- Verás múltiples transiciones en ~5-10ms
- Esto muestra por qué necesitas debouncing en software

---

### 6. Analizar Salida de un DAC

**Escenario**: Verificar la salida de un convertidor digital-analógico.

**Código ESP32 Generador** (otro ESP32):
```cpp
void setup() {
  dacWrite(25, 128); // 1.65V (mitad de escala)
}
```

**Conexiones**:
```
ESP32 DAC (GPIO25) ──→ ESP32-C3 GPIO2
GND                ──→ GND
```

**Configuración del Osciloscopio**:
- Base de tiempo: **1000 µs**
- Trigger: Desactivado
- Modo: Continuo

**Verificación**:
- Vavg debe ser ≈ 1.65V para valor 128
- Verificar estabilidad (bajo ripple)

---

### 7. Medir Tiempo de Respuesta de un Sensor

**Escenario**: Ver qué tan rápido responde un sensor de luz al cambio.

**Conexiones**:
```
LDR en divisor de voltaje
3.3V ──[10kΩ]──┬──→ GPIO2
               │
             [LDR]
               │
              GND
```

**Configuración del Osciloscopio**:
- Base de tiempo: **500 µs**
- Trigger: **2.0V**, flanco ascendente
- Modo: Único

**Prueba**:
1. Tapa y destapa el sensor
2. Captura en modo único
3. Mide el tiempo de subida (rise time)

---

### 8. Verificar Filtro RC

**Escenario**: Comprobar la frecuencia de corte de un filtro pasa-bajos.

**Circuito del Filtro**:
```
Señal ──[R 1kΩ]──┬──→ Salida (a GPIO2)
                 │
               [C 100nF]
                 │
                GND

Frecuencia de corte teórica:
fc = 1/(2π×R×C) = 1/(2π×1000×100e-9) ≈ 1.6 kHz
```

**Prueba**:
1. Genera señal PWM a diferentes frecuencias
2. Observa cómo se suaviza la señal
3. A fc, la amplitud cae a ~70% del original

---

### 9. Debug de Señal I2C (Clock)

**Escenario**: Verificar que el reloj I2C funciona correctamente.

**Conexiones**:
```
I2C SCL ──→ GPIO2
I2C GND ──→ GND
```

**Configuración**:
- Base de tiempo: **1 µs**
- Trigger: **2.0V**, flanco ascendente
- Modo: Continuo

**I2C Standard (100 kHz)**:
- Período del clock ≈ 10 µs
- Frecuencia ≈ 100 kHz

**I2C Fast (400 kHz)**:
- Período del clock ≈ 2.5 µs
- Frecuencia ≈ 400 kHz

---

### 10. Capturar Pulso Ultrasónico (HC-SR04)

**Escenario**: Ver el pulso de Echo del sensor de distancia.

**Conexiones**:
```
HC-SR04 Echo ──[10kΩ]──→ GPIO2 (divisor de 5V a 3.3V)
        GND  ──────────→ GND
```

**Configuración**:
- Base de tiempo: **100 µs**
- Trigger: **2.0V**, flanco ascendente
- Modo: Único

**Cálculo de distancia**:
- Ancho del pulso en µs ÷ 58 = distancia en cm
- Ejemplo: 1160 µs = 20 cm

---

## 🔧 Configuraciones Típicas por Aplicación

| Aplicación | Base Tiempo | Trigger | Escala V |
|------------|-------------|---------|----------|
| PWM Arduino (1kHz) | 100 µs | 2.5V ↑ | 1.0x |
| Señal Audio | 10-50 µs | OFF | 2.0x |
| UART 9600 baud | 10 µs | 1.65V ↓ | 1.0x |
| I2C Clock | 1 µs | 2.0V ↑ | 1.0x |
| SPI Clock | 0.5-1 µs | 1.65V ↑ | 1.0x |
| Sensor Analógico | 1000 µs | OFF | 1.0x |
| Rebotes Pulsador | 100 µs | 2.0V ↓ | 1.0x |

---

## 🎯 Tips y Trucos

### Para señales de 5V:

**Opción 1 - Divisor de voltaje simple**:
```
5V ──[10kΩ]──┬──→ GPIO2 (≈1.65V)
             │
          [10kΩ]
             │
            GND
```

**Opción 2 - Divisor 3:2**:
```
5V ──[10kΩ]──┬──→ GPIO2 (≈3.3V max)
             │
          [22kΩ]
             │
            GND
```

### Para AC con offset DC:
```
Señal AC ──[1µF]──┬──[100kΩ]── 3.3V/2 (bias)
                  │
                GPIO2
```

### Mejorar la resolución temporal:
- Usa el buffer completo (1024 muestras)
- Ajusta la base de tiempo al mínimo necesario
- Para análisis detallado, captura en modo único

### Reducir ruido:
- Cables cortos y apantallados
- Tierra común sólida
- Añadir capacitor de 10nF en la entrada
- Alejar de fuentes de interferencia (WiFi, motores)

---

## 🐛 Troubleshooting por Caso

### No veo la señal PWM:
- ✓ Verifica que el divisor de voltaje esté correcto
- ✓ Ajusta el trigger al nivel medio
- ✓ Reduce la base de tiempo
- ✓ Comprueba las conexiones GND

### La frecuencia calculada es incorrecta:
- ✓ Captura al menos 2-3 ciclos completos
- ✓ Señales muy lentas: aumenta base de tiempo
- ✓ Señales ruidosas: añade filtro hardware
- ✓ Verifica que el trigger esté bien ajustado

### La forma de onda se ve distorsionada:
- ✓ Impedancia de entrada: añade buffer
- ✓ Frecuencia demasiado alta: límite ADC ~100kHz
- ✓ Cables largos: reducir longitud o usar blindados

---

**¡Experimenta y diviértete con tu osciloscopio ESP32-C3!** 🚀

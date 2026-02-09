# Osciloscopio Digital ESP32-C3 v0.4

## 📋 Descripción

Osciloscopio digital completo para ESP32-C3 (QFN32) con interfaz web, captura en tiempo real y múltiples funcionalidades profesionales.

## ✨ Características

- **Muestreo**: Hasta ~100 kHz (ajustable)
- **Resolución**: 12 bits (0-4095 valores)
- **Rango de voltaje**: 0-3.3V
- **Buffer**: 1024 muestras
- **Trigger**: Configurable en nivel, flanco ascendente/descendente
- **Modos**: Captura única o continua
- **Interfaz web**: Visualización en tiempo real con gráficos suaves
- **Mediciones automáticas**: Vpp, Vmax, Vmin, Vavg, Frecuencia
- **Base de tiempo**: Ajustable de 1µs a 1ms

## 🔌 Pinout ESP32-C3 (QFN32)

```
┌─────────────────────────────────┐
│      ESP32-C3-MINI-1 (QFN32)    │
├─────────────────────────────────┤
│ GPIO2 (ADC1_CH2) ← Entrada señal│
│ GPIO8           ← LED estado    │
│ GND             ← Tierra        │
│ 3V3             ← Alimentación  │
└─────────────────────────────────┘
```

### Conexiones:

1. **GPIO2 (ADC1_CH2)**: Entrada de señal analógica
   - Rango: 0-3.3V
   - ⚠️ **NO CONECTAR MÁS DE 3.3V**
   
2. **GPIO8**: LED indicador de captura

3. **GND**: Tierra común con la señal a medir

## 🚀 Instalación

### Requisitos:
- Arduino IDE 1.8.x o superior / PlatformIO
- Placa ESP32-C3 instalada en el gestor de placas
- Librerías incluidas en el core ESP32

### Pasos:

1. **Configurar WiFi**:
   ```cpp
   const char* ssid = "TU_WIFI_SSID";
   const char* password = "TU_WIFI_PASSWORD";
   ```

2. **Seleccionar placa**:
   - Placa: "ESP32C3 Dev Module"
   - Flash Size: 4MB
   - Partition Scheme: "Default 4MB with spiffs"
   - Core Debug Level: "None"

3. **Compilar y subir** el código

4. **Conectar** al osciloscopio:
   - Si se conecta a WiFi: Abre la IP mostrada en el Serial Monitor
   - Si falla WiFi: Conecta a la red "ESP32-Oscilloscope" (password: 12345678) y abre 192.168.4.1

## 📊 Uso

### Interfaz Web

Accede a la interfaz web y encontrarás:

1. **Display principal**: Visualización de la forma de onda en tiempo real

2. **Estadísticas**:
   - **Vpp**: Voltaje pico a pico
   - **Vmax/Vmin**: Voltajes máximo y mínimo
   - **Vavg**: Voltaje promedio
   - **Frecuencia**: Calculada automáticamente

3. **Controles**:
   - **Capturar**: Inicia una captura manual
   - **Modo**: Alterna entre continuo y único
   - **Base de tiempo**: Ajusta el tiempo entre muestras (1-1000 µs)
   - **Nivel de Trigger**: Ajusta el voltaje de disparo (0-3.3V)
   - **Trigger ON/OFF**: Activa/desactiva y cambia flanco (↑/↓)
   - **Escala Vertical**: Zoom en el eje Y (0.1x - 5x)

### Ejemplos de Medición

#### Medir una señal PWM:
1. Conecta la salida PWM a GPIO2
2. Ajusta la base de tiempo a ~100µs
3. Configura el trigger a 1.65V (mitad de escala)
4. Observa la forma de onda cuadrada

#### Medir una señal de audio:
1. Conecta la señal de audio (0-3.3V DC offset requerido)
2. Ajusta la base de tiempo a 10-50µs
3. Desactiva el trigger o ajústalo según necesites
4. La frecuencia se calculará automáticamente

#### Medir tensión DC:
1. Conecta la fuente DC a GPIO2
2. La lectura se mostrará en Vavg
3. Vpp debería ser ~0V si es DC pura

## ⚙️ Configuración Avanzada

### Cambiar el pin de entrada:
```cpp
#define ADC_PIN 3  // Por ejemplo, GPIO3 (ADC1_CH3)
#define ADC_CHANNEL ADC1_CHANNEL_3
```

### Aumentar el buffer (requiere más RAM):
```cpp
#define BUFFER_SIZE 2048  // Duplica las muestras
```

### Ajustar rango de voltaje:
```cpp
#define ADC_ATTEN ADC_ATTEN_DB_11  // 0-3.3V (actual)
// ADC_ATTEN_DB_6   // 0-2.2V
// ADC_ATTEN_DB_2_5 // 0-1.5V
// ADC_ATTEN_DB_0   // 0-1.1V
```

## 🛠️ Especificaciones Técnicas

| Parámetro | Valor |
|-----------|-------|
| Resolución ADC | 12 bits (4096 niveles) |
| Frecuencia máx muestreo | ~100 kHz* |
| Buffer de muestras | 1024 muestras |
| Rango entrada | 0 - 3.3V |
| Impedancia entrada | ~10 MΩ |
| Trigger | Nivel ajustable, flanco ↑/↓ |
| Interfaz | Web (WiFi) |

*La frecuencia real depende de la base de tiempo configurada

## 🔧 Troubleshooting

### El osciloscopio no se conecta al WiFi:
- Verifica el SSID y password
- Espera 15 segundos
- Si sigue sin conectar, crea un AP automáticamente

### La señal se ve distorsionada:
- Verifica que la señal esté en el rango 0-3.3V
- Ajusta la escala vertical
- Reduce la base de tiempo para señales rápidas

### No hay trigger:
- Asegúrate de que la señal cruza el nivel de trigger
- Intenta desactivar el trigger temporalmente
- Verifica que la señal tenga suficiente amplitud

### La frecuencia no se calcula correctamente:
- La señal debe tener cruces claros
- Ajusta la base de tiempo para capturar al menos 2-3 ciclos
- Señales muy lentas o ruidosas pueden dar lecturas incorrectas

## 📝 Limitaciones

1. **Voltaje máximo**: 3.3V (usar divisor de voltaje para señales mayores)
2. **Impedancia entrada**: No es alta impedancia pura, puede cargar circuitos sensibles
3. **AC coupling**: No incluido, solo DC (añadir capacitor externo si necesario)
4. **Frecuencia**: Limitada por el ADC del ESP32-C3
5. **WiFi**: Puede introducir pequeños delays en captura

## 🔒 Protección de Entrada (Recomendado)

Para proteger el ESP32-C3, considera añadir:

```
Señal ──[R 10kΩ]──┬──[D Zener 3.3V]── GND
                  │
                GPIO2
```

## 📚 Recursos Adicionales

- [Datasheet ESP32-C3](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)
- [ESP32 ADC Calibration](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/api-reference/peripherals/adc.html)

## 📄 Licencia

Código libre para uso personal y educativo.

## 👨‍💻 Autor

Creado para ESP32-C3 (QFN32) v0.4

---

**⚠️ ADVERTENCIA**: Nunca conectes voltajes superiores a 3.3V directamente al ESP32-C3. Usa divisores de voltaje o atenuadores para señales mayores.

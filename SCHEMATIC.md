# Esquemático de Conexiones - Osciloscopio ESP32-C3

## Diagrama Básico de Conexión

```
                    ESP32-C3 (QFN32)
                   ┌─────────────────┐
    USB-C ────────┤ USB D-/D+       │
                  │                  │
    3.3V ─────────┤ 3V3             │
                  │                  │
    GND ──────────┤ GND         GPIO2├──── Entrada Señal (0-3.3V)
                  │                  │
                  │            GPIO8 ├──── LED ──[330Ω]── GND
                  │                  │
                  │            GPIO9 │ (Reservado)
                  │                  │
                  └─────────────────┘
```

## Circuito de Protección de Entrada (Recomendado)

```
                              ┌─────────────┐
 Señal Externa                │  ESP32-C3   │
     ───┬───[R1 10kΩ]────┬───┤GPIO2 (ADC)  │
        │                 │   │             │
        │              [C1 10nF]            │
        │                 │   │             │
        │            [D1 Zener]             │
        │              3.3V│   │             │
        │                 │   │             │
       GND ──────────────┴───┤GND          │
                              └─────────────┘

Componentes:
- R1: 10kΩ - Resistencia de protección
- C1: 10nF - Filtro anti-ruido (opcional)
- D1: Diodo Zener 3.3V - Protección sobrevoltaje
```

## Divisor de Voltaje para Señales > 3.3V

Para medir señales de hasta 12V:

```
                              ┌─────────────┐
 Señal 0-12V                  │  ESP32-C3   │
     ───[R1 27kΩ]────┬────────┤GPIO2 (ADC)  │
                     │        │             │
                  [R2 10kΩ]   │             │
                     │        │             │
                    GND ──────┤GND          │
                              └─────────────┘

Factor de división: 3.7:1
Voltaje en GPIO2 = Vin × (10k / (27k + 10k)) = Vin × 0.27
12V → 3.24V (seguro para el ADC)

Para reconstruir el voltaje original en software:
V_real = V_medido × 3.7
```

## Conexión de LED Indicador

```
                    ┌─────────────┐
                    │  ESP32-C3   │
                    │             │
                    │       GPIO8 ├───[330Ω]───┤>├── GND
                    │             │           LED
                    │             │         (Verde)
                    └─────────────┘
```

## Circuito Completo con Protección

```
                                      ┌──────────────────┐
                                      │    ESP32-C3      │
 Señal de Prueba                      │                  │
      │                               │                  │
      └──[R1 10kΩ]──┬──[C1 10nF]─────┤GPIO2 (ADC1_CH2) │
                    │                 │                  │
              [D1 Zener 3.3V]         │                  │
                    │                 │                  │
      GND ──────────┴─────────────────┤GND              │
                                      │                  │
      +5V USB-C ─────────────────────┤VBUS             │
                                      │                  │
                                      │GPIO8 ├─[330Ω]─┤>├─ GND
                                      │                 LED
                                      │                  │
                                      └──────────────────┘

Lista de Componentes:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Item    Valor       Descripción
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
R1      10kΩ        Protección entrada
R2      330Ω        Limitador LED
C1      10nF        Filtro ruido (opcional)
D1      Zener 3.3V  Protección sobrevoltaje
LED     Verde 3mm   Indicador captura
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

## Asignación de Pines ESP32-C3 (QFN32)

```
Pin físico | GPIO | Función en este proyecto
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    1      | GND  | Tierra
    2      | GND  | Tierra
    3      | 3V3  | Alimentación 3.3V
    4      | 3V3  | Alimentación 3.3V
    5      | GPIO2| ADC1_CH2 - Entrada señal
    6      | GPIO3| Libre (ADC1_CH3 alternativo)
    7      | GND  | Tierra
    8      | GPIO4| Libre
    9      | GPIO5| Libre
    10     | GPIO6| Libre
    11     | GPIO7| Libre
    12     | GPIO8| LED indicador
    13     | GPIO9| Libre (BOOT - cuidado)
    14-17  | USB  | Datos USB
    18-20  | NC   | No conectado
```

## Sonda de Entrada Sugerida

Para usar como osciloscopio práctico:

```
    Punta de Prueba
         │
         ├───[Conector BNC o Jack]
         │
    [Cable Blindado]
         │
    ┌────┴────┐
    │  R 10kΩ │  ← Resistencia protección
    └────┬────┘
         │
    ┌────┴────┐
    │ C 10nF  │  ← Filtro (opcional)
    └────┬────┘
         │
    ┌────┴────┐
    │ Zener   │  ← Protección 3.3V
    │  3.3V   │
    └────┬────┘
         │
    ───GPIO2───
         │
    ────GND────
         │
    Pinza Cocodrilo
```

## Notas Importantes

### 🔴 Límites Absolutos:
- **Voltaje máximo en GPIO2**: 3.6V (destruye el chip si se supera)
- **Corriente máxima por pin**: 40 mA
- **Frecuencia máxima muestreo**: ~100 kHz (limitado por software)

### 🟡 Recomendaciones:
1. **Siempre** usa tierra común entre la señal y el ESP32
2. Añade el circuito de protección si vas a medir señales desconocidas
3. Para señales AC, añade un capacitor de acoplamiento (1µF) + bias resistor
4. No conectes directamente a redes de 220V/110V AC

### 🟢 Mejoras Opcionales:
- Añadir amplificador operacional buffer (alta impedancia)
- Añadir conmutador de ganancia x1/x10
- Incluir acoplamiento AC/DC seleccionable
- Añadir offset ajustable

## PCB Layout Sugerido

```
┌─────────────────────────────────────┐
│  ESP32-C3 Oscilloscope PCB v0.4     │
├─────────────────────────────────────┤
│                                     │
│  [BNC/SMA]          [ESP32-C3]      │
│   Input  ──R1──C1──D1── Module      │
│   o  o                              │
│  SIG GND            [USB-C]         │
│                                     │
│  [LED]                              │
│   PWR  ACQ                          │
│    •    •                           │
│                                     │
│  [Headers para expansión]           │
│   3V3 GND GPIO3 GPIO4 GPIO5 ...     │
└─────────────────────────────────────┘

Dimensiones sugeridas: 50mm x 35mm
```

## Calibración del ADC

El código incluye calibración automática del ADC. Si necesitas mayor precisión:

1. Conecta una fuente de voltaje conocida (ej: 1.00V)
2. Lee el valor ADC
3. Calcula el factor de corrección
4. Ajusta en el código:

```cpp
// Ejemplo de calibración
float voltage_correction = 1.00 / measured_voltage;
float real_voltage = (adc_value / 4095.0) * 3.3 * voltage_correction;
```

---

**Creado para ESP32-C3 (QFN32) Revision v0.4**

/*
 * Osciloscopio Digital para ESP32-C3 (QFN32) v0.4
 * 
 * Características:
 * - Muestreo de señales analógicas hasta ~100kHz
 * - Visualización web en tiempo real
 * - Trigger configurable
 * - Base de tiempo ajustable
 * - Modo continuo y único
 * 
 * Pines:
 * - GPIO2 (ADC1_CH2): Canal de entrada analógica
 * - GPIO8: LED de estado
 */

#include <WiFi.h>
#include <WebServer.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

// ===== CONFIGURACIÓN WiFi =====
const char* ssid = "NOMBRE-DE-MI-RED-WIFI";
const char* password = "PASSWORD DE MI RED WIFI";

// ===== CONFIGURACIÓN DEL OSCILOSCOPIO =====
#define ADC_PIN 2                    // GPIO2 = ADC1_CH2
#define LED_PIN 8                    // LED de estado
#define BUFFER_SIZE 1024             // Tamaño del buffer de muestras
#define ADC_CHANNEL ADC1_CHANNEL_2   // Canal ADC1_CH2
#define ADC_ATTEN ADC_ATTEN_DB_11    // 0-3.3V
#define ADC_WIDTH ADC_WIDTH_BIT_12   // 12 bits (0-4095)

// Variables globales
WebServer server(80);
uint16_t samples[BUFFER_SIZE];
volatile bool captureComplete = false;
volatile bool triggerEnabled = true;
volatile uint16_t triggerLevel = 2048;  // Nivel medio (12 bits)
volatile bool triggerRising = true;
volatile int timebaseUs = 100;          // Microsegundos entre muestras
volatile bool continuousMode = true;

esp_adc_cal_characteristics_t adc_chars;

// ===== FUNCIONES DE CAPTURA =====

void setupADC() {
  // Configurar el ADC
  adc1_config_width(ADC_WIDTH);
  adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);
  
  // Calibrar el ADC
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH, 1100, &adc_chars);
  
  Serial.println("ADC configurado");
}

void captureSamples() {
  int i = 0;
  bool triggered = false;
  uint16_t previousSample = 0;
  
  // Si el trigger está habilitado, esperar por él
  if (triggerEnabled) {
    unsigned long timeout = millis() + 1000; // 1 segundo de timeout
    
    while (!triggered && millis() < timeout) {
      uint16_t currentSample = adc1_get_raw(ADC_CHANNEL);
      
      if (triggerRising) {
        // Trigger en flanco ascendente
        if (previousSample < triggerLevel && currentSample >= triggerLevel) {
          triggered = true;
        }
      } else {
        // Trigger en flanco descendente
        if (previousSample > triggerLevel && currentSample <= triggerLevel) {
          triggered = true;
        }
      }
      
      previousSample = currentSample;
      delayMicroseconds(10);
    }
    
    if (!triggered) {
      Serial.println("Trigger timeout");
    }
  }
  
  // Capturar muestras
  digitalWrite(LED_PIN, HIGH);
  unsigned long startTime = micros();
  
  for (i = 0; i < BUFFER_SIZE; i++) {
    samples[i] = adc1_get_raw(ADC_CHANNEL);
    
    // Esperar el tiempo de la base de tiempo
    if (timebaseUs > 0) {
      while (micros() - startTime < (unsigned long)(i + 1) * timebaseUs) {
        // Espera activa para precisión
      }
    }
  }
  
  digitalWrite(LED_PIN, LOW);
  captureComplete = true;
}

// ===== PÁGINAS WEB =====

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-C3 Oscilloscope</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: #1a1a2e;
            color: #eee;
            padding: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        h1 {
            text-align: center;
            color: #0f3460;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            margin-bottom: 20px;
        }
        .canvas-container {
            background: #16213e;
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 20px;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3);
        }
        canvas {
            width: 100%;
            height: 400px;
            background: #0f0f23;
            border-radius: 5px;
            cursor: crosshair;
        }
        .controls {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 15px;
            background: #16213e;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3);
        }
        .control-group {
            background: #0f3460;
            padding: 15px;
            border-radius: 8px;
        }
        .control-group label {
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            color: #00d4ff;
        }
        input[type="range"] {
            width: 100%;
            margin: 10px 0;
        }
        button {
            width: 100%;
            padding: 12px;
            margin: 5px 0;
            border: none;
            border-radius: 5px;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s;
            font-size: 14px;
        }
        .btn-primary {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }
        .btn-primary:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 20px rgba(102, 126, 234, 0.4);
        }
        .btn-secondary {
            background: #0f3460;
            color: #00d4ff;
        }
        .btn-secondary:hover {
            background: #1a4d7a;
        }
        .stats {
            display: flex;
            justify-content: space-around;
            margin-top: 15px;
            padding: 15px;
            background: #0f0f23;
            border-radius: 5px;
        }
        .stat {
            text-align: center;
        }
        .stat-label {
            font-size: 12px;
            color: #888;
        }
        .stat-value {
            font-size: 20px;
            font-weight: bold;
            color: #00d4ff;
        }
        .value-display {
            color: #00ff88;
            font-weight: bold;
            margin-left: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔬 ESP32-C3 Oscilloscope v0.4</h1>
        
        <div class="canvas-container">
            <canvas id="oscCanvas"></canvas>
            <div class="stats">
                <div class="stat">
                    <div class="stat-label">Vpp</div>
                    <div class="stat-value" id="vpp">0.00 V</div>
                </div>
                <div class="stat">
                    <div class="stat-label">Vmax</div>
                    <div class="stat-value" id="vmax">0.00 V</div>
                </div>
                <div class="stat">
                    <div class="stat-label">Vmin</div>
                    <div class="stat-value" id="vmin">0.00 V</div>
                </div>
                <div class="stat">
                    <div class="stat-label">Vavg</div>
                    <div class="stat-value" id="vavg">0.00 V</div>
                </div>
                <div class="stat">
                    <div class="stat-label">Frecuencia</div>
                    <div class="stat-value" id="freq">0 Hz</div>
                </div>
            </div>
        </div>
        
        <div class="controls">
            <div class="control-group">
                <label>Captura</label>
                <button class="btn-primary" onclick="startCapture()">▶ Capturar</button>
                <button class="btn-secondary" onclick="toggleMode()">
                    <span id="modeText">Modo: Continuo</span>
                </button>
            </div>
            
            <div class="control-group">
                <label>Base de Tiempo: <span class="value-display" id="timebaseVal">100 µs</span></label>
                <input type="range" id="timebase" min="1" max="1000" value="100" 
                       oninput="updateTimebase(this.value)">
            </div>
            
            <div class="control-group">
                <label>Nivel de Trigger: <span class="value-display" id="triggerVal">1.65 V</span></label>
                <input type="range" id="trigger" min="0" max="4095" value="2048" 
                       oninput="updateTrigger(this.value)">
                <button class="btn-secondary" onclick="toggleTrigger()">
                    <span id="triggerText">Trigger: ON ↑</span>
                </button>
            </div>
            
            <div class="control-group">
                <label>Escala Vertical: <span class="value-display" id="scaleVal">1.0x</span></label>
                <input type="range" id="scale" min="0.1" max="5" step="0.1" value="1" 
                       oninput="updateScale(this.value)">
            </div>
        </div>
    </div>

    <script>
        const canvas = document.getElementById('oscCanvas');
        const ctx = canvas.getContext('2d');
        let continuousMode = true;
        let triggerEnabled = true;
        let triggerRising = true;
        let captureInterval;
        let verticalScale = 1.0;
        
        // Ajustar canvas al tamaño real
        function resizeCanvas() {
            canvas.width = canvas.offsetWidth;
            canvas.height = canvas.offsetHeight;
        }
        resizeCanvas();
        window.addEventListener('resize', resizeCanvas);
        
        function drawGrid() {
            ctx.strokeStyle = '#1a3a4a';
            ctx.lineWidth = 1;
            
            // Líneas verticales
            for (let x = 0; x < canvas.width; x += canvas.width / 10) {
                ctx.beginPath();
                ctx.moveTo(x, 0);
                ctx.lineTo(x, canvas.height);
                ctx.stroke();
            }
            
            // Líneas horizontales
            for (let y = 0; y < canvas.height; y += canvas.height / 8) {
                ctx.beginPath();
                ctx.moveTo(0, y);
                ctx.lineTo(canvas.width, y);
                ctx.stroke();
            }
            
            // Línea central
            ctx.strokeStyle = '#2a5a6a';
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.moveTo(0, canvas.height / 2);
            ctx.lineTo(canvas.width, canvas.height / 2);
            ctx.stroke();
        }
        
        function calculateStats(data) {
            if (data.length === 0) return;
            
            let max = Math.max(...data);
            let min = Math.min(...data);
            let sum = data.reduce((a, b) => a + b, 0);
            let avg = sum / data.length;
            
            // Convertir a voltios (12 bits, 3.3V referencia)
            let vMax = (max / 4095) * 3.3;
            let vMin = (min / 4095) * 3.3;
            let vAvg = (avg / 4095) * 3.3;
            let vpp = vMax - vMin;
            
            document.getElementById('vmax').textContent = vMax.toFixed(2) + ' V';
            document.getElementById('vmin').textContent = vMin.toFixed(2) + ' V';
            document.getElementById('vavg').textContent = vAvg.toFixed(2) + ' V';
            document.getElementById('vpp').textContent = vpp.toFixed(2) + ' V';
            
            // Detectar frecuencia por cruces por cero
            let crossings = 0;
            let threshold = avg;
            for (let i = 1; i < data.length; i++) {
                if ((data[i-1] < threshold && data[i] >= threshold) ||
                    (data[i-1] > threshold && data[i] <= threshold)) {
                    crossings++;
                }
            }
            
            let timebase = parseInt(document.getElementById('timebase').value);
            let totalTime = (data.length * timebase) / 1000000; // en segundos
            let freq = (crossings / 2) / totalTime;
            
            document.getElementById('freq').textContent = 
                freq > 1000 ? (freq/1000).toFixed(2) + ' kHz' : freq.toFixed(2) + ' Hz';
        }
        
        function drawWaveform(data) {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            drawGrid();
            
            if (data.length === 0) return;
            
            calculateStats(data);
            
            ctx.strokeStyle = '#00ff88';
            ctx.lineWidth = 2;
            ctx.beginPath();
            
            let xStep = canvas.width / data.length;
            
            for (let i = 0; i < data.length; i++) {
                let x = i * xStep;
                // Invertir Y y aplicar escala
                let y = canvas.height - ((data[i] / 4095) * canvas.height * verticalScale);
                
                if (i === 0) {
                    ctx.moveTo(x, y);
                } else {
                    ctx.lineTo(x, y);
                }
            }
            
            ctx.stroke();
            
            // Dibujar nivel de trigger
            if (triggerEnabled) {
                let triggerLevel = parseInt(document.getElementById('trigger').value);
                let triggerY = canvas.height - ((triggerLevel / 4095) * canvas.height * verticalScale);
                
                ctx.strokeStyle = triggerRising ? '#ff6b6b' : '#4ecdc4';
                ctx.lineWidth = 1;
                ctx.setLineDash([5, 5]);
                ctx.beginPath();
                ctx.moveTo(0, triggerY);
                ctx.lineTo(canvas.width, triggerY);
                ctx.stroke();
                ctx.setLineDash([]);
            }
        }
        
        async function fetchData() {
            try {
                const response = await fetch('/data');
                const data = await response.json();
                drawWaveform(data.samples);
            } catch (error) {
                console.error('Error fetching data:', error);
            }
        }
        
        function startCapture() {
            fetch('/capture');
            setTimeout(fetchData, 100);
            
            if (continuousMode && !captureInterval) {
                captureInterval = setInterval(() => {
                    fetch('/capture');
                    setTimeout(fetchData, 100);
                }, 500);
            }
        }
        
        function toggleMode() {
            continuousMode = !continuousMode;
            document.getElementById('modeText').textContent = 
                'Modo: ' + (continuousMode ? 'Continuo' : 'Único');
            
            if (!continuousMode && captureInterval) {
                clearInterval(captureInterval);
                captureInterval = null;
            }
            
            fetch('/setmode?continuous=' + (continuousMode ? '1' : '0'));
        }
        
        function updateTimebase(value) {
            document.getElementById('timebaseVal').textContent = value + ' µs';
            fetch('/settimebase?value=' + value);
        }
        
        function updateTrigger(value) {
            let voltage = (value / 4095) * 3.3;
            document.getElementById('triggerVal').textContent = voltage.toFixed(2) + ' V';
            fetch('/settrigger?level=' + value);
        }
        
        function toggleTrigger() {
            if (triggerEnabled) {
                triggerRising = !triggerRising;
                document.getElementById('triggerText').textContent = 
                    'Trigger: ON ' + (triggerRising ? '↑' : '↓');
                fetch('/settrigger?rising=' + (triggerRising ? '1' : '0'));
            } else {
                triggerEnabled = true;
                document.getElementById('triggerText').textContent = 'Trigger: ON ↑';
                fetch('/settrigger?enable=1');
            }
        }
        
        function updateScale(value) {
            verticalScale = parseFloat(value);
            document.getElementById('scaleVal').textContent = value + 'x';
            drawWaveform(window.lastData || []);
        }
        
        // Iniciar captura automática
        setTimeout(startCapture, 1000);
    </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleCapture() {
  captureSamples();
  server.send(200, "text/plain", "OK");
}

void handleData() {
  String json = "{\"samples\":[";
  for (int i = 0; i < BUFFER_SIZE; i++) {
    json += String(samples[i]);
    if (i < BUFFER_SIZE - 1) json += ",";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleSetTimebase() {
  if (server.hasArg("value")) {
    timebaseUs = server.arg("value").toInt();
    Serial.print("Timebase: ");
    Serial.print(timebaseUs);
    Serial.println(" µs");
  }
  server.send(200, "text/plain", "OK");
}

void handleSetTrigger() {
  if (server.hasArg("level")) {
    triggerLevel = server.arg("level").toInt();
    Serial.print("Trigger level: ");
    Serial.println(triggerLevel);
  }
  if (server.hasArg("enable")) {
    triggerEnabled = server.arg("enable").toInt() == 1;
    Serial.print("Trigger enabled: ");
    Serial.println(triggerEnabled);
  }
  if (server.hasArg("rising")) {
    triggerRising = server.arg("rising").toInt() == 1;
    Serial.print("Trigger rising: ");
    Serial.println(triggerRising);
  }
  server.send(200, "text/plain", "OK");
}

void handleSetMode() {
  if (server.hasArg("continuous")) {
    continuousMode = server.arg("continuous").toInt() == 1;
    Serial.print("Continuous mode: ");
    Serial.println(continuousMode);
  }
  server.send(200, "text/plain", "OK");
}

// ===== SETUP Y LOOP =====

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nESP32-C3 Oscilloscope v0.4");
  Serial.println("===========================");
  
  // Configurar LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Configurar ADC
  setupADC();
  
  // Conectar a WiFi
  Serial.print("Conectando a WiFi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
  } else {
    Serial.println("\nError conectando WiFi");
    Serial.println("Iniciando AP...");
    WiFi.softAP("ESP32-Oscilloscope", "12345678");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
  }
  
  // Configurar servidor web
  server.on("/", handleRoot);
  server.on("/capture", handleCapture);
  server.on("/data", handleData);
  server.on("/settimebase", handleSetTimebase);
  server.on("/settrigger", handleSetTrigger);
  server.on("/setmode", handleSetMode);
  
  server.begin();
  Serial.println("Servidor web iniciado");
  Serial.println("===========================\n");
}

void loop() {
  server.handleClient();
  
  // Captura automática en modo continuo
  if (continuousMode && !captureComplete) {
    captureSamples();
  }
  
  delay(10);
}

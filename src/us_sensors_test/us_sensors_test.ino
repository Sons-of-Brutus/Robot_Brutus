// Configuración de Pines (ajusta a tus pines)
const int TRIGGER_PIN = 17;
const int ECHO_PIN = 5;

// Variables para la comunicación entre ISR y Tarea
volatile uint32_t start_micros = 0;
volatile uint32_t duration_micros = 0;
SemaphoreHandle_t echoSemaphore;

// --- FUNCIÓN DE INTERRUPCIÓN (ISR) ---
// Se ejecuta en cada cambio de estado del pin ECHO
void IRAM_ATTR echo_isr() {
    if (digitalRead(ECHO_PIN) == HIGH) {
        // El pulso acaba de empezar
        start_micros = micros();
    } else {
        // El pulso acaba de terminar
        duration_micros = micros() - start_micros;
        
        // Cuando salta la interrupción, si estaba en una tarea de menor prioridad que la 
        // de percepción, si es necesario que se ejecute la percepción, lo hará, ya que se
        // lo estamos permitiendo porque el dato de duración ya se ha tomado.
        // Porque le cede ya el semáforo en el que se había bloqueado la task de percepción
        // al no tener el dato.
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(echoSemaphore, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR(); // Cambia de contexto si es necesario
        }
    }
}

// --- TAREA DE PERCEPCIÓN ---
void perception_task(void *pvParameters) {
    echoSemaphore = xSemaphoreCreateBinary();
    
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    // Configure the interruption for any change in ECHO pin (RISING and FALLING)
    attachInterrupt(digitalPinToInterrupt(ECHO_PIN), echo_isr, CHANGE);

    while (true) {
        // Trigger
        digitalWrite(TRIGGER_PIN, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIGGER_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIGGER_PIN, LOW);

        // Wait for ECHO semaphore in the timeout (ej: 30ms)
        if (xSemaphoreTake(echoSemaphore, pdMS_TO_TICKS(30)) == pdTRUE) {
            float distance = duration_micros * 0.034 / 2.0;
            Serial.printf("Distancia: %.2f cm\n", distance);
        } else {
            Serial.println("Error: Timeout de sensor");
        }

        // 3. Frecuencia de la tarea (ej: cada 100ms)
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void setup() {
    Serial.begin(115200);
    
    // Creamos la tarea en el Core 1 para dejar el Core 0 al sistema/WiFi
    xTaskCreatePinnedToCore(
        perception_task,   // Función
        "Perception",      // Nombre
        4096,              // Stack
        NULL,              // Parámetros
        2,                 // Prioridad (media-alta)
        NULL,              // Handle
        1                  // Core
    );
}

void loop() {
    // El loop queda libre para otras cosas o vacío
    vTaskDelete(NULL); 
}
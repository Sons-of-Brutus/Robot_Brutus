# Robot_Brutus
Proyecto final de Mecatronica de 4º de Ingeniería Robótica Software

# QUÉ TENEIS QUE HACER PRIMERO
1. Copiar de la rama `gait` a la rama `software` todos los `.h` del directorio [`src/brutus_esp32`](src/brutus_esp32/), incluso recomiendo copiar también [`brutus_esp32.ino`](src/brutus_esp32/brutus_esp32.ino).
2. **YA EN LA RAMA `software`** tener en cuenta todo lo que he dicho para programar la lógica, comunicaciones, etc.

# GUIA PARA USAR BRUTUS *(para nosotros)*

Como es para nosotros, no voy a añadir ciertos detalles de las funciones y demás porque eso ya es más para el usuario final y esto quiero que sea para programar bien la lógica.

## Inicialización
Hay que hacer lo que está en [`brutus_esp32.ino`](src/brutus_esp32/brutus_esp32.ino) de la rama `gait`.

**Antes del setup() de `brutus_esp32.ino`.**
1. Inicializar la PCA y llamar al constructor del objeto de tipo Brutus.
```cpp
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();
Brutus brutus;
```
**En el setup() del `brutus_esp32.ino`.**
2. Llamar al setup de brutus.
Los valores de las constantes están en [`brutus_params.h`](src/brutus_esp32/brutus_params.h)
```cpp
brutus.setup(&pca,
             PCA9685_OE,
             R_PIN,
             B_PIN,
             G_PIN,
             PWM_SERVO_FREQ);
```

3. Llamar a los setups de las patas.
```cpp
brutus.setup_front_right_leg(...);

brutus.setup_front_left_leg(...);

brutus.setup_back_right_leg(...);

brutus.setup_back_left_leg(...);
```

4. Llamar a `brutus.start()`.

5. Crear la task de movimiento.
```cpp
brutus.create_motion_task(DEFAULT_MOTION_PERIOD, MOTION_CORE);
```
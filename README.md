# Robot_Brutus
Proyecto final de Mecatronica de 4º de Ingeniería Robótica Software

# QUÉ TENEIS QUE HACER PRIMERO
1. Copiar de la rama `gait` a la rama `software` todos los `.h` del directorio [`src/brutus_esp32`](src/brutus_esp32/), incluso recomiendo copiar también [`brutus_esp32.ino`](src/brutus_esp32/brutus_esp32.ino).
2. **YA EN LA RAMA `software`** tener en cuenta todo lo que he dicho para programar la lógica, comunicaciones, etc.

# GUIA PARA USAR BRUTUS *(para nosotros)*

Como es para nosotros, no voy a añadir ciertos detalles de las funciones y demás porque eso ya es más para el usuario final y esto quiero que sea para programar bien la lógica.

## Inicialización en `brutus_esp32.ino`
Hay que hacer lo que está en [`brutus_esp32.ino`](src/brutus_esp32/brutus_esp32.ino) de la rama `gait.

1. [**Antes del setup() de `brutus_esp32.ino`**] Inicializar la PCA y llamar al constructor del objeto de tipo Brutus.
```cpp
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();
Brutus brutus;
```

2. [**En el setup() del `brutus_esp32.ino`**] Llamar al setup de brutus.
Los valores de las constantes están en [`brutus_params.h`](src/brutus_esp32/brutus_params.h)
```cpp
brutus.setup(&pca,
             PCA9685_OE,
             R_PIN,
             B_PIN,
             G_PIN,
             PWM_SERVO_FREQ);
```

3. [**En el setup() del `brutus_esp32.ino`**] Llamar a los setups de las patas.
```cpp
brutus.setup_front_right_leg(...);

brutus.setup_front_left_leg(...);

brutus.setup_back_right_leg(...);

brutus.setup_back_left_leg(...);
```

4. [**En el setup() del `brutus_esp32.ino`**] Llamar al setup de la percepción.
```cpp
brutus.setup_perception(US_R_TRIG,
                        US_R_ECHO,
                        US_L_TRIG,
                        US_L_ECHO,
                        US_F_TRIG,
                        US_F_ECHO);
```

4. [**En el setup() del `brutus_esp32.ino`**] Llamar a `brutus.start()`.

5. Configuración de comunicaciones, tasks de lógica, etc.

6. [**En el setup() del `brutus_esp32.ino`**] Crear la task de movimiento.
```cpp
brutus.create_motion_task(DEFAULT_MOTION_PERIOD, MOTION_CORE);
```

## Constantes que debeis tener en cuenta ([`brutus_params.h`](src/brutus_esp32/brutus_params.h))

**PINES DE LOS ULTRASONIDOS**

Aquí hay un TODO que indica que todavía hay que asignar cada par de pines TRIGGER y ECHO a *frente*, *izquierda* y *derecha*, que todavía están sin asignar.

```cpp
#define US_1_TRIG 15
#define US_1_ECHO 2

#define US_2_TRIG 4
#define US_2_ECHO 16

#define US_3_TRIG 17
#define US_3_ECHO 5

#define US_R_TRIG 15
#define US_R_ECHO 15

#define US_L_TRIG 15
#define US_L_ECHO 15

#define US_F_TRIG 15
#define US_F_ECHO 15
```

La cosa es que ASIGNEIS los valores que hay en 1,2 y 3 en las constantes que tienen R,L y F,y después quiteis las constantes con 1, 2 y 3 (según sea cada uno, no sabemos a cual corresponde ni 1, ni 2, ni 3).

**CORES**

+ `MOTION_CORE`: core que utiliza la task de locomoción de Brutus.
+ `LOGIN_CORE`: core que utilizan todas las tasks que no son de locomoción de Brutus.

**DEFAULT_MOTION_PERIOD**

Por si quereis intentar que se mueva más rapido, la **task de locomoción** se ejecuta cada `DEFAULT_MOTION_PERIOD` milisegundos.

*OJO: No es la frecuecia de trabajo, si `DEFAULT_MOTION_PERIOD` disminuye, la task de locomoción se ejecuta más rápido.*

**Los SPINS**
Arreglad los pasos `SPIN_1`, `SPIN_2`, `SPIN_3`, `SPIN_4`, `SPIN_5` y `SPIN_6` para que gire bien sobre sí mismo.

## COSAS DE LA CLASE `Brutus` PARA LA LÓGICA QUE QUEREMOS HACER
+ **ESTABLECER LA VELOCIDAD LINEAL/ANGULAR:**
  - Velocidad lineal: `set_linear_speed_ts`
  - Velocidad angular: `set_angular_speed_ts`
+ **LEER LA VELOCIDAD LINEAL/ANGULAR:**
  - Velocidad lineal: `get_linear_speed_ts`
  - Velocidad angular: `get_angular_speed_ts`
+ **OBTENER LA POSE ACTUAL DEL ROBOT:**
  - `check_pose(true)`. El true es para que os devuelva los ángulos con la inversión aplicada, ponedlo.
+ **ESTABLECER UNA POSE:**
  - El método que TIENE QUE USAR la LÓGICA: `change_target_pose`
    * Esto es porque `change_target_pose` actualiza un atributo `target_pose_` que es el que utiliza la task de locomoción para moverse.
  - !!!!!!!!!!! **NO SE DEBE USAR `set_pose` MIENTRAS SE EJECUTA LA TASK DE LOCOMOCIÓN** !!!!!!!!!!!
+ **MODO DE MOVIMIENTO:**
  - El **modo de movimiento** es el que utiliza la task de locomoción. (NO TIENE NADA QUE VER CON LOS MODOS DE LA LÓGICA)
  - `enum BrutusMotionControlMode`:
    * `POSE_CONTROL = 0`: Control de Brutus por pose (Usar `change_target_pose`).
    * `SPEED_CONTROL = 1`: Control de Brutus por velocidad (Usar los métodos para establecer velocidades).
  - *Métodos del modo de movimiento:*
    * `set_motion_control_mode`
    * `get_motion_control_mode`
#ifndef BRUTUS_COMMS__H
#define BRUTUS_COMMS__H

#include "Brutus.h"
#include "brutus_params.h"

/*
TODO

Leeros esto antes de tirar codigo, porfis. Es todo lo que hablamos ayer en clase, necesidades por concurrencias, necesidades
por la clase Brutus y alguna idea que se me ha ocurrido.

BrutusComms.h:

  - Struct BrutusCommsCmd:
    · [int] mode: Modo de funcionamiento (aquí es un entero, en brutus_esp32.ino se parsea en un enum,
                                          pero ese enum estaría genial que estuviera abstraido de esta clase,
                                          ya que ese enum contiene los estados de la lógica principal)
    · [float] v: Velocidad lineal.
    · [float] w: Velocidad angular.
    · [BrutusPose] pose: La pose que queremos que le llegue al robot.

  - Struct BrutusCommsData:
    · [BrutusPose] pose: La pose que el robot envía al servidor.
    · [float] front_us: Distancia detectada por el ultrasonidos frontal.
    · [float] left_us: Distancia detectada por el ultrasonidos derecho.
    · [float] right_us: Distancia detectada por el ultrasonidos izquierdo.

  - Clase BrutusComms (Esto es lo necesario, que hablamos ayer, que no tiene que ver con MQTT, que eso ya es cosa vuestra):
    · Atributos que sea un puntero a un objeto de clase Brutus:
      - Puntero a un objeto de clase Brutus.
      - Un struct BrutusCommsCmd.
      - Un struct BrutusCommsData.
      (Esto de separar BrutusCommsCmd y BrutusCommsData es un poco una
       idea que os doy para separar lo que llegue de los topics brutus/cmd y lo
       que se envíe a los topics brutus/data. Si veis que es más util abordarlo
       de otra forma, adelante con ello. Pero ***SÍ ES NECESARIO*** algo para guardar
       lo que os llegue por los topics brutus/cmd)

    · Aquí también necesitais hacer lo del constructor vacío y una función setup, como en la clase Brutus, por el puntero a la clase Brutus.
    · Función de creación de la task [argumentos: core, prioridad, periodo de trabajo].
    · Función que ejecuta la propia task. (Os recomiendo inspiraros en lo que he hecho del wrapper
                                           con la motion task en la clase brutus)
    · El resto de cosas las veis vosotros según vuestras necesidades de software.

    ! La task utiliza las funciones thread-safe del atributo puntero de tipo Brutus.
    ! Los callbacks de MQTT ***NO PUEDEN*** utilizar las funciones thread-safe de la clase Brutus, para eso está la struct de BrutusCommsCmd.

Cosas a tener en cuenta en brutus_esp32.ino:
  - Llamar al setup de la global de BrutusComms, como mínimo, después de brutus.start()
  - El core que utilice la task del BrutusComms debe ser la constante LOGIC_CORE (que está definida en brutus_params.h).
  - La prioridad de la task intentad que sea la mínima (0).
  - El enum BrutusMotionMode no es lo de standing, ex1, ex2,...  Eso lo tendremos en cuenta cuando programemos la lógica.
    Ahora, con que llegue el numero del modo por MQTT basta (que ese número si es lo de standing, ex1, ex2,...).
    Al enum de la lógica lo voy a llamar en este comentario LogicMode, por decir algo.

Cosas importantes de la clase Brutus:
  - Métodos thread-safe:
    · [void] set_linear_speed_ts(float v)
    · [float] get_linear_speed_ts()
    · [void] set_angular_speed_ts(float w)
    · [float] get_angular_speed()
    · [void] change_target_pose(BrutusPose pose): Este no creo que os vaya a hacer falta para algo referente a las comunicaciones.
                                                  Pero, lo que si que quiero que tengáis en cuenta, es que está función solo
                                                  puede tener efecto directo si el LogicMode es el de controlar las piernas. De
                                                  todos modos, os lo explicaré mejor cuando nos pongamos con la lógica.
    · [void] change_motion_mode(enum BrutusMotionMode mode): Este no creo que os vaya a hacer falta para algo referente a las
                                                             comunicaciones.

  - La tarea de locomoción no está hecha todavía, la haré cuando pueda y tenga el robot. Ahora mismo, lo unico que hace es poner la pose target_pose_.
    Así que al menos podéis probar las barras deslizadoras para mover los brazos utilizando el método change_target_pose.
  
  - Lo otro que tenía que decir se me ha olvidado así que os leeis el código muertos.


A meterle bien duro chavales.

*/

#endif // BRUTUS_COMMS__H
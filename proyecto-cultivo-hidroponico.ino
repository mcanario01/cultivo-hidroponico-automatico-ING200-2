//INCLUSIÓN DE LIBRERÍAS
#include <ezButton.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DHT.h>
#include <DHT_U.h>

#define CANTIDAD_DE_PANTALLAS 3 //define cantidad de pantallas de la interfaz de la pantalla LCD

#define DELAY_PANTALLA 1000 //define el tiempo de refresco de la pantalla en mili hercios
#define LIMITE_INFERIOR_SENSOR_LUZ 0 //define el rango minimo de lectura del sensor de luz
#define LIMITE_SUPERIOR_SENSOR_LUZ 1023 //define el rango maximo de lectura del sensor de luz
#define LIMITE_INFERIOR_SENSOR_NIVEL_AGUA 0 //define el rango minimo de lectura del sensor de nivel de agua
#define LIMITE_SUPERIOR_SENSOR_NIVEL_AGUA 1023 //define el rango maximo de lectura del sensor de nivel de agua

//PANTALLA
#define DIRECCION_LCD 0x27 //define la dirección de memoria de la interfaz I2C

//PINES BOTON Y LED
#define BOTON2 9 //define el pin del boton de interfaz
#define BOTON1 4 //define el pin de interacción
#define LED1 6 //define el pin de los LED

//MODULO TEMPERATURA Y HUMEDAD
#define DHTPIN 7 //define el pin del sensor de temperatura y humedad
#define DHTTYPE DHT11 //define el tipo de sensor de temperatura y humedad

//PINES SENSOR NIVEL DEL AGUA
#define NIVEL_AGUA A0 //define el pin analógico del sensor de nivel de agua

//PINES SENSOR DE LUZ
#define NIVEL_LUZ A1 //define el pin analógico de la fotoresistencia

//BOMBA DE AGUA
#define BOMBA_AGUA 3 //define el pin de la bomba de agua

//BOCINA
#define BOCINA 5 //define el pin del buzzer

//OBJETOS DE PANTALLA Y SENSOR TH
DHT sensor_tyh(DHTPIN, DHTTYPE);
LiquidCrystal_I2C pantalla(DIRECCION_LCD, 16, 2);
ezButton boton(BOTON1);
ezButton boton_interfaz(BOTON2);

//CARACTERES PERSONALIZADOS PANTALLA

byte icono_temperatura[8] = {
  B01000,
  B10101,
  B10100,
  B10101,
  B11100,
  B11101,
  B11100,
  B01000,
};

byte icono_alerta[8] = {
  B01110,
  B01110,
  B01110,
  B01110,
  B01110,
  B00000,
  B01110,
  B01110,
};

byte icono_humedad[8] = {
  B00100,
  B00100,
  B01010,
  B01010,
  B10001,
  B10001,
  B10001,
  B01110,
};

byte icono_luz_encendida[8] = {
  B11111,
  B10001,
  B01110,
  B01110,
  B01110,
  B10001,
  B10001,
  B11111,
};

byte icono_luz_apagada[8] = {
  B00000,
  B01110,
  B10001,
  B10001,
  B10001,
  B01110,
  B01110,
  B00000,
};

byte icono_nivel_agua[8] = {
  B00000,
  B10001,
  B10000,
  B11111,
  B11110,
  B11111,
  B11110,
  B01111,
};

byte icono_nivel_luz[8] = {
  B00100,
  B10101,
  B01110,
  B11011,
  B01110,
  B10101,
  B00100,
  B00000,
};

bool estadoLed = 0; //controla el estado de los LED (despreciado)

unsigned long millis_pasados_pantalla = 0; //controla la variable de la diferencia de tiempo

int pantalla_interfaz_actual = 1; //controla la variable de la pantalla actual

unsigned int intensidad_de_LEDs = 511; //controla la intensidad de los LED

unsigned int intensidad_de_bomba = 511; //controla la intensidad de la bomba

void setup() {

  Serial.begin(9600); //inicialización de comunicación serial

  //INICIALIZACIÓN SENSOR TH
  sensor_tyh.begin();  //inicia objeto dht

  //INICIALIZACIÓN PANTALLA
  pantalla.init();       //inicia objeto pantalla
  pantalla.backlight();  //prende iluminación pantalla

  //INICIALIZACIÓN BOTON, LED, BOMBA y BUZZER
  pinMode(BOMBA_AGUA, OUTPUT);         //declara bomba de agua
  pinMode(LED1, OUTPUT);               //declara pin de LEDs
  pinMode(BOCINA, OUTPUT);             //declara pin de BUZZER
  boton.setDebounceTime(50);           //declara tiempo de refresco de boton
  boton_interfaz.setDebounceTime(50);  //declara el tiempo de refresco de boton_interfaz

  //INICIALIZACIÓN DE CARACTERES PERSONALIZADOS
  pantalla.createChar(0, icono_alerta);
  pantalla.createChar(1, icono_temperatura);
  pantalla.createChar(2, icono_humedad);
  pantalla.createChar(3, icono_luz_encendida);
  pantalla.createChar(4, icono_luz_apagada);
  pantalla.createChar(5, icono_nivel_agua);
  pantalla.createChar(6, icono_nivel_luz);

  //ANIMACIÓN DE INICIO
  pantalla.setCursor(4, 0);
  pantalla.print("Proyecto");
  pantalla.setCursor(3, 1);
  pantalla.print("H.I.D.R.O.");
  delay(2000);
  pantalla.clear();
  pantalla.setCursor(3, 0);
  pantalla.print("Taller de");
  pantalla.setCursor(2, 1);
  pantalla.print("Diseno UFRO.");
  delay(2000);
  pantalla.clear();
  pantalla.setCursor(0, 0);
  pantalla.print("Autores: MC, JV,");
  pantalla.setCursor(0, 1);
  pantalla.print("GA, SQ, DC.");
  delay(2000);
  pantalla.clear();
}

void loop() {
  //BOTON Y ESCRITURA DE LED ------------------------------
  boton.loop();           //actualización del valor del boton
  boton_interfaz.loop();  //actualización del valor del boton_interfaz

  analogWrite(LED1, intensidad_de_LEDs);  //escribe el estado del LED

  analogWrite(BOMBA_AGUA, intensidad_de_bomba);  //escribe el estado de la bomba de agua

  if (boton_interfaz.isPressed()) {
    pantalla_interfaz_actual++;
    if (pantalla_interfaz_actual > CANTIDAD_DE_PANTALLAS) {
      pantalla_interfaz_actual = 1;
    }
  }

  //ESCRUTURA EN PANTALLA ---------------------------------
  //pantalla.clear();

  switch (pantalla_interfaz_actual) {

    //Pantalla 1 - Vista general de datos
    case 1:
      {
        pantalla.setCursor(15, 0);  //escritura de icono de LEDs
        if (estadoLed) {
          pantalla.print((char)3);  //escritura de icono_luz_encendida
        } else {
          pantalla.print((char)4);  //escritura de icono_luz_apagada
        }
        pantalla.setCursor(15, 1);
        pantalla.print((char)0);  //icono_alerta
        //Actualización de pantalla con diferencial del tiempo. (Para leer mejor los valores)
        unsigned long millis_actuales_pantalla = millis();
        if (millis_actuales_pantalla - millis_pasados_pantalla >= DELAY_PANTALLA) {
          millis_pasados_pantalla = millis_actuales_pantalla;
          pantalla.setCursor(0, 0);
          pantalla.print((char)1);  //escritura de icono_temperatura
          pantalla.setCursor(1, 0);
          pantalla.print((int)sensor_tyh.readTemperature());  //escritura de temperatura
          pantalla.print("  ");


          pantalla.setCursor(0, 1);
          pantalla.print((char)2);  //escritura de icono_humedad
          pantalla.setCursor(1, 1);
          pantalla.print((int)sensor_tyh.readHumidity());  //escritura de humedad
          pantalla.print("  ");

          pantalla.setCursor(5, 0);
          pantalla.print((char)5);  //escritura de icono_nivel_agua
          pantalla.setCursor(6, 0);
          pantalla.print(map(analogRead(NIVEL_AGUA), LIMITE_INFERIOR_SENSOR_NIVEL_AGUA, LIMITE_SUPERIOR_SENSOR_NIVEL_AGUA, 0, 99));  //escritura de nivel de agua
          pantalla.print("  ");

          pantalla.setCursor(5, 1);
          pantalla.print((char)6);  //escritura de icono_nivel_luz
          pantalla.setCursor(6, 1);
          pantalla.print(map(analogRead(NIVEL_LUZ), LIMITE_INFERIOR_SENSOR_LUZ, LIMITE_SUPERIOR_SENSOR_LUZ, 0, 99));  //nivel de luz
          pantalla.print("  ");


          //Impresión por monitor serial de valores de pruebas
          Serial.print(map(analogRead(NIVEL_LUZ), LIMITE_INFERIOR_SENSOR_LUZ, LIMITE_SUPERIOR_SENSOR_LUZ, 0, 99));
          Serial.print(" - ");
          Serial.println(-(1 / sqrt(map(analogRead(NIVEL_LUZ), LIMITE_INFERIOR_SENSOR_LUZ, LIMITE_SUPERIOR_SENSOR_LUZ, 0, 99))) + 99);
        }
      }

    //Pantalla 2 - Ajuste de LEDS
    case 2:
      {
        pantalla.setCursor(0, 0);
        pantalla.print("Ajuste de LEDs");
        pantalla.setCursor(0, 0);

        if (intensidad_de_LEDs != 0) {
          pantalla.print((char)3);  //escritura de icono_luz_encendida
        } else {
          pantalla.print((char)4);  //escritura de icono_luz_apagada
        }
        pantalla.setCursor(3, 1);
        pantalla.print(map(intensidad_de_LEDs, 0, 1023, 0, 10));

        if (boton.isPressed())  //pregunta si el boton fué presionado
        {
          intensidad_de_LEDs = intensidad_de_LEDs + 102;
          if (intensidad_de_LEDs > 1023) {
            intensidad_de_LEDs = 0;
          }
        }
      }

    case 3:
      {
        pantalla.setCursor(0, 0);
        pantalla.print("Ajuste de Bomba");
        pantalla.setCursor(0, 0);
        pantalla.setCursor(3, 1);
        pantalla.print(map(intensidad_de_bomba, 0, 1023, 0, 10));

        if (boton.isPressed())  //pregunta si el boton fué presionado
        {
          intensidad_de_bomba = intensidad_de_bomba + 102;
          if (intensidad_de_bomba > 1023) {
            intensidad_de_bomba = 0;
          }
        }
      }
  }
}

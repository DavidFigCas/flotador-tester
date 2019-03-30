/*
El mecanismo lleva un ACME 8mm, un motor de 200 pasos por vuelta y un driver de 16 micropasos
Esto nos da una cuenta de 0.0025 mm por paso

Utiliza un relevador conectado en serie a la alimentación de 5V que cierra un circuito
si la bobina del motor a probar tiene continuidad, el circuito enciende el LED de un optoacoplador
4N25, que está configurado en Emisor común y una resistencia de PullUp para indicar
cuando hay o no continuidad, pasando la prueba el releavor se desactiva


*/

#include <Arduino.h>

#define   paso    2
#define   dir     3
#define   fin     4
#define   bomba   5
#define   rele    6
#define   pinADC  A0
#define   pres    0
#define   arriba  0
#define   abajo   1
#define   lim_sup   30000
#define   lim_inf   -10000
#define   abierto   1
#define   cerrado   0
#define   pas_mm    0.0025

bool sen = abajo;
char letra;
String argumento = "00000";
int adc;
int ohm;
float pos;
int pas;
int aux = 0;
int con = 0;
int pulso = 100;


//----------------------------------------------------------- continuidad
void continuidad ()
{
//Se activa el relevador
  digitalWrite(rele,LOW);
  delay(100);
  con = digitalRead(bomba);
  digitalWrite(rele,HIGH);
  //Se desactiva el releavor

  if(con == cerrado) // Hay continuidad
  {
    Serial.println("Continuidad: OK");
  }
  else              //no hay continuidad
  {
    Serial.println("Continuidad: NO OK!!!");
  }

}

//----------------------------------------------------------- leer
void leer()
{
  adc = analogRead(pinADC);             // Lee el canal analógico para determinar la resisitencia del flotador
  ohm = map(adc,0,1023,300,0);

  Serial.print("ADC: ");
  Serial.print(adc);
  Serial.print("\t");
  Serial.print("OHM: ");
  Serial.print(ohm);
  Serial.print("\t");
  Serial.print("POS: ");
  Serial.println(pos);

}

//------------------------------------------------------------mover
void mover(float dist)                  // Dist es el argumento de desplazamiento en incremental
{
  aux = 0;                              // Esta variable se usa para evitar que se salga de los limites de movimiento
                                        // Seleciono y escribo la direccion al motor
  if(dist > 0)                          // Si la distancia es positiva
    sen = arriba;                       // apunto el motor hacia arriba
  else                                  // Si la distancia es negativa
    sen = abajo;                        // apunto el motor hacia abajo

  digitalWrite(dir,sen);                // Escribo el pin de direccion con el sentido

                                        // Envio los pulsos para conseguir la dist
  for(int i = 0;(i<abs(dist) && (aux == 0)) ;i++)
  {
    if(sen == arriba)                   // Si el sentido apunta arriba
    {
      pos = pos + pas_mm;               // Se incrementa la posicion

      if(pos > lim_sup)                 // Si se supera el limite superior, cambia aux
      {
        aux = 1;                        // Aux indica que se debe salir del ciclo "for"
      }
    }
    else                                // Si el sentido apunta hacia abajo
    {
      pos = pos - pas_mm;
      if((pos < lim_inf) || (digitalRead(fin) == pres)) // Si supero el limite inferior o detecto el fin
      {
        aux = 1;                        // Aux indica que debo salir del ciclo "for"
      }
    }


    if(aux == 0)                        // Si no estoy en algun limite, puedo dar pasos
    {
      digitalWrite(paso,1);             // Envio el pulso
      delayMicroseconds(pulso);         // El retardo está controlado por un tiempo en uS
      digitalWrite(paso,0);             // Termino el pulso
      delayMicroseconds(pulso);
      //Serial.print(aux);
    }
    //leer();
  }

}

//------------------------------------------------------------inicializar
void inicializar()                      // Inicializa la posición en Home
{
  sen = abajo;                          // Apunto el sentido hacia abajo que es donde está el sensor
  digitalWrite(dir,sen);                // Escribo la dirección en sentido abajo
  Serial.println("Buscando home...");   // Envío la indicación de busqueda

  while(digitalRead(fin) != pres)       // Mientras el botón de fin no esté presionado
  {                                     // Debería existir un tiempo límite para terminar la busqueda
    //Serial.println(digitalRead(fin));
    digitalWrite(paso,1);               // Envío el pulsos
    delayMicroseconds(pulso);
    digitalWrite(paso,0);
    delayMicroseconds(pulso);           // Termino el pulso
    //Serial.println("+");              // Repito los pulsos-pasos hasta tocar home
  }
  Serial.println("Home");               // Home encontrada
  pos = lim_inf;                        // La posicion toma el limite inferior
}

//--------------------------------------------------------------------- Setup
void setup()
{
  Serial.begin(9600);
  Serial.println("Iniciando...");

  pinMode(paso,OUTPUT);
  pinMode(dir,OUTPUT);
  pinMode(fin,INPUT_PULLUP);
  pinMode(bomba,INPUT_PULLUP);
  pinMode(rele,OUTPUT);
  digitalWrite(rele,HIGH);   //rele apagado
  digitalWrite(dir,sen);

  continuidad();
  //inicializar();
  //leer();
}


//------------------------------------------------------------- loop
void loop()
{
  //leer();
  //delay(1000);
  if(Serial.available())
  {
    letra = Serial.read();

    switch(letra)
    {
      case '#':
        argumento = Serial.readStringUntil("OK");
        mover(argumento.toFloat());
        leer();
      break;

      case '$':
        inicializar();
        leer();
      break;

    }

  }

}

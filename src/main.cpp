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
#define   lim_sup   200
#define   lim_inf   -100
#define   abierto   1
#define   cerrado   0
#define   pas_mm    0.0025

bool sen = abajo;
char letra;
String argumento = "000000000";
int adc;
int ohm;
double pos = 0;
double test_vacio = lim_inf;
double test_reserva = lim_inf;
double test_uncuarto = lim_inf;
double test_medio = lim_inf;
double test_trescuarto = lim_inf;
double test_lleno = lim_inf;
double test_fin = lim_sup;
double test_inicio = lim_inf;
double test_escalon = 1.0;            // Milimetros de separación entre cada medición
int aux = 0;
int con = 0;
int pulso = 100;
int j; //auxiliar para pasos



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
    Serial.println("Continuidad: NO CONTINUIDAD!!!");
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
  Serial.println(pos,2);

}


//----------------------------------------------------------- comandos
void comando_pausa()
{
/*  if(Serial.available())
  {
    letra = Serial.read();

    switch(letra)
    {
      case '$':
        argumento = Serial.readStringUntil("OK");
        if(argumento == "X")
          aux = 1;
      break;

    }

  }*/
}


//------------------------------------------------------------mover
void mover(double dist)                 // Dist es el argumento de desplazamiento en incremental
{
  double avance = dist / pas_mm;        // avance suirve para calcular los pasos en funcion de los pasos por mm
  //Serial.println(avance);
  aux = 0;                              // Esta variable se usa para evitar que se salga de los limites de movimiento
                                        // Seleciono y escribo la direccion al motor
  if(dist > 0)                          // Si la distancia es positiva
    sen = arriba;                       // apunto el motor hacia arriba
  else                                  // Si la distancia es negativa
    sen = abajo;                        // apunto el motor hacia abajo

  digitalWrite(dir,sen);                // Escribo el pin de direccion con el sentido

                                        // Envio los pulsos para conseguir la dist
  for(double i = 0;(i<abs(avance) && (aux == 0)) ;i++) // La condicion de movimiento está en funcion de los pasos
  {

    if((pos > lim_sup) && (sen == arriba))
      aux = 1;                        // Aux indica que se debe salir del ciclo "for"
    else if(((pos < lim_inf) || (digitalRead(fin) == pres)) && sen == abajo)                // Si se supera el limite superior, cambia aux
      aux = 1;


    if(aux == 0)                        // Si no estoy en algun limite, puedo dar pasos
    {
      digitalWrite(paso,1);             // Envio el pulso
      delayMicroseconds(pulso);         // El retardo está controlado por un tiempo en uS
      digitalWrite(paso,0);             // Termino el pulso
      delayMicroseconds(pulso);
      if(sen == arriba)
        pos = pos + pas_mm;
      else
        pos = pos - pas_mm;
    }

  //  if( (j%(test_escalon * 400) == 0 )) // Cada 40 pasos - 0.1mm, test_escalon marca la separación entre las lecturas
  //  {
      //Serial.println(pos,4);
  //    leer();
  //    j=0;
  //  }
  //  j++;
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
    //leer_comando();
  }
  Serial.println("Home");               // Home encontrada
  pos = lim_inf;                        // La posicion toma el limite inferior
}

//---------------------------------------------------------- test
void test(double tt_inicio, double tt_escalon, double tt_fin)
{
    // Calcular el desplazamiento necesario para llegar desde la pos actual al test_inicio
    // inicializar(); // Se asegura que se está en la posición correcta
    Serial.println("Buscando limite inferior...");
    mover(tt_inicio - pos);
    Serial.println("Iniciando prueba");
    leer();
    while(pos < tt_fin)
    {
      mover(tt_escalon);
      // delay(1);        // Retardo para asegurar lecturas o sacar promedios
      leer();
    }
}

//---------------------------------------------------------- test 2 lecturas definidas por usuario (vacio, 1/4, medio...)
void test2(double tt_test_vacio, double tt_test_reserva, double tt_test_uncuarto, double tt_test_medio, double tt_test_trescuarto, double tt_test_lleno, double tt_test_fin)
{
    // Calcular el desplazamiento necesario para llegar desde la pos actual al test_inicio
    // inicializar(); // Se asegura que se está en la posición correcta
    Serial.println("Buscando posicion en vacio...");
    mover(tt_test_vacio - pos);
    Serial.println("Posicion en vacio");
    leer();
    mover(tt_test_reserva - pos);
    Serial.println("Posicion en reserva");
    leer();
    mover(tt_test_uncuarto - pos);
    Serial.println("Posicion en 1/4");
    leer();
    mover(tt_test_medio - pos);
    Serial.println("Posicion en 1/2");
    leer();
    mover(tt_test_trescuarto - pos);
    Serial.println("Posicion en 3/4");
    leer();
    mover(tt_test_lleno - pos);
    Serial.println("Posicion en Lleno");
    leer();
    mover(tt_test_fin - pos);
    Serial.println("Posicion en Full");
    leer();
}


//--------------------------------------------------------------------- Setup
void setup()
{
  Serial.begin(115200);
  Serial.println("Iniciando...");

  pinMode(paso,OUTPUT);
  pinMode(dir,OUTPUT);
  pinMode(fin,INPUT_PULLUP);
  pinMode(bomba,INPUT_PULLUP);
  pinMode(rele,OUTPUT);
  digitalWrite(rele,HIGH);   //rele apagado
  digitalWrite(dir,sen);

  //continuidad();
  //inicializar();
  //leer();
}


//------------------------------------------------------------- loop
void loop()
{

  if(Serial.available())                        // Revisa si hay un dato disponible por el perto Serial
  {
    letra = Serial.read();                      // Captura una letra inicial para el tipo de comandos a ejecurtar

    switch(letra)
    {

      case '#':                                     // Realiza un movimiento
        argumento = Serial.readStringUntil("OK");
        mover(argumento.toFloat());
        leer();
      break;

      case '$':                                      // Argumentos de lecturas
        argumento = Serial.readStringUntil("OK");
        if(argumento == "H")                         // Mandar a Home reiniciar lecturas de posicion
          inicializar();
          //leer();

        if(argumento == "C")                         // Leer CONTINUIDAD
          continuidad();
          //leer();
        if(argumento == "L")                         // Leer OHM + POS
          leer();

        if(argumento == "T")
          test(test_inicio, test_escalon, test_fin);

        if(argumento == "P")
          test2(test_vacio, test_reserva, test_uncuarto, test_medio, test_trescuarto, test_lleno, test_fin);

      break;

          case '%':                                  // Realiza el TEST con sus parametros (inicio,escalon,fin)
        argumento = Serial.readStringUntil("OK");
        test_inicio = argumento.toFloat();
        Serial.println(test_inicio);
      break;

      case '&':
        argumento = Serial.readStringUntil("OK");
        test_escalon = argumento.toFloat();
        Serial.println(test_escalon);
      break;

      case '/':
        argumento = Serial.readStringUntil("OK");
        test_fin = argumento.toFloat();
        Serial.println(test_fin);
      break;

      case 'X':                                     // Introduce valor en vacio
        argumento = Serial.readStringUntil("OK");
        test_vacio = argumento.toFloat();
        Serial.println(test_vacio);
      break;

      case 'Y':                                     // Introduce valor de reserva
        argumento = Serial.readStringUntil("OK");
        test_reserva = argumento.toFloat();
        Serial.println(test_reserva);
      break;

      case 'Z':                                     // Introduce valor en 1/4
        argumento = Serial.readStringUntil("OK");
        test_uncuarto = argumento.toFloat();
        Serial.println(test_uncuarto);
      break;

      case 'J':                                     // Introduce valor medio
        argumento = Serial.readStringUntil("OK");
        test_medio = argumento.toFloat();
        Serial.println(test_medio);
      break;

      case 'K':                                     // Intoducinf 3/4 del tanque
        argumento = Serial.readStringUntil("OK");
        test_trescuarto = argumento.toFloat();
        Serial.println(test_trescuarto);

        case 'M':                                    // Intoducinf Full del tanque
          argumento = Serial.readStringUntil("OK");
          test_lleno = argumento.toFloat();
          Serial.println(test_lleno);

      break;
    }

  }
}

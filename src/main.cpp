#include <Arduino.h>

#define   paso    2
#define   dir     3
#define   fin     4
#define   bomba   5
#define   rele    6
#define   pinADC  A0
#define   pres    1
#define   arriba  0
#define   abajo   1
#define   lim_sup   2900
#define   lim_inf   -99
#define   abierto   1
#define   cerrado   0
//1000 pasos = 6.98mm => 0.00698mm por pasos

bool sen = abajo;
char letra;
String argumento = "00000";
int adc;
int ohm;
int pos;
int aux = 0;
int con = 0;


//----------------------------------------------------------- continuidad
void continuidad ()
{
  digitalWrite(rele,LOW);
  delay(100);
  con = digitalRead(bomba);
  digitalWrite(rele,HIGH);
  if(con == cerrado)
  {
    Serial.println("Continuidad: OK");
  }
  else
  {
    Serial.println("Continuidad: NO OK!!!");
  }

}

//----------------------------------------------------------- leer
void leer()
{
  adc = analogRead(pinADC);
  ohm = map(adc,0,1023,300,0);

  Serial.print("ADC:\t");
  Serial.print(adc);
  Serial.print("\t");
  Serial.print("OHM:\t");
  Serial.print(ohm);
  Serial.print("\t");
  Serial.print("POS:\t");
  Serial.println(pos);

}

//------------------------------------------------------------mover
void mover(int dist)
{
  aux = 0;
  if(dist > 0)
    sen = arriba;
  else
    sen = abajo;

  digitalWrite(dir,sen);

  for(int i = 0;(i<abs(dist) && (aux == 0)) ;i++)
  {
    if(sen == arriba)
    {
      pos++;
      if(pos > lim_sup)
      {
        aux = 1;
      }
    }
    else
    {
      pos--;
      if((pos < lim_inf) || (digitalRead(fin) != pres))
      {
        aux = 1;
      }
    }
    digitalWrite(paso,1);
    delay(1);
    digitalWrite(paso,0);
    delay(1);
    //Serial.print(aux);
  }

}

//------------------------------------------------------------inicializar
void inicializar()
{
  sen = abajo;
  digitalWrite(dir,sen);
  Serial.println("Buscando home...");
  while(digitalRead(fin) != pres)
  {
    //Serial.println(digitalRead(fin));
    digitalWrite(paso,1);
    delay(1);
    digitalWrite(paso,0);
    delay(1);
    //Serial.println("+");
  }
  Serial.println("Home");
  pos = -100;
}

//---------------------------------------------------------------------

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
        mover(argumento.toInt());
        leer();
      break;

      case '$':
        inicializar();
        leer();
      break;

    }

  }

}

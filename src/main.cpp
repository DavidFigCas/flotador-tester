#include <Arduino.h>

#define   paso    2
#define   dir     3
#define   fin     4
#define   pres    1
#define   arriba  0
#define   abajo   1
//1000 pasos = 6.98mm => 0.00698mm por pasos

bool sen = abajo;
char letra;
String argumento = "00000";

//------------------------------------------------------------mover
void mover(int dist)
{
  if(dist > 0)
    digitalWrite(dir,arriba);
  else
    digitalWrite(dir,abajo);

  for(int i = 0;i<=dist;i++)
  {
    digitalWrite(paso,1);
    delay(1);
    digitalWrite(paso,0);
    delay(1);
    Serial.print(".");
  }

}

//------------------------------------------------------------inicializar
void inicializar()
{
  while(digitalRead(fin) != pres)
  {
    Serial.println(digitalRead(fin));
    digitalWrite(paso,1);
    delay(1);
    digitalWrite(paso,0);
    delay(1);
  }
}

//---------------------------------------------------------------------

void setup()
{
  Serial.begin(9600);
  Serial.println("Iniciando...");

  pinMode(paso,OUTPUT);
  pinMode(dir,OUTPUT);
  pinMode(fin,INPUT_PULLUP);

  digitalWrite(dir,sen);
  inicializar();
}

void loop()
{
  if(Serial.available())
  {
    //letra = Serial.read();
    argumento = Serial.readStringUntil("OK");
    mover(argumento.toInt());
  }

}

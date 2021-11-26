#include <Scheduler.h>

unsigned long antes;
unsigned long despues;
unsigned long lapso;

#define DELAY1 10 /* Retraso en ms */ 
#define DELAY2  2 /* Retraso en sg */
#define DELAY3  5 /*Retraso en sg para el cambio de referencia*/

#define SALIDA_MAX 4095  /* Salida maxima para 12 bits */
#define SALIDA_MIN    0

#define escala_in 10/2047   /*Cambio de variables*/
#define escala_out 2047/10
#define offset 2047

/* Variables globales.*/

int flag = 0;
unsigned long tim1, tim2;
int T;

/* Bloque de inicializacion */

int ref=2, salida=offset, x, vel, pos, aux;

/*Inicializamos valores del pid con los resultados obtenidos en la práctica anterior, pero después de probar llegamos a los siguientes valores que funcionan*/

int kp=10, ti=20, td=100;
int ek=0, ek1=0, Ik=0;

void setup() {

  /* Linea serie a 9600 baudios */

  Serial.begin(9600);

  /* Resolucion de DAC y ADC de 12 bits (máxima) */
  
  analogReadResolution(12);
  analogWriteResolution(12);

  /* Inicialización de variables */

  T = 10; /*Tiempo de muestreo de la práctica anterior*/

  /* Referencia de tiempo */
  
  tim1 = millis();

  /* Arrancar bucles del Scheduler */
  
  Scheduler.startLoop(bucle1);
  Scheduler.startLoop(bucle2);
  Scheduler.startLoop(bucle3);
  Scheduler.startLoop(bucle4);
  
}

/* Bucle principal: Acceso a ADC y DAC, actualizando la salida con el PID */

void loop() {

  int in1;
  int in2;

    
  /* Leer periodicamente las entradas y actualizar la salida */

  delay(T);  /* Esperar T milisegundos */
  /* delayMicroseconds(T*1000); */ /* Espera alternativa, en principio
                                      más precisa */
  
  antes = micros();                    /* Referencia de tiempo */

  in1 = analogRead(A0);                /* Leer canal ADC 0 */
  in2 = analogRead(A1);                /* Leer canal ADC 1 */

  vel=(in1-offset)*escala_in;   /*Aplicamos escala a la velocidad*/
  pos=(in2-offset)*escala_in;   /*Aplicamos escala a la posición*/

  /* Actualizar salida con PID*/

  ek = ref-pos;
  Ik += ek;
  x = kp*(ek+(T/ti)*Ik+(td/T)*(ek-ek1));
  salida=x*escala_out+offset;
  ek1=ek;



  
  /* Limitar rango entre minimo y máximo. La electrónica de
     adaptación hace que en la mitad del rango se tenga un
     cero a la salida, o motor parado */

     
  if (x>10) x = 10;
  if (x<-10) x = -10;   
  if (salida > SALIDA_MAX) salida = SALIDA_MAX; 
  if (salida < SALIDA_MIN) salida = SALIDA_MIN;

  /* Actualizacion de salidas */
  
  analogWrite(DAC0, salida);    /* Escritura en canal 0 del DAC */
 
  despues = micros();           /* Tomar referencia de tiempo */

}

/* Bucle 1: Cambio de valores del usuario */

void bucle1(void)
{
   float tiempo;
   int buf;
   float tf;
   
   while(Serial.available() == 0) yield(); /* Esperar tecla */
   buf = Serial.read();



   /* Medir tiempo */
   
   if(buf == 't')
   {
     tim2 = millis();
     tiempo = (tim2-tim1)/1000.;
     tim1 = tim2;
   
     Serial.println("");
     Serial.print("Tecla pulsada; has tardado "); Serial.print(tiempo,DEC);
     Serial.println(" segundos"); 
     Serial.println("");
     
     /* Avisar para que bucle2 se active */
   
     flag = 1;

   }
   
   /* Leer un valor de referencia */
   
   if(buf == 'r')
   {
     
     Serial.println("");
     Serial.println("Escriba Referencia"); 
     Serial.println("");
     while(Serial.available() == 0) yield(); /* Esperar tecla */
     ref=Serial.parseFloat();
     Serial.println("");
     Serial.print(" Error: "); Serial.print(ek, DEC);
     flag=1; 
   }

   /* Leer un valor de Kp */

   if(buf == 'p')
   {
     
     Serial.println("");
     Serial.println("Escriba Kp"); 
     Serial.println("");
     while(Serial.available() == 0) yield(); /* Esperar tecla */
     kp=Serial.parseFloat();
     flag=1; 
   }

   /* Leer un valor de Ti */

   if(buf == 'i')
   {
     
     Serial.println("");
     Serial.println("Escriba Ti"); 
     Serial.println("");
     while(Serial.available() == 0) yield(); /* Esperar tecla */
     ti=Serial.parseFloat();
     flag=1; 
   }

   /* Leer un valor de Td */

   if(buf == 'd')
   {
     
     Serial.println("");
     Serial.println("Escriba Td"); 
     Serial.println("");
     while(Serial.available() == 0) yield(); /* Esperar tecla */
     td=Serial.parseFloat();
     flag=1; 
   }
   /* Eliminar posibles caracteres que no nos interesan */
   
   while (Serial.available()) buf = Serial.read();
   
}


/* Bucle 2 */

void bucle2(void)
{
    while(flag == 0) yield();
    flag = 0;
    delay(DELAY2*1000);
    Serial.println("");
    Serial.println("Dos segundos transcurridos desde que cambié la variable");       
    Serial.println("");


}

/*Bucle 3: Cambio de referencia en torno al cero*/

void bucle3(void)
{
  delay(DELAY3*1000);
  aux=abs(ref-0);
  if(ref>0){
    ref=0-aux;
  }
  else{
    ref=0+aux;
  }
}


/*Bucle 4: Salida de datos*/

void bucle4(void)
{
//   static int cnt=0;
   static int tiempo=0;
   
   delay(T);
//   cnt++;
//   if(cnt == 500) 
//   {
//        
//      cnt = 0;
// 
//      Serial.print("Tiempo de calculo: "); Serial.print(despues - antes, DEC); 
//      Serial.println(" microsegundos");   
//      Serial.print(" ref: "); Serial.print(ref, DEC);
//      Serial.print(" vel: "); Serial.print(vel, DEC);
//      Serial.print(" pos: "); Serial.print(pos, DEC);
//      Serial.print(" out: "); Serial.println(x, DEC);
//      Serial.print(" err: "); Serial.println(ek, DEC);
//   }
   Serial.print(ref, DEC);
   Serial.print("\t");
   Serial.print(pos, DEC);
   Serial.print("\t");
   Serial.print(tiempo, DEC);
   Serial.print("\n");
   tiempo++;
}



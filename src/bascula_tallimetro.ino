const int Trigger = 2; //pin digital 2 para el trigger
const int Echo = 3; //pin analogo 3 para recibir el eco
long t; //tiempo que tarda en regresar el eco
double d;  //distancia en centimetros
int e; //guarda la altura del objeto

void setup() {
  Serial.begin(9600); //inicia la comunicacion por puerto serial
  pinMode(Trigger,OUTPUT);  //pin de salida
  pinMode(Echo,INPUT);      //pin de entrada
  digitalWrite(Trigger,LOW);
}

void loop() {
  digitalWrite(Trigger, HIGH);
  delayMicroseconds(10);  //enviar pulso de 10 us
  digitalWrite(Trigger,LOW);

  t = pulseIn(Echo,HIGH); //obtener el ancho del pulso
  d = (0.034/2)*t;  //escalar el tiempo a una distancia en cm
  d = p(d, 193.5);
  e = (int)(((d/2)/193.5));
  Serial.print("Distancia: ");
  Serial.print(d);
  Serial.print("cm");
  Serial.println();
  Serial.println();
  Serial.print("Altura del Objeto: ");
  Serial.print(e);  //enviamos por serial el valor de la distancia
  Serial.print("cm");
  Serial.println();
  delay(500);
}

double p(double d, double m) {
  if(d>m) return m;
  return 0;  
}

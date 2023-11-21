#define pinMotor1A 11  //vd
#define pinMotor1B 10  //az
#define pinMotor2A 9   //br
#define pinMotor2B 6   //cz

#define pinSensorLinD 4  //cz
#define pinSensorLinE 5  //br

#define pinDistEcho 2     //am
#define pinDistTrigger 3  //lj

#define pinBtRx 8  //vd
#define pinBtTx 7  //az

#include <SoftwareSerial.h>

SoftwareSerial bluetooth(pinBtRx, pinBtTx);  // RX, TX

void enviaPulso();
void medeDistancia();

//Variaveis para sensor de distancia
volatile unsigned long inicioPulso = 0;
volatile float distancia = 0;
volatile int modo = -1;

//Variaveis para motor
int velocidade = 0;
int estado = 0;
int velMin = 80;
int velFaixa = 0;

//Variaveis para controle remoto
char comando;
bool estadoSE;
bool estadoSD;

//Variaveis para sensor de distância
bool sensorAtivo = false;

//Variaveis para seguidor de linhas
bool seguidorAtivo = false;

void setup() {
  pinMode(pinMotor1A, OUTPUT);
  pinMode(pinMotor1B, OUTPUT);
  pinMode(pinMotor2A, OUTPUT);
  pinMode(pinMotor2B, OUTPUT);

  pinMode(pinSensorLinD, INPUT);
  pinMode(pinSensorLinE, INPUT);

  pinMode(pinDistEcho, INPUT);
  pinMode(pinDistTrigger, OUTPUT);
  digitalWrite(pinDistTrigger, LOW);

  Serial.begin(9600);
  bluetooth.begin(9600);

  // CONFIGURA A INTERRUPÇÃO PARA SENSOR DE DISTANCIA
  attachInterrupt(digitalPinToInterrupt(pinDistEcho), medeDistancia, CHANGE);
}

void loop() {

  //Captura dos Comandos via Bluetooth
  if (bluetooth.available()) {
    comando = bluetooth.read();
    Serial.write(comando);
    Serial.print("- ");
  } else {
    Serial.print(" - ");
  }

  //Captura dos Sensores de Linha
  estadoSD = digitalRead(pinSensorLinD);
  Serial.print("SD:");
  Serial.print(estadoSD);

  estadoSE = digitalRead(pinSensorLinE);
  Serial.print(" SE:");
  Serial.print(estadoSE);

  //Aciona o Sensor de Distancia
  // ENVIA O COMANDO PARA O MÓDULO LER A DISTANCIA
  enviaPulso();
  // A RESPOSTA DA DISTANCIA VEM POR INTERRUPÇÃO, SÓ PRECISA ESPERAR ALGUNS MILISSEGUNDOS

  delay(25);  // TEMPO DE RESPOSTA APÓS A LEITURA

  Serial.print(" Dist:");
  Serial.print(distancia);
  Serial.println("cm");

  //Estados dos motores (variavel "estado")
  //0 = Parado
  //1 = Para Frente
  //2 = Para Tras
  //3 = Para Esquerda
  //4 = Para Direita
  //5 = Para Frente e Esquerda
  //6 = Para Frente e Direita
  //7 = Para Tras e Esquerda
  //8 = Para Tras e Direita

  //Controle dos Motores
  switch (comando) {

    case 'S':  //parar
      estado = 0;
      seguidorAtivo = false;
      break;

    case 'D':  //parar tudo
      estado = 0;
      seguidorAtivo = false;
      break;

    case 'F':  //para frente
      estado = 1;
      seguidorAtivo = false;
      break;

    case 'B':  //para tras
      estado = 2;
      seguidorAtivo = false;
      break;

    case 'L':  //para esquerda
      estado = 3;
      seguidorAtivo = false;
      break;

    case 'R':  //para direita
      estado = 4;
      seguidorAtivo = false;
      break;

    case 'G':  //para frente e esquerda
      estado = 5;
      seguidorAtivo = false;
      break;

    case 'I':  //para frente e direita
      estado = 6;
      seguidorAtivo = false;
      break;

    case 'H':  //para tras e esquerda
      estado = 7;
      seguidorAtivo = false;
      break;

    case 'J':  //para tras e direita
      estado = 8;
      seguidorAtivo = false;
      break;

    case '0':  //velocidade
      velFaixa = 0;
      break;

    case '1':  //velocidade
      velFaixa = 1;
      break;

    case '2':  //velocidade
      velFaixa = 2;
      break;

    case '3':  //velocidade
      velFaixa = 3;
      break;

    case '4':  //velocidade
      velFaixa = 4;
      break;

    case '5':  //velocidade
      velFaixa = 5;
      break;

    case '6':  //velocidade
      velFaixa = 6;
      break;

    case '7':  //velocidade
      velFaixa = 7;
      break;

    case '8':  //velocidade
      velFaixa = 8;
      break;

    case '9':  //velocidade
      velFaixa = 9;
      break;

    case 'q':  //velocidade
      velFaixa = 10;
      break;

    case 'v':  //desliga sensor de distancia
      sensorAtivo = false;
      break;

    case 'V':  //liga sensor de distancia
      sensorAtivo = true;
      break;

    case 'x':  //desliga seguidor de linha
      seguidorAtivo = false;
      estado = 0;
      break;

    case 'X':  //liga seguidor de linha
      seguidorAtivo = true;
      break;
  }

  //Calculo da Velocidade
  velocidade = velMin + (((255 - velMin) / 10) * velFaixa);

  //Controle do seguidor de linha
  if (seguidorAtivo) {
    velocidade = velMin;

    if (!estadoSD && !estadoSE) {  //Nenhuma linha detectada
      estado = 1;
    }

    if (estadoSE && !estadoSD) {  //Linha detectada no sensor esquerdo
      estado = 3;
    }

    if (estadoSD && !estadoSE) {  //Linha detectada no sensor direito
      estado = 4;
    }

    if (estadoSD && estadoSE) {  //Linha detectada nos dois sensores
      estado = 0;
    }
  }

  //Redutor de velocidade relativo à distância de objeto detectado a frente
  if ((sensorAtivo) && (distancia < 60) && ((estado == 1) || (estado == 5) || (estado == 6))) {
    if (distancia < 5) {
      velocidade = 0;
    } else {
      velocidade = map(distancia, 60, 5, velocidade, 0);
    }
  }

  switch (estado) {
    case 0:  //parado
      digitalWrite(pinMotor1A, LOW);
      digitalWrite(pinMotor1B, LOW);
      digitalWrite(pinMotor2A, LOW);
      digitalWrite(pinMotor2B, LOW);
      break;
    case 1:  //frente
      analogWrite(pinMotor1A, velocidade);
      analogWrite(pinMotor1B, 0);
      analogWrite(pinMotor2A, velocidade);
      analogWrite(pinMotor2B, 0);
      break;
    case 2:  //tras
      analogWrite(pinMotor1A, 0);
      analogWrite(pinMotor1B, velocidade);
      analogWrite(pinMotor2A, 0);
      analogWrite(pinMotor2B, velocidade);
      break;
    case 3:  //esquerda
      analogWrite(pinMotor1A, velocidade);
      analogWrite(pinMotor1B, 0);
      analogWrite(pinMotor2A, 0);
      analogWrite(pinMotor2B, velocidade);
      break;
    case 4:  //direita
      analogWrite(pinMotor1A, 0);
      analogWrite(pinMotor1B, velocidade);
      analogWrite(pinMotor2A, velocidade);
      analogWrite(pinMotor2B, 0);
      break;
    case 5:  //frente e esquerda
      analogWrite(pinMotor1A, velocidade);
      analogWrite(pinMotor1B, 0);
      analogWrite(pinMotor2A, velocidade / 3);
      analogWrite(pinMotor2B, 0);
      break;
    case 6:  //frente e direita
      analogWrite(pinMotor1A, velocidade / 3);
      analogWrite(pinMotor1B, 0);
      analogWrite(pinMotor2A, velocidade);
      analogWrite(pinMotor2B, 0);
      break;
    case 7:  //tras e esquerda
      analogWrite(pinMotor1A, 0);
      analogWrite(pinMotor1B, velocidade);
      analogWrite(pinMotor2A, 0);
      analogWrite(pinMotor2B, velocidade / 3);
      break;
    case 8:  //tras e direita
      analogWrite(pinMotor1A, 0);
      analogWrite(pinMotor1B, velocidade / 3);
      analogWrite(pinMotor2A, 0);
      analogWrite(pinMotor2B, velocidade);
      break;
  }

  delay(20);
}

// IMPLEMENTO DE FUNÇÕES
void medeDistancia() {
  switch (modo) {
    case 0:
      {
        inicioPulso = micros();
        modo = 1;
        break;
      }
    case 1:
      {
        distancia = (float)(micros() - inicioPulso) / 58.3;  // distancia em CM
        inicioPulso = 0;
        modo = -1;
        break;
      }
  }
}

void enviaPulso() {
  // ENVIA O SINAL PARA O MÓDULO INICIAR O FUNCIONAMENTO
  digitalWrite(pinDistTrigger, HIGH);
  // AGUARDAR 10 uS PARA GARANTIR QUE O MÓDULO VAI INICIAR O ENVIO
  delayMicroseconds(10);
  // DESLIGA A PORTA PARA FICAR PRONTO PARA PROXIMA MEDIÇÃO
  digitalWrite(pinDistTrigger, LOW);
  // INDICA O MODO DE FUNCIONAMENTO (AGUARDAR PULSO)
  modo = 0;
}

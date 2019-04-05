 #include <SoftwareSerial.h>

//블루투스
int Tx=2; //전송,보라
int Rx=3; //수신,파랑
 
SoftwareSerial btSerial(Tx, Rx);

//차 운행 기준
//1,2가 오른쪽 뒤 모터
//3,4가 왼쪽 뒤 모터
int IN1=7;//빨
int IN2=6;//주
int IN3=4;//노
int IN4=5;//초

//5,6가 오른쪽 앞 모터
//7,8가 왼쪽 앞 모터
//앞바퀴
int IN5=10;//빨
int IN6=11;//주
int IN7=8;//노
int IN8=9;//초

void setup() {
  //블루투스
  Serial.begin(9600);
  btSerial.begin(9600);
  
  //자동차이동
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);
  pinMode(IN5,OUTPUT);
  pinMode(IN6,OUTPUT);
  pinMode(IN7,OUTPUT);
  pinMode(IN8,OUTPUT);
}

void loop() {
  //블루트스
   
    
  if (btSerial.available()) {    //수신
    char command = btSerial.read();
    Serial.write(command);
    switch(command)
    {
      case '1'://좌
      Stop();
      Left();
      break;

      case '2'://전진
      Stop();
      Up();
      break;

      case '3'://우
      Stop();
      Right();
      break;

      case '4'://정지
      Stop();
      break;
     
      case '5'://후진
      Stop();
      Down();
      break;

   
    }
  }
  if (Serial.available()) {//전송         
    btSerial.write(Serial.read());
  }
}


//LOW,HIGH 전진
//HIGH,LOW 후진
//좌전진, case1
void Left()
{
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,HIGH);
    digitalWrite(IN3,HIGH);
    digitalWrite(IN4,LOW);
    digitalWrite(IN5,LOW);
    digitalWrite(IN6,HIGH);
    digitalWrite(IN7,HIGH);
    digitalWrite(IN8,LOW);
}
//전진, case2
void Up()
{
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,HIGH);
    digitalWrite(IN3,LOW);
    digitalWrite(IN4,HIGH);
    digitalWrite(IN5,LOW);
    digitalWrite(IN6,HIGH);
    digitalWrite(IN7,LOW);
    digitalWrite(IN8,HIGH);
}

//우전진, case3
void Right()
{
    digitalWrite(IN1,HIGH);
    digitalWrite(IN2,LOW);
    digitalWrite(IN3,LOW);
    digitalWrite(IN4,HIGH);
    digitalWrite(IN5,HIGH);
    digitalWrite(IN6,LOW);
    digitalWrite(IN7,LOW);
    digitalWrite(IN8,HIGH);
}


//정지, case4
void Stop()
{
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,LOW);
    digitalWrite(IN3,LOW);
    digitalWrite(IN4,LOW);
    digitalWrite(IN5,LOW);
    digitalWrite(IN6,LOW);
    digitalWrite(IN7,LOW);
    digitalWrite(IN8,LOW);
}


//후진, case6
void Down()
{
    digitalWrite(IN1,HIGH);
    digitalWrite(IN2,LOW);
    digitalWrite(IN3,HIGH);
    digitalWrite(IN4,LOW);
    digitalWrite(IN5,HIGH);
    digitalWrite(IN6,LOW);
    digitalWrite(IN7,HIGH);
    digitalWrite(IN8,LOW);
}


 

/*
#include <SoftwareSerial.h>
 
int Tx=6; //전송
int Rx=7; //수신
 
SoftwareSerial btSerial(Tx, Rx);
 
void setup() 
{
  Serial.begin(9600);
  btSerial.begin(9600);
}
 
void loop()
{
  if (btSerial.available()) {       
    Serial.write(btSerial.read());
  }
  if (Serial.available()) {         
    btSerial.write(Serial.read());
  }


  
}
*/

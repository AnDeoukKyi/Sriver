#include <SoftwareSerial.h>

//블루투스
int Tx=2; //전송,보라
int Rx=3; //수신,파랑
 
SoftwareSerial btSerial(Tx, Rx);

//1,2가 오른쪽 모터
//3,4가 왼쪽 모터
int IN1=7;//빨
int IN2=6;//주
 
//모터B 컨트롤
int IN3=5;//노
int IN4=4;//초
 
void setup() {
  //블루투스
  Serial.begin(9600);
  btSerial.begin(9600);
  
  //자동차이동
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);
}

void loop() {
  //블루트스
  if (btSerial.available()) {    //수신
    char command = btSerial.read();
    Serial.write(command);
    switch(command)
    {
      case '1'://좌전진
      Stop();
      UpLeft();
      break;

      case '2'://전진
      Stop();
      Up();
      break;

      case '3'://우전진
      Stop();
      UpRight();
      break;

      case '4'://정지
      Stop();
      break;

      case '5'://좌후진
      Stop();
      DownLeft();
      break;

      case '6'://후진
      Stop();
      Down();
      break;

      case '7'://우후진
      Stop();
      DownRight();
      break;
    }
  }
  if (Serial.available()) {//전송         
    btSerial.write(Serial.read());
  }
}

//좌전진, case1
void UpLeft()
{
    digitalWrite(IN1,HIGH);
    digitalWrite(IN2,LOW);
    digitalWrite(IN3,LOW);
    digitalWrite(IN4,LOW);
}
//전진, case2
void Up()
{
    digitalWrite(IN1,HIGH);
    digitalWrite(IN2,LOW);
    digitalWrite(IN3,HIGH);
    digitalWrite(IN4,LOW);
}

//우전진, case3
void UpRight()
{
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,LOW);
    digitalWrite(IN3,HIGH);
    digitalWrite(IN4,LOW);
}


//정지, case4
void Stop()
{
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,LOW);
    digitalWrite(IN3,LOW);
    digitalWrite(IN4,LOW);
}


//좌후진, case5
void DownLeft()
{
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,HIGH);
    digitalWrite(IN3,LOW);
    digitalWrite(IN4,LOW);
}
//후진, case6
void Down()
{
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,HIGH);
    digitalWrite(IN3,LOW);
    digitalWrite(IN4,HIGH);
}
//우후진, case7
void DownRight()
{
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,LOW);
    digitalWrite(IN3,LOW);
    digitalWrite(IN4,HIGH);
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

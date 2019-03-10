//1,2가 오른쪽 모터
//3,4가 왼쪽 모터
int IN1=7;//빨
int IN2=6;//주
 
//모터B 컨트롤
int IN3=5;//노
int IN4=4;//초
 
void setup() {
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);
}
 
void loop() {

}

//좌전진
void UpLeft()
{
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,LOW);
    digitalWrite(IN3,HIGH);
    digitalWrite(IN4,LOW);
}
//전진
void Up()
{
    digitalWrite(IN1,HIGH);
    digitalWrite(IN2,LOW);
    digitalWrite(IN3,HIGH);
    digitalWrite(IN4,LOW);
}

//우전진
void UpRight()
{
    digitalWrite(IN1,HIGH);
    digitalWrite(IN2,LOW);
    digitalWrite(IN3,LOW);
    digitalWrite(IN4,LOW);
}


//정지
void Stop()
{
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,LOW);
    digitalWrite(IN3,LOW);
    digitalWrite(IN4,LOW);
}


//좌후진
void DownLeft()
{
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,LOW);
    digitalWrite(IN3,LOW);
    digitalWrite(IN4,HIGH);
}
//후진
void Down()
{
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,HIGH);
    digitalWrite(IN3,LOW);
    digitalWrite(IN4,HIGH);
}
//우후진
void DownRight()
{
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,HIGH);
    digitalWrite(IN3,LOW);
    digitalWrite(IN4,LOW);
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

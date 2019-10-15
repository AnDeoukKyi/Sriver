#include <SoftwareSerial.h>

//�������
int Tx=2; //����,����
int Rx=3; //����,�Ķ�
 
SoftwareSerial btSerial(Tx, Rx);

//�� ���� ����
//1,2�� ������ �� ����
//3,4�� ���� �� ����
int IN1=7;//��
int IN2=6;//��
int IN3=4;//��
int IN4=5;//��

//5,6�� ������ �� ����
//7,8�� ���� �� ����
//�չ���
int IN5=10;//��
int IN6=11;//��
int IN7=8;//��
int IN8=9;//��

void setup() {
  //�������
  Serial.begin(9600);
  btSerial.begin(9600);
  
  //�ڵ����̵�
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
  //���Ʈ��
   
    
  if (btSerial.available() > 0) {    //����
    char command = btSerial.read();
    Serial.write(command);
    switch(command)
    {
      case '1'://��
      Left();
      break;

      case '2'://����
      Up();
      break;

      case '3'://��
      Right();
      break;

      case '4'://����
      Stop();
      break;
     
      case '5'://����
      Down();
      break;

   
    }
  }
  if (Serial.available()) {//����         
    btSerial.write(Serial.read());
  }
}


//LOW,HIGH ����
//HIGH,LOW ����
//������, case1
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
//����, case2
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

//������, case3
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


//����, case4
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


//����, case6
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
 
int Tx=6; //����
int Rx=7; //����
 
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

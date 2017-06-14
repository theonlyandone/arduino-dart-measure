
#include <Servo.h>
Servo myServo;

#define SERVO_SPEED 900
#define MAXDISTANCE 600 //mm
#define STARTANGLE 40

// always mm
float dartWidth = 8;

float laserDistanceToCenter = 422; //mm

float angle = STARTANGLE;
int factor = 1;

float alpha = 0;
float beta = 0;
float gamma = 0;

float measurementCount = 0;
float totalDistance = 0;
float totalAngle = 0;
float totalDartDistanceToCenter = 0;
float totalDartAngleToCenter = 0;

const float pi = 3.14159265359;

boolean recdata = true;
boolean data;
int buf[64];
int rc=0;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  myServo.attach(13);
  myServo.write(angle);
}

void loop()
{
 static unsigned long servo_time;
 
 // check time since last servo position update 
 if ((millis()-servo_time) >= SERVO_SPEED) {
   servo_time = millis();
   moveServo();
 }

static unsigned long t = 0;
if (millis() > (t + 2000)){
Serial1.write("*004545#");
t = millis();
}
getdist();
if (recdata) t = millis();
}

int getdist(){
int litera;

if (Serial1.available() > 0){
while (Serial1.available() > 0){
litera = Serial1.read();
if (litera == 42) { 
data = true; 
}

if (litera == 35) { 
data = false;
recdata = true;  
}
if(data==true && rc<40 && litera>47){  
litera = litera-48;
buf[rc] = litera; 
rc++;
}
}

}else{
if (recdata == true){
boolean dig=true;
int countdata=0;  
int data=0; 
int sum=0; 
int src=0; 
int countLaser=0; 
int dist=0; 
for(int p = 0; p<rc; p++){
  if(dig){
    data=buf[p]*10;
    countdata++;
    }
    else{
      data+=buf[p];
      if(countdata<10)sum+=data;  
      if(countdata==5)countLaser=data; 
      if(countdata==7)dist=data*10000; 
      if(countdata==8)dist+=data*100;
      if(countdata==9)dist+=data; 
      if(countdata==10)src=data;
      data=0;
      }
      buf[p]=0; 
      dig=!dig;
      }
      if(sum>= 100) {
        int a=sum;
        sum=sum/100;
        sum=sum*100;
        sum =a-sum;
    }
if(sum==src){// If the amount of bits (except the last) is the checksum (last digit) then ... 

if (dist < MAXDISTANCE && dist > 0) {
  
  float tempAngle = angle;
  measurementCount++;
  totalAngle+=tempAngle;
  totalDistance+=dist;
  
  Serial.print(tempAngle);
  Serial.print(" & ");
  Serial.print(dist);

  Serial.print(" || total angle: ");
  Serial.print(totalAngle);
  Serial.print(" & total distance ");
  Serial.print(totalDistance);
  
  float angleAverage = totalAngle/measurementCount;
  float distanceAverage = totalDistance/measurementCount;
  Serial.print(" & average angle: ");
  Serial.print(angleAverage);
  Serial.print(" & averageDistance: ");
  Serial.print(distanceAverage);

  float dartAngleLaser = abs(angleAverage - STARTANGLE);
  
  float dartDistance = getDistanceFromMiddleToDart(dartAngleLaser*(pi/180), distanceAverage); //Dart distance to middlepoint
  Serial.print(" & dartangleLaser: ");
  Serial.print(dartAngleLaser);
  totalDartDistanceToCenter += dartDistance;
  
  beta = getAngleFromMiddlePoint(dartAngleLaser*(pi/180), dartDistance, distanceAverage);
  
  totalDartAngleToCenter += beta;
  
  float dartDistanceAverage = totalDartDistanceToCenter / measurementCount; 
  float dartAngleAverage = totalDartAngleToCenter / measurementCount;

  Serial.print(" & avgDartDistance from center [mm]: ");
  Serial.print(dartDistanceAverage);
  Serial.print(" & avgDartAngle to centerline: ");
  Serial.print(dartAngleAverage);
  Serial.println();
  delay(1000);
}

  if(countLaser==99){
    Serial1.write("*004545#");
  }
}
src = 0;
countdata = 0;
countLaser = 0;
rc=0;
recdata = false;
}
}
}


void moveServo() { 
    angle += 1;
    myServo.write(angle);
    if (angle > 60) {
      myServo.write(STARTANGLE);
      angle = STARTANGLE;
    }
    
}

float getDistanceFromMiddleToDart(float alfa, float b ) {

      return sqrt((pow(b,2) + pow(laserDistanceToCenter,2) - 2*b*laserDistanceToCenter * cos(alfa)));
  }

float getAngleFromMiddlePoint(float alfa, float a, float b) {
    return asin(b*sin(alfa) / a) * (180 / pi);
  }



#include <Servo.h>
Servo myServo;

#define SERVO_SPEED 900
#define MAXDISTANCE 600
#define STARTANGLE 40

// always mm
float dartWidth = 8;

float laserDistance = 383;

int distanceToCenter = 0;

int angle = STARTANGLE;
int factor = 1;

int alpha = 0;
int beta = 0;
int gamma = 0;

int measurementCount = 0;
int totalDistance = 0;
int totalAngle = 0;
int totalDartDistance = 0;

const float pi = 3.14159265359;

boolean recdata = true;
boolean data;
int buf[64];
int rc=0;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  myServo.attach(13);
  myServo.write(STARTANGLE);
}

void loop()
{
 static unsigned long servo_time;
 
 // check time since last servo position update 
 if ((millis()-servo_time) >= SERVO_SPEED) {
   servo_time = millis(); // save time reference for next position update
   moveServo();
 }

static unsigned long t = 0;
if (millis() > (t + 2000)){ // timer start when no data from the sensor 
Serial1.write("*004545#");
t = millis();
}
getdist();
if (recdata) t = millis(); // Reset the timer when it receives data from the sensor 
}

int getdist(){
int litera;

if (Serial1.available() > 0){
while (Serial1.available() > 0){
litera = Serial1.read();
if (litera == 42) { // If adopted a "*" 
data = true; //Then set the sign of the beginning of the packet 
}

if (litera == 35) { // If adopted, the "#" 
data = false; //Then set the sign of the end of the package ... 
recdata = true; //And install a sign to obtain data for the control (reset) the timer and further processing of the packet 
}
if(data==true && rc<40 && litera>47){ // If there is a sign of the beginning of the packet, the packet length is reasonable and litera has a numeric value to ASCII, the ... 
litera = litera-48;// convert ASCII to figure ... 
buf[rc] = litera; // And add it to the array. 
rc++;
}
}

}else{
if (recdata == true){
boolean dig=true; //This variable will work to separate the package into categories 2 digits 
int countdata=0; //This variable will be considered level 
int data=0; //This variable will take the values ​​of bits 
int sum=0; //This sum of all digits except for the last 
int src=0; //This is the last category (10), which defines the checksum 
int countLaser=0; //This is an internal counter in the 5th digit 
int dist=0; //It is the distance, we calculate 
for(int p = 0; p<rc; p++){
  if(dig){
    data=buf[p]*10;//Here" we="" have="" the="" first="" sign="" of="" any="" new="" discharge="" multiply="" by="" 10="" ....="" 
    countdata++;
    }
    else{
      data+=buf[p];// and="" here="" we="" add="" to="" it,="" the="" second="" value.="" 
      if(countdata<10)sum+=data; //here" podschitvaem="" checksum="" 
      if(countdata==5)countLaser=data; //here" to="" see="" the="" counter="" 
      if(countdata==7)dist=data*10000; //" here="" believe="" distantsiyayu="" ---------|="" 
      if(countdata==8)dist+=data*100;//" |="" 
      if(countdata==9)dist+=data;//------------------------------------------|" 
      if(countdata==10)src=data;//here" extract="" the="" checksum="" of="" the="" package="" 
      data=0;
      }
      buf[p]=0; 
      dig=!dig;
      }
      if(sum>= 100) {// If the checksum is greater than 99, then cut off the excess, leaving only the last two 
        int a=sum;
        sum=sum/100;
        sum=sum*100;
        sum =a-sum;
    }
if(sum==src){// If the amount of bits (except the last) is the checksum (last digit) then ... 
// Serial.print(" ");
// Serial.print(sum);
// Serial.print(" ");
// Serial.print(src);
if (dist < MAXDISTANCE && dist > 0) {
  
  int tempAngle = angle;
  measurementCount++;
  totalAngle+=tempAngle;
  totalDistance+=dist;
  Serial.print(tempAngle);
  Serial.print(" & ");
  Serial.print(dist);//Output the distance and ... ¨

  Serial.print(" || total: ");
  Serial.print(totalAngle);
  Serial.print(" & ");
  Serial.print(totalDistance);
  Serial.print(" & ");
  
  int angleAverage = totalAngle/measurementCount;
  int distanceAverage = totalDistance/measurementCount;
  Serial.print("average: ");
  Serial.print(angleAverage);
  Serial.print(" & ");
  Serial.print(distanceAverage);

  int dartAngle = abs(angleAverage - STARTANGLE);
  
  int dartDistance = getDistanceFromMiddleToDart(dartAngle*(pi/180), dist); //Dart distance to middlepoint
  
  totalDartDistance += dartDistance; //sin(dartAngle*(pi/180))*laserDistance;
  
  beta = getAngleFromMiddlePoint(dartAngle*(pi/180), dist, dartDistance);

  int dartDistanceAverage = totalDartDistance / measurementCount; 

  Serial.print(" & dartDistance: ");
  Serial.print(dartDistance);
  Serial.print(" & avgDartDistance: ");
  Serial.print(dartDistanceAverage);
  Serial.print(" & beta: ");
  Serial.print(beta);
  Serial.print(" & gamma: ");
  Serial.print(gamma);
  Serial.println();
}


//if(measurementCount == 10) {

  //Serial1.write("r"); //give the command to start a new cycle
//}

if(countLaser==99){// if the counter has reached the limit, then ... 
Serial1.write("*004545#"); //give the command to start a new cycle 
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
   if (angle < 11 || angle > 69) {
    factor *= -1;
    }
    angle += factor;  
    myServo.write(angle);
}

int getDistanceFromMiddleToDart(int alpha, int dist) {
    return sqrt(pow(laserDistance,2) + pow(dist,2)-(2*laserDistance*dist)*cos(alpha));
  }

int getAngleFromMiddlePoint(int alpha, int dist, int dartDist) {
    return acos((pow(laserDistance,2) - pow(dist,2) - pow(dartDist,2))/(-2*dartDist*laserDistance));
  }

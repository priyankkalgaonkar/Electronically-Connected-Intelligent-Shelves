#include "mbed.h"
#include "Servo.h"
#include "hcsr04.h"
#include "TextLCD.h"
#include "ESP8266.h"
#include "math.h"
#define IP "184.106.153.149"            //ThingSpeak IoT Server's IP Address

HCSR04  Sensor1(A4,A5);                 //Ultrasonic Sensor; A4 = PTC10; A5 = PTC11
    //TextLCD lcd(PTE0,PTE1);           //Note: Giving Error Here
    //I2C I2C(PTE25, PTE24);            //Accelerometer //Remove if not required
Serial PC(USBTX, USBRX);                //Serial Communication with PC (TeraTerm)
Serial OutputDevice(PTC15,PTC14);
ESP8266 Wifi(PTC17, PTC16, 115200);     //Wifi Module; Baud Rate = 115200
DigitalOut RLed(LED1);                  //Onboard Red LED = Warning
DigitalOut GLed(LED2);                  //Onboard Green LED = All OK
DigitalOut BLed(LED3);                  //Onboard Blue LED = Wifi Tx

char Tx[255], Rx[1000];
void Wifi_Tx(void);

int distance;
int Dist_Percent;
//int State = 0;

int main()
{
   RLed = 0;                            //Red LED = OFF
   GLed = 0;                            //Green LED = OFF
   unsigned char count = 0;
   
   while(1)
   {
        Sensor1.start();                //Ultrasonic Sesnor = ON
        wait_ms(0.0025);
        int distance = Sensor1.get_dist_cm();
        Dist_Percent = (30 - distance)/30 * 100;//Calculates % distance Remaining
        wait(0.5);
        PC.printf("Stock Remaining % = %i\r\n", Dist_Percent);
        
/*//Displaying on External 2x16 LCD Display        
        lcd.cls();
        lcd.locate(0,0);
        lcd.printf("cm:%ld",distance);
 
        count++;
        lcd.locate(0,1);
        lcd.printf("distance =%d",count);
//End of LCD Display Processing*/
        
        if(distance >= 30 )
        {
            GLed = 0;                   //Green LED = OFF
            wait(0.5);
            RLed = 1;                   //Red LED = ON
            wait(0.5);
            OutputDevice.printf("Shelf is Empty. Re-Stock Immediately.");  
        }
        else
        {
            RLed = 0;                   //Red LED = OFF
            wait(0.5);
            GLed = 1;                   //Green LED = ON
        }
    }
//Wifi (ESP Module) Setup and Transmission to ThingSpeak IoT Cloud
    PC.baud(115200);
    
    PC.printf("Setting mode to Access Point\r\n");
    Wifi.SetMode(1);
    Wifi.RcvReply(Rx, 1000);
    PC.printf("%s",Rx);                 //Print Response from ESP Module
    
    PC.printf("Conneting to Wi-Fi\r\n");
    Wifi.Join("Network SSID", "Password");//Configure Wifi Username & Pwd here 
    Wifi.RcvReply(Rx, 1000);
    PC.printf("%s\n", Rx);              //Print Response from ESP Module
    
    wait(8);
    
    PC.printf("Getting IP\r\n");
    Wifi.GetIP(Rx);
    PC.printf("%s\n", Rx);              //Display AP's IP Address
    
    while (1) 
    {
        PC.printf("Syncing with Cloud. Please Wait.\n");
        Wifi_Tx();
        BLed = 1;
        wait(2.0f);
        BLed = 0;
        wait(1.5f);
    }    
}

void Wifi_Tx(void)
{
    strcpy(Tx,"AT+CIPMUX=1\n");        //Start Up Multi-IP Connection
    Wifi.SendCMD(Tx);
    PC.printf(Tx);
    
    wait(3);
    
    Wifi.RcvReply(Rx, 1000);
    PC.printf("%s\n", Rx);
    
    wait(3);
    
    sprintf(Tx,"AT+CIPSTART=4,\"TCP\",\"%s\",80\n",IP); //Connect to ThingSpeak server 
    PC.printf(Tx);
    
    wait(3);
    
    Wifi.RcvReply(Rx, 1000);
    PC.printf("%s\n", Rx);
    
    wait(3);
    
    strcpy(Tx,"AT+CIPSEND=4,47\n");    //ESP sends data to ThingSpeak server 
    Wifi.SendCMD(Tx);
    PC.printf(Tx);
    
    wait(3);
    
    Wifi.RcvReply(Rx, 1000);
    PC.printf("%s\n", Rx);
    
    wait(3);
    
    //Write to ThingSpeak Channel Feed (Push data to Cloud)
    sprintf(Tx,"GET https://api.thingspeak.com/update?api_key=NZ7C54KZ7AQ4T6IP&field1=0");
    PC.printf("%s",Tx);
    Wifi.SendCMD(Tx);
    
    wait(3);
    
    Wifi.RcvReply(Rx, 1000);
    PC.printf("%s", Rx);
    
    Wifi.SendCMD("AT+CIPCLOSE");        //Terminate Tranmission
    Wifi.RcvReply(Rx, 1000);
    PC.printf("%s", Rx);
}
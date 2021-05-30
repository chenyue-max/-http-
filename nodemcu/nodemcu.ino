#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>   // 使用WiFiMulti库 
#include <ESP8266WebServer.h>   // 使用WebServer库
#include <BH1750.h>
#include <IRsend.h>
#include <IRutils.h>
#include <IRremoteESP8266.h>
#include <Thinary_AHT10.h>
#include <Adafruit_BMP085.h>
ESP8266WiFiMulti wifiMulti;     // 建立ESP8266WiFiMulti对象,对象名称是 'wifiMulti'
ESP8266WebServer server(80); 
AHT10Class AHT10;  
Adafruit_BMP085 bmp; 
BH1750 lightMeter(0x23);
float AHT10_temp, AHT10_hum,temperature,pressure,Light;
const char* host = "192.168.3.254";    // 即将连接服务器网址/IP
const int httpPort = 9000;               // 即将连接服务器端口
String comdata="990423300";
unsigned int buf[4]={};
uint16_t initial[280]={8950,4500,600,600,600,600,600,1650,650,550,650,1650,600,1700,600,600,600,550,650,1650,600,1700,600,1700,600,550,650,550,650,550,650,550,600,600,600,600,600,600,600,600,600,550,650,550,650,550,650,550,600,600,600,600,600,600,600,600,600,600,600,1650,650,550,650,1650,600,600,600,600,600,1650,650,550,650,19800,600,600,600,600,600,600,600,550,650,550,650,550,650,550,600,600,600,600,600,600,600,600,600,600,600,550,650,550,650,550,650,550,600,600,600,600,600,600,600,600,600,600,600,550,650,550,650,550,650,550,600,600,600,600,600,1700,600,1650,650,550,600,1700,600,600,600,25886,8850,4500,600,600,600,550,650,1650,650,550,600,1700,600,1650,650,550,650,550,650,1650,600,1700,600,1650,650,550,600,600,600,600,600,600,600,600,600,600,600,600,600,550,650,550,650,550,600,600,600,600,600,600,600,600,600,600,600,600,600,550,650,1650,600,1700,600,1700,600,550,650,550,650,1650,600,600,600,19850,600,600,600,550,650,550,650,550,650,550,600,600,600,600,600,600,600,600,600,550,650,550,650,550,600,600,600,600,600,600,600,600,600,600,600,600,600,550,650,550,650,1650,600,1700,600,600,600,600,600,550,650,550,650,550,650,550,600,600,600,600,600,600,600,1650,650,14126
};
int check_code=99;
int power=0;   //开关 
int mode=4;   //模式                                                      
int temp=23;  //温度
int airspeed=1;  //风速
int ud=0;      //上下扫风
int lr=0;       //左右扫风

int se=0;     //节能
int light=0;    //灯光
int swing=0;    //扫风标志位
int i;
const uint16_t kIrLed =15;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

void setup(void){
  Serial.begin(9600);                  // 启动串口通讯
  Serial.println("");
      irsend.begin();
   AHT10.begin();
   bmp.begin();
   lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
  
  wifiMulti.addAP("N6-506-ARM", "armn6-506qwg"); // 将需要连接的一系列WiFi ID和密码输入这里
  Serial.println("Connecting ...");                            // 则尝试使用此处存储的密码进行连接。
  
  while (wifiMulti.run() != WL_CONNECTED) { // 尝试进行wifi连接。
    delay(250);
    Serial.print('.');
  }
 
  // WiFi连接成功后将通过串口监视器输出连接成功信息 
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // 通过串口监视器输出连接的WiFi名称
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // 通过串口监视器输出ESP8266-NodeMCU的IP
  server.on("/update", handleUpdate);   
  server.begin(); 
 Serial.println("HTTP server started");
}
 
void loop(void){
 AHT10_temp = AHT10.GetTemperature();
  AHT10_hum = AHT10.GetHumidity();
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure() ;
  Light=  lightMeter.readLightLevel();
 wifiClientRequest(); // 发送请求
 server.handleClient();  // 服务端接收
 delay(1000);
}
void handleUpdate(){
  //处理接收的数据
  server.send(200, "text/plain", "Received");   
       String open_1 = server.arg("Open") ;                                       
     String mode_1= server.arg("model");
     String temp_1= server.arg("temp");
     String speed_1= server.arg("speed");
     String ud_1= server.arg("ud");
     String lr_1= server.arg("lr"); 
      if(open_1!="-2")
          power=open_1.toInt();
     if(mode_1!="-2")
          mode=mode_1.toInt();
     if(temp_1!="-2")
          temp=temp+temp_1.toInt();
     if(speed_1!="-2")
     {
      if(speed_1=="1" && airspeed<3 || speed_1=="-1" && airspeed>0)
      {
        airspeed=airspeed+speed_1.toInt();
      }
     }
      if(ud_1!="-2")
          ud=ud_1.toInt();
     if(lr_1!="-2")
          lr=lr_1.toInt();
  comdata=String(check_code)+String(power)+String(mode)+String(temp)+String(airspeed)+String(ud)+String(lr);
      send_infrared_signal();
}


void wifiClientRequest(){
  WiFiClient client;  
  // 将需要发送的数据信息放入客户端请求
  String url = "/data/" + String(AHT10_temp)+"/" + String(AHT10_hum)+"/" + String(temperature)+"/" + String(pressure)+"/" + String(Light);
                       
//   建立字符串，用于HTTP请求
  String httpRequest =  String("GET ") + url + " HTTP/1.1\r\n" +
                        "Host: " + host + "\r\n" +
                        "Connection: close\r\n" +
                        "\r\n";

   Serial.println(httpRequest);                  
  Serial.print("Connecting to "); 
  Serial.print(host); 
  
  if (client.connect(host, httpPort)) {  //如果连接失败则串口输出信息告知用户然后返回loop
    Serial.println(" Sucess");
    
    client.print(httpRequest);          // 向服务器发送HTTP请求
    Serial.println("Sending request: ");// 通过串口输出HTTP请求信息内容以便查阅
    Serial.println(httpRequest);        
  } else{
    Serial.println(" failed");
  }
  
  client.stop();                         
}
void check()
{
  //第一段校验码
  int i=0;
  unsigned int buf[4];
  int mode1,mode2,mode3;
  int temp1,temp2,temp3,temp4;
  int airspeed1,airspeed2;
  int check1_deci,check2_deci;
  unsigned int check1_bin[4];

  mode1=*(initial+3)>1000?1:0;
  mode2=*(initial+5)>1000?1:0;
  mode3=*(initial+7)>1000?1:0;
  temp1=*(initial+19)>1000?1:0;
  temp2=*(initial+21)>1000?1:0;
  temp3=*(initial+23)>1000?1:0;
  temp4=*(initial+25)>1000?1:0;
  airspeed1=*(initial+11)>1000?1:0;
  airspeed2=*(initial+13)>1000?1:0;
  mode=mode1+mode2*2+mode3*4;
  temp=temp1+temp2*2+temp3*4+temp4*8+16;
  airspeed=airspeed1+airspeed2*2;
  
  if(power==1)
  {
  check1_deci=mode+(temp-16)+power+lr+1;
  check2_deci=mode+(temp-16)+power+airspeed+1;
  }

  else
  {
  check1_deci=mode+(temp-16)+lr+10;
  check2_deci=mode+(temp-16)+airspeed+10;
  }

  while(i<4)
  {
    buf[i]=check1_deci%2;
    check1_deci=check1_deci/2;
    i++;
    
  }
  *(initial+131)=(buf[0]==1?1700:600);
  *(initial+133)=(buf[1]==1?1700:600);
  *(initial+135)=(buf[2]==1?1700:600);
  *(initial+137)=(buf[3]==1?1700:600);

//第二段校验码
  i=0;
  while(i<4)
  {
    buf[i]=check2_deci%2;
    check2_deci=check2_deci/2;
    i++;
  }
  *(initial+271)=(buf[0]==1?1700:600);
  *(initial+273)=(buf[1]==1?1700:600);
  *(initial+275)=(buf[2]==1?1700:600);
  *(initial+277)=(buf[3]==1?1700:600);

}
void send_infrared_signal()
{

    /*    //前两位为校验码
    if(!comdata.startsWith("99"))
    return 0;
    */

    power = comdata[2]-48;
    mode = comdata[3]-48;
    temp = 10*(comdata[4]-48)+comdata[5]-48;
    airspeed = comdata[6]-48;
    ud = comdata[7]-48;
    lr = comdata[8]-48;
    swing = (comdata[7]-48)||(comdata[8]-48);
     
    //开关
    if(power==1)
      {
      initial[9]=1700;
      initial[149]=1700;
      }
      else
      {
      initial[9]=600;
      initial[149]=600;
      }
      delay(200);
      
      //模式
    i=0;
    if (mode==4)
      initial[129]=1700;
    else
      initial[129]=600;
      
    while(i<3)
      {
        buf[i]=mode%2;
        mode=mode/2;
        i++;
        
      }

        initial[3]=(buf[0]==1?1700:600);
        initial[5]=(buf[1]==1?1700:600);
        initial[7]=(buf[2]==1?1700:600);

        initial[143]=(buf[0]==1?1700:600);
        initial[145]=(buf[1]==1?1700:600);
        initial[147]=(buf[2]==1?1700:600);
    //温度
      i=0;
      temp=temp-16;
      while(i<4)
        {
          buf[i]=temp%2;
          temp=temp/2;
          i++;
        }
        initial[19]=(buf[0]==1?1700:600);
        initial[21]=(buf[1]==1?1700:600);
        initial[23]=(buf[2]==1?1700:600);
        initial[25]=(buf[3]==1?1700:600);

        initial[159]=(buf[0]==1?1700:600);
        initial[161]=(buf[1]==1?1700:600);
        initial[163]=(buf[2]==1?1700:600);
        initial[165]=(buf[3]==1?1700:600);

    //风速
        i=0;
        while(i<2)
        {
          buf[i]=airspeed%2;
          airspeed=airspeed/2;
          i++;
         
        }
      initial[11]=(buf[0]==1?1700:600);
      initial[13]=(buf[1]==1?1700:600);

      initial[151]=(buf[0]==1?1700:600);
      initial[153]=(buf[1]==1?1700:600);

      initial[255]=(buf[0]==1?1700:600);
      initial[257]=(buf[1]==1?1700:600);

      check();

    //扫风
      initial[15]=(swing==1?1700:600);
      initial[75]=(ud==1?1700:600);
      initial[83]=(lr==1?1700:600);
      initial[155]=(swing==1?1700:600);

    //灯光
    initial[45]=1700;
    initial[185]=1700;   
    irsend.sendRaw(initial,280,38); 
    Serial.println("Data Sent");

}

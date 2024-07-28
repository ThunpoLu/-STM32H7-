#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <UrlEncode.h>

#define SSID              "HUAWEI-CR1LZ2"
#define PASSWORD          "52580000"
#define ACCESS_TOKEN      "24.c23643ae6275994ca3624b696ffb6aa9.2592000.1718369090.282335-33772553"
#define IMAGE             "image="


SoftwareSerial mySerial(D6,D7);  //自定义串口 (RX, TX)   # D6接RX D7接TX

String buffers ;   //用于存放接收到的数据
char* output;      //用来存放服务器响应数据
String urlData;    //用于存放发送body数据
String body;       //响应体
const char *host = "183.230.40.33";
const int tcpPort = 80;
int Result;

//采用https方法访问百度服务器
static void Baidu_AI(){ 
    uint32_t len =  buffers.length();
    len = len+6;
    Serial.print(len);
    WiFiClientSecure client;
    client.setInsecure();
    delay(1000);
    if(client.connect("aip.baidubce.com",443)){
        Serial.println("Connection succeeded");
        String url = "https://aip.baidubce.com/rest/2.0/image-classify/v2/advanced_general";
        client.print(String("POST ") + url + "?access_token=" + String(ACCESS_TOKEN)+" HTTP/1.1\r\n");
        client.print(String("HOST: ") + "aip.baidubce.com\r\n");
        client.println("Content-Length: " + String(len));
        client.print("Content-Type: application/x-www-form-urlencoded\r\n\r\n");
        client.print(String(IMAGE));
        client.print(buffers);
        client.print("\r\n");
        buffers = "";
        uint8_t i_i = 0;
        while (!client.available()){
            i_i += 1;
            delay(100);
            if(i_i > 200){ //超时
                Serial.println("No response...");
                break;
            }
        }
        
        while (client.available()){ 
          body= client.readString();
          output= const_cast<char*>(body.c_str());
          Serial.print(output);//串口输出服务器响应结果
        }
        client.stop();
        }else Serial.println("Connection failed");
}

void Json(){
  const char* keywordStart = strstr(output, "\"keyword\":\"");  
  if (keywordStart == NULL) {  
    Serial.println("Failed to find 'keyword':");  
    return;  
  }  
  // 跳过前面的部分，指向"keyword"的值  
  keywordStart += strlen("\"keyword\":\"");  
  
  // 找到"keyword"值后面的结束引号  
  const char* keywordEnd = strchr(keywordStart, '"');  
  if (keywordEnd == NULL) {  
    Serial.println("Failed to find end of 'keyword' value:");  
    mySerial.write('0');
    Result = 0;
    return;  
  }  
  
  // 计算"keyword"值的长度  
  size_t keywordLength = keywordEnd - keywordStart;  
  
  // 创建一个字符数组来存储截取的"keyword"值（包括结束的空字符）  
  char keywordBuffer[keywordLength + 1];  
  strncpy(keywordBuffer, keywordStart, keywordLength);  
  keywordBuffer[keywordLength] = '\0'; // 添加结束的空字符  

  Serial.println(keywordBuffer);  
  String key = String(keywordBuffer);
  if(key == "盲道"){
    mySerial.write('1');
    Result = 1;
  }
  else{
    mySerial.write('0');
    Result = 0;
  }
}

void Onenet(){
  //ONEnet部分
  WiFiClient out; //声明一个客户端对象，用于与服务器进行连接
  while (!out.connected())//若未连接到服务端，则客户端进行连接。
  {
    if (!out.connect(host, tcpPort))//实际上这一步就在连接服务端，如果连接上，该函数返回true
    {
      Serial.println("connection....");
      delay(500);
    }
  }
  Serial.println("尝试访问服务器");
  if (out.connected()) {
    Serial.println("访问服务器成功");
    String url = "http://api.heclouds.com/devices/1183391372/datapoints?type=3";  //参数type=3表示用 简洁数据格式,592465545是你在移动OneNET上创建的设备ID 不是产品的ID!!!! 一定要记住
    out.print(String("POST ") + url + " HTTP/1.1\r\n");                                             //使用HTTP/1.1协议
    out.print(String("api-key: ") + "V0BSC8kSkpDyDegnBQSKQgVDQyU=\r\n");           //一定要改为设备的API key，不是产品的API key!!!! 一定要记住
    out.print(String("Host: ") + "api.heclouds.com\r\n");
    out.print("Connection: close\r\n");
    
    String result = "{'result':" +  String(Result) + "}";
    Serial.print(result);
    out.print("Content-Length:" + String(result.length()) + "\r\n\r\n");
    out.print(result);
    out.print("\r\n");
  }
  out.stop();
  yield();
}

void setup() { 
  //初始化串口
  Serial.begin(115200);
  mySerial.begin(115200);
  //连接WIFI
  WiFi.begin(SSID,PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println("connected...");
  }
  Serial.println("wifi已连接");
}
 
void loop() {
  while(1){
    if(mySerial.available()){
      char data = (char)mySerial.read();
      if(data == '@'){
        Serial.println();
        break;
      }else{
        Serial.print(data);
      buffers += data;
      }
    }
  }
  buffers = urlEncode(buffers);
  Baidu_AI();
  buffers = "";
  Json();
  Onenet();
}

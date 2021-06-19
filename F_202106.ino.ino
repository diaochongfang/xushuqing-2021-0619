#include <FastLED.h>
#include <TuyaWifi.h>
//#include <SoftwareSerial.h>

#define NUM_LEDS 32//彩灯个数
#define DATA_PIN  8//驱动引脚

#define work_accelerate 0//加速
#define work_turn       1//翻转
#define work_rainbow    2//彩虹
#define work_starlight  3//星光
#define work_colour     4//调色

TuyaWifi Tuya_fast;//建立一个涂鸦设备
CRGB leds[NUM_LEDS];//建立一个彩灯的构造函数
CHSV myHSV(0,150,180);//定义一个HSV颜色，360，1000，1000
CHSV turnHSV(0,180,180);//定义一个用于跑马灯的HSV颜色，255，255，255
//SoftwareSerial DebugSerial(18,19);

uint8_t power = 0;//电源开关
uint8_t work  = 0;//工作模式，默认为加速模式
uint16_t bright = 500;//亮度值，0-1000；
//String STR_colour = "";//彩光接收数据，字符串

bool succ = false;//配网成功标志
bool pw   = false;//配网标志

#define dpid_switch 20//开关，用于开关机
#define dpid_work   21//工作模式，用于切换闪烁模式
#define dpid_bright 22//亮度调节
#define dpid_colour 24//彩光模式

uint8_t dp_array[][2] = {
  {dpid_switch , DP_TYPE_BOOL},
  {dpid_work   , DP_TYPE_ENUM},
  {dpid_bright , DP_TYPE_VALUE},
  {dpid_colour , DP_TYPE_STRING}
};

uint8_t pid[] = {"9wjzavfzz3alrk05"};
uint8_t mcu_ver[] = {"1.0.0"};

uint32_t t_time;

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(150);
  Serial.begin(9600);
  //DebugSerial.begin(9600);
  Tuya_fast.init(pid , mcu_ver);//初始化设备
  Tuya_fast.set_dp_cmd_total(dp_array , 4);//设置四个各种接收量
  Tuya_fast.dp_process_func_register(dp_process);//接收函数
  Tuya_fast.dp_update_all_func_register(dp_update);//发送处理函数
  t_time = millis();//闪烁时间
}

uint8_t dp_process(uint8_t dpid, const uint8_t value[],uint8_t length){
  switch(dpid){
    case dpid_switch:
      power = Tuya_fast.mcu_get_dp_download_data(dpid , value , length);
      Tuya_fast.mcu_dp_update(dpid_switch , power , length);
      break;
    case dpid_work:
      work = Tuya_fast.mcu_get_dp_download_data(dpid, value, length);
      Tuya_fast.mcu_dp_update(dpid_work , work ,length);
      break;
    case dpid_bright:
      bright = Tuya_fast.mcu_get_dp_download_data(dpid , value ,length);
      FastLED.setBrightness(map(bright, 0, 1000, 0, 255));
      Tuya_fast.mcu_dp_update(dpid_bright, bright ,length);
      break;
    case dpid_colour:
          toColour(value , length);//接收HSV数值
          //DebugSerial.println(myHSV.h);
          //DebugSerial.println(myHSV.s);
          //DebugSerial.println(myHSV.v);
      Tuya_fast.mcu_dp_update(dpid_colour , value , length);
      break;
    default:break; 
  }
  return SUCCESS;
}

void dp_update(){
  Tuya_fast.mcu_dp_update(dpid_switch , power , 1);
}

void loop() {
  //accelerate();//加速
  //turn();//翻转
 // rainbow();//彩虹
  //starlight();//星光
  Tuya_fast.uart_service();
  if(digitalRead(19) == LOW){
    delay(30);
    if(digitalRead(19) == LOW){
      while(digitalRead(19) == LOW);
      Tuya_fast.mcu_set_wifi_mode(SMART_CONFIG);//开始配网
      pw = true;
    }
  }
  if ((Tuya_fast.mcu_get_wifi_work_state() != WIFI_LOW_POWER) && (Tuya_fast.mcu_get_wifi_work_state() != WIFI_CONN_CLOUD) && (Tuya_fast.mcu_get_wifi_work_state() != WIFI_SATE_UNKNOW)) 
    {
      if(pw == true)
      smart();//联网灯
      else{
        if(millis() - t_time < 5000) accelerate();
        else if(millis() - t_time < 10000) turn();
        else if(millis() - t_time < 15000) rainbow();
        else if(millis() - t_time < 20000) starlight();
        else t_time = millis();
        }
    }
  else {
    if(succ == false){
      Tuya_fast.mcu_dp_update(dpid_work , work ,1);
      Tuya_fast.mcu_dp_update(dpid_bright, bright ,1);
      succ = true;
      pw   = false;
    }
    if(power == 0){
      fill_solid(leds, NUM_LEDS , CRGB::Black);
      FastLED.show();
    }//如果电源开关是关闭的，直接熄灭所有灯
    else{
      switch(work){
        case work_accelerate:accelerate();break;
        case work_turn:turn();break;
        case work_rainbow: rainbow();break;
        case work_starlight:starlight(); break;
        case work_colour: 
          fill_solid(leds, NUM_LEDS , myHSV);
          FastLED.show();
          break;
        default:break;    
      }
    }
  }
  
  /*for(int i = 0;i < 3; i ++){
    uint8_t c = random8(255);//色调随机
    for(int j = 0;j < NUM_LEDS; j ++){
      leds[j] = ColorFromPalette(RainbowColors_p,c,50,LINEARBLEND);
      FastLED.show();
      delay(20);
    }
    for(int j = 0;j < NUM_LEDS; j ++){
      leds[j] = CRGB::Black;
      FastLED.show();
      delay(10);
    }
  }*/
  
  /*for(int i = 0;i < 10;i ++){
    uint8_t c = random8(255);//色调随机
    fill_solid(leds,NUM_LEDS,ColorFromPalette(RainbowColors_p,c,50,LINEARBLEND));
    FastLED.show();
    delay(100);
    fill_solid(leds,NUM_LEDS,CRGB::Black);
    FastLED.show();
    delay(200);
  }*/

//CHSV myHSV(255,255,255);//hsv颜色
//fill_solid(leds, NUM_LEDS , myHSV);


}

/*void water(bool Direction){
  uint8_t c = random8(255);
  for(int i = 0; i < NUM_LEDS; i ++){
    if(Direction) leds[i] = ColorFromPalette( PartyColors_p, c, 50, LINEARBLEND);
    else          leds[NUM_LEDS - i] = ColorFromPalette( PartyColors_p, c, 50, LINEARBLEND);
    FastLED.show();
    delay(20);
    for(char i = 0;i < NUM_LEDS; i ++)
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(50);
  }
}*/

void toColour(char value[] ,char length){
  uint8_t V[12];
  for(uint8_t i = 0; i < length; i++){
    if(value[i] < 'a')V[i] = value[i] - '0';
    else              V[i] = value[i] - 'a' + 10;
    //DebugSerial.write(V[i]);
  }
  //DebugSerial.println("");
  myHSV.h = map(V[0]*0x1000+V[1]*0x100+V[2]*0x10+V[3],0,360,0,255);
  myHSV.s = map(V[4]*0x1000+V[5]*0x100+V[6]*0x10+V[7],0,1000,0,255);
  myHSV.v = map(V[8]*0x1000+V[9]*0x100+V[10]*0x10+V[11],0,1000,0,255);
}//解析接收到的数据，获取HSV数值

void smart(){
  for(uint8_t i = 0 ;i < NUM_LEDS; i ++){
    leds[i] = CRGB::Black;
  }
  leds[0] = CRGB::Red;
  FastLED.show();
  delay(200);
  leds[0] = CRGB::Green;
  FastLED.show();
  delay(200);
}//配网显示状态

void starlight(){
  //uint32_t tim = millis();
  //while(millis() - tim < 35000){
  int pos = random8(NUM_LEDS);
  EVERY_N_MILLISECONDS( 500 ) { 
    if( random8() < 200) {
      if(work = work_starlight)
      leds[pos] = ColorFromPalette( PartyColors_p, random8(255), 200, LINEARBLEND);
    }   
  }
  EVERY_N_MILLISECONDS( 20 ) { 
    if(work = work_starlight)
    fadeToBlackBy( leds, NUM_LEDS, 10);
  }
  if(work = work_starlight){    
  FastLED.show(); 
  delay(50);  }
  //}
      if(work = work_starlight){
  fill_solid(leds,NUM_LEDS,CRGB::Black);
  FastLED.show();}
}//星光显示

void rainbow(){
   //for(int i = 0;i < 500;i ++){
   static uint16_t i = 0;
   if(i > 5000) i = 0;
    fill_rainbow(leds,NUM_LEDS,i);
    addGlitter(30);
    FastLED.show();
    delay(100);
    i++;
  //}
}//彩虹显示

void addGlitter(uint8_t chanceofGlitter){
  if(random8() < chanceofGlitter){
    leds[random8(NUM_LEDS)] = CRGB::White;
  }
}

void accelerate(){
  static uint8_t i = 0;
  leds[i] = turnHSV;
  FastLED.show();
  delay(100);
  leds[i] = CRGB::Black;
  FastLED.show();
  delay(100);
  i ++;
  if(i >= NUM_LEDS) i = 0;
  turnHSV.h ++;
  if(turnHSV.h > 255)turnHSV.h = 0;
  /*for(uint32_t T = 1000; T > 200;){
    for(uint8_t i = 0; i < NUM_LEDS; i ++){
      leds[i] = CRGB::Red;
      FastLED.show();
      delay(T);
      leds[i] = CRGB::Black;
      FastLED.show();
      delay(50);
      T -= 30;
    }
  }
  for(uint32_t T = 1000; T > 200;){
    for(uint8_t i = 0; i < NUM_LEDS / 2; i ++){
      leds[i] = CRGB::Green;
      leds[NUM_LEDS - i] = CRGB::Green;
      FastLED.show();
      delay(T);
      leds[i] = CRGB::Black;
      leds[NUM_LEDS - i] = CRGB::Black;
      FastLED.show();
      delay(50);
      T -= 50;
    }
  }
  for(uint8_t i = 0; i <= NUM_LEDS / 2; i ++){
      leds[i] = CRGB::Blue;
      leds[NUM_LEDS - i] = CRGB::Blue;
      FastLED.show();
      delay(100);
    }
  delay(1000);  */
}//彩色流水灯

void turn(){
  //for(int i = 0;i < 20;i ++){
    //for(int j = NUM_LEDS / 2;j >= 0;j --){
    static int i = 0;
    static int j = NUM_LEDS / 2;
      leds[j] = ColorFromPalette(RainbowColors_p,i * 100 % 255,128,LINEARBLEND);
      leds[NUM_LEDS - j] = ColorFromPalette(RainbowColors_p,i * 100 % 255,128,LINEARBLEND);
      FastLED.show();
      delay(50);
    j --;
    if(j < 0){
      j = NUM_LEDS / 2;
      i ++;
      if(i > 20) i = 0;
    }
    //}
  //}
}//翻转

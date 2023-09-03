#include<SPI.h> 
#include <LiquidCrystal_I2C.h> 
LiquidCrystal_I2C lcd (0x27, 16,2); // Создание объекта lcd для работы с ЖК-дисплеем.
#define FSYNC 4 // Определение пина FSYNC.

#define WAVE_SINE     0x2000 // Определение типов волн: синусоидальная, прямоугольная, треугольная.
#define WAVE_SQUARE   0x2028
#define WAVE_TRIANGLE 0x2002
// Определение пинов кнопок для управления частотой и функцией генератора.
#define b_UP    5 
#define b_DOWN  17
#define b_MULTI 15
#define b_FUNC  1

long int counter = 0;  // Установка начальной частоты.
long int counter_ant = 0; // Переменная для хранения предыдущей частоты.
long int function = 0; // Переменная для хранения типа волны (синусоидальная, прямоугольная, треугольная).
long int function_ant = 0; // Переменная для хранения предыдущего типа волны.
int multi = 0; // Переменная для хранения текущей степени десяти.
int func = 0; // Переменная для хранения текущей функции.
int func_ant = 0; // Переменная для хранения предыдущей функции.
const byte pwm_pin = 25; // Установка пина для ШИМ мигнала.
const byte pot_pin = 12; // Установка пина потенциометра.
int potValue = analogRead(pot_pin); // Считываем показания с потенциометра.
int duty = map(potValue, 0, 4095, 0, 255); // Определяем скважность ШИМ.



void AD9833setup(){ // Функция настройки генератора.
   pinMode(FSYNC, OUTPUT); // Установка пина FSYNC на выход.
   digitalWrite(FSYNC, HIGH); 
   SPI.begin(); 
   delay(50); 
   AD9833reset(); 
}

void AD9833reset(){ // Функция сброса генератора.
   WriteRegister(0x100); // Отправка сигнала сброса.
   delay(10); // Задержка 10 миллисекунд.
}

void AD9833setFrequency(long frequency, int Waveform){ // Функция установки частоты и типа волны.
   long FreqWord = (frequency * pow(2, 28)) / 25.0E6; // Расчет значения частоты.
   int MSB = (int)((FreqWord & 0xFFFC000) >> 14); 
   int LSB = (int)(FreqWord & 0x3FFF); 
   LSB |= 0x4000;
   MSB |= 0x4000; 
   WriteRegister(0x2100);   
   WriteRegister(LSB);
   WriteRegister(MSB);
   WriteRegister(0xC000);
   WriteRegister(Waveform);
}
//записываем данные в AD9833
void WriteRegister(int dat){ 
   SPI.setDataMode(SPI_MODE2);        
   digitalWrite(FSYNC, LOW);
   delayMicroseconds(10); 
   SPI.transfer(dat>>8);
   SPI.transfer(dat&0xFF);
   digitalWrite(FSYNC, HIGH);
   SPI.setDataMode(SPI_MODE0);
}

void setup(){
   ledcAttachPin (pwm_pin,0); // Определения канала ШИМ.
   ledcSetup(0,10000,8); // Настройки ШИМ сигнала (10 кГц, 8 бит).
   Serial.begin(9600);
   AD9833setup();
   pinMode(b_UP, INPUT_PULLUP);
   pinMode(b_DOWN,INPUT_PULLUP);
   pinMode(b_MULTI,INPUT_PULLUP);
   pinMode(b_FUNC,INPUT_PULLUP);
   //инциализация дисплея
   lcd.init ();
   lcd.setCursor(0, 0);
   lcd.print("Wafeform Generat");
   lcd.backlight ();
   lcd.setCursor(1, 3);
   lcd.print("ChepurkoM 2023");
   delay(5000);
   lcd.clear();

}
//обновление информации 
void actualiza_func(){
   if(!digitalRead(b_FUNC)){
      delay(50);
      if(!digitalRead(b_FUNC)){ 
         if(func_ant == 0)
            func = 1;
         if(func_ant == 1)
            func = 2;
         if(func_ant == 2)
            func = 0;
         func_ant = func;
         if(func_ant == 3)
            func = 3;
         func_ant = func;
      }
   }
   if(func == 0){
      lcd.setCursor(6, 1);
      lcd.print("      SINE");
      function = WAVE_SINE;
   }
   if(func == 1){
      lcd.setCursor(6, 1);
      lcd.print("  TRIANGLE");
      function = WAVE_TRIANGLE;
   }
   if(func == 2){
      lcd.setCursor(6, 1);
      lcd.print("  SQUARE");
      function = WAVE_SQUARE;
   }
   if(func == 3){
      lcd.setCursor(6, 1);
      lcd.print("     PWM");
      function = WAVE_SQUARE;
   }
   if(counter_ant != counter || function_ant != function){
      AD9833setFrequency(counter, function);
   }
   counter_ant = counter;
   function_ant = function;
}

void limpiaDigitos(){
   if(counter < 100){
      lcd.setCursor(2, 0);
      lcd.print("      ");  
   }else{
      if(counter < 1000){
         lcd.setCursor(3, 0);
         lcd.print("     "); 
      }else{
         if(counter < 10000){
            lcd.setCursor(4, 0);
            lcd.print("    "); 
         }else{
            if(counter < 100000){
               lcd.setCursor(5, 0);
               lcd.print("   "); 
            }else{
               if(counter < 1000000){
                  lcd.setCursor(6, 0);
                  lcd.print("  "); 
               }else{
                  if(counter < 1999000){
                     lcd.setCursor(7, 0);
                     lcd.print(" "); 
                  }
               }
            }
         }
      }                
   }
}
void PWM(){
    int potValue = analogRead(pot_pin); // Считываем показания с потенциометра.
    int duty = map(potValue, 0, 4095, 0, 255); // Определяем скважность ШИМ.
    ledcWrite(0,duty);
}
void loop(){ 
   PWM();
   switch(multi){
   case 0:
      if(!digitalRead(b_MULTI)){ 
         delay(50);
         if(!digitalRead(b_MULTI)){ 
            multi = 1;
         }
      }
      if(!digitalRead(b_UP)){ 
         delay(50);
         if(!digitalRead(b_UP)){ 
            counter++;
         }
      }
      if(!digitalRead(b_DOWN)){ 
         delay(50);
         if(!digitalRead(b_DOWN)){ 
            if(counter>0)
               counter--;
         }
      }
      lcd.setCursor(0, 1);
      lcd.print("10^0");
      lcd.setCursor(14, 0);
      lcd.print("Hz");
      lcd.setCursor(0, 0);
      lcd.print(counter);
      limpiaDigitos();
      actualiza_func();
      break;
   case 1:
      if(!digitalRead(b_MULTI)){ 
         delay(50);
         if(!digitalRead(b_MULTI)){ 
            multi = 2;
         }
      }
      if(!digitalRead(b_UP)){ 
         delay(50);
         if(!digitalRead(b_UP)){ 
            counter = counter + 1000;
         }
      }
      if(!digitalRead(b_DOWN)){ 
         delay(50);
         if(!digitalRead(b_DOWN)){ 
            if(counter>1000)
               counter = counter - 1000;
         }
      }
      lcd.setCursor(0, 1);
      lcd.print("10^3");
      lcd.setCursor(14, 0);
      lcd.print("Hz");
      lcd.setCursor(0, 0);
      lcd.print(counter);
      limpiaDigitos();
      actualiza_func();
      break;
   case 2:
      if(!digitalRead(b_MULTI)){ 
         delay(50);
         if(!digitalRead(b_MULTI)){ 
            multi = 0;
         }
      }
      if(!digitalRead(b_UP)){ 
         delay(50);
         if(!digitalRead(b_UP)){ 
            if(counter<1000000)
               counter = counter + 1000000;
         }
      }
      if(!digitalRead(b_DOWN)){ 
         delay(50);
         if(!digitalRead(b_DOWN)){ 
            if(counter>1000000)
               counter = counter - 1000000;
         }
      }
      lcd.setCursor(0, 1);
      lcd.print("10^6");
      lcd.setCursor(14, 0);
      lcd.print("Hz");
      lcd.setCursor(0, 0);
      lcd.print(counter);
      limpiaDigitos();
      actualiza_func();
      break;
   }
}

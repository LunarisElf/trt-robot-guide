// ไฟล์รวม: TRT_arduino_nano.ino + Mission.ino อยู่ในไฟล์เดียว
// เส้นทางแข่งเขียนใน Mission() (ท้ายไฟล์) เหมือนเดิม
// สร้างจากไฟล์ต้นฉบับวันที่ 17 พ.ย. ถ้าแก้ไฟล์ต้นฉบับ ต้องรวมใหม่

#include <EEPROM.h>
#include <Servo.h>

#define BUZZER_PIN 12
#define PWMA 5
#define AIN1 3
#define AIN2 4
#define PWMB 6
#define BIN1 7
#define BIN2 8

Servo servoup;
Servo servoL;
Servo servoR;

unsigned long start_time, real_time;

float Kp = 20, Ki = 0, Kd = 1;
float P = 0, I = 0, D = 0, PID_value = 0;
float previous_error = 0, previous_I = 0;
int error = 0;

int initial_motor_speed = 200;
int speed_L = 120;
int speed_R = 120;
int SPEED;
bool stopped = true;              // true = หุ่นหยุดอยู่ (B() ตั้งค่า) ออกตัวครั้งหน้าจะเร่งนุ่ม
unsigned long t_go = 0;           // เวลาที่เริ่มออกตัวรอบล่าสุด
int RAMP_MS = 250;                // เวลาเร่งจาก 0 ถึงความเร็วเต็ม ms (ยังยกหน้า = เพิ่มค่านี้)

int left_motor_speed;
int right_motor_speed;
int speed_cross = 100;
int speed_trun = 150;
int trim_L = 10;  // ลดความเร็วล้อซ้าย (วัด 2026-11-16 ที่ 240: เบ้ขวา ลด 10 แล้ววิ่งตรง)
int trim_R = 0;   // ลดความเร็วล้อขวา (ใส่เมื่อล้อขวาเร็วกว่า)
unsigned long time_cross = 300;

int val_min[6], val_max[6], val_sensor[6];
int S[5];
int C[5];

void calculate_pid(void);
void motor_control(void);
void Mission(void);   // ฟังก์ชันอยู่ท้ายไฟล์นี้

void setup() {
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(2, INPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  // A6/A7 บน Nano ใช้เป็น analog ได้อย่างเดียว ถ้าสั่ง pinMode() จะอ่านเกินตารางขา เลยลบออก

  Serial.begin(9600);
  Serial.println();

  if (digitalRead(2) == 1) {
    // โหมด calibrate แบบหมุนกวาด: วางหุ่นคร่อมเส้นดำ กดปุ่มค้างตอนเปิดเครื่อง แล้วปล่อย
    Serial.println(" Mode calibrate (sweep) ");
    beeb();
    while (digitalRead(2) == 1) {}   // รอปล่อยปุ่ม
    delay(500);                       // เอามือออกจากหุ่นก่อนเริ่มหมุน
    for (int i = 0; i < 5; i++) { val_min[i] = 1023; val_max[i] = 0; }
    analogWrite(PWMA, speed_trun);
    analogWrite(PWMB, speed_trun);
    unsigned long t0 = millis();
    // ponytail: กวาดตามเวลา ถ้าเซนเซอร์ริมไม่ผ่านเส้นให้เพิ่ม 750 หรือ speed_trun
    while (millis() - t0 < 3000) {
      if ((millis() - t0) / 750 % 2 == 0) drive_motor(1, 0, 0, 1);  // หมุนซ้าย
      else drive_motor(0, 1, 1, 0);                                 // หมุนขวา
      read_sensor();
      for (int i = 0; i < 5; i++) {
        val_min[i] = min(val_min[i], C[i]);
        val_max[i] = max(val_max[i], C[i]);
      }
    }
    STOP(100);
    bool ok = true;
    for (int i = 0; i < 5; i++) {
      Serial.print(val_min[i]); Serial.print("/"); Serial.print(val_max[i]); Serial.print("  ");
      if (val_max[i] - val_min[i] < 100) ok = false;   // ตัวนี้ไม่เคยเห็นเส้นดำ
    }
    Serial.println();
    if (ok) {
      for (int i = 0; i < 5; i++) EEPROM.write(i, (val_min[i] + val_max[i]) / 20);
      for (int k = 0; k < 3; k++) { beeb(); delay(400); }   // สำเร็จ: beep สั้น 3 ครั้ง
    } else {
      tone(BUZZER_PIN, 2000, 1500);   // ล้มเหลว: เสียงยาวครั้งเดียว ไม่บันทึกค่า
    }
    while (1) {}
  }

  Serial.println("_____________________________START_____________________________");
  Serial.print("sensor is : ");
  for (int i = 0 ; i <= 4 ; i++) {
    S[i] = (EEPROM.read(i)) * 10;
    Serial.print(S[i]);
    Serial.print("  ");
  }
  tone(BUZZER_PIN, 2000, 80);
  delay(100);
  tone(BUZZER_PIN, 1000, 80);
  delay(100);
  tone(BUZZER_PIN, 2000, 150);
  delay(100);
  STOP(200);

  SW();
  STOP(200);
  Mission();
  STOP(1000);
  //b_megalovania();   // เล่นเพลงตอนจบ mission (หุ่นหยุดนิ่งแล้ว)
}

void loop() {
  // put your main code here, to run repeatedly:

}
//____________________________________ควบคุมการหมุนของ motor โดยใช้ delay ______________________________//
void read_sensor() {
  C[0] = analogRead(A0);
  C[1] = analogRead(A1);
  C[2] = analogRead(A2);
  C[3] = analogRead(A3);
  C[4] = analogRead(A4);
}
void cheack_error() {
  if (C[1] > S[1] && C[2] < S[2] && C[3] > S[3]) error = 0;
  else if (C[2] < S[2] && C[1] < S[1]) error = 1;
  else if (C[2] < S[2] && C[3] < S[3]) error = -1;
  else if (C[1] < S[1]) error = 5;
  else if (C[3] < S[3]) error = -5;
  else if (C[0] < S[0]) error = 7;
  else if (C[4] < S[4]) error = -7;
  else {
    if (error >= 7)error = 9;
    else if (error <= -7)error = -9;
    else if (error >= 5)error = 6;
    else if (error <= -5)error = -6;
    else error = 0;
  }
}
void cheack_error_down() {
  // ใช้แค่ C[1]..C[3] ไม่สนเซนเซอร์ริม เพราะตอนข้ามแยกมันเห็นแขนของแยก
  if (C[1] < S[1] && C[3] < S[3]) error = 0;
  else if (C[2] < S[2] && C[1] < S[1]) error = 1;
  else if (C[2] < S[2] && C[3] < S[3]) error = -1;
  else if (C[1] < S[1]) error = 3;
  else if (C[3] < S[3]) error = -3;
  else error = 0;
}
void calculate_pid() {
  P = error;
  I = previous_I + error;   // สะสมค่า error (เดิมเป็น I + previous_I ซึ่งไม่เคยสะสม error)
  D = error - previous_error;
  PID_value = (Kp * P) + (Ki * I) + (Kd * D);
  previous_I = I;
  previous_error = error;
}
// เร่งนุ่ม: ช่วง RAMP_MS แรกหลังออกตัว PWM ค่อยๆ ขึ้นจาก 0 ถึง v
// ทุกล้อขึ้นเป็นสัดส่วนเดียวกัน ไม่ทำให้หุ่นเบ้
int ramp(int v, unsigned long t) {
  return t < (unsigned long)RAMP_MS ? (long)v * t / RAMP_MS : v;
}
void motor_control() {
  if (stopped) { stopped = false; t_go = millis(); }
  int base = ramp(initial_motor_speed, millis() - t_go);
  right_motor_speed = base + PID_value - trim_R;
  left_motor_speed  = base - PID_value - trim_L;
  if (left_motor_speed > 255) left_motor_speed = 255;
  else if (left_motor_speed < 0) left_motor_speed = 0;
  if (right_motor_speed > 255) right_motor_speed = 255;
  else if (right_motor_speed < 0) right_motor_speed = 0;
  if (error == 9) {
    analogWrite(PWMB, right_motor_speed);
    analogWrite(PWMA, right_motor_speed);
    drive_motor(1, 0, 0, 1);
  }
  else if (error == -9) {
    analogWrite(PWMB, left_motor_speed);
    analogWrite(PWMA, left_motor_speed);
    drive_motor(0, 1, 1, 0);
  }
  else {
    analogWrite(PWMB, left_motor_speed);
    analogWrite(PWMA, right_motor_speed);
    drive_motor(1, 0, 1, 0);
    
  }
}

void motor_control_back() {
  // กลับด้านผลของ PID เพราะตอนนี้เราถอยหลัง
  right_motor_speed = initial_motor_speed - PID_value - trim_R;
  left_motor_speed  = initial_motor_speed + PID_value - trim_L;

  if (left_motor_speed > 255) left_motor_speed = 255;
  else if (left_motor_speed < 0) left_motor_speed = 0;

  if (right_motor_speed > 255) right_motor_speed = 255;
  else if (right_motor_speed < 0) right_motor_speed = 0;

  // เคสหลุดเส้นหนัก ๆ ใช้หมุนหาทิศทาง
  // speed-80 ติดลบได้ และ analogWrite(ค่าติดลบ) จะวนกลับเป็นความเร็วสูง เลยจำกัดไม่ให้ต่ำกว่า 0
  int back_L = max(left_motor_speed - 80, 0);
  int back_R = max(right_motor_speed - 80, 0);

  if (error == 9) {
    // หมุนทิศเดิม แต่ตอนนี้เราอยู่ในโหมดถอยหลัง
    analogWrite(PWMB, back_R);
    analogWrite(PWMA, back_R);
    drive_motor(1, 0, 0, 1);
  }
  else if (error == -9) {
    analogWrite(PWMB, back_L);
    analogWrite(PWMA, back_L);
    drive_motor(0, 1, 1, 0);
  }
  else {
    // เคสปกติ = ถอยหลังตามเส้น
    analogWrite(PWMB, back_L);
    analogWrite(PWMA, back_R);
    // ทิศทางถอยหลัง
    drive_motor(0, 1, 0, 1);   // = s();
  }
}


void cheak_sensor() {
  while (1) {
    read_sensor();
    for (int i = 0 ; i <= 4 ; i++) {
      Serial.print(C[i]);
      Serial.print("  ");
    }
    Serial.println();
  }
}
void follow_line() {
  calculate_pid();
  motor_control();
}



void follow_line_back() {
  calculate_pid();
  motor_control_back();
}

void BL() {
  read_sensor();
  while (C[0] > S[0] || C[4] > S[4]) {
    cheack_error();
    follow_line_back();
    read_sensor();
  }
  PID_speed_reset();
}

void BLR() {
  read_sensor();
  while (C[4] > S[4]) {
    cheack_error();
    follow_line_back();
    read_sensor();
  }
  PID_speed_reset();
}

void BLL() {
  read_sensor();
  while (C[0] > S[0]) {
    cheack_error();
    follow_line_back();
    read_sensor();
  }
  PID_speed_reset();
}

void BL_time(unsigned long x) {
  start_time = millis();
  while (millis() - start_time < x) {
    read_sensor();
    cheack_error();
    follow_line_back();
  }
  PID_speed_reset();
}


void FL() {
  read_sensor();
  while (C[0] > S[0] || C[4] > S[4]) {
    cheack_error();
    follow_line();
    read_sensor();
  }
  PID_speed_reset();
}

void FLR() {
  read_sensor();
  while (C[4] > S[4]) {
    cheack_error();
    follow_line();
    read_sensor();
  }
  PID_speed_reset();
}

void FLR2() {
  int hitCount = 0;
  bool onLine = false;      // true ถ้า C[4] กำลังอยู่บนเส้นอยู่ตอนนี้

  read_sensor();

  while (hitCount < 2) {
    cheack_error();
    follow_line();
    read_sensor();

    bool nowOnLine = (C[4] <= S[4]);    // เห็นเส้นเมื่อ C[4] <= S[4]

    // ตรวจจับ "transition" จากไม่อยู่บนเส้น -> อยู่บนเส้น
    if (!onLine && nowOnLine) {
      hitCount++;
      // ถ้าถึงครั้งที่ 2 แล้ว ก็ออกเลย
      if (hitCount >= 2) break;
    }

    onLine = nowOnLine;
  }

  PID_speed_reset();
}

void FLL2() {
  int hitCount = 0;
  bool onLine = false;      // true ถ้า C[0] กำลังอยู่บนเส้นอยู่ตอนนี้

  read_sensor();

  while (hitCount < 2) {
    cheack_error();
    follow_line();
    read_sensor();

    bool nowOnLine = (C[0] <= S[0]);    // เห็นเส้นเมื่อ C[0] <= S[0]

    // ตรวจจับ "transition" จากไม่อยู่บนเส้น -> อยู่บนเส้น
    if (!onLine && nowOnLine) {
      hitCount++;
      if (hitCount >= 2) break;
    }

    onLine = nowOnLine;
  }

  PID_speed_reset();
}

void FLL() {
  read_sensor();
  while (C[0] > S[0]) {
    cheack_error();
    follow_line();
    read_sensor();
  }
  PID_speed_reset();
}
void FL_time(unsigned long x) {
  start_time = millis();
  real_time = millis();
  while (real_time - start_time < x) {
    read_sensor();
    cheack_error();
    follow_line();
    real_time = millis();
  }
  PID_speed_reset();
}
void TL() {
  analogWrite(PWMB, speed_trun);
  analogWrite(PWMA, speed_trun);
  read_sensor();
  while (C[0] >  S[0]) {
    drive_motor(1, 0, 0, 1);
    read_sensor();
  }
  analogWrite(PWMB, speed_trun * 2 / 3);   // ช่วงเข้าหาเส้นด้วย C[2] ลดความเร็ว กันหมุนเลยเส้น
  analogWrite(PWMA, speed_trun * 2 / 3);
  read_sensor();
  while (C[2] < S[2]) {   // C[2] ต้องพ้นเส้นหลักก่อน กันกรณี C[0] ทับกิ่งตั้งแต่เริ่ม แล้วหุ่นไม่เลี้ยว
    drive_motor(1, 0, 0, 1);
    read_sensor();
  }
  while (C[2] > S[2]) {   // เบรกตอนเส้นอยู่กลาง (C[2])
    drive_motor(1, 0, 0, 1);
    read_sensor();
  }
  B();
  PID_speed_reset();
}
void TR() {
  analogWrite(PWMB, speed_trun);
  analogWrite(PWMA, speed_trun);
  read_sensor();
  read_sensor();
  while (C[4] > S[4]) {
    drive_motor(0, 1, 1, 0);
    read_sensor();
  }
  analogWrite(PWMB, speed_trun * 2 / 3);   // ช่วงเข้าหาเส้นด้วย C[2] ลดความเร็ว กันหมุนเลยเส้น
  analogWrite(PWMA, speed_trun * 2 / 3);
  read_sensor();
  while (C[2] < S[2]) {   // C[2] ต้องพ้นเส้นหลักก่อน กันกรณี C[4] ทับกิ่งตั้งแต่เริ่ม แล้วหุ่นไม่เลี้ยว
    drive_motor(0, 1, 1, 0);
    read_sensor();
  }
  while (C[2] > S[2]) {   // เบรกตอนเส้นอยู่กลาง (C[2])
    drive_motor(0, 1, 1, 0);
    read_sensor();
  }
  B();
  PID_speed_reset();
}
void cross() {
  start_time = millis();
  real_time = millis();
  while (real_time - start_time < 200) {
    read_sensor();
    cheack_error();
    follow_line();
    real_time = millis();
  }
  PID_speed_reset();
}
void cross_down() {
  initial_motor_speed = speed_cross;
  start_time = millis();
  real_time = millis();
  while (real_time - start_time < time_cross) {
    read_sensor();
    cheack_error_down();
    follow_line();
    real_time = millis();
  }
  PID_speed_reset();
  B();   // เบรก 50 ms พอ ไม่ต้องรอเพิ่ม (เดิม STOP(100))
}

void bcross_down() {
  initial_motor_speed = speed_cross + 80;   // back ลบ 80
  start_time = millis();
  while (millis() - start_time < time_cross) {
    read_sensor();          // อ่านเซนเซอร์
    cheack_error_down();    // ใช้ logic cross_down เดิม
    follow_line_back();     // แต่ตามเส้นแบบถอยหลัง
  }
  PID_speed_reset();
  B();   // เดิม STOP(100)
}


void R(int x) {
  B();
  analogWrite(PWMA, speed_L);
  analogWrite(PWMB, speed_R);
  drive_motor(0, 1, 1, 0);
  
  delay(x);
  B();
  PID_speed_reset();
}
void L(int x) {
  B();
  analogWrite(PWMA, speed_L);
  analogWrite(PWMB, speed_R);
  drive_motor(1, 0, 0, 1);
  delay(x);
  B();
  PID_speed_reset();
}
void GO(int y) {
  B();
  w();
  unsigned long t0 = millis(), t;
  while ((t = millis() - t0) < (unsigned long)y) {   // เร่งนุ่ม กันหน้ายก
    analogWrite(PWMA, ramp(speed_L, t));
    analogWrite(PWMB, ramp(speed_R, t));
  }
  B();
  PID_speed_reset();
}
void GB(int y) {
  B();
  analogWrite(PWMA, speed_L);
  analogWrite(PWMB, speed_R);
  s();
  delay(y);
  B();
  PID_speed_reset();
}
// ถอยหลังตามเส้น x ms แล้วหยุด (ใช้แทน GB ตอนถอยออกจากกิ่ง จะได้ถอยตรงตามเส้น)
// ความเร็วฐาน = speed_L เท่ากับ GB เวลาที่จูนไว้กับ GB ใช้ต่อได้
// ใช้ C[1]-C[3] จับเส้น ไม่สนเซนเซอร์ริม ถอยผ่านหัว T หรือจุดแยกจะไม่ส่าย
void GBL(unsigned long x) {   // ไม่เบรกตอนเริ่ม: push() เบรกมาแล้ว (ถ้าเรียกตอนหุ่นวิ่งอยู่ ให้ใส่ B() ก่อน)
  initial_motor_speed = speed_L + 80;   // motor_control_back ลบ 80 ออก เลยบวกคืน ให้ถอยจริงที่ speed_L
  start_time = millis();
  while (millis() - start_time < x) {
    read_sensor();
    cheack_error_down();
    follow_line_back();
  }
  PID_speed_reset();
  B();
}
void M_A(int y) {
  B();
  analogWrite(PWMA, speed_L);
  analogWrite(PWMB, speed_R);
  drive_motor(1, 1, 1, 0);
  delay(y);
  B();
  PID_speed_reset();
}
void M_AB(int y) {
  B();
  analogWrite(PWMA, speed_L);
  analogWrite(PWMB, speed_R);
  drive_motor(1, 1, 0, 1);
  delay(y);
  B();
  PID_speed_reset();
}
void M_DB(int y) {
  B();
  analogWrite(PWMA, speed_L);
  analogWrite(PWMB, speed_R);
  drive_motor(0, 1, 1, 1);
  delay(y);
  B();
  PID_speed_reset();
}
void M_D(int x) {
  B();
  analogWrite(PWMA, speed_L);
  analogWrite(PWMB, speed_R);
  drive_motor(1, 0, 1, 1);
  delay(x);
  B();
  PID_speed_reset();
}
void STOP(int x) {
  B();
  delay(x);
}
void Set_kp(int x) {
  Kp = x;
}
void Speed_motor(int x) {
  SPEED = x;
  initial_motor_speed = SPEED;
  analogWrite(PWMB, SPEED);
  analogWrite(PWMA, SPEED);
}
void Speed_LR(int x, int y) {
  speed_L = x;
  speed_R = y;
}
void PID_speed_reset() {
  P = 0;
  I = 0;
  D = 0;
  error = 0;
  PID_value = 0;
  previous_error = 0;
  previous_I = 0;
  initial_motor_speed = SPEED;
}
void SW() {
  B();
  while (1) {
    int sw = digitalRead(2);   // เดิมชื่อ "S" ซึ่งไปบังตัวแปร S[] (ค่า threshold) ที่ประกาศไว้ด้านบน
    if (sw == 1) {
      tone(BUZZER_PIN, 2000, 80);
      delay(100);
      tone(BUZZER_PIN, 1000, 80);
      delay(100);
      tone(BUZZER_PIN, 2000, 80);
      delay(100);
      tone(BUZZER_PIN, 2500, 80);
      delay(100);
      break;
    }
  }
}
void speed_LR(int x, int y) {   // ชื่อตัวเล็กของ Speed_LR() ทำงานเหมือนกัน
  Speed_LR(x, y);
}
void drive_motor(int IN1, int IN2, int IN3, int IN4) {
  digitalWrite(AIN1, IN1);
  digitalWrite(AIN2, IN2);
  digitalWrite(BIN1, IN3);
  digitalWrite(BIN2, IN4);
}
void w() {
  drive_motor(1, 0, 1, 0);
}
void s() {
  drive_motor(0, 1, 0, 1);
}
void a() {
  drive_motor(0, 0, 1, 0);
}
void aa() {
  drive_motor(1, 1, 1, 0);
}
void d() {
  drive_motor(1, 0, 0, 0);
}
void dd() {
  drive_motor(1, 0, 1, 1);
}
void B() {
  drive_motor(1, 1, 1, 1);
  delay(50);
  drive_motor(0, 0, 0, 0);
  stopped = true;   // ออกตัวครั้งหน้าจะเร่งนุ่ม
}
void r() {
  drive_motor(1, 0, 0, 1);
}
void l() {
  drive_motor(0, 1, 1, 0);
}
void b_beebb() {
  tone(BUZZER_PIN, 2000, 80);
  delay(100);
  tone(BUZZER_PIN, 1000, 80);
  delay(100);
  tone(BUZZER_PIN, 2000, 80);
  delay(100);
  tone(BUZZER_PIN, 2500, 80);
  delay(100);
}
void b_beeb() {
  tone(BUZZER_PIN, 2000, 80);
  delay(100);
  tone(BUZZER_PIN, 1000, 80);
  delay(100);
  tone(BUZZER_PIN, 2000, 80);
  delay(100);
}
void beeb() {
  tone(BUZZER_PIN, 2000, 200);
}

//____________________________________ เพลง Megalovania (ท่อนเปิด) ______________________________//
#define N_AS3 233
#define N_B3  247
#define N_C4  262
#define N_D4  294
#define N_F4  349
#define N_G4  392
#define N_GS4 415
#define N_A4  440
#define N_D5  587

// ห้องละ 10 โน้ต: 2 โน้ตแรกเปลี่ยนไปทุกห้อง (D, C, B, Bb) ส่วนที่เหลือเหมือนเดิม
const int mega_bass[4] = { N_D4, N_C4, N_B3, N_AS3 };
const int mega_rest[8] = { N_D5, N_A4, N_GS4, N_G4, N_F4, N_D4, N_F4, N_G4 };
const byte mega_len[10] = { 1, 1, 2, 3, 2, 2, 2, 1, 1, 1 };   // ความยาวเป็นหน่วยโน้ต 1/16

void b_megalovania() {
  const int u = 125;                    // 1/16 = 125ms (120 BPM) น้อย = เร็วขึ้น
  for (int bar = 0; bar < 4; bar++) {
    for (int i = 0; i < 10; i++) {
      int note = (i < 2) ? mega_bass[bar] : mega_rest[i - 2];
      int len = mega_len[i] * u;
      tone(BUZZER_PIN, note, len - 20); // เว้น 20ms ให้โน้ตแยกกันชัด
      delay(len);
    }
  }
  noTone(BUZZER_PIN);
}

//____________________________________ Mission (เดิมอยู่ในไฟล์ Mission.ino) ______________________________//
void Mission() {
  ///______________________________________พารามิเตอร์__________________________________________________//
  Set_kp(30);               //ค่าการเปลี่ยนความเร็ว motor ตอนวิ่งตามเส้น (เส้นตรง 10-20 , เส้นโค้งเยอะๆ 20-30)
  Speed_motor(255);         //ความเร็ว motor วิ่งตามเส้น (ทางตรง 150-220 , ทางโค้ง 100-120)
  Speed_LR(120, 120);       //ความเร็วถอยตามเส้น GBL (และ GO/GB)
  speed_trun = 150;         //ความเร็ว motor เลี้ยวเข้าเส้นแบบใช้ sensor (TR,TL)
  speed_cross = 95;        //ความเร็ว motor เลี้ยวข้ามแยกเพื่อเลี้ยว (FL_CD,FLL_CD,FLR_CD)
  time_cross = 175;         //เวลาเดิน motor เลี้ยวข้ามแยกเพื่อเลี้ยว (FL_CD,FLL_CD,FLR_CD)
  ///______________________________________เขียนโปรแกรมภายในคอมเม้น__________________________________________________//
  FL(); cross_down(); can_LR(); FL_time(1700); FLL(); cross_down(); TL(); push(); TR(); // 1
  FL(); cross(); TR(); FL(); cross(); FL(); cross(); FL(); cross(); FL(); cross_down(); can_LR(); // 2
  FL_time(1000); FLL(); cross_down(); TL(); push(); TR(); FL(); cross_down(); TL();          // 3
  FL(); cross_down(); can_R(); FL_time(1800); FLL(); cross_down(); can_L(); FL(); cross();
  
  return;

  FL();  cross();                     // ขอบบนกล่องเริ่ม แล้ววิ่งเลยขอบไปนิด
  FLR(); cross_down(); can_R();                         // 1
  FL();  cross_down(); can_L();                         // 2
  FL_time(2000); FLR(); cross_down(); can_R();          // 3
  FL();  cross_down(); can_LR();                        // 4-5
  FL();  cross_down(); can_LR();                        // 6-7
  FL_time(2000); FLL(); cross_down();   TL(); push(); TR();         // 8
  FL(); cross();                                        // end

  
 }///______________________________________***ระวัง อย่าเผลอลบปีกกา__________________________________________________//
 void slow() {
  Speed_motor(120); Set_kp(20);
 }
 void normal() {
  Set_kp(20); Speed_motor(240);
 }
 void push() {
  Speed_motor(240);
  start_time = millis();
  while (millis() - start_time < 300) {
    read_sensor();
    cheack_error_down();   // พ้นปลายเส้น (ขาวหมด) = error 0 วิ่งตรง ไม่เลี้ยว/หมุนตามค่าที่จำไว้
    follow_line();
  }
  PID_speed_reset();
  B();
 }
 
void FL_C() {
  FL();
  cross();
}
void FLL_C() {
  FLL();
  cross();
}
void FLR_C() {
  FLR();
  cross();
}
void FL_CD() {
  FL();
  cross_down();
}
void FLL_CD() {
  FLL();
  cross_down();
}
void FLR_CD() {
  FLR();
  cross_down();
}
void FL_L() {
  FL();
  cross_down();
  TL();
}
void FL_R() {
  FL();
  cross_down();
  TR();
}
void FLL_L() {
  FLL();
  cross_down();
  TL();
}
void FLL_R() {
  FLL();
  cross_down();
  TR();
}
void FLR_L() {
  FLR();
  cross_down();
  TL();
}
void FLR_R() {
  FLR();
  cross_down();
  TR();
}

void can_L()  { TL(); push(); TR(); FL(); cross_down(); TL(); }   // ดันแล้วกลับหลัง วิ่งกลับถึงแยก เลี้ยวไปทางเดิม
void can_R()  { TR(); push(); TR(); FL(); cross_down(); TR(); }
void can_LR() {
  TL(); push(); TR(); FL(); cross_down();   // ดันซ้าย กลับหลัง วิ่งข้ามแยกตรงเข้ากิ่งขวา
  push(); TR(); FL(); cross_down(); TR();   // ดันขวา กลับหลัง กลับถึงแยก เลี้ยวไปทางเดิม
}


// ===== โหมดหาค่าล้อ =====
// วิธีใช้: วางหุ่นคร่อมเส้นตรง กดปุ่ม 1 ครั้ง = วิ่งทดสอบ 1 รอบ แล้วหุ่นหยุดรอ
// วางหุ่นกลับที่เดิม กดปุ่มอีกครั้ง = ลองค่าถัดไป นับว่ากดครั้งที่เท่าไรหุ่นถึงวิ่งตรง

// ชุด A: ถอยหลัง 1.5 วิ ล้อซ้าย 100 คงที่ เพิ่มล้อขวาทีละ 5 (ฝั่งขวาอ่อนกว่า)
// กดครั้งที่ 1=100  2=105  3=110  4=115  5=120  6=125  7=130  8=135  9=140
// ครั้งที่ถอยตรง -> เอาเลขนั้นไปใส่ Speed_LR(เลขนั้น, 100)
void tune_GB() {
  for (int right = 100; right <= 140; right += 5) {
    Serial.println(right);
    Speed_LR(right, 100);
    GB(1500);
    SW();   // รอกดปุ่มเพื่อลองค่าถัดไป
  }
}

// ชุด B: เดินหน้าแบบไม่จับเส้น 1.5 วิ แล้วหยุดรอ กดปุ่ม = ลองค่าถัดไป
// กดครั้งที่ 1=trim 0  2=5  3=10  4=15  5=20  6=25  7=30  8=35  9=40
// ครั้งที่ 1 (trim 0) = ล้อเท่ากัน ใช้ดูว่ามอเตอร์เบ้ไปทางไหน เท่าไร
// เบ้ขวา -> tune_trim(true)  ลดล้อซ้าย -> เลขที่วิ่งตรงใส่ trim_L
// เบ้ซ้าย -> tune_trim(false) ลดล้อขวา -> เลขที่วิ่งตรงใส่ trim_R
void tune_trim(bool lower_left) {
  const int SPD = SPEED;   // ใช้ความเร็วเดียวกับ Speed_motor() ใน Mission
  for (int t = 0; t <= 40; t += 5) {
    Serial.println(t);
    if (lower_left) Speed_LR(SPD, SPD - t);   // Speed_LR(ขวา, ซ้าย)
    else            Speed_LR(SPD - t, SPD);
    GO(1500);
    SW();   // รอกดปุ่มเพื่อลองค่าถัดไป
  }
}

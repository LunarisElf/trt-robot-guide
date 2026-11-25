#include <EEPROM.h>
#include <Servo.h>

#define BUZZER_PIN 12
#define PWMA 5
#define AIN1 3
#define AIN2 4
#define PWMB 6
#define BIN1 7
#define BIN2 8
#define TRIG_PIN 11   // ultrasonic Trig (สลับสายแล้ว)
#define ECHO_PIN 13   // ultrasonic Echo

Servo servoup;
Servo servoL;
Servo servoR;

unsigned long start_time, real_time;

float Kp = 20, Ki = 0, Kd = 1;
float Kp_gain = 0;   // Kp เพิ่มต่อหน่วย |error| (0 = ปิด)
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
int speed_fix = 90;   // ความเร็วหมุนส่ายหากระป๋อง
int trim_L = 0;  // ลดความเร็วล้อซ้าย (วัด 2026-11-16 ที่ 240: เบ้ขวา ลด 10 แล้ววิ่งตรง)
int trim_R = 0;   // ลดความเร็วล้อขวา (ใส่เมื่อล้อขวาเร็วกว่า)
unsigned long time_cross = 300;

int val_min[6], val_max[6], val_sensor[6];
int S[5];
int C[5];
long cm = 999;   // ระยะ ultrasonic ล่าสุด (cm)
bool seen_L, seen_R;   // cross_down รอบล่าสุดเห็นแขนซ้าย/ขวา
bool flj_L, flj_R;     // FLJ เห็นแขนซ้าย/ขวาตอนหยุด
int can_cm = 30;   // ระยะนับว่ามีกระป๋อง (cm)
bool coin_on = false;   // true = ดันกระป๋องเสร็จแล้วเล่นเสียงเหรียญ
const char *end_pat = "32112";   // ลำดับแยกสุดท้ายก่อนจอด (auto_run)
bool debug_beep = false;   // true = can_auto บี๊บบอกแยกที่เห็น

void calculate_pid(void);
void motor_control(void);
void Mission(void);   // ฟังก์ชันอยู่ในไฟล์ Mission.ino
int can_auto(int x = 0);        // อยู่ใน Mission.ino (ไม่ใส่เลข = can_cm)
void cheak_can_L(int x = 0);
void cheak_can_R(int x = 0);
void cheak_can_LR(int x = 0);

// ===== Funky Town เล่นเบื้องหลัง (คุณภาพต่ำ: เสียงเดียว สี่เหลี่ยม) =====
// tone() เล่นต่อเองด้วย Timer2 ไม่บล็อก; music_tick() แค่สลับโน้ตตามเวลา (เรียกจาก read_sensor)
bool music_on = false;
byte music_mul = 4;   // คูณความถี่: 1 = ต่ำสุด, 4 = สูงขึ้น 2 octave (บัซเซอร์ดังสุดแถว 2-3 kHz) ลอง 3-5
byte music_i = 0;
unsigned long music_next = 0;
const int  ft_f[] = {523, 523, 466, 523, 0, 392, 0, 392, 523, 698, 659, 523, 0};
const byte ft_u[] = {  2,   2,   2,   2, 2,   2, 2,   2,   2,   2,   2,   2, 8};  // หน่วย 1/16
void music_tick() {
  if (!music_on || millis() < music_next) return;
  int ms = ft_u[music_i] * 134;                       // ~112 BPM; ลด 134 = เร็วขึ้น
  if (ft_f[music_i]) tone(BUZZER_PIN, ft_f[music_i] * music_mul, ms - 30);
  music_next = millis() + ms;
  music_i = (music_i + 1) % 13;
}

// เกณฑ์เซนเซอร์ริมหลวมกว่ากลาง: ความเร็วสูงหัวยก ริมอ่านเส้นเป็นเทา
int edge_margin = 0;   // ปรับ 0-80; เพิ่ม = จับแยกง่ายขึ้น แต่โค้งหลอกง่ายขึ้น
bool seeR() { return C[4] < S[4] + edge_margin; }
bool seeL() { return C[0] < S[0] + edge_margin; }

#define ADC_DIV 0x05   // prescaler: 0x04=16 (1MHz) 0x05=32 (500kHz) 0x06=64 (250kHz) 0x07=128 (เดิม)
void setup() {
  ADCSRA = (ADCSRA & ~0x07) | ADC_DIV;   // ความเร็วอ่านเซนเซอร์ (ดูตารางที่ ADC_DIV) เปลี่ยนแล้วต้อง calibrate ใหม่
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
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
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
  //music_on = true;
  Mission();
  //music_on = false; noTone(BUZZER_PIN);
  STOP(1000);
  //b_megalovania();   // เล่นเพลงตอนจบ mission (หุ่นหยุดนิ่งแล้ว)
}

void loop() {
  // put your main code here, to run repeatedly:

}
//____________________________________ควบคุมการหมุนของ motor โดยใช้ delay ______________________________//
void read_sensor() {
  music_tick();
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
  float Kp_now = Kp + Kp_gain * abs(error);   // error ยิ่งมาก Kp ยิ่งสูง
  PID_value = (Kp_now * P) + (Ki * I) + (Kd * D);
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
  while (!seeR()) {
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

    bool nowOnLine = seeR();            // เห็นเส้นด้านขวา (มี edge_margin)

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

    bool nowOnLine = seeL();            // เห็นเส้นด้านซ้าย (มี edge_margin)

    // ตรวจจับ "transition" จากไม่อยู่บนเส้น -> อยู่บนเส้น
    if (!onLine && nowOnLine) {
      hitCount++;
      if (hitCount >= 2) break;
    }

    onLine = nowOnLine;
  }

  PID_speed_reset();
}

// แขนแยกจริง = เซนเซอร์ริม (i) กับกลาง C[2] ดำพร้อมกัน
// โค้งเส้นเดียวกว้างไม่พอทับ C[2] กับเซนเซอร์ริมพร้อมกัน เลยไม่นับ
bool is_arm(int i) {
  return C[i] < S[i] && C[2] < S[2];
}
#define LATCH_MS 30   // จำริมดำไว้กี่ ms (จูนกับหุ่นจริง)
#define WIDE_R 1      // แขนขวายอมให้ C[3] แทน C[2] (จับแยกบนโค้งขวา แต่เสี่ยงโค้งขวาหลอก) 0 = ปิด
#define WIDE_L 0      // แขนซ้ายยอมให้ C[1] แทน C[2] (ปิดไว้: โค้งซ้ายทำให้ C0+C1 ดำเองได้)
void FLJ(unsigned long guard = 0) {   // ตามเส้นจนเจอแยก ข้างไหนก็ได้ (guard ms แรกไม่สนแยก)
  unsigned long t0 = millis(), tL = 0, tR = 0;   // tL/tR = เวลาที่ริมซ้าย/ขวาดำล่าสุด (0 = ยังไม่เห็น)
  read_sensor();
  while (true) {
    unsigned long now = millis();
    if (C[0] < S[0]) tL = now;
    if (C[4] < S[4]) tR = now;
    bool c2 = C[2] < S[2];
    bool armR = tR && now - tR < LATCH_MS && (c2 || (WIDE_R && C[3] < S[3]));   // ริมขวาดำภายใน LATCH_MS แล้ว C[2] (หรือ C[3]) ดำ
    bool armL = tL && now - tL < LATCH_MS && (c2 || (WIDE_L && C[1] < S[1]));
    if (now - t0 >= guard && (armL || armR)) {
      flj_L = armL;  flj_R = armR;   // จดเฉพาะข้างที่ผ่านเงื่อนไข
      break;
    }
    cheack_error();
    follow_line();
    read_sensor();
  }
  PID_speed_reset();
}

void FLL() {
  read_sensor();
  while (!seeL()) {
    cheack_error();
    follow_line();
    read_sensor();
  }
  PID_speed_reset();
}
void Read_ultarsonic() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long t = pulseIn(ECHO_PIN, HIGH, 6000);   // รอสูงสุด ~1 m
  cm = t ? t / 58 : 999;                    // ไม่มีเสียงสะท้อน = ไกล
}
void FL_U(int x) {   // ตามเส้นจนเจอวัตถุใกล้กว่า x cm
  Read_ultarsonic();
  while (cm > x) {
    read_sensor();
    cheack_error();
    follow_line();
    Read_ultarsonic();
  }
  PID_speed_reset();
  B();
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
// หมุนช้าจนเซนเซอร์ i เห็นเส้น (ใช้ส่ายหากระป๋อง) กันค้าง 300 ms
void aim(int i, bool left) {
  analogWrite(PWMA, speed_fix);
  analogWrite(PWMB, speed_fix);
  unsigned long t0 = millis();
  read_sensor();
  while (C[i] > S[i] && millis() - t0 < 300) {
    if (left) drive_motor(1, 0, 0, 1);
    else      drive_motor(0, 1, 1, 0);
    read_sensor();
  }
  B();
}
void TL() {
  analogWrite(PWMB, speed_trun);
  analogWrite(PWMA, speed_trun);
  read_sensor();
  while (C[0] >  S[0]) {
    drive_motor(1, 0, 0, 1);
    read_sensor();
  }
  analogWrite(PWMB, speed_trun * 2/3);   // ช้าลงก่อนถึงเส้น
  analogWrite(PWMA, speed_trun * 2/3);
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
  analogWrite(PWMB, speed_trun * 2/3);   // ช้าลงก่อนถึงเส้น
  analogWrite(PWMA, speed_trun * 2/3);
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
  seen_L = seen_R = false;
  while (real_time - start_time < time_cross) {
    read_sensor();
    if (is_arm(0)) seen_L = true;
    if (is_arm(4)) seen_R = true;
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
void Set_kp_gain(float x) {
  Kp_gain = x;
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

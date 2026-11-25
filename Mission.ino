void Mission() {
  ///______________________________________พารามิเตอร์__________________________________________________//
  can_cm = 25;              //ระยะที่นับว่ามีกระป๋อง cm (can_auto, cheak_can_*)
  coin_on = true;          //true = ดันกระป๋องแล้วเล่นเสียงเหรียญ Mario
  end_pat = "1312";        //auto_run: ลำดับแยกสุดท้ายก่อนจอด 1 ซ้าย 2 ขวา 3 คู่ (สูงสุด 15 ตัว)
  debug_beep = false;        //true = can_auto บี๊บบอกแยก: 1 ซ้าย 2 ขวา 3 สองข้าง (ปิดตอนแข่ง)
  Set_kp(20);               //ค่าการเปลี่ยนความเร็ว motor ตอนวิ่งตามเส้น (เส้นตรง 10-20 , เส้นโค้งเยอะๆ 20-30)
  Set_kp_gain(3);           //Kp เพิ่มตาม |error| (0 = ปิด) โค้งแรง Kp ขึ้นเอง
  Speed_motor(200);         //ความเร็ว motor วิ่งตามเส้น (ทางตรง 150-220 , ทางโค้ง 100-120)
  Speed_LR(120, 120);       //ความเร็วถอยตามเส้น GBL (และ GO/GB)
  speed_trun = 255;         //ความเร็ว motor เลี้ยวเข้าเส้นแบบใช้ sensor (TR,TL)
  speed_cross = 95;         //ความเร็ว motor เลี้ยวข้ามแยกเพื่อเลี้ยว (FL_CD,FLL_CD,FLR_CD)
  time_cross = 175;         //เวลาเดิน motor เลี้ยวข้ามแยกเพื่อเลี้ยว (FL_CD,FLL_CD,FLR_CD)
  ///______________________________________เขียนโปรแกรมภายในคอมเม้น__________________________________________________//
  auto_run(); return;

  FL_C(); FL_time(4300); FLR_CD(); can_R();                     // 1
  FL_time(650); FLL_CD(); can_L();                              // 2
  FL_time(4000); FLL_CD(); can_L();                             // 3
  FLR_CD(); can_R();                                            // 4
  FL_time(1300); FL_CD(); TR(); push(); TR(); FLL_CD(); TR();   // 5
  FL_time(500); FLL_CD(); can_L();                              // 6
  FL();

 }///______________________________________***ระวัง อย่าเผลอลบปีกกา__________________________________________________//
 void push() {
  start_time = millis();
  while (millis() - start_time < 365) {
    read_sensor();
    cheack_error_down();   // พ้นปลายเส้น (ขาวหมด) = error 0 วิ่งตรง ไม่เลี้ยว/หมุนตามค่าที่จำไว้
    follow_line();
  }
  PID_speed_reset();
  B();
  if (coin_on) coin();
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
// ยิง 3 ครั้ง เอาค่ากลาง
bool ping_can(int x) {
  long r[3];
  for (int i = 0; i < 3; i++) {
    if (i) delay(30);   // รอเสียงสะท้อนรอบก่อนหาย
    Read_ultarsonic();
    r[i] = cm;
  }
  cm = max(min(r[0], r[1]), min(max(r[0], r[1]), r[2]));
  return cm < x;
}
// เช็คกระป๋อง: ตรงกลางก่อน ไม่เจอค่อยส่ายซ้าย ขวา
bool see_can(int x) {
  if (x == 0) x = can_cm;   // ไม่ใส่เลข = ใช้ can_cm
  bool found = ping_can(x);
  if (!found) {
    aim(3, true);  found = ping_can(x);                  // หันซ้ายจนเส้นไปอยู่ C[3]
    if (!found) { aim(1, false); found = ping_can(x); }  // หันขวาจนเส้นไปอยู่ C[1]
  }
  if (!found) tone(BUZZER_PIN, 2000, 400);   // ไม่เจอ: บี๊บยาว 1 ครั้ง
  return found;
}
void coin() {   // เสียงเหรียญ Mario: ติ๊ง(B5) แล้วติ๊ง ยาว(E6)
  tone(BUZZER_PIN, 988, 80);
  delay(90);
  tone(BUZZER_PIN, 1319, 350);   // ค้างเล่นต่อระหว่างหุ่นเลี้ยว ไม่หยุดรอ
}
void beep_n(int n) {   // บี๊บสั้น n ครั้ง
  for (int i = 0; i < n; i++) { tone(BUZZER_PIN, 2000, 80); delay(200); }
}
// เลี้ยวเข้าเส้นแยก เจอกระป๋อง = ดัน แล้วกลับ / ไม่เจอ = เลี้ยวกลับทางเดิม
void cheak_can_L(int x) {
  TL();
  if (see_can(x)) { push(); TR(); FL(); cross_down(); TL(); }
  else TR();
}
void cheak_can_R(int x) {
  TR();
  if (see_can(x)) { push(); TR(); FL(); cross_down(); TR(); }
  else TL();
}


void cheak_can_LR(int x) {
  TL();
  if (see_can(x)) { push(); TR(); FL(); cross_down(); }   // กลับมาหันเข้าเส้นแยกขวา
  else { TR(); TR(); }
  if (see_can(x)) { push(); TR(); FL(); cross_down(); TR(); }
  else TL();
}
// ใช้หลัง FLJ(): ข้ามแยก ดูว่าแยกเดียวหรือแยกคู่ แล้วเช็คกระป๋องทุกเส้นแยก
int can_auto(int x) {
  cross_down();
  seen_L |= flj_L;  seen_R |= flj_R;   // แขนอาจผ่านไปก่อน cross_down เริ่ม
  int j = seen_L + 2 * seen_R;   // 1 ซ้าย 2 ขวา 3 สองข้าง 0 ไม่เห็น
  if (debug_beep) beep_n(j);
  if (j == 3) cheak_can_LR(x);
  else if (j == 1) cheak_can_L(x);
  else if (j == 2) cheak_can_R(x);
  return j;
}
// วิ่งเองทั้งสนาม: เจอแยกก็เช็คกระป๋อง จนเจอลำดับ end_pat แล้วเข้าจุดจอด
void auto_run() {
  int n = strlen(end_pat);
  if (n > 15) n = 15;
  const char *pat = end_pat + strlen(end_pat) - n;
  char last[16] = "";   // แยกล่าสุด n แยก
  FL_C();
  while (strcmp(last, pat)) {
    FLJ();
    int j = can_auto();
    if (!j) continue;
    int k = strlen(last);
    if (k == n) memmove(last, last + 1, k--);   // ทิ้งแยกเก่าสุด
    last[k] = '0' + j;
    last[k + 1] = 0;
  }
  FL();
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

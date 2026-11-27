# Pit Lane build (เว็บคู่มือโครงใหม่)

เนื้อหาอยู่ที่ `../robot-guide.html` (แก้ที่นี่) แล้วรัน:

    python docs/pitlane/build.py

ได้ `docs/pitlane/dist/index.html` (ไฟล์เดียว ฟอนต์ฝังในไฟล์ ใช้ออฟไลน์ได้) คัดลอกไปเป็น `index.html` ของ repo trt-robot-guide

- `new.css` ธีมมืด + ฟอนต์ · `new2.css` โครง Command Browser · `new3.css` animation + หน้าอัปเดต
- `browser.js` สร้างเมนู/ค้นหา/แสดงทีละหน้าจาก DOM ของ robot-guide.html
- หน้าใหม่ที่เป็น `<section class="sec" id="...">` ต้องเพิ่มชื่อใน `browser.js` (ฟังก์ชัน `secView`) ถึงจะขึ้นเมนู
- ฟอนต์ IBM Plex Sans Thai: SIL Open Font License

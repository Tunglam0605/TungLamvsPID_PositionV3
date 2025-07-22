# 🚀 MotorPID_Position V3 (by Nguyễn Khắc Tùng Lâm)  
**Thư viện điều khiển vị trí động cơ DC sử dụng encoder + PID, hỗ trợ đa động cơ, tối ưu cho Arduino Due, Mega, Uno...**

> **Tác giả:** Nguyễn Khắc Tùng Lâm  
> Facebook: [Lâm Tùng](https://facebook.com/tunglam060504)  
> TikTok: [@tunglam.automatic](https://tiktok.com/@tunglam.automatic)  
> SĐT: 0325270213

---

## ✨ TÍNH NĂNG NỔI BẬT

- 🔥 **Quản lý đồng thời nhiều động cơ** – mỗi động cơ là 1 object, tự động nhận interrupt, không cần attachInterrupt thủ công.
- 🎯 **PID Position “thật sự”** – giữ vị trí siêu ổn định, tự động scale hệ số khi sai số lớn, không rung khi về đích.
- 🔄 **moveNRound(n, autoCorrection)**  
  - Quay đúng số vòng thực tế dựa trên encoder, bù xung linh hoạt.
  - Chống cộng dồn sai số do encoder/mất xung/trượt cơ khí.
- ⏸ **Bật/Tắt PID** – Motor tự do hoặc khóa giữ vị trí mới mọi lúc.
- 🛠️ **API trực quan, style “thương mại” – code cực gọn, dễ nhúng vào mọi project!**
- 🏡 **Auto Homing** – tự động đưa về vị trí chuẩn ban đầu, hỗ trợ bù offset lắp ráp.
- 🔗 **Điều khiển nối sóng, điều khiển tuần tự/đồng thời nhiều động cơ (robot chân, truyền động nhiều bậc tự do,...)**
- 💬 **Nhận lệnh Serial thông minh** – hỗ trợ lệnh đặt góc, quay vòng, về home, bật/tắt từng động cơ ngay qua Serial Monitor.

---

## ⚡️ CÀI ĐẶT

1. Copy 2 file MotorPID_Position.h và MotorPID_Position.cpp vào thư mục libraries/MotorPID_Position của Arduino IDE.
2. Hoặc nén cả thư mục thành .zip rồi cài từ IDE (Sketch > Include Library > Add .ZIP Library…).
3. Tạo mới file .ino, dùng như ví dụ phía dưới.
---

## 🎮 CÁCH SỬ DỤNG CƠ BẢN
1️⃣ Khai báo động cơ

```cpp
#include "MotorPID_Position_V3.h"
#define PPR 3600 // Số xung mỗi vòng (tuỳ encoder thực tế)

MotorPID_Position motor1(37, 36, 2, 3, 5.65, 0.065, 0.1, PPR); 
MotorPID_Position motor2(35, 34, 4, 5, 5.65, 0.065, 0.1, PPR); // Thêm động cơ thoải mái!
```
2️⃣ Khởi tạo trong setup()
```cpp
void setup() {
    Serial.begin(115200);
    motor1.Init();
    motor2.Init();
    motor1.Home();
    motor2.Home();
}
```
3️⃣ Điều khiển vị trí / Quay n vòng trong loop()
```cpp
motor1.Position(90);           // Đặt motor1 giữ vị trí góc 90 độ
motor2.moveNRound(-1);         // Motor2 quay -1 vòng (ngược)
motor1.moveNRound(2);          // Motor1 quay 2 vòng 
```
🛠️ BẢNG HÀM API CHÍNH
```
Hàm / Method	--------------------- Chức năng chính
Position(angle)--------------------	Giữ vị trí góc (độ)
moveNRound(n)----------------------	Quay n vòng
setEnable(bool)--------------------	Tắt/bật PID (motor tự do/giữ vị trí mới)
Home()	--------------------------- Reset encoder về 0, dừng motor
Stop()	--------------------------- Ngắt motor ngay (PWM=0)
setPID(kp,ki,kd) ------------------	Đổi hệ số PID chính
setFastPIDOffset(kp_off, kd_off) -- Offset hệ số PID khi sai số lớn (auto scale)
setPulsePerRev(ppr)	--------------- Đổi số xung/vòng
setISat(val)	--------------------- Đổi giới hạn tích phân
getCurrentAngle()	----------------- Đọc góc hiện tại (độ)
getCurrentPulse()	----------------- Đọc số xung encoder hiện tại
```
🚦 VÍ DỤ ĐẦY ĐỦ – ĐIỀU KHIỂN 2 ĐỘNG CƠ
```cpp
#include "MotorPID_Position_V3.h"
#define PPR 3600

MotorPID_Position motor1(37, 36, 2, 3, 5.65, 0.065, 0.1, PPR);
MotorPID_Position motor2(35, 34, 4, 5, 5.65, 0.065, 0.1, PPR);

void setup() {
    Serial.begin(115200);
    motor1.Init();
    motor2.Init();
    motor1.setEnable(false);
    motor2.setEnable(false);
    motor1.Home();
    motor2.Home();
    Serial.println(F("Gửi: ROUND 2 | ROUND2 -1 | HOME1 | HOME2 | OFF1 | ON1 | SET1 180 | SET2 -90"));
}

void loop() {
    if (Serial.available()) {
        // ... Xử lý lệnh Serial như hướng dẫn dưới đây!
    }
    motor1.Position();
    motor2.Position();
}
```
💬 CÁC LỆNH SERIAL HỖ TRỢ
```
Lệnh	Chức năng
ROUND n	Motor1 quay n vòng
ROUND2 n	Motor2 quay n vòng
SET1 góc	Motor1 giữ góc bất kỳ (vd: SET1 180)
SET2 góc	Motor2 giữ góc bất kỳ (vd: SET2 -90)
HOME1/HOME2	Đưa motor về home
ON1/OFF1	Bật/tắt PID motor1
ON2/OFF2	Bật/tắt PID motor2
STOP1/STOP2	Stop motor
```
🧑‍🔬 HƯỚNG DẪN DÒ PID – TỐI ƯU HIỆU QUẢ
Tăng dần kp đến khi motor giữ vị trí chắc, bắt đầu rung nhẹ (quá lớn sẽ rung mạnh).

Tăng kd để giảm rung, động cơ về đích “êm”.

Thêm nhẹ ki nếu còn sai số tĩnh (ki lớn dễ bão hòa).

Quay nhiều vòng test với cả bù xung (true/false) để kiểm tra chính xác lặp lại.

Nếu nhiều loại motor/encoder, test luôn setFastPIDOffset để tối ưu lực khi chạy xa, giữ chắc khi về đích.

🌈 LIÊN HỆ – HỖ TRỢ – GÓP Ý
Tác giả: Nguyễn Khắc Tùng Lâm

Facebook: Lâm Tùng

TikTok: @tunglam.automatic

SDT: 0325270213

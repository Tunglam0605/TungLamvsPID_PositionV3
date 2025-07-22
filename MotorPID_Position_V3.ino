#include "MotorPID_Position_V3.h"
#define PPR 3600  // Số xung trên 1 vòng encoder (tùy loại động cơ/encoder thực tế)

// ==== Khai báo 2 động cơ PID Position ====
// ENCA, ENCB, PWM thuận, PWM nghịch, Kp, Ki, Kd, PPR
MotorPID_Position motor1(37, 36, 2, 3, 5.5, 0.065, 0.1, PPR);
MotorPID_Position motor2(35, 34, 4, 5, 5.5, 0.065, 0.1, PPR);

bool ROBOT_Up = 0; // Cờ đánh dấu robot đã đưa về trạng thái đứng gốc

// ==== Enum trạng thái cho từng Job stepper (chạy nối sóng các động cơ) ====
enum StepperState {
    STEP_IDLE,      // Chưa chạy hoặc đã hoàn thành
    STEP_RUNNING,   // Đang quay tới target
    STEP_DONE       // Đã hoàn thành bước này
};

// ==== Cấu trúc Job cho từng động cơ, quản lý từng bước ====
struct StepperJob {
    MotorPID_Position* motor;    // Động cơ cần điều khiển
    float startAngle;            // Góc ban đầu tại thời điểm chạy
    float rounds;                // Số vòng cần quay (âm/ngược)
    float targetAngle;           // Lưu lại target cụ thể, dùng kiểm tra chính xác
    StepperState state;          // Trạng thái hiện tại của job
};

#define NUM_JOBS 2   // Số job/bước (ở đây là 2 động cơ)
StepperJob jobs[NUM_JOBS] = {
    {&motor1, 0, 1, STEP_IDLE},   // motor1 quay 1 vòng thuận
    {&motor2, 0, -1, STEP_IDLE}   // motor2 quay 1 vòng ngược
};

// Khi động cơ 1 quay đủ START_NEXT_JOB_THRESHOLD độ thì động cơ 2 bắt đầu chạy song song
const float START_NEXT_JOB_THRESHOLD = 356.0f; // Gần 1 vòng, cho phép bắt đầu động cơ 2

// ===== Hàm bắt đầu 1 job cho động cơ chỉ định =====
void startJob(int idx) {
    jobs[idx].startAngle = jobs[idx].motor->getCurrentAngle();      // Lưu lại vị trí bắt đầu
    jobs[idx].motor->moveNRound(jobs[idx].rounds);                  // Đặt mục tiêu quay n vòng
    jobs[idx].state = STEP_RUNNING;                                 // Đánh dấu đang chạy
    jobs[idx].targetAngle = jobs[idx].motor->getTargetAngle();      // Lưu lại targetAngle thực tế
    Serial.print("Started job for motor "); Serial.println(idx + 1);
}

// ===== Hàm xử lý logic nối sóng động cơ (step sequence) =====
void stepperSequence() {
    // Nếu cả 2 motor chưa chạy gì, bỏ qua
    if (jobs[0].state == STEP_IDLE && jobs[1].state == STEP_IDLE) return;

    // Khi motor1 quay đủ ngưỡng, cho motor2 chạy song song
    if (jobs[0].state == STEP_RUNNING && jobs[1].state == STEP_IDLE) {
        float m1Progress = fabs(jobs[0].motor->getCurrentAngle() - jobs[0].startAngle);
        if (m1Progress >= START_NEXT_JOB_THRESHOLD) {
            startJob(1);
            Serial.println("Motor2 START song song với Motor1!");
        }
    }

    // Kiểm tra hoàn thành từng motor: nếu gần tới target thì DONE
    const float TOLERANCE = 2.0f; // Sai số chấp nhận cho việc dừng
    for (int i = 0; i < NUM_JOBS; i++) {
        if (jobs[i].state == STEP_RUNNING) {
            float angleNow = jobs[i].motor->getCurrentAngle();
            float target = jobs[i].motor->getTargetAngle(); // Lấy target động cơ thực tế
            if (fabs(angleNow - target) < TOLERANCE) {      // Đủ gần thì DONE
                jobs[i].state = STEP_DONE;
                Serial.print("Job for motor "); Serial.print(i+1); Serial.println(" DONE!");
            }
        }
    }
}

// ===== SETUP Arduino =====
void setup() {
    Serial.begin(115200); // Khởi tạo Serial cho debug/lệnh
    motor1.Init();
    motor2.Init();

    // Khởi tạo giao tiếp Serial cho thư viện động cơ (tùy chọn Serial1, 2, 3...)
    MotorPID_Position::initSerialControl(&Serial, 115200);

    // --- HOMING toàn bộ động cơ ---
    int homeSensorPins[2] = {A1, A2};           // Chân cảm biến home cho từng motor
    int pwmF_Pins[2]      = {2, 4};             // PWM thuận cho từng motor
    int pwmB_Pins[2]      = {3, 5};             // PWM nghịch cho từng motor
    int pwmSpeed[2]       = {200, 200};         // Tốc độ quay về home (0-255)
    bool directions[2]    = {false, true};      // true: thuận, false: nghịch (cho từng motor)
    float offsets[2]      = {-20, 18};          // Bù góc cho home (tùy lắp đặt)
    MotorPID_Position* motors[2] = {&motor1, &motor2};

    MotorPID_Position::autoHomeAll(
        motors,
        homeSensorPins,
        pwmF_Pins,
        pwmB_Pins,
        pwmSpeed,
        directions,
        offsets,
        2,
        127 // Ngưỡng cảm biến ADC, tuỳ loại cảm biến
    );
    delay(1000); // Đợi 1 giây cho hệ thống ổn định
}

// ===== LOOP chính của chương trình =====
void loop() {
    // Giao tiếp Serial cho động cơ (tùy chọn lệnh qua máy tính)
    MotorPID_Position* motors[2] = {&motor1, &motor2};
    MotorPID_Position::handleSerialControl(motors, 2);

    // Đưa robot về trạng thái đứng thẳng (setup 1 lần đầu khi bật máy)
    if (ROBOT_Up == 0) { 
      motor1.Position(-90);    // Chân 1 về -90 độ
      motor2.Position(90);     // Chân 2 về 90 độ
      ROBOT_Up = 1;            // Đặt cờ đã đứng dậy
    }

    // Update PID cho từng motor liên tục
    motor1.Position();
    motor2.Position();

    // Đọc giá trị joystick A4 (trên Due), điều khiển động cơ step-sequence
    int joy = analogRead(A4);

    static bool sequenceStarted = false;     // Đã bắt đầu chu kỳ step chưa?
    static bool readyForNextCycle = false;   // Flag cho phép lặp lại chu kỳ mới

    if (joy > 800) { // Nếu kéo cần joystick hết sang phải
        if (!sequenceStarted) { // Nếu chưa chạy thì bắt đầu lại
            for (int i = 0; i < NUM_JOBS; i++) jobs[i].state = STEP_IDLE;
            startJob(0);            // Luôn bắt đầu lại từ motor1
            sequenceStarted = true;
            readyForNextCycle = false;
            Serial.println("START STEP-SEQUENCE!");
        }
        stepperSequence(); // Quản lý chạy động cơ nối sóng

        // Nếu đã hoàn thành tất cả các jobs, lặp lại nếu vẫn giữ joystick
        bool allDone = true;
        for (int i = 0; i < NUM_JOBS; i++) {
            if (jobs[i].state != STEP_DONE) {
                allDone = false;
                break;
            }
        }
        if (allDone && !readyForNextCycle) {
            for (int i = 0; i < NUM_JOBS; i++) jobs[i].state = STEP_IDLE;
            startJob(0);                    // Bắt đầu lại từ đầu
            readyForNextCycle = true;
            Serial.println("LOOP STEP-SEQUENCE AGAIN!");
        }
        if (!allDone) readyForNextCycle = false;
    } else {
        sequenceStarted = false;    // Khi nhả joystick thì cho phép bắt đầu lại
        readyForNextCycle = false;
    }

    // --- In trạng thái động cơ mỗi 300ms ---
    static unsigned long tPrint = 0;
    if (millis() - tPrint > 300) {
        Serial.print("[M1] Angle: "); Serial.print(motor1.getCurrentAngle(), 1);
        Serial.print(" | Target: "); Serial.print(motor1.getTargetAngle(), 1);
        Serial.print(" || [M2] Angle: "); Serial.print(motor2.getCurrentAngle(), 1);
        Serial.print(" | Target: "); Serial.print(motor2.getTargetAngle(), 1);
        Serial.print(" | JOY: "); Serial.println(joy);
        tPrint = millis();
    }
}

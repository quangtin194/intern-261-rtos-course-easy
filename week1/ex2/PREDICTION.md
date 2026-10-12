# Dự đoán — Week 1 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2413505    Ưu tiên của tôi: A=1 B=3 C=4

1. Bước 2 (`configUSE_PREEMPTION`=1, ưu tiên khác nhau): task nào chạy? task nào
   **không bao giờ** chạy? CPU share dự đoán:

- Task chạy: Task C (có priorty cao nhất).
- Task không bao giờ chạy: Task A & Task B. Task C có độ ưu tiên cao nhất và liên tục chạy busy-loop mà không gọi vTaskDelay(), không block hoặc chủ động nhường CPU. Vì vậy, Task C luôn ở trạng thái Ready và được scheduler ưu tiên thực thi. Task A và Task B vẫn ở trạng thái Ready nhưng không được cấp CPU, dẫn đến starvation.

   | Task | CPU share dự đoán (%) |
   |---|---|
   | A |0%|
   | B |0%|
   | C |100%|

2. Bước 3 (`configUSE_PREEMPTION`=0): dự đoán thay đổi gì so với bước 2? Vì sao?

**Trả lời:** 
   
- Nếu Task C là task đầu tiên được chạy sau khi scheduler khởi động và không nhường CPU, kết quả dự đoán vẫn là A = 0%, B = 0%, C = 100%.

- Giải thích: Khi tắt preemption, FreeRTOS không tự động chuyển sang task có ưu tiên cao hơn chỉ vì task đó trở thành Ready. Nếu Task C đã chạy và liên tục busy-loop mà không block hoặc nhường CPU, các task còn lại không có cơ hội thực thi.

3. Bước 4 (3 task cùng ưu tiên): time slice dự đoán = 1 ms. Dựa vào đâu?

**Trả lời:** Nếu cho phép preemption, scheduler có thể tự động chuyển task mà không cần task hiện tại phải Blocked hay kết thúc. Cả 3 Task cùng priorty, ở trạng thái Ready và configUSE_TIME_SLICING = 1, scheduler sẽ lập lịch thực hiện luân phiên các Task này khi có tick xảy ra. Tick đang được config 1 Tick = 1ms, nên time slice sẽ là 1ms.

4. Bước 5: tôi sẽ sửa starvation bằng cách: ______ . Cách này đánh đổi gì?

**Trả lời:** Dùng vTaskDelay() trong vòng lặp của các Task. Cách này đánh đổi thời gian CPU dành cho xử lý để đổi lấy việc các Task nhường CPU định kỳ, giúp các Task ưu tiên thấp có cơ hội thực thi. Tuy nhiên, độ đáp ứng của Task có thể giảm và thời gian thực thi phụ thuộc vào chu kỳ delay cũng như thời điểm Task được lập lịch.

5. Chân nào sẽ không bao giờ đổi trạng thái trên logic analyzer ở bước 2? Chân PA1 (Task A) và PA4 (Task B).

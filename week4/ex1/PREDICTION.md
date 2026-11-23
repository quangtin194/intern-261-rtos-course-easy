# Dự đoán — Week 4 / Ex1

> **Commit TRƯỚC khi gửi design doc cho mentor duyệt.**

`student_id`: 2413505

1. Với `SAMPLE_RATE_HZ` của bạn, chu kỳ lấy mẫu là bao nhiêu us? Dựa trên latency ISR→task thật bạn đã đo ở `week3/ex1`, dự đoán task xử lý có kịp `ISR_TO_PROC_DEADLINE_US` không? Trình bày phép tính, không đoán.

**Trả lời:** 
- Với `SAMPLE_RATE_HZ = 400Hz`, chu kỳ lấy mẫu là 1/400Hz = 2.5 (ms) = 2500 (us).
- Note: "Task xử lý kịp `ISR_TO_PROC_DEADLINE_US`" ở đây được hiểu là khoảng thời gian chuyển từ I2C ISR sang Process task (tức là sau khi I2C đã đọc sensor thành công và phát ngắt I2C, rồi callback I2C xử lý ghép byte + gửi lên Queue1, nên không tính thời gian I2C lấy mẫu) + thời gian workload của Process task.
- **Dự đoán** 
   + Latency ISR -> task em đo được ở week3/ex1 khoảng 12.99 us (trường hợp có `portYIELD_FROM_ISR()`)
   + Giải thuật xử lý mẫu trong Process task dự tính sử dụng là bộ lọc trung bình trượt, tương đối đơn giản nên sẽ không tốn nhiều chu kỳ CPU, cộng thêm thời gian `xQueueReceive` từ Queue 1 và `xQueueSend` ra Queue 2 => ước tính khoảng **1000 chu kỳ CPU** => Với cấu hình 72MHz, dự tính tổng thời gian workload của Process task sẽ mất khoảng 1000 / 72Mhz = 13.89 (us). 
   + Tổng thời gian từ lúc chuyển context ISR đến sau khi Process task chạy xong khoảng 12.99 + 13.89 = 26.88 (us), bé hơn nhiều so với `ISR_TO_PROC_DEADLINE_US = 300 us`, nên sẽ đáp ứng kịp deadline.

2. Dự đoán tổng RAM (queue + stack mọi task) theo thiết kế của bạn, tính
   bằng bytes. So với `MAX_HEAP_BYTES`: còn dư bao nhiêu?

**Trả lời:** 
- **Giả định:** mỗi mẫu `sensor_sample_t` 16 byte (6 trục `int16_t` = 12 byte + timestamp 4 byte); TCB 76 byte (đã đo ở week1/ex4); overhead heap_4 = 14 byte mỗi lần cấp phát; phần quản lý queue (`Queue_t`) = 76 byte.
- **Tính toán:**
   + **RAM cho task:** 3 task (Process, UART, Idle), mỗi task stack 128 words = 512 byte + TCB 76 byte + overhead 20 byte = 608 byte => 3 * 608 = **1824 byte**. Không dùng software timer nên không có Timer task.
   + **RAM cho queue:** Queue 1: 16 * 16 + 76 + 12 = **344 byte**; Queue 2: 64 * 16 + 76 + 12 = **1112 byte**; buffer gom khối 64 mẫu: 1024 + 8 = **1032 byte**. Tổng = **2488 byte**.
   + **Tổng RAM** = 1824 + 2488 = **4312 byte**.
- **So sánh:** `MAX_HEAP_BYTES` = 10240 byte, còn dư 10240 − 4312 = **5928 byte** (= 57.9%).

3. Nếu task xử lý tạm thời chậm hơn ISR lấy mẫu (ví dụ do một task khác
   chiếm CPU), hàng đợi của bạn đầy sau bao nhiêu mẫu? Điều gì xảy ra với
   mẫu tiếp theo trong thiết kế của bạn?

**Trả lời:**
- **Queue 1 đầy sau bao nhiêu mẫu:** Queue 1 có 16 phần tử. Nếu Process task bị chiếm CPU và không lấy mẫu ra, mỗi 2.5 ms ISR đẩy thêm một mẫu nên queue đầy sau **16 mẫu** ≈ 16 * 2.5 ms = **40 ms**.
- **Chuyện xảy ra với mẫu tiếp theo:** ISR gọi `xQueueSendFromISR` với queue đầy. Hàm này không chờ trong ISR, nó trả về `errQUEUE_FULL`. Mẫu mới bị bỏ, 16 mẫu cũ trong queue giữ nguyên.

4. Bạn dự đoán priority nào (ISR task xử lý vs. task ghi UART) sẽ cho kết
   quả đạt deadline tốt hơn? Vì sao?

**Trả lời:** 
- **Dự đoán:** thứ tự Process task (priority 2) > UART task (priority 1) cho kết quả đạt deadline tốt hơn. ISR luôn chạy trước mọi task vì nó chạy ở mức ngắt phần cứng.
- **Giải thích:** deadline tính từ callback hoàn tất I2C đến lúc Process xử lý xong.
  + Nếu Process > UART: khi callback gọi `xQueueSendFromISR` + `portYIELD_FROM_ISR`, Process task chuyển sang Ready và chen ngay vào UART task đang chạy. Đây là trường hợp em đã phân tích ở câu 1 phía trên (26.88 us << 300 us), nên sẽ kịp deadline.
  + Nếu UART > Process: khi UART đang gửi khối 1024 byte ở 115200 baud (= 1024 * 10 / 115200 ≈ 88.9 ms), Process không chen vào được, phải chờ UART nhường CPU. Latency có thể lên tới hàng chục ms, vượt xa 300 µs.

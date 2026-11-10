# Dự đoán — Week 3 / Ex1

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2413505

1. Với `ISR_NVIC_PRIO` của bạn và nhóm priority `NVIC_PRIORITYGROUP_4`, số này nằm trong khoảng cho phép (`configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` ..
   `configLIBRARY_LOWEST_INTERRUPT_PRIORITY`) không? Trình bày phép so sánh.

**Trả lời:** Với `NVIC_PRIORITYGROUP_4`, hệ thống sử dụng 4 bit cho interrupt priority nên các mức priority logic nằm trong khoảng 0–15. Do `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5` và `configLIBRARY_LOWEST_INTERRUPT_PRIORITY = 15`, ISR sử dụng API FreeRTOS phải có priority trong khoảng 5–15. Với `ISR_NVIC_PRIO = 8` => nằm trong khoảng cho phép (5 <= 8 <= 15).

2. Khi đặt priority EXTI cao hơn ngưỡng cho phép rồi gọi
   `vTaskNotifyGiveFromISR`, bạn dự đoán hành vi quan sát được là gì (reset?
   treo? thông báo gì trên UART/debugger)?

**Trả lời:** Hành vi quan sát được là khi nhấn nút ngắt sẽ không thấy pin `ACK_PIN` được toggle vì chương trình đã kẹt trong loop for vô hạn của configASSERT.

3. Dự đoán latency ISR → task (có `portYIELD_FROM_ISR`) theo đơn vị us, dựa
   trên tần số CPU 72 MHz và số lệnh ước lượng trong đường đi ISR → context
   switch. So với `LATENCY_BUDGET_US`: đạt hay không?

**Trả lời:** 
- Chu kỳ máy: Với cấu hình tần số CPU 72MHz, chu kỳ lệnh xấp xỉ 13.89 ns.
- Ước lượng từ ISR -> Task có khoảng 1000 lệnh, giả sử mỗi lệnh tốn 1 chu kỳ máy. Khi đó:
```
latency ISR = 1000 * 13.89 = 13.89 (us)
```
- So với `LATENCY_BUDGET_US = 50 us` thì đạt yêu cầu.

4. Nếu bỏ `portYIELD_FROM_ISR`, `ack_task` sẽ chạy **khi nào** (ngay lập tức
   hay phải chờ tick ngắt kế tiếp)? Latency tăng thêm cỡ bao nhiêu?

**Trả lời:** 
- Khi bỏ portYIELD_FROM_ISR, ack_task không được chuyển sang chạy ngay sau khi ISR kết thúc. Task sẽ phải chờ đến lần tick tiếp theo để scheduler có cơ hội chuyển sang ack_task (nếu ack_task có priority cao hơn task đang chạy).
- Xét trong bài này, với configTICK_RATE_HZ = 1000 Hz, latency có thể tăng thêm tối đa khoảng **1 ms.** Trung bình 500us.

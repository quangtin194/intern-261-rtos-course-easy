# Dự đoán — Week 1 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2413505    `PERIOD_MS` = 17ms    `WORK_JITTER_US` = 1400us    `SAMPLE_COUNT` = 800 lần

1. Bảng dự đoán (điền trước khi đo):

| # | Delay | Tick | Chu kỳ tb (ms) | Jitter (us) | Drift tổng sau SAMPLE_COUNT (ms) |
|---|---|---|---|---|---|
| 1 | `vTaskDelay` | 1000 |17.28|1000|228|
| 2 | `xTaskDelayUntil` | 1000 |~17|~0|~0|
| 3 | `vTaskDelay` | 100 |~10|~0|-5600|
| 4 | `xTaskDelayUntil` | 100 |~10|~0|-5600|

2. Cấu hình nào có drift lớn nhất? Vì sao, viết bằng công thức có `WORK_JITTER_US`.

**Trả lời:**
- Trong trường hợp bài này, **cấu hình 3 và 4 có độ lớn drift tổng lớn nhất.**
   + Cả 2 cấu hình đều config `configTICK_RATE_HZ = 100`, lúc này 1 tick sẽ tương đương 10ms. Với chu kỳ được yêu cầu là 17ms, nhưng thời gian delay được biểu diễn theo số nguyên tick. Nếu sử dụng cách chuyển đổi thông thường `pdMS_TO_TICKS(17)`, giá trị này có thể bị làm tròn xuống còn 1 tick, tương ứng 10ms. Khi đó, sai số chu kỳ sẽ là -7ms, qua 800 lần thì drift tích lũy sẽ là -5600ms.
   + Vì giá trị tối đa của `work_us` chỉ là 1400us = 1.4ms. Nếu cấu hình `configTICK_RATE_HZ = 1000` (như cấu hình 1 và 2) thì sẽ nhìn thấy rõ sự khác biệt giữa drift của 2 cấu hình. **Tuy nhiên, với trường hợp cấu hình 3 và 4, `configTICK_RATE_HZ = 100` (1 tick = 10ms) nên `work_us` lúc này sẽ nằm gọn trong tick hiện tại, nên sẽ vẫn không thấy rõ được sự khác biệt giữa việc dùng `vTaskDelay` và `xTaskDelayUntil`.**
- Giả sử nếu `work_us` được tăng lên (giá trị random trong khoảng giữa min và WORK_JITTER_US, với min = 0, WORK_JITTER_US = 14ms), `configTICK_RATE_HZ = 100`. Khi đó, **cấu hình 4 sẽ gây ra drift tích lũy lớn nhất**. Hiểu đơn giản là vì cấu hình 4 sẽ chỉ toàn gây drift âm, trong khi cấu hình 3 vừa có thể gây drift âm (-7ms) nếu `work_us < 10ms` hoặc drift dương (3ms) nếu `10ms < work_us < 14ms`. Vậy nên khi tổng drift lại, cấu hình 3 sẽ tự bù trừ, dẫn đến drift tổng bé hơn cấu hình 4. Em sẽ xây dựng công thức tính drift tổng của cấu hình 3 như sau:
   
   + Giá trị `work_us` sẽ chạy từ 0 -> 14ms, nếu `work_us` lớn hơn 10ms (vượt qua 1tick) thì mới gây ảnh hưởng đến chu kỳ. Giả sử giá trị `work_us` là phân bố đều, xác suất xuất hiện giá trị lớn hơn 10ms (sau đây gọi là A) sẽ là:
   ```
   A = (WORK_JITTER_US - 10ms) / (WORK_JITTER_US - min) = (14ms - 10ms) / (14ms - 0) = ~28.57 (%)
   ```
   + Như vậy, ta sẽ có A% chu kỳ bị kéo dài ra 20ms và (1- A)% chu kỳ 10ms. Chu kỳ 20ms gây ra drift 3ms, chu kỳ 10ms gây ra drift -7ms. Khi đó, drift tổng sau SAMPLE_COUNT lần sẽ là:
   ```
   Drift tổng = SAMPLE_COUNNT * [(20 - 17)*A + (10 - 17)*(1 - A)] = -3314.4 (ms)
   ```
   + Kiểm chứng: Em sẽ kiểm chứng lại kết quả ở phía cuối REPORT.md 
    

3. `PERIOD_MS` = 17ms . Với tick 100 Hz, một tick = 10 ms, nên chu kỳ thực tế sẽ bị làm tròn thành 10 ms. Sai số hệ thống: ~41.176 %.
4. Jitter ở tick 1000 Hz so với 100 Hz:  Nếu so sánh ở cấu hình `configTICK_RATE_HZ = 1000`, Jitter khi dùng vTaskDelay() lớn hơn khi dùng xTaskDelay() vì ảnh hưởng của `work_us`.

5. Nếu task này **không** phải ưu tiên cao nhất, đại lượng nào trong bảng bị ảnh hưởng nhiều nhất? 

**Trả lời:** Drift tổng, vì khi task không có ưu tiên cao nhất, task có thể thường xuyên bị trì hoãn bởi các task ưu tiên cao hơn. Những lần trì hoãn này khiến thời điểm thực thi thực tế ngày càng lệch khỏi lịch dự kiến, làm drift tích lũy tăng lên.

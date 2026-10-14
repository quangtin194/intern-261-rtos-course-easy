# Dự đoán — Week 1 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2413505    `PERIOD_MS` = 17ms    `WORK_JITTER_US` = 600us    `SAMPLE_COUNT` = 800 lần

1. Bảng dự đoán (điền trước khi đo):

| # | Delay | Tick | Chu kỳ tb (ms) | Jitter (us) | Drift tổng sau SAMPLE_COUNT (ms) |
|---|---|---|---|---|---|
| 1 | `vTaskDelay` | 1000 |16.995|10|-0.05|
| 2 | `xTaskDelayUntil` | 1000 |16.995|10|-0.05|
| 3 | `vTaskDelay` | 100 |9.995|10|-5604|
| 4 | `xTaskDelayUntil` | 100 |9.995|10|-5604|

2. Cấu hình nào có drift lớn nhất? Vì sao, viết bằng công thức có `WORK_JITTER_US`.



3. `PERIOD_MS` = 17ms . Với tick 100 Hz, một tick = 10 ms, nên chu kỳ thực tế sẽ bị làm tròn thành 10 ms. Sai số hệ thống: ~41.176 %.
4. Jitter ở tick 1000 Hz so với 100 Hz: 



5. Nếu task này **không** phải ưu tiên cao nhất, đại lượng nào trong bảng bị ảnh hưởng nhiều nhất? 

**Trả lời:** Drift tổng, vì khi task không có ưu tiên cao nhất, task có thể thường xuyên bị trì hoãn bởi các task ưu tiên cao hơn. Những lần trì hoãn này khiến thời điểm thực thi thực tế ngày càng lệch khỏi lịch dự kiến, làm drift tích lũy tăng lên.

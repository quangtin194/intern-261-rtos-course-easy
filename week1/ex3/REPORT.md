# Report — Week 1 / Ex3

`student_id`: 2413505    
Commit của PREDICTION.md: `"feat: submit PREDICTION.md file for ex3"`

## 1. Bảng thống kê (tính từ CSV, không đọc tay)
| # | Delay | Tick | min (ms) | max (ms) | tb (ms) | jitter = max−min (us) | drift tổng (ms) |
|---|---|---|---|---|---|---|---|
| 1 | `vTaskDelay` | 1000 |16.999|18.001|17.294105|1002|234.99|
| 2 | `xTaskDelayUntil` | 1000 |16.99|17.001|16.999987|11|-0.01|
| 3 | `vTaskDelay` | 100 |9.999|10.001|10|2|-5593|
| 4 | `xTaskDelayUntil` | 100 |9.99|10.01|9.999987|20|-5593.01|

- Script dùng để tính: [analyze_ex3_resultcsv.py](./tool/analyze_ex3_resultcsv.py)
- CSV gốc: [xem /captures](./captures/) (commit kèm)
- Ảnh: [xem /image](./image/)

## 2. Tại sao `vTaskDelay` bị drift
Công thức drift theo `PERIOD_MS` và `WORK_JITTER_US` của **bạn**, rồi so với số đo: 
- Xét cấu hình 1. Dùng công thức tự lập trong `PREDICTION.md`:

| | Tính theo công thức | Đo được | Lệch |
|---|---|---|---|
| Drift tổng (ms) |228ms|234.99ms|6.99ms|
- Xét cấu hình 3, với `WORK_JITTER_US` giả định là 14ms. Tính theo công thức tự lập trong `PREDICTION.md`: [Minh chứng](./image/work_us_14ms.jpg) 

| | Tính theo công thức | Đo được | Lệch |
|---|---|---|---|
| Drift tổng (ms) |-3314.4ms|-3243.01ms|71.39ms|

## 3. Tại sao tick 100 Hz sai nhiều hơn
`PERIOD_MS` = 17ms ; một tick = 10 ms. Chứng minh bằng số đo, không bằng lý thuyết: Lý do em đã trình bày trong file `PREDICTION.md`. Số đo có thể xem bảng phía trên + file CSV + hình ảnh đính kèm.

## 4. `xTaskDelayUntil` trả về gì
Giá trị trả về nghĩa là gì, và trong 4 lần chạy của bạn nó có bao giờ báo
"đã trễ deadline" không? Bao nhiêu lần / `SAMPLE_COUNT`?

**Trả lời:**

`xTaskDelayUntil()` trả về giá trị kiểu `BaseType_t`, cho biết task có bị trễ so với thời điểm đánh thức dự kiến hay không:

- `pdTRUE`: Task được đánh thức đúng lịch hoặc trước khi deadline bị bỏ lỡ.
- `pdFALSE`: Thời điểm đánh thức dự kiến đã nằm trong quá khứ, nghĩa là task đã trễ deadline và không thể duy trì lịch trình ban đầu.

Nếu xét về việc báo trễ deadline, tức `xTaskDelayUntil()` return về giá trị `pdFALSE`. Trong 2 trường hợp sử dụng `xTaskDelayUntil()` trong bài (cấu hình 2 & 4), task đều không bị trễ deadline.


## 5. Lệch so với dự đoán
Mỗi dòng lệch trong bảng → lệch bao nhiêu + nguyên nhân vật lý.

| # | Đại lượng | Predict | Đo thực tế | Sai lệch (thực tế − predict) | Nguyên nhân vật lý |
|---|---|---:|---:|---:|---|
| 1 | Chu kỳ TB (ms) | 17.28 | 17.294105 | +0.014105 ms (+14.105 µs) | Dự đoán khá sát. `vTaskDelay()` tạo delay tương đối sau khi workload kết thúc. Tuy nhiên, khi tính toán giá trị dự đoán, em lý tưởng phân phối của giá trị random `work_us` là phân phối đều, nên nhiều khi thực tế khi random giá trị sẽ khác đi một chút, nên kết quả vẫn có sai lệch nhỏ.|
| 1 | Jitter (µs) | 1000 | 1002 | +2 µs | Jitter gần đúng dự đoán. Sai số nhỏ có thể đến từ Logic Analyzer, nhiễu đường dây,...|
| 1 | Drift tổng (ms) | 228 | 234.99 | +6.99 ms |Drift tổng cũng là trường hợp em lý tưởng phân phối đều để tính toán, nên có thể sẽ sai lệch khi random giá trị thực tế. Ngoài ra có thể có thêm sai số từ thiết bị (đường dây, Logic Analyzer,..)|
| 2 | Chu kỳ TB (ms) | ≈17 | 16.999987 | Gần 0 | Rất sát với dự đoán|. `xTaskDelayUntil()` lập lịch theo mốc đánh thức tuyệt đối, hạn chế việc thời gian thực thi của workload cộng dồn vào chu kỳ. |
| 2 | Jitter (µs) | ≈0 | 11 | +11 µs | Jitter gần đúng dự đoán. Sai số nhỏ có thể đến từ Logic Analyzer, nhiễu đường dây,...|
| 2 | Drift tổng (ms) | ≈0 | -0.01 | -0.01 ms |Gần sát với dự đoán, sai số nhỏ có thể đến từ sai số lấy mẫu, nhiễu đường dây,...|
| 3 | Chu kỳ TB (ms) | ≈10 | 10 | Gần 0 | Rất khớp dự đoán.|
| 3 | Jitter (µs) | ≈0 | 2 | +2 µs | Gần đúng dự đoán. Biến thiên nhỏ có thể đến từ sai số lấy mẫu, nhiễu,...|
| 3 | Drift tổng (ms) | -5600 | -5593 | +7 ms | Sai lệch do cách đếm khoảng thời gian: 800 cạnh chỉ tạo 799 khoảng giữa các cạnh liên tiếp. Vì vậy, drift lý tưởng là `799 × (10 − 17) = -5593 ms`, không phải -5600 ms. |
| 4 | Chu kỳ TB (ms) | ≈10 | 9.999987 | Gần 0 | Rất khớp dự đoán.|
| 4 | Jitter (µs) | ≈0 | 20 | +20 µs | Jitter nhỏ, có thể là do sai số của thiết bị, nhiễu dây|
| 4 | Drift tổng (ms) | -5600 | -5593.01 | +6.99 ms | Tương tự cấu hình 3, phần lớn sai lệch đến từ việc tính 799 khoảng giữa 800 cạnh. Sai lệch thêm 0.01ms có thể là do sai số của thiết bi.|

## 6. Câu hỏi đánh đổi
Tick 1000 Hz cho độ chính xác tốt hơn nhưng **tốn** gì? Dựa vào dữ liệu của bạn:
đo CPU overhead của tick ISR ở 100 Hz và 1000 Hz (gợi ý: cho một task ưu tiên thấp nhất đếm vòng lặp trong 1 giây, so sánh 2 cấu hình). Điền số:

**Trả lời:**

Tick 1000 Hz có độ phân giải thời gian tốt hơn tick 100 Hz, nhưng làm tăng số lần Tick ISR được gọi từ 100 lên 1000 lần/s, dẫn đến tăng CPU overhead và có thể tăng năng lượng tiêu thụ.

| Tick rate | Số vòng lặp idle trong 1 s | Overhead tăng thêm |
|---|---:|---:|
| 100 Hz |1302950| 0% (mốc tham chiếu)|
| 1000 Hz |1292167|0.827%|



**Lựa chọn:** Với `PERIOD_MS = 17 ms`, em chọn tick 1000 Hz. Ở 100 Hz, chu kỳ yêu cầu 17 ms bị chuyển thành 1 tick, tương ứng 10 ms. Trong khi đó, ở 1000 Hz, `xTaskDelayUntil()` đạt chu kỳ trung bình 16.999987 ms, gần với mục tiêu 17 ms hơn đáng kể. Nên em chấp nhận overhead CPU tăng để đáp ứng yêu cầu thời gian của ứng dụng.

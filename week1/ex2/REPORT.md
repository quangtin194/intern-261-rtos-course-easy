# Report — Week 1 / Ex2

`student_id`: 2413505    
Commit của PREDICTION.md: `feat: submit PREDICTION.md file for ex2` (10g, 12/10/2026)

## 1. CPU share đo được
| Cấu hình | A (%) | B (%) | C (%) | Time slice đo được | Ảnh |
|---|---|---|---|---|---|
| B2: preempt=1, ưu tiên khác nhau |0|0|100| — |[ex2_1.png](ex2_1.png)|
| B3: preempt=0, ưu tiên khác nhau |0|0|100| — |[ex2_2.jpg](ex2_2.jpg)|
| B4: preempt=1, cùng ưu tiên |~33.33|~33.33|~33.33| 1ms |[ex2_3.jpg](ex2_3.jpg)|
| B5: sau khi sửa starvation |~18|~30|~42|—|[ex2_4.jpg](ex2_4.jpg)|

Cách tính CPU share từ ảnh PulseView (mô tả ngắn): CPU share (%) = (Tổng thời gian task chiếm CPU trong một đoạn thời gian đo / Tổng thời gian của đoạn đo đó) x 100%.

## 2. Preemption nhìn thấy ở đâu
Trên ảnh **bước 4** (cùng ưu tiên, round-robin), chỉ ra ít nhất **2 thời điểm**
preemption xảy ra. Với mỗi thời điểm: dấu thời gian, task bị cắt, task chiếm CPU,
và vì sao (hết time slice, không phải block).

| t (us) | Task bị preempt | Task chiếm CPU | Lý do |
|---|---|---|---|
| | | | |

Ở **bước 2** (ưu tiên khác nhau, đều busy-loop không block): giải thích vì sao ảnh
của bạn cho thấy **0 thời điểm preemption**, dù chạy bao lâu đi nữa — dựa vào điều
kiện preemption của FreeRTOS, không phải "vì ảnh chụp chưa đủ dài".

## 3. Time slice
Time slice đo được ở bước 4: 1 ms. `configTICK_RATE_HZ` = 1000.
Sai số: 0.4 %. Giải thích sai số: Có thể xuất phát từ độ phân giải lấy mẫu của Logic Analyzer.

## 4. DWT wrap-around
Tại sao so sánh `DWT->CYCCNT > target` bị sai sau ~59 giây, và cách viết đúng:
- DWT->CYCCNT là bộ đếm 32-bit không dấu, tự quay về 0 khi tràn. Với CPU 72 MHz, thời điểm tràn (tính từ lúc bắt đầu đếm) là sau khoảng 59,65 s (2^32 / 72×10^6).

- Cách viết sai là so sánh giá trị tuyệt đối: tính target = CYCCNT + duration rồi chờ CYCCNT > target. Gần điểm tràn, target có thể quay về số nhỏ nên vòng lặp thoát ngay, hoặc CYCCNT đã tràn mà target vẫn là số lớn nên phải chờ gần 59 s.

- Cách viết đúng là so sánh hiệu: đọc start = DWT->CYCCNT một lần ngay trước khi chờ, rồi lặp trong khi (uint32_t)(DWT->CYCCNT - start) < duration_cycles. Phép trừ unsigned 32-bit là phép modulo 2^32 nên luôn cho đúng số chu kỳ đã trôi qua, kể cả khi CYCCNT đã tràn giữa chừng

## 5. Sửa starvation


## 6. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân vật lý.

## 7. Câu hỏi đánh đổi
Với **bộ ưu tiên của bạn**, nếu `TASK_C_WORK_US` tăng gấp 3 lần thì bảng CPU share ở
bước 2 thay đổi thế nào? Trả lời bằng số suy ra từ dữ liệu của bạn, rồi **đo lại để
kiểm chứng**. Nếu lệch, giải thích.

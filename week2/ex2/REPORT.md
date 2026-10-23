# Report — Week 2 / Ex2

`student_id`: 2413505    Commit của PREDICTION.md: `"feat: submit Prediction file for ex2 w2"`

## 1. Lỗi tái hiện (UART_GUARD_MODE = 0)
Log UART dán nguyên văn, đánh dấu vị trí ký tự lẫn lộn:

```
Serial port COM4 opened
ID: 3; SeID: 0; SID: 1; SID: 2; Sq: 000000000eq: 00000000eq: 0000000eq: 00000000
0
000

ID: 3; SID: 0; SID: 2; SID: 1; Seq: 00000000eq: 00000000eq: 0000000eq: 000000001
1
01
1
ID: 3; SID: 2; SID: 1; Seq: 00000000ID: 0; Seq: 00000000eq: 000000002
eq: 002
2
0000002
ID: 3; SID: 2; SID: 1; Seq: 00000000ID: 0; Seq: 00000000eq: 000000003
eq: 003
3
0000003
ID: 3; SID: 2; SID: 1; Seq: 00000000ID: 0; Seq: 00000000eq: 000000004
eq: 004
4
0000004
ID: 3; SID: 2; SID: 1; Seq: 00000000ID: 0; Seeq: 0000000eq: 000000005
q: 00005
5
000005
ID: 3; SID: 2; SID: 1; Seq: 00000000ID: 0; Seq: 00000000eq: 000000006
eq: 006
6
0000006
...
```

Tái hiện lại bao nhiêu lần / bao nhiêu lần thấy lỗi: 5 / 5.

## 2. Bảng so sánh 3 cách
| Cách | Độ trễ in trung bình (us) | Độ trễ in max (us) | Ảnh hưởng task khác | RAM thêm (bytes) |
|---|---|---|---|---|
| Mutex |5372|8506|9245|96|
| Critical section |3234|7646|5710|0|
| Gatekeeper |2263|2281|2351|1096|

Cách đo độ trễ in (mô tả ngắn, kèm ảnh logic analyzer nếu dùng pin đánh dấu):
- Sử dụng bộ đếm chu kỳ DWT (DWT->CYCCNT) của Cortex-M3 tại thời điểm ngay trước khi gọi uart_guard_send(). Sau khi thông điệp được truyền xong, lấy số chu kỳ chênh lệch và quy đổi sang microsecond theo tần số CPU.

## 3. Vì sao critical section nguy hiểm nhất
Giải thích bằng số đo của bạn: đoạn in dài bao nhiêu us khi ở trong critical section, và điều đó chặn ngắt nào trong hệ thống bao lâu.

**Trả lời:** Thời gian in trung bình là 3,234 ms. Trong khoảng thời gian từ ngay sau taskENTER_CRITICAL() đến khi gọi taskEXIT_CRITICAL(), các ngắt có mức ưu tiên thuộc phạm vi bị mask (trong đó có Systick và PendSV) sẽ bị trì hoãn xử lý trong khoảng thời gian này.

## 4. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân vật lý.

**Trả lời:** 
- **Mutex:** Thời gian in trễ cỡ vài chục µs được đề cập trong phần PREDICTION là độ trễ phát sinh do thao tác lấy mutex và chuyển đổi ngữ cảnh giữa các task. Trong khi đó, thời gian em đo trong phần REPORT là khoảng thời gian từ khi task yêu cầu in đến khi thông điệp được truyền xong, bao gồm cả thời gian chờ mutex nếu mutex đang bị task khác giữ, thời gian chuyển đổi ngữ cảnh và thời gian truyền UART. Vì vậy, kết quả đo lớn hơn dự đoán ban đầu. Em cũng đã thử đo riêng phần độ trễ liên quan đến mutex bằng cách đặt mốc thời gian ngay trước thao tác `xSemaphoreTake()` bên trong hàm uart_guard_send(). Kết quả thu được ở mức vài chục µs, phù hợp với dự đoán ban đầu.

- **Critical section:** Tương tự Mutex, dự đoán ban đầu tập trung vào chi phí của thao tác vào/ra critical section và chuyển đổi ngữ cảnh. Tuy nhiên, phép đo thực tế bao gồm toàn bộ thời gian truyền UART khi đang ở trong critical section. Với thời gian in trung bình 3.234 ms, phần lớn độ trễ đến từ thời gian truyền dữ liệu nối tiếp và chi phí polling thanh ghi UART, chứ không chỉ từ bản thân hai API taskENTER_CRITICAL() và taskEXIT_CRITICAL().

- Gatekeeper: Theo dự đoán, nếu xét độ trễ từ lúc task yêu cầu in đến khi thông điệp được truyền xong, Gatekeeper có thể gây thêm độ trễ do phải gửi thông điệp vào queue, chuyển quyền thực thi sang Gatekeeper, nhận thông điệp từ queue rồi mới truyền UART. Chi phí bổ sung này được ước tính ở mức vài chục đến vài trăm µs. Tuy nhiên, cần phân biệt độ trễ từ yêu cầu đến khi in xong với thời gian chiếm CPU của task gửi. Sau khi gửi thành công vào queue, writer có thể tiếp tục công việc khác mà không phải chờ UART truyền xong. Vì vậy, dù thời gian hoàn tất thông điệp có thể dài hơn do phải chờ Gatekeeper xử lý, phương pháp này vẫn giúp giảm thời gian writer bị giữ lại và tránh phải duy trì critical section trong suốt quá trình truyền UART.

## 5. Câu hỏi đánh đổi
Với `WRITER_COUNT` của **bạn**, nếu tăng gấp đôi thì cách nào bị ảnh hưởng độ trễ nhiều nhất? Trả lời bằng số từ dữ liệu của bạn, rồi đo lại để kiểm chứng.

**Trả lời:**
- Mutex sẽ bị ảnh hưởng nhiều nhất. Vì các writer bị chặn nằm trong danh sách chờ của mutex. Mỗi lần nhả mutex, kernel phải đánh thức một task rồi đổi ngữ cảnh.

Đo lại để kiểm chứng:
- Trước khi tăng:

| Cách | Độ trễ in trung bình (us) | Độ trễ in max (us)|
|---|---|---|
| Mutex |5372|8506|
| Critical section |3234|7646|

- Sau khi tăng:
    
| Cách | Độ trễ in trung bình (us) | Độ trễ in max (us)|
|---|---|---|
| Mutex |14915|33771|
| Critical section |7745|13923|

=> Khi tăng gấp đôi số writer, độ trễ cực đại của Mutex tăng từ 8.506 µs lên 33.771 µs, gần 4 lần. Nguyên nhân có thể do các task phải chờ mutex lâu hơn khi tranh chấp UART gia tăng. Kết quả này hợp lý nếu độ trễ được đo từ lúc yêu cầu in đến khi truyền xong, bao gồm cả thời gian chờ mutex và thời gian truyền UART.

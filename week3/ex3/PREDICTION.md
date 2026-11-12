# Dự đoán — Week 3 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2413505

1. Thời gian truyền 1 byte ở `RX_BAUD` của bạn là bao nhiêu us? (8N1, tính cả
   start/stop bit). Trong thời gian đó `parser_task` có kịp xử lý 1 byte từ
   stream buffer không, nếu nó là task ưu tiên cao nhất hệ thống?

**Trả lời:** 
- Thời gian truyền 1 byte (cấu hình 8N1) ở `RX_BAUD = 57600 baud1`: 10/57600 = 173.6 (us).
- `parser_task` xử lý kịp 1 byte từ stream buffer, vì workload tương đối của nó khá ít nên thời gian xử lý nhanh, và quan trọng hơn là nó có priorty cao nhất nên trong quá trình xử lý sẽ không có task nào chen vào.

2. `STREAM_BUFFER_BYTES` của bạn chứa tối đa bao nhiêu lệnh dài `MAX_CMD_LEN` byte trước khi đầy?

**Trả lời:** Chứa tối đa 96/16 = 6 lệnh trước khi đầy.

3. Nếu `xStreamBufferSendFromISR` trả về 0 (stream buffer đầy) trong
   `uart_rx_isr_handler`, byte đó sẽ bị mất. Theo bạn, trường hợp nào thực tế khiến điều này xảy ra (gửi quá nhanh / `parser_task` ưu tiên quá thấp / cái khác)?

**Trả lời:** Theo em, 2 trường hợp thực tế khiến điều này xảy ra là do UART gửi quá nhanh và parser_task có priorty quá thấp, cụ thể:
- **UART gửi quá nhanh:** Code hiện tại dùng `HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1)` để nhận data gửi từ UART và kích hoạt ngắt, cộng thêm bên trong hàm ngắt có call API `xStreamBufferSendFromISR`. Với cấu hình này, mỗi lần gửi 1 byte sẽ gọi hàm ngắt UART 1 lần và 1 byte sẽ được gửi vào stream buffer. Nếu UART gửi quá nhanh (nhanh hơn thời gian parser_task) thực thi, stream buffer sẽ bị gửi liên tục và bị đầy trước khi parser_task kịp đọc dữ liệu ra và xử lý.
- **Parser_task ưu tiên quá thấp:** Nếu parser task có độ ưu tiên thấp hơn các task khác, nó có thể bị các task ưu tiên cao hơn preempt hoặc chiếm CPU trong thời gian dài. Khi đó, tốc độ đọc dữ liệu khỏi Stream Buffer giảm, trong khi UART vẫn tiếp tục nhận byte. Buffer sẽ ngày càng đầy và cuối cùng có thể không còn chỗ cho byte mới.

4. Dự đoán: gửi lệnh bằng gõ tay bình thường có mất byte không? Gửi bằng
   script gửi liên tục không nghỉ thì sao?

**Trả lời:** 
- **Gõ tay bình thường:** Dự đoán không mất byte, vì khoảng thời gian giữa các byte khi người dùng gõ lớn hơn nhiều so với thời gian parser_task cần để đọc và xử lý dữ liệu. Stream Buffer luôn có đủ thời gian để được tiêu thụ trước khi bị đầy.
- **Script gửi liên tục không nghỉ:** Dự đoán có khả năng mất byte nếu tốc độ gửi của script đủ cao và duy trì liên tục khiến parser_task không xử lý kịp. Khi đó dữ liệu sẽ tích tụ trong Stream Buffer. Nếu buffer đầy, các byte tiếp theo sẽ bị mất.

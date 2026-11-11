# Dự đoán — Week 3 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2413505

1. Với nút nhấn cơ khí thông thường, nảy phím thường kéo dài bao lâu
   (tra cứu/ước lượng)? So với `DEBOUNCE_MS` của bạn: đủ lớn để lọc hết không?

**Trả lời:** Ước lượng thời gian nảy phím thường từ vài ms đến vài chục ms. Với `DEBOUNCE_MS = 15 ms`, giá trị này nằm trong khoảng debounce thường dùng và đủ lớn để lọc phần lớn bounce thông thường. Tuy nhiên, nếu thời gian nảy phím thực tế lớn hơn 15 ms thì khoảng debounce này có thể không đủ để lọc hoàn toàn.

2. Nếu `DEBOUNCE_MS` quá lớn, điều gì xảy ra khi người dùng nhấn nhả rất
   nhanh liên tiếp (double-click)?

**Trả lời:** Khi người dùng nhấn nhả rất nhanh liên tiếp, ngắt có thể xảy ra nhiều lần trong khoảng thời gian ngắn. Mỗi cạnh ngắt sẽ gửi các command vào Timer Command Queue (RESET cho debounce_timer, sau đó START hoặc STOP cho longpress_timer) tùy trạng thái chân nút. Việc này có thể dẫn đến hai vấn đề:
- Thứ nhất: Các lần nhấn có thể không được xác nhận thành các sự kiện PRESS riêng biệt.
  Vd, khi nhận cạnh xuống, ISR gửi RESET cho debounce_timer và START cho longpress_timer. Nếu người dùng nhả rất nhanh, cạnh lên tiếp tục gửi RESET cho debounce_timer và STOP cho longpress_timer. Khi Daemon Task xử lý các command theo thứ tự FIFO, nó vẫn xử lý từng command riêng biệt. Tuy nhiên, debounce_timer có thể đang được reset lại bởi các RESET tiếp theo trước khi đạt DEBOUNCE_MS. Mỗi lần RESET lại làm thời điểm expiry của timer được tính lại. Vì vậy, nếu các cạnh liên tiếp xảy ra với khoảng cách nhỏ hơn DEBOUNCE_MS, timer có thể chưa kịp expire để gọi debounce_callback(). Kết quả là nhiều cạnh vật lý liên tiếp có thể chỉ được xác nhận thành một sự kiện PRESS.
- Thứ hai: Timer Command Queue có thể bị đầy.
  Nếu các cạnh xảy ra quá nhanh, ISR có thể tạo command nhanh hơn tốc độ Daemon Task xử lý. Khi đó, Timer Command Queue có thể đầy. Các API như xTimerResetFromISR(), xTimerStartFromISR() hoặc xTimerStopFromISR() có thể trả về pdFAIL khi không thể gửi command vào queue. Nếu chương trình không kiểm tra giá trị trả về, command tương ứng sẽ không được xử lý, dẫn đến mất thông tin về một cạnh hoặc một lần nhấn.

3. Khi callback của `debounce_timer` bị block 50 ms (thử nghiệm bước 4), bạn
   dự đoán `longpress_timer` sẽ bị trễ bao nhiêu? Giải thích bằng cơ chế
   daemon task + queue lệnh, không chỉ đoán số.

**Trả lời:** 
- **Dự đoán:** Bị trễ khoảng 50ms. 
- **Giải thích:** Khi Daemon Task gọi debounce_callback(), callback này thực hiện vTaskDelay(50 ms). Trong thời gian đó, chính Daemon Task bị block nên không thể xử lý các command trong Timer Command Queue hoặc thực thi longpress_callback(). Nếu longpress_timer đến hạn trong khoảng thời gian này, callback của nó phải chờ cho đến khi debounce_callback() kết thúc. Khi Daemon Task chạy trở lại, nó mới tiếp tục xử lý command và thực thi callback của longpress_timer, gây ra độ trễ khoảng 50 ms.

4. `LONGPRESS_MS` của bạn là bao nhiêu? Giữ phím đúng ngưỡng này, bạn dự đoán
   callback `longpress_callback` chạy lần đầu ở thời điểm nào tính từ lúc
   nhấn xuống?

**Trả lời:** 
- LONGPRESS_MS = 1000 ms.
- **Dự đoán:** `longpress_callback()` sẽ chạy lần đầu xấp xỉ 1000 ms sau thời điểm nhấn xuống.
   - Trường hợp lý tưởng, không có nảy phím: Ngay khi nhấn xuống, ISR nhận cạnh xuống và gửi RESET cho debounce_timer và START cho longpress_timer. Daemon Task xử lý các command này theo thứ tự FIFO. Sau khi xử lý START, longpress_timer bắt đầu đếm 1000 ms. Vì vậy, callback longpress_callback() sẽ chạy khoảng 1000 ms tính từ lúc nhấn xuống, cộng thêm một độ trễ rất nhỏ do việc đưa command vào queue và scheduling của Daemon Task.
   - Trong thực tế, có nảy phím: Các cạnh do bounce tiếp tục tạo START và STOP cho longpress_timer. Nếu một STOP được xử lý trước khi timer hết 1000 ms, timer sẽ dừng, một START sau đó sẽ bắt đầu lại một chu kỳ 1000 ms. Vì vậy, thời điểm longpress_callback() tính từ lần nhấn xuống đầu tiên có thể sẽ lớn hơn 1000 ms, tùy vào thuộc vào thời gian bounce phím.

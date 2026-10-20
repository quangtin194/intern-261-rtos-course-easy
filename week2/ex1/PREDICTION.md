# Dự đoán — Week 2 / Ex1

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2413505

1. Phép tính kích thước queue tối thiểu (dùng `SAMPLE_PERIOD_MS`, thời gian xử lý ước lượng của `filter_task` và `uart_task` ở `UART_BAUD`):

**Trả lời:** 
- Xét `sensor task`: Với thời gian lấy mẫu `SAMPLE_PERIOD_MS` là 5ms, cộng thêm việc task xử dụng `xTaskDelayUntil()` (sẽ giúp cho task gần như không bị ảnh hưởng với workload của các lệnh khác trong task). Vậy nên, có thể ước lượng thời gian thực thi task này sẽ khoảng **5ms**. Hay, thời gian lấy mẫu sensor sẽ là **5ms**.
```
T_s = 5 ms
```
- Xét `filter task`: Ước tính thời lượng thực thi là **1ms**.
- Xét `uart_task`: Với baurate là 115200 (bit/s). Ước tính thời gian thực thi tối đa khi gửi max 36 byte xấp xỉ:
```
T_uart = (10 x 36) / 115200 = 3.125 (ms)

Note: Khung truyền 8-N-1 => 10 bit 
```
- Để không mất mẫu tại queue, điều kiện là queue phải có chỗ trống khi sensor_task gửi mẫu vào queue. Xét luồng hoạt động như sau: Ban đầu, 3 task đều ready, nhưng sensor task sẽ được thực thi (vì có priorty cao nhất). Sensor task hoàn thành việc lấy mẫu từ cảm biến, rồi chuyển sang blocked trong 5ms. Ngay sau đó, filter task sẽ được chạy (dù cùng mức priorty với uart task, nhưng nó được khởi tạo trước). Ngay tại time slice sau đó (khoảng 1ms), scheduler sẽ luân chuyển sang uart task để chạy. Rồi lại đến time slice tiếp theo, scheduler sẽ luân chuyển sang lại filter task, nhưng vì lúc này raw queue trống (vì sensor task còn bị blocked, chưa lấy mẫu để nạp lại vào raw queue), nên filter task cũng bị blocked đến sau khi sensor task ready rồi thực thi xong trong lần tiếp theo luôn. Vậy nên, trong khoảng thời gian còn lại, chỉ có uart task chạy, nó gửi log UART xong rồi cũng sẽ bị blocked cho đến lần filter task hoàn thiện việc nạp data vào filter queue tiếp theo. Vậy nên cuối cùng, trong khoảng thời gian 5ms, hệ thống chỉ nhận 1 raw data, rồi log ra UART 1 filtered data. 

```
N = (T_filter + T_uart) / T_s ~= 0.825
Làm tròn lên số nguyên ta được N = 1
```
=> Kích thước tối thiểu cho raw queue và filter queue là **1 phần tử**. Tương đương **12 byte** (32 bit + [16bit + 16 bit] + [16 bit + padding]).


   Kích thước tối thiểu dự đoán: `queue_raw` = 12 byte , `queue_filtered` = 12 byte .
   So với `QUEUE_LENGTH` được cấp (4 phần tử, tương ứng 48 byte) => đủ kích thước lớn hơn kích thước tối thiểu cần để không mất mẫu.

2. Khi hạ `UART_BAUD` xuống 9600 (tạm thời), queue nào sẽ đầy trước? Vì sao?

**Trả lời:** **Filter queue** sẽ đầy trước. Vì với cấu hình baudrate 9600, UART sẽ mất tối đa 37.5ms để hàon tất log UART (xét trong trường hợp gửi chuỗi 36 byte). Trong khi sensor task vẫn mất 5ms để lấy mẫu mà filter task mất 1ms để xử lý raw data và đưa vào filtered queue, trong khi uart task vẫn chưa kịp đọc từ filtered queue. Vậy nên filtered queue sẽ đầy trước.

3. Với `xQueueSend(..., 0)` (timeout = 0) khi queue đầy: điều gì xảy ra với mẫu đó?

**Trả lời:** Với timeout = 0, nếu queue đã đầy, xQueueSend() sẽ thất bại và trả về errQUEUE_FULL (tương đương pdFAIL) ngay lập tức, không làm task chuyển sang trạng thái Blocked để chờ. Nếu là sensor_task và chương trình không xử lý việc gửi thất bại hoặc thử gửi lại, mẫu đó sẽ bị bỏ qua, dẫn đến mất mẫu. Task tiếp tục thực thi các câu lệnh tiếp theo và thực hiện chu kỳ lấy mẫu sau theo logic của chương trình.

4. Với `xQueueSend(..., portMAX_DELAY)` khi queue đầy: `sensor_task` sẽ ở trạng thái nào? Có nguy cơ gì cho chu kỳ lấy mẫu?

**Trả lời:** Với xQueueSend(..., portMAX_DELAY), nếu raw queue đầy, sensor_task sẽ chuyển sang trạng thái Blocked và chờ vô thời hạn cho đến khi queue có chỗ trống. Ví dụ, nếu queue có sức chứa 4 phần tử và đã đầy, khi gửi mẫu thứ 5, task sẽ bị Blocked tại lời gọi xQueueSend(). Khi filter_task lấy một phần tử ra khỏi queue, sensor_task có thể tiếp tục thực thi và gửi mẫu đang chờ.

Tuy nhiên, nếu thời gian chờ kéo dài, sensor_task không thể tiếp tục lấy mẫu theo lịch dự kiến. Do đó, chu kỳ lấy mẫu thực tế có thể dài hơn chu kỳ 5 ms mong muốn, khiến các thời điểm lấy mẫu dự kiến trong khoảng thời gian task bị Blocked bị bỏ lỡ.

5. Độ đầy `queue_raw` theo thời gian ở cấu hình bình thường sẽ trông như thế nào
   (tăng dần / dao động quanh một mức / luôn gần 0)? Vì sao?

**Trả lời:** Nếu theo cấu hình bài 1, độ đầy queue_raw theo thời gian dự kiến sẽ dao động quanh mức thấp, thường gần 0 hoặc khoảng 1 phần tử, chứ không tăng dần liên tục. Vì sensor_task tạo một mẫu mỗi 5 ms, trong khi filter_task chỉ mất khoảng 1 ms để xử lý một mẫu (theo giả định của bài). Do đó, tốc độ xử lý của filter_task đủ để theo kịp tốc độ tạo dữ liệu của sensor_task. Sau khi nhận và xử lý một mẫu, filter_task sẽ chờ mẫu tiếp theo nếu queue_raw đang rỗng. Vì vậy, dữ liệu không tích lũy liên tục trong queue và nguy cơ queue_raw bị đầy là thấp trong điều kiện hoạt động bình thường.

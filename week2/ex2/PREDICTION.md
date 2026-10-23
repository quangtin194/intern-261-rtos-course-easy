# Dự đoán — Week 2 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2413505    `WRITER_COUNT` = 4   `MSG_LEN` = 24 bytes

1. Ở `UART_GUARD_MODE = 0`: dự đoán lỗi lẫn lộn sẽ xuất hiện sau khoảng bao
   nhiêu dòng log? Vì sao MSG_LEN và WRITER_PERIOD_MS của bạn ảnh hưởng đến
   con số này?

**Trả lời:** 
- Lỗi này sẽ xuất hiện ở ngay dòng log đầu tiên. Vì thời gian phát 1 msg 24 bytes với baudrate 115200 là khoảng 2.08ms, trong khi time slicing cắt ngang mỗi 1ms. Điều này dẫn đến write1 chưa log xong thì write2 đã lấy UART để phát msg của nó, dẫn đến hiện tường lẫn lộn ký tự.
- `MSG_LEN`: Thông số này quyết định độ dài byte msg cần truyền. Điều kiện để có lỗi là tại thời điểm tick, ít nhất một writer đang gửi dở và còn writer khác đang Ready. Nên MSG_LEN càng lớn thì càng xảy ra lẫn lộn ký tự.
- `WRITER_PERIOD_MS`: Thông số này sẽ quyết định chu kỳ log msg ra UART của 1 write task. Ban đầu, cả 4 writer thức dậy cùng lúc và cùng ở trạng thái Ready, nên trong mỗi chu kỳ 7 ms chúng chia sẻ CPU theo time slicing và log UART, ở Mode 0, khiến byte của các task đan xen nhau. Task nào gửi xong sẽ vào Blocked cho đến chu kỳ kế tiếp, nên số task tranh chấp giảm dần (4 -> 3 -> 2 -> 1) và mức đan xen giảm theo. Phần đuôi của task cuối được gửi liền mạch, nhưng msg của nó chỉ liên tục hoàn toàn nếu nó bắt đầu gửi khi đã là task duy nhất còn Ready.

2. Dự đoán thứ tự **độ trễ in** (latency) từ thấp đến cao giữa mutex / critical section / gatekeeper. Vì sao?

**Trả lời:** Dự đoán: Critical section < Mutex < Gatekeeper
- Critical section: thấp nhất, vì chỉ có thao tác tăng/giảm `BASEPRI`, không có đổi ngữ cảnh khi đang gửi.
- Mutex: thêm Take/Give và vài lần đổi ngữ cảnh khi các writer sau bị chặn rồi được đánh thức, cỡ vài chục µs mỗi bản tin.
- Gatekeeper: thêm copy 24 byte vào queue, và mỗi bản tin cần hai lần đổi ngữ cảnh (writer -> gatekeeper -> writer), cỡ vài chục µs hoặc hơn mỗi bản tin.

3. Với critical section, task nào trong hệ thống có nguy cơ bị ảnh hưởng xấu
   nhất nếu đoạn in quá dài? (gợi ý: nghĩ đến ngắt, không chỉ task)

**Trả lời:** Bên bị ảnh hưởng xấu nhất là **SysTick**. taskENTER_CRITICAL() nâng BASEPRI lên ngưỡng configMAX_SYSCALL_INTERRUPT_PRIORITY, chặn mọi ngắt có ưu tiên bằng hoặc thấp hơn ngưỡng này, trong đó có SysTick vì nó được cấu hình ưu tiên thấp nhất. Một bản tin 24 byte ở 115200 baud mất khoảng 2.08 ms, dài hơn một tick (1 ms), nên SysTick bị chặn trong cả khoảng này. Hệ quả là các cơ chế dựa trên tick như vTaskDelay(), vTaskDelayUntil(), timeout và time slicing đều bị lệch.

4. RAM dùng thêm của gatekeeper so với mutex: dự đoán tăng hay giảm, vì sao?

**Trả lời:** RAM dùng thêm của gatekeeper dự đoán sẽ tăng so với mutex, vì mutex chỉ cần một cấu trúc queue không có vùng lưu trữ dữ liệu (item size = 0), khoảng 80 byte. Gatekeeper phải tạo thêm ba vùng:

- Queue trung gian: cấu trúc queue (80 byte) cộng vùng lưu trữ WRITER_COUNT × MSG_LEN = 4 x 24 = 96 byte.
- TCB của gatekeeper: 76 byte.
- Stack của gatekeeper: 200 word x 4 = 800 byte.

5. Nếu `WRITER_COUNT` tăng gấp đôi, cách nào trong 3 cách bị ảnh hưởng về độ trễ nhiều nhất? Vì sao?

**Trả lời:** Mutex sẽ bị ảnh hưởng nhiều nhất. Vì các writer bị chặn nằm trong danh sách chờ của mutex. Mỗi lần nhả mutex, kernel phải đánh thức một task rồi đổi ngữ cảnh, và số lần chặn/đánh thức tăng theo N (lần lượt N−1 writer bị chặn). Nếu mutex có priority inheritance thì còn thêm thao tác nâng/hạ priority. Overhead cộng dồn tăng nhanh hơn hai cách còn lại.

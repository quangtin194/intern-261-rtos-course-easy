# Dự đoán — Week 2 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2413505    `LOW_HOLD_MS` = 20 ms  `MID_WORK_MS` = 40 ms
`HIGH_PERIOD_MS` = 50 ms  `DEADLINE_MS` = 25 ms

1. Cấu hình binary semaphore: thời gian `task_high` bị block dự đoán = 50 ms.
   Phép tính dựa trên đâu (liệt kê các thành phần cộng lại)?

**Trả lời:** Lập luận như sau:
- Ban đầu, high_task sẽ được bộ lập lịch cho chạy trước. High_task chạy xong thì bị Blocked trong **50ms**.
- Tiếp theo, mid_task sẽ chạy, thời gian thực thi của mid_task ước tính bằng `MID_WORK_MS` (tức 40 ms). Sau đó, mid_task bị Blocked trong 10 ms, low_task được thực thi. Ngoài ra, tại thời điểm này, high_task cũng đã bị Blocked khoảng 40ms và chỉ còn 10ms nữa thì nó cũng sẽ Ready.
- Low_task sẽ lấy binary semaphore rồi thực thi. Vì thời gian thực thi của low_task xấp xỉ khoảng `LOW_WORK_MS` (tức 20 ms), nên khi thực thi đến 10ms, high_task và mid_task sẽ Ready. Scheduler lập lịch cho high_task chạy trước, nhưng vì high_task có yêu cầu lấy binary semaphore, nhưng low_task đang lấy nó, nên high_task bị Blocked đến khi binary semaphore available. High_task bị Blocked thì mid_task sẽ được thực thi.
- Mid_task thực thi **40 ms** rồi mới nhượng cho low_task chạy tiếp, low_task hoàn thành workload của lại của mình (**10 ms nữa**) rồi nhả binary semaphore rồi bị bị Blocked. Sau đó, high task mới lấy được binary semaphore và được thực thi.

=> Ước tính tổng thời gian bị **Blocked do semaphore: 40 + 10 = 50 ms**. Không tính 50ms bị Blocked do high_task gọi Delay.

2. `task_high` có đạt `DEADLINE_MS` ở cấu hình binary semaphore không? Vì sao?

**Trả lời:** 
- Không, task_high không đạt deadline 25 ms trong cấu hình binary semaphore theo kịch bản lập lịch đang xét (50ms > 25ms).
- Nguyên nhân là binary semaphore không hỗ trợ priority inheritance. Khi task_high bị block vì task_low đang giữ tài nguyên, task_mid có thể preempt task_low, làm chậm thời điểm task_low nhả semaphore.

3. Cấu hình mutex: thời gian `task_high` bị block dự đoán = 10 ms. Vì sao ngắn hơn (hoặc không) so với câu 1?

**Trả lời:** Điểm khác biệt so với cấu binary semaphore là mutex có hỗ trợ priority inheritance. Vì vậy, khi low_task đang lấy mutex và thực thi, độ ưu tiên của nó sẽ tạm thời được nâng lên bằng độ ưu tiên của high_task, khiến mid_task không thể chen ngang vào quá trình low_task chạy. Từ đó dẫn đến bây giờ, trong trường hợp này, **thời gian bị Blocked do mutex gây ra cho high_task chỉ là khoảng 10 ms**, ngắn hơn cấu hình binary semaphore.

4. Trên ảnh logic analyzer ở cấu hình binary semaphore, kênh `task_mid` sẽ ở
   trạng thái nào trong lúc `task_high` bị chặn — chạy liên tục, chạy ngắt
   quãng, hay không chạy? Vì sao?

**Trả lời:** Kênh task_mid sẽ ở **trạng thái chạy ngắt quãng** trong lúc task_high bị chặn. Khi task_high bị Blocked do chờ binary semaphore, task_mid có thể chiếm CPU vì có độ ưu tiên cao hơn task_low, khiến task_low chưa thể tiếp tục để nhả semaphore. Khi task_mid gọi vTaskDelay(10), nó chuyển sang trạng thái Blocked, tạo cơ hội cho task_low chạy tiếp. Khi hết thời gian delay, task_mid trở lại Ready và có thể preempt task_low một lần nữa. Quá trình này có thể lặp lại cho đến khi task_low nhả semaphore.

5. Nếu đổi `task_mid` sang ưu tiên **thấp hơn** `task_low`, inversion còn xảy ra
   không? Vì sao (đây là cách phân biệt priority inversion với việc chỉ đơn
   giản là "task_low chạy lâu").

**Trả lời:** Không, priority inversion theo kịch bản này sẽ không xảy ra nếu task_mid có mức ưu tiên thấp hơn task_low. Khi task_high yêu cầu tài nguyên đang được task_low giữ, task_high chuyển sang Blocked. Lúc này, task_low vẫn có thể tiếp tục chạy vì có mức ưu tiên cao hơn task_mid, nên task_mid không thể preempt và trì hoãn việc task_low nhả tài nguyên. Khi tài nguyên được giải phóng, task_high sẽ được đánh thức và chạy. Điều này cho thấy priority inversion không đơn thuần là việc task_low giữ tài nguyên lâu, mà là tình huống task ưu tiên trung bình cản trở task ưu tiên thấp giải phóng tài nguyên mà task ưu tiên cao đang chờ.

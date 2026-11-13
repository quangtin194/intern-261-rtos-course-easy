# Dự đoán — Week 3 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2413505

1. Chạy `app/deadlock_demo.c` y nguyên như được cấp. Dự đoán: hệ thống sẽ
   treo sau khoảng bao lâu (ngay lập tức, hay sau vài chu kỳ)? Vì sao không
   phải ngay lập tức (gợi ý: cần cả 2 task "đua" vào đúng thời điểm)?

**Trả lời:** 
- **Dự đoán:** `task_alpha` và `task_beta` bị deadlock ngay chu kỳ đầu tiên,
  khoảng 5 ms sau khi scheduler khởi động. Các task còn lại vẫn chạy bình thường.
- **Giải thích:**
  + Deadlock chỉ xảy ra khi cả hai task cùng giữ mutex đầu tiên của mình
    trước khi đòi mutex thứ hai (task_alpha giữ A, task_beta giữ B). Nếu một task lấy xong cả hai mutex trước khi task kia kịp chen vào thì không deadlock. Khoảng trống để "đua" là `vTaskDelay(5ms)` nằm giữa hai lần lấy mutex.
  + Ở code này điều kiện đó thỏa ngay từ đầu: task_alpha lấy A rồi Blocked 5 ms, task_beta lấy B rồi Blocked 5 ms. Hai lần gọi `vTaskDelay` cách nhau rất ngắn (nhỏ hơn 1 tick = 1 ms) nên cả hai cùng thức ở một tick.
  + Sau khoảng 5 ms cả hai cùng Ready. task_alpha chạy, đòi B thì Blocked (task_beta đang giữ). Task_beta chạy, đòi A thì Blocked (task_alpha đang giữ) => Deadlock.
  + Sau đó chỉ `runtime_stats_task` (đang blocked từng 100 ms) và Idle task còn chạy. Hệ thống không đứng hoàn toàn, chỉ hai task kia là bị kẹt vĩnh viễn.
  + Tuy nhiên, nếu hiểu treo hệ thống theo hướng không lấy log UART ra thì sẽ đúng vì với cấu hình hiện tại, `xEventGroupWaitBits` không bao giờ chờ đủ bit, IWDG liên tục reset hệ thống, trong khi runtime_stats_task cần 2s sau mới log được UART => Khi quan sát Hercules thì khả năng sẽ không thấy dòng text nào được log ra.

2. Khi treo, `uxTaskGetSystemState` sẽ báo trạng thái gì cho `task_alpha` và
   `task_beta`? Dự đoán tên mutex mỗi task đang chờ (dựa trên đọc code, chưa
   chạy công cụ).

**Trả lời:** 
- **Dự đoán trạng thái task khi bị treo:** `task_alpha` và `task_beta` có trạng thái `eBlocked`. Cả hai bị chặn bởi `xSemaphoreTake(..., portMAX_DELAY)` nên không có timeout để tự thoát.
- **Dự đoán tên mutex mỗi task đang chờ:** Treo xảy ra sau khi deadlock. Trong khi deadlock xảy ra vì `task_alpha` xin lấy mutex_b (nhưng `task_beta` đang giữ nó) và ngược lại => **`task_alpha` đang chờ mutex_b, `task_beta` đang chờ mutex_a.**

3. Tính reload value cho IWDG với `IWDG_TIMEOUT_MS` của bạn (LSI ~40 kHz).

**Trả lời:** 
- Công thức tính thời gian của IWDG: 
```
Timeout = (Reload + 1) * Prescaler / f_LSI
```
- Theo công thức trên, ta sẽ thấy có 2 thông số tùy chỉnh là Reload value và Prescaler. Cố định 1 thống số, giả sử ta chọn Prescaler = 16, với `IWDG_TIMEOUT_MS = 1000 ms`, khi đó:
```
Reload = (Timeout * f_LSI / Prescaler) - 1
       = (1 * 40K / 16) - 1 = 2499
```
- **Vậy reload value của em là 2499.**

4. Sau khi sửa lỗi deadlock và thêm supervisor + IWDG, nếu bạn cố tình tái
   tạo lại tình huống deadlock cũ (tạm thời), dự đoán board sẽ phản ứng thế
   nào và sau bao lâu.

**Trả lời:** 
- **Dự đoán:** Board tự reset qua IWDG sau khoảng 1000 ms (IWDG_TIMEOUT_MS), tính từ lần refresh cuối cùng (hoặc từ lúc `MX_IWDG_Init()`). Sau reset, deadlock lại xảy ra nên board reset lặp lại mỗi khoảng 1s.
- **Giải thích:**
  + Khi đảo lại thứ tự lấy mutex, task_alpha và task_beta kẹt vĩnh viễn ở chu kỳ đầu, nên không gọi `supervisor_checkin` nữa (check-in đặt sau khi nhả mutex).
  + supervisor_task vẫn chạy, nhưng `xEventGroupWaitBits` (wait for all) không bao giờ thu đủ ALL_BITS vì thiếu bit ALPHA và BETA. Sau mỗi 500 ms nó hết timeout, điều kiện `(bits & ALL_BITS) == ALL_BITS` sai, nên không gọi `HAL_IWDG_Refresh`.
  + Bộ đếm IWDG chạy tới 0 sau khoảng 1000 ms rồi tạo reset.
  + Vì reset xảy ra trước mốc in 2000 ms của runtime_stats_task nên trên UART không thấy bảng stats.

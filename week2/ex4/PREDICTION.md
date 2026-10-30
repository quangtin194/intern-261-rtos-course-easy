# Dự đoán — Week 2 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2413505    `EVENT_BIT_COUNT` = 4 bit  `SYNC_TIMEOUT_MS` = 80 ms

1. RAM event group chiếm (bytes) dự đoán: 24 bytes . Dựa vào đâu (cấu trúc dữ liệu nào trong FreeRTOS)?

**Trả lời:** Dựa vào cấu trúc dữ liệu của `EventGroup_t`, cụ thể:
- Event group được tạo bằng API `xEventGroupCreate()`, khi go to definition thì ta sẽ thấy bên trong API này cấp phát động 1 vùng nhớ có kích thước bằng với sizeof của `EventGroup_t`. Đây chính là vùng nhớ cấp phát cho Event Group.
- Khi phân giải struct `EventGroup_t` thì ta sẽ thấy bên trong cấu trúc có khai báo 1 biến kiểu EventBits_t và 1 biến kiểu List_t.
   + EventBits_t được định nghĩa từ kiểu TickType_t. TickType_t có size 32-bit => EventBits_t là kiểu **4 bytes.**
   + List_t là 1 struct, khi phân giải ra ta sẽ thấy: 1 biến UBaseType_t (**4 bytes**), 1 con trỏ ListItem_t (con trỏ trong ARM-CM3 thì có size **4 bytes**), 1 struct MiniListItem_t (bên trong có 2 con trỏ và 1 biến kiểu Ticktype_t nên tổng là **12 bytes**).
   
   => Tổng byte của `EventGroup_t`: 4+4+4+12 = **24 (bytes)**

2. RAM thêm khi dùng task notification cho 1 điểm đồng bộ: dự đoán = 0
   bytes. Vì sao?

**Trả lời:** 
- Mặc dù khi bật Task Notifications (configUSE_TASK_NOTIFICATIONS = 1 trong file config). FreeRTOS bổ sung 1 ít RAM nhỏ ngay trong TCB của task để lưu Notification Value và Notification State. **Nhưng việc bổ sung này áp dụng cho mọi task, dù task có dùng hay không.** Vì vậy, nếu thêm 1/nhiều điểm đồng bộ, thì RAM cũng sẽ không tốn thêm, vì nó đã được cấp sẵn khi cấu hình rồi. 

3. Dự đoán thời gian đánh thức (producer gọi → sync_task chạy) của notification so với event-group-bit: nhanh hơn / chậm hơn / xấp xỉ? Vì sao (nghĩ đến số lớp hàm bên dưới mỗi API)?

**Trả lời:** **Nhanh hơn**, cụ thể:
- xTaskNotifyGive đi thẳng vào TCB của task nhận: một đoạn critical section, ghi giá trị notification, và nếu task đang chờ thì chuyển nó từ danh sách chờ sang danh sách ready rồi yield. Không có object trung gian.
- xEventGroupSetBits phải tạm dừng scheduler (vTaskSuspendAll), duyệt danh sách các task đang chờ event group, kiểm tra điều kiện chờ cho từng task, gỡ task khỏi danh sách chờ, xử lý clear-on-exit, rồi gọi xTaskResumeAll để yield. Phía nhận xEventGroupWaitBits cũng làm thêm việc sau khi thức dậy so với ulTaskNotifyTake. 

=> Số lớp xử lý nhiều hơn nên dự đoán event group chậm hơn.

4. Nếu `sync_task` bị timeout (`SYNC_TIMEOUT_MS` hết hạn) vì một producer
   không set bit kịp, `xEventGroupWaitBits` trả về gì? Làm sao phân biệt được "đủ bit" với "timeout"?

**Trả lời:** 
- Nếu `sync_task` bị timeout vì một producer không set bit kịp, `xEventGroupWaitBits` trả về trạng thái các bit tại thời điểm hiện tại.
- Để phân biệt "đủ bit" với "timeout", ta AND biến nhận giá trị trả về của `xEventGroupWaitBits` với bit mask mà ta mong muốn.
   + Nếu kết quả AND bằng bit mask: Thỏa điều kiện (đủ bit).
   + Ngược lại, khác bit mask: Chưa đủ bit, giá trị trả về là do timeout. 

5. Nêu trước một tình huống bạn nghĩ **không thể** thay toàn bộ event group
   bằng notification — sẽ kiểm chứng lại ở REPORT.md.

**Trả lời:**
- **Tình huống:** Nhiều task cùng chờ 1 bit sự kiện.
- **Giải thích:** Task Notification sẽ **ghi trực tiếp đến một task** thông qua handle của task đó, nên chỉ có task đó chờ và đọc được. Muốn đánh thức nhiều task thì phải gọi notify nhiều lần với nhiều handle khác nhau, task gửi phải biết hết task nhận thì mới đánh thức hết được.
   + Event group thì ngược lại, bit nằm trong object dùng chung, xEventGroupSetBits đánh thức mọi task đang chờ bit đó trong một lần gọi.



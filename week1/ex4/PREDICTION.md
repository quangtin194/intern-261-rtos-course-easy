# Dự đoán — Week 1 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2413505    `HEAP_SIZE_BYTES` = 8192 (bytes)    `DYN_TASK_STACK_WORDS` = 128 (words)
`STATIC_TASK_STACK_WORDS` = 128 (words)    `HOG_DEPTH_BYTES` = 384 (bytes)    `TARGET_FREE_HEAP_BYTES` = 4096 (bytes)

1. Ngân sách RAM dự đoán (tổng phải = 20480 bytes):

| Mục | Bytes dự đoán |
|---|---|
| `.data` |0 + 100 = 100|
| `.bss` (không kể heap FreeRTOS) |1776 => 2000|
| Heap FreeRTOS (`ucHeap`) |8192|
| Stack của main (MSP) |1024 (Min_Stack_Size)|
| Còn lại / chưa dùng |9164|
| **Tổng** | **20480** |

2. `xPortGetFreeHeapSize()` ngay sau khi scheduler chạy: 6728 bytes.
   `HEAP_SIZE_BYTES` − số này = 1464 bytes . Số đó là của những gì?

**Trả lời:** 1864 bytes là tổng bytes đã được sử dụng trong vùng heap được cấp phát. Số đó bao gồm:
   + 512 bytes (tương đương 128 words) cấp cho stack của Task "dyn task".
   + 800 bytes (tương đương 200 words) cấp cho stack của Task "dyn task".
   + 152 bytes (tổng của 2 TCB, mỗi TCB có kích thước 76 bytes) cấp cho TCB của 2 Task trên.

3. High water mark dự đoán (words): dyn 400 bytes , stat 400 bytes , idle 460 bytes , timer 380 bytes.
4. `dyn_task` có stack 128 words = 512 bytes. Mảng `HOG_DEPTH_BYTES` = 384 bytes.
   Hook tràn stack **có** chạy không? Vì sao?

**Trả lời:** Hook tràn stack có thể sẽ chạy. Theo dự đoán ở câu 3, sau khi thực hiện workload có lời gọi vTaskDelay(), dyn_task vẫn còn dư khoảng 400 bytes stack. Nếu bổ sung mảng HOG_DEPTH_BYTES = 384 bytes, phần stack chưa được sử dụng còn lại ước tính chỉ khoảng 16 bytes, chưa kể phần stack phát sinh thêm do các biến cục bộ, lời gọi hàm và quá trình chuyển đổi ngữ cảnh. Vì vậy, nếu tổng lượng stack sử dụng vượt quá giới hạn 512 bytes và FreeRTOS phát hiện được tình trạng tràn, `vApplicationStackOverflowHook()` sẽ được gọi.

5. Heap trong `.map` và `xPortGetFreeHeapSize()` sẽ lệch nhau. Lệch vì: 

**Trả lời:** Heap trong file .map thể hiện vùng RAM được cấp phát tĩnh cho ucHeap (ví dụ 8192 bytes), tức tổng dung lượng heap được dành riêng cho FreeRTOS. Trong khi đó, xPortGetFreeHeapSize() trả về dung lượng heap hiện còn trống tại thời điểm gọi hàm. Hai giá trị lệch nhau vì heap đã được sử dụng để cấp phát stack và TCB cho các task được tạo bằng xTaskCreate().

6. Stack nhỏ nhất an toàn của `dyn_task` dự đoán: 132 words. Dựa vào đâu?

**Trả lời:** Dựa vào phân tích ở câu 4, em ước tính cần tăng stack của dyn_task thêm khoảng 4 words (16 bytes), từ 128 words lên 132 words (528 bytes), để bù cho phần stack phát sinh thêm khi thực thi các lời gọi hàm và xử lý chuyển đổi ngữ cảnh, qua đó giảm nguy cơ stack overflow.

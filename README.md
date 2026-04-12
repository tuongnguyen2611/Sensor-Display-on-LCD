#  Display sensor on LCD
## Giới thiệu

Dự án xây dựng hệ thống đo nhiệt độ và độ ẩm sử dụng vi điều khiển STM32F401RE, kết hợp:  

Cảm biến SI7020 (giao tiếp I2C)  
Màn hình LCD (giao tiếp SPI – dùng thư viện Ucglib)  

## Hệ thống thực hiện:

Đọc dữ liệu cảm biến theo chu kỳ  
Hiển thị dữ liệu lên LCD theo điều kiện thông minh  
## Mục tiêu
Giao tiếp I2C với cảm biến SI7020  
Tính toán nhiệt độ và độ ẩm từ dữ liệu raw  
Hiển thị dữ liệu lên LCD qua SPI  
Giảm nhiễu và tránh cập nhật dư thừa  
## Phần cứng sử dụng 
STM32F401RE  
Cảm biến SI7020  
LCD SPI (Ucglib)  
## Nguyên lý hoạt động

Sau khi khởi tạo hệ thống, chương trình sẽ chạy vòng lặp vô hạn. Trong mỗi vòng lặp:  

Timer được cập nhật  
Dữ liệu cảm biến được đọc theo chu kỳ  
Giá trị được xử lý  
LCD được cập nhật nếu thỏa điều kiện  
## Chu kỳ hoạt động  
Dữ liệu cảm biến được lấy mỗi 1 giây  
LCD được cập nhật mỗi 5 giây  
## Chuyển đổi dữ liệu

Giá trị raw từ cảm biến sẽ được chuyển đổi sang đơn vị thực tế theo công thức:  

Nhiệt độ = (175.72 × raw / 65536) − 46.85  
Độ ẩm = (125 × raw / 65536) − 6  

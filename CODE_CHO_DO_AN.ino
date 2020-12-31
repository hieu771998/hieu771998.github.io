// KHAI BÁO CÁC THƯ VIỆN CHO CHƯƠNG TRÌNH 
#include <ESP8266WiFi.h> //thư viện cho module
#include <time.h> //thư viện thời gian
#include <Adafruit_GFX.h> //thư viện màn hình để hiển thị các định dạng
#include <FreeMonoBold12pt7b.h> //thư viện cho font chữ
#include <kongtext4pt7b.h> //thư viện cho font chữ
#include <PxMatrix.h> //thư viện cho led matrix
#include <Ticker.h> //thư viện cho arduino
#include "DHT.h" //thư viện cho cảm biến nhiệt độ, độ ẩm dht11


//KHAI BÁO CHO CÁC CHÂN CỦA CẢM BIẾN VÀ MÀN HÌNH LED MATRIX
#define DHTPIN 2 // chân data của cảm biến dht11 
#define DHTTYPE DHT11   // cho biết cảm biến trên là loại DHT 11

Ticker display_ticker;
#define P_LAT 16 // khai báo các chân của màn hình led matrix
#define P_A 10
#define P_B 4
#define P_C 15
#define P_D 12
#define P_OE 5

#define matrix_width 64 // cho biết đây là loại màn hình led matrix 64x32
#define matrix_height 32

//khai báo các kiểu giá trị
DHT dht(DHTPIN, DHTTYPE);
  float Humi; // nhiệt độ và độ ẩm được khai báo theo kiểu số thực
  float Temp;
// CHO CẢM BIẾN BỤI
int rh,t; //int là giá trị của số nguyên float và int đều có giá trị 4 byte
int iled = 0; //pin 2 ket noi voi led cua sensor
int vout = A0; //analog input
float COV_RATIO = 0.2 ;
float NO_DUST_VOLTAGE =  400; // khi mà không có bụi thì giá trị điện áp mà cảm biến trả về là 400mv
float SYS_VOLTAGE = 3300; // đây là khai báo giá trị điện áp đầu vào của cảm biến để nuôi cảm biến, thường là 3,3v hoặc 5v
float density, voltage;
int   adcvalue, level;
int Filter(int m) // hàm lọc để phục vụ cho việc tính toán nồng độ bụi
{
 int flag_first = 0, _buff[10], sum;
 int _buff_max = 10;
  int i;
  
  if(flag_first == 0)
  {
    flag_first = 1;
 
    for(i = 0, sum = 0; i < _buff_max; i++)
    {
      _buff[i] = m;
      sum += _buff[i];
    }
    return m;
  }
  else
  {
    sum -= _buff[0];
    for(i = 0; i < (_buff_max - 1); i++)
    {
      _buff[i] = _buff[i + 1];
    }
    _buff[9] = m;
    sum += _buff[9];
    
    i = sum / 10.0;
    return i;
  }
}
int pollution_level (float x){ //để phục vụ phân ra các trường hợp ô nhiễm nặng hay nhẹ
  if (x <= 35) return 0;
  if (x <= 75) return 1;
  if (x <= 115) return 2;
  if (x <= 150) return 3;
  if (x <= 250) return 4;
  return 5;
  
  }



int timezone = 1; //múi giờ vị trí của người dùng
int dst = 0;
static uint32_t lastTime = 0; // millis() memory
static bool flasher = false;  // hàm bool trả về hai giá trị đúng hoặc sai
uint8_t frameDelay = 10;  // giá trị cho độ trễ (để phục vụ cho việc tính s)

int h, m, s, d; // khai báo giờ phút giây và ngày
uint8_t dow;
int  day;
uint8_t month;
String  year;
String date;
String WeatherT;
String WeatherH;
String  text;
#define MAX_MESG  9
#define MAX_MES  20
#define MAX_ME  50
#define BUF_SIZE  612
char curMessage[BUF_SIZE] = { "DAI HOC GIAO THONG VAN TAI" }; // hiển thị dòng chữ này chạy bên dưới màn hình
char newMessage[BUF_SIZE];
bool newMessageAvailable = false;// khai báo  kiểu boll sẽ trả về hai giá trị đúng hoặc sai
static bool Mode = true;
uint8_t r = 0, g = 0, b = 0; 
unsigned int NewRTCh = 24; // giờ 
unsigned int NewRTCm = 60; // phút 
unsigned int NewRTCs = 10;//có thể nhảy 10s một lần
char szTime[4];    
char szMesg[10] = "";
char  szBuf[10];

const char* ssid     = "Xiaomi";     // tên wifi
const char* password = "88888888";   // mật khẩu wifi
WiFiServer server(80);

uint8_t display_draw_time = 10; //điều chỉnh độ sáng màn hình từ 10-50 là hợp lý

PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D); // các chân nối sang led vi điều khiển

//định nghĩa một vài loại màu 
uint16_t myRED = display.color565(255, 0, 0);
uint16_t myGREEN = display.color565(0, 255, 0);
uint16_t myBLUE = display.color565(0, 0, 255);
uint16_t myWHITE = display.color565(255, 255, 255);
uint16_t myYELLOW = display.color565(255, 255, 0);
uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myMAGENTA = display.color565(255, 0, 255);
uint16_t myBLACK = display.color565(255, 255, 255);

uint16_t myCOLORS[8] = {myRED, myGREEN, myBLUE, myWHITE, myYELLOW, myCYAN, myMAGENTA, myBLACK};


#ifdef ESP8266
//làm mới màn hình khi đã nhận được tín hiệu wifi
void display_updater()
{
  display.display(display_draw_time);
}
#endif

void display_update_enable(bool is_enable)
{

#ifdef ESP8266
  if (is_enable)
    display_ticker.attach(0.002, display_updater);
  else
    display_ticker.detach();
#endif

}

void(* resetFunc) (void) = 0;
char *mon2str(uint8_t mon, char *psz, uint8_t len)// khai báo cho các tháng
{
  static const char str[][4] PROGMEM =
  {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "DEc"
  };

  *psz = '\0'; // \0 tương tự với NULL
  mon--; // mon là tháng
  if (mon < 12) // nếu mon nhỏ hơn 12 thì tiến dần lên
  {
    strncpy_P(psz, str[mon], len);
    psz[len] = '\0';
  }

  return (psz); // đặt lại giá trị ban đầu
}
// khai báo cho các ngày
char *dow2str(uint8_t code, char *psz, uint8_t len)
{
  static const char str[][10] PROGMEM =
  {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
  };

  *psz = '\0';
  code--; // code là phần khai báo cho ngày
  if (code < 7)
  {
    strncpy_P(psz, str[code], len);
    psz[len] = '\0';
  }

  return (psz);
}
// lấy dữ liệu ngày tháng , phút , giây
void getDateWeb()
{
  date = dow2str(dow, szBuf, sizeof(szBuf) - 1);
  date += " ";
  if (day < 10) { // nếu như ngày nhỏ hơn 10 thì thêm số 0 vào đầu 
    date += "0";
  }
  date += day;
  date += mon2str(month, szBuf, sizeof(szBuf) - 1);
  date += year;
  date += "   ";
  if (h < 10) { // nếu giờ nhỏ hơn 10 thì thêm số 0 vào đầu
    date += "0";
  }
  date += h;
  date += ":";
  if (m < 10) { // nếu phút nhỏ hơn 10 thì thêm số 0 vào đầu
    date += "0";
  }
  date += m;
  date += ":";
  if (s < 10) { // nếu giây nhỏ hơn 10 thì thêm số 0 vào đầu
    date += "0";
  }
  date += s;
}

// DỌC DỮ LIỆU CHO CẢM BIẾN DHT11
void getWeather()
{
  dht.begin(); // khởi tạo cảm biến
  Temp = dht.readTemperature(); // đọc nhiệt độ
  Humi = dht.readHumidity(); // đọc độ ẩm
  if (isnan(Humi) || isnan(Temp)) { // || có nghĩa là và
      delay(100);
    return;
  }
  WeatherT = "Temperature : ";
  WeatherT += Temp;
  WeatherT += " $"; // kí hiệu độ c
  WeatherH = "Humidity : ";
  WeatherH += Humi;
  WeatherH += " %";
  t = Temp; 
  rh = Humi;
}


void getDate()
// hiển thị ngày tháng lên màn hình
{
  text = mon2str(month, szBuf, sizeof(szBuf) - 1); 
  display.setCursor(0, 0);// hiển thị lên dòng thứ 0 và ô thứ 0
  display.fillRect(0, 0, 64, 8, display.color565(0, 0, 0)); // làm sạch màn hình từ ô thứ 0 tới ô thứ 64 của hàng ngang và từ ô thứ 4 tới ô thứ 8 của hàng dọc
  display.setFont(&kongtext4pt7b); // font chữ
  display.setTextColor(myRED); // màu chữ
  if (day < 10) {  // nếu ngày nhỏ hơn 10 thêm 0 và đầu
    display.print("0");
  }
  display.print(day); // hiển thị ngày 
  display.setTextColor(myGREEN);
  display.print(text);
  display.setTextColor(myRED);
  display.print(year); // hiển thị năm
  display.setFont();
}
void getDowe()
{
  text = dow2str(dow, szBuf, sizeof(szBuf) - 1);
  uint16_t text_length = text.length();
  int xpos = (matrix_width - text_length * 7) / 2; 
  display.setCursor(xpos, 0); // hình như là hiển thị một đoạn xong sẽ hiển thị sang đoạn khác
  display.fillRect(0, 0, 64, 8, display.color565(0, 0, 0)); 
  display.setFont(&kongtext4pt7b);
  uint8_t y = 0;
  for (y = 0; y < 10; y++) {
    display.setTextColor(Whel(y));// hiển thị các màu chữ
    display.print(text[y]);
  }
  display.setFont();
}
void getRTCh(char *psz)
//hiển thị giờ
{
  sprintf(psz, "%02d", h); // hiển thị giờ %02d là sẽ in ra số có hai chữ số nếu không có số đằng trước sẽ thêm số 0 vào, ví dụ 8 thì sẽ thêm vào thành 08
  display.setCursor(10, 16); // hiển thị vào ô thứ 10 hàng ngang và ô 16 hàng dọc
  display.setFont(&kongtext4pt7b); // font chữ
  display.setTextColor(myBLUE); // màu chữ
  display.fillRect(0, 8, 24, 15, display.color565(0, 0, 0)); 
  display.print(szTime); // lấy dữ liệu thời gian
  display.setFont();
  NewRTCh = h;
}

void getRTCm(char *psz)
// hiển thi phút
{
  sprintf(psz, "%02d", m);
  display.setCursor(29, 16); // ô thứ 29 , của dòng 16
  display.setFont(&kongtext4pt7b);
  display.setTextColor(myBLUE);
  display.fillRect(26, 8, 25, 15, display.color565(0, 0, 0)); // làm sạch màn hình 
  display.print(szTime);
  display.setFont();
  NewRTCm = m;
}

// phục vụ cho hiển thị s và các phần như dấu : , có thể có hình trái tim chạy theo s 
void getTim(char *psz, bool f = true)
{
  // loại này sẽ hiển thị 1s thì sẽ chuyển
  if (NewRTCs != s / 10)
  {
    display.setCursor(21, 14); 
    display.setTextSize(2); // nếu là 1 sẽ hiển thị : thay vì ::
    display.setTextColor(myCOLORS[g]);
    //display.fillRect(25, 17, 2, 6, display.color565(0, 0, 0)); // đây là xóa màn hình lúc đầu là 24, 17, 2, 6, sửa 24 thành 25 sẽ nhấp nháy cả hai dòng :: và dòng này để tạo nhấp nháy cho dấu ::
    display.print(f ? ':' : ' ');


    //HIỂN THỊ U/MG
    display.setCursor(63, 7); 
    display.setTextSize(1);
    display.setFont(&kongtext4pt7b);
    display.setTextColor(myCOLORS[b]); // 
   //display.print(f ? ' ' : '*');// kí hiệu hình trái tim là *
    display.print(".");
    display.setFont();


    display.setCursor(28, 8); // hiển thị nồng độ bụi 
    display.setTextColor(myBLUE);
    sprintf(psz, "%02d", s);
    display.fillRect(25, 8, 45, 7, display.color565(0, 0, 0));
    display.setFont(&kongtext4pt7b);
    display.print(density);
    display.setFont();

       
    // ĐÂY LÀ HIỂN THỊ SỐ S LÊN MÀN HÌNH
    display.setCursor(49, 16);
    display.setTextSize(1);
    display.setTextColor(myCOLORS[r]); 
    sprintf(psz, "%02d", s);
    display.fillRect(49, 17, 15, 6, display.color565(0, 0, 0)); 
    display.setFont(&kongtext4pt7b);
    display.print(szTime);
    display.setFont();
    NewRTCs = s / 10; // NẾU KHÔNG ĐEM CHIA 10 THÌ SẼ HIỂN THỊ 10 S MỘT LẦN
  }
  // loại này hiển thị 10s thì sẽ chuyển
  else
  {
    display.setCursor(21, 14);  
    display.setTextSize(2);
    display.setTextColor(myCOLORS[g]);
   display.print(f ? ':' : ' ');

   // HIỂN THỊ U/MG
   display.setCursor(63, 7); // ô hiển thị trái tim
   display.setTextSize(1);
   ////display.fillRect(54, 10, 10, 6, display.color565(0, 0, 0));
   //display.setFont(&kongtext4pt7b);
   display.setTextColor(myCOLORS[b]);
   //// display.print(f ? ' ' : '*'); // đây là kí hiệu cho hình trái tim là *
   display.print(".");
   display.setFont();

    display.setCursor(28, 8); // hiển thị nồng độ bụi
    display.setTextColor(myBLUE);
    sprintf(psz, "%02d", s);
    display.fillRect(25, 8, 45, 7, display.color565(0, 0, 0));
    display.setFont(&kongtext4pt7b);
    display.print(density);
    display.setFont();

    // ĐÂY LÀ HIỂN THỊ SỐ S LÊN MÀN HÌNH
    display.setCursor(49, 16); 
    display.setTextColor(myCOLORS[r]);
    sprintf(psz, "%02d", s);
    display.fillRect(49, 17, 17, 7, display.color565(0, 0, 0));
    display.setFont(&kongtext4pt7b);
    display.print(szTime);
    display.setFont();
  }
}

void scroll_text(uint8_t ypos, unsigned long scroll_delay, String text) // cuộn dòng chữ đang chạy 
{
  uint16_t text_length = text.length();
  // giả sử chiều rộng trung bình của kí tự là 5 pixel
  for (int xpos = matrix_width; xpos > -(matrix_width + text_length * 5); xpos--)
  {
    handleWiFi();
    display.setCursor(xpos, ypos);
    display.fillRect(0, 23, 64, 8, display.color565(0, 0, 0));
    display.setFont(&kongtext4pt7b);
    display.print(text);
    display.setFont();
    delay(scroll_delay);
    yield();
    if (millis() - lastTime >= 1000)
    {
      lastTime = millis();
      updateTime();
      getTim(szTime, flasher);
      flasher = !flasher;
      if (NewRTCh != h)
      {
        getTime();
        getRTCh(szTime);
      }
      if (NewRTCm != m)
      {
        getRTCm(szTime);
        getWeather();
        Mode = true;
      }
    }
  }
  r++;
  if (r == 8) {
    r = 0;
    g++;
    if (g == 8) {
      g = 0;
      b++;
      if (b == 8) {
        b = 0;
      }
    }
  }
}


// #ff0000 màu đỏ
// #ffff00 màu vàng
// #ffffff màu trắng
// #ff00ff màu tím
// #ff93bd màu trắng mờ
// #0000ff màu xanh

const char WebResponse[] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n"; 
char WebPage[] = 
  "<!DOCTYPE html>" \ 
  "<html>" \
  "<head>" \
  "<meta http-equiv=\"refresh\" content=\"60\"; url=\"http://192.168.1.24\" />" \
  "<title>ESP8266 Weather Station</title>" \ 
  "<script>" \
  "strLine = \"\";" \
  "function DisplayCurrentTime()" \ 
  "{" \
  "  var dt = new Date();" \
  "  var weekday = new Array(7);" \
  "  weekday[0] = \"Sunday\";" \
  "  weekday[1] = \"Monday\";" \
  "  weekday[2] = \"Tuesday\";" \
  "  weekday[3] = \"Wednesday\";" \
  "  weekday[4] = \"Thursday\";" \
  "  weekday[5] = \"Friday\";" \
  "  weekday[6] = \"Saturday\";" \
  "  var dow = weekday[dt.getDay()];" \
  "  document.getElementById(\"datetime\").innerHTML = (dow) +\" \"+ (dt.toLocaleString());" \ 
  "  setTimeout('DisplayCurrentTime()', 1000);" \
  "}" \
  "function SendData()" \
  "{" \
  "  nocache = \"/&nocache=\" + Math.random() * 1000000;" \
  "  var request = new XMLHttpRequest();" \
  "  strLine = \"&MSG=\" + document.getElementById(\"data_form\").Message.value;" \
  "  strLine = strLine + \"/&SP=\" + document.getElementById(\"data_form\").Speed.value;" \
  "  request.open(\"GET\", strLine + nocache, false);" \
  "  request.send(null);" \
  "}" \
  "function restart()" \ 
  "{" \
  "  nocache = \"/&nocache=\" + Math.random() * 1000000;" \
  "  var request = new XMLHttpRequest();" \
  "  strLine = \"/&Rst=\" + document.getElementById(\"date_form\").rerset.value;" \
  "  request.open(\"GET\", strLine + nocache, false);" \
  "  request.send(null);" \
  "}" \
  "function resttime()" \ 
  "{" \
  "  nocache = \"/&nocache=\";" \
  "  var request = new XMLHttpRequest();" \
  "  strLine = \"/&RT=\" + document.getElementById(\"date_form\").restmie.value;" \
  "  request.open(\"GET\", strLine + nocache, false);" \
  "  request.send(null);" \
  "}" \
  "document.addEventListener('DOMContentLoaded', function() {" \
  "DisplayCurrentTime();" \
  "}, false);" \
  "</script>" \
  "<style>" \
  "body {" \
  "text-align: center;" \
  "margin: 5;" \
  "padding: 5;" \
  "background-color: rgba(72,72,72,0.4);" \
  "}" \
  "#wrapper { " \
  "margin: 0 auto;" \
  "width: 100%;" \
  "}" \
  "#form-div {" \
  "background-color: rgba(72,72,72,0.4);" \
  "padding-left: 10px;" \
  "padding-right: 10px;" \
  "padding-bottom: 80px;" \
  "padding-top: 5px;" \
  "-moz-border-radius: 7px;" \
  "-webkit-border-radius: 7px;" \
  "}" \
  ".data-input {" \
  "padding: 10px 5px 5px 5px;" \
  "background-color: #bbbbff;" \
  "font-size:26px;" \
  "color:red;" \
  "padding-bottom:46px;" \
  "border: 5px solid #444444;" \
  "-moz-border-radius: 7px;" \
  "-webkit-border-radius: 7px;" \
  "}" \
  ".button-blue {" \
  "padding: 5px 5px 5px 5px;" \
  "float:left;" \
  "width: 100%;" \
  "border: #fbfb00 solid 3px;" \
  "cursor:pointer;" \
  "background-color: #4444ff;" \
  "color:white;" \
  "font-size:20px;" \
  "padding-bottom:5px;" \
  "font-weight:700;" \
  "-moz-border-radius: 7px;" \
  "-webkit-border-radius: 7px;" \
  "}" \
  ".button-blue:hover {" \
  "background-color: #2222aa;" \
  "color: #ff93bd;" \
  "}" \
  ".text {" \
  "background-color: #0000ff;" \
  "font-size:76px;" \
  "color: #ffff99;" \
  "}" \
  "table {" \
  "border: 2px solid #ff00ff;" \
  "background-color: #ffffff;" \
  "width:100%;" \
  "color: #0000ff;" \
  "-moz-border-radius: 7px;" \
  "-webkit-border-radius: 7px;" \
  "}" \
  "h1 {" \
  "color: #ffffff;" \
  "background-color: #0000ff;" \
  "-moz-border-radius: 7px;" \
  "-webkit-border-radius: 7px;" \
  "}" \
  "tr {" \
  "border: 2px solid #0000ff;" \
  "background-color: #ffffff;" \
  "color: #0000ff;" \
  "-moz-border-radius: 7px;" \
  "-webkit-border-radius: 7px;" \
  "}" \
  "td {" \
  "border: 2px solid #ff0000;" \
  "padding: 16px;" \
  "-moz-border-radius: 7px;" \
  "-webkit-border-radius: 7px;" \
  "}" \
  "</style>" \
  "</head>" \
  "<body>" \
  "<div id=\"wrapper\">" \
  "<div id=\"form-div\">" \
  "<div class=\"submit\">" \
  "<h1>ESP8266 Weather Station</h1>" \
  "<h1>Local Date-Time: <span id=\"datetime\"></span></h1>" \
  "</div>" \
  "<form id=\"data_form\" name=\"frmText\">" \
  "<div class=\"data-input\">" \
  "Message <input type=\"text\" name=\"Message\" maxlength=\"655\" class=\"button-white\" style=\"width:90%;height:35px;font-size:30px\">" \
  "<input type=\"range\" name=\"Speed\"min=\"0\" max=\"50\" class=\"button-blue\" style=\"width:76%\">" \
  "<input type=\"submit\" value=\"Send\" style=\"width:20%;float:right\" onclick=\"SendData()\" class=\"button-blue\">" \
  "</div></form></br></br>" \
  "<form id=\"date_form\" name=\"frmText\">" \
  "<div class=\"data-input\">" \
  "<b class=\"button-blue\" style=\"width:8%\">GMT+</b>" \
  "<input type=\"number\" value=\"0\" name=\"restmie\" min=\"-12\" max=\"+12\" class=\"button-blue\" style=\"width:6%\">" \
  "<input type=\"submit\" value=\"Update Time\" onClick=\"resttime()\" class=\"button-blue\" style=\"width:80%\">" \
  "</div><div class=\"data-input\">" \
  "<input type=\"submit\" name = \"rerset\" value=\"Restart ESP8266\" onClick=\"restart()\" class=\"button-blue\">" \  
  "</div></form>" \
  "<div class=\"data-input\">" \
  "<table>" \
  "<tr>" \
  "<td style=\"width:40%\"><b>concentration of dust</b></td>" \
  "<td style=\"width:28%\"><b>Temp. Celsius</b></td>" \
   "<td style=\"width:28%\"><b>Humidity</b></td>" \
   "</tr>" \
  "<tr class=\"text\">";
  

uint8_t htoi(char c) // cuộn dữ liệu
{
  c = toupper(c);
  if ((c >= '0') && (c <= '9')) return (c - '0');
  if ((c >= 'A') && (c <= 'F')) return (c - 'A' + 0xa);
  return (0);
}

void getData(char *szMesg, uint8_t len) // đọc dữ liệu cho websever
// đoạn văn bản có thể chứa
// một đoạn văn (/&MSG=)
// cuộn chữ (/&I=)
// tốc độ (/&SP=)
{
  char *pStart, *pEnd;  // điểm ban đầu và điểm kết thúc

  // check text message
  pStart = strstr(szMesg, "/&MSG=");
  if (pStart != NULL) // thiết lập điểm bắt đầu
  {
    char *psz = newMessage;

    pStart += 6;  
    pEnd = strstr(pStart, "/&");

    if (pEnd != NULL) // thiết lập điểm kết thúc
    {
      while (pStart != pEnd)
      {
        if ((*pStart == '%') && isdigit(*(pStart + 1)))
        {
          char c = 0;
          pStart++;
          c += (htoi(*pStart++) << 4);
          c += htoi(*pStart++);
          *psz++ = c;
        }
        else
          *psz++ = *pStart++;
      }

      *psz = '\0'; // kết thúc chuỗi
      newMessageAvailable = (strlen(newMessage) != 0);
    }
  }
  // tạo tốc độ cho dòng chữ đang chạy
  pStart = strstr(szMesg, "/&SP=");
  if (pStart != NULL)
  {
    pStart += 5; 

    int16_t speed = atoi(pStart);
    frameDelay = speed;
  }

  pStart = strstr(szMesg, "/&RT=");
  if (pStart != NULL)
  {
    pStart += 5 ; 

    timezone = atoi(pStart);
    getTime();
    updateTime();
  }

  pStart = strstr(szMesg, "/&Rst=");
  if (pStart != NULL)
  {
    pStart += 6;  
    if (*pStart != NULL)
    {
      resetFunc();
    }
  }
}

void handleWiFi(void) // phần này để thiết lập cho node MCU PHẦN NÀY PHỤC VỤ CHO TRUYỀN DỮ LIỆU LÊN WEB SERVER
{
  static enum { S_IDLE, S_WAIT_CONN, S_READ, S_EXTRACT, S_RESPONSE, S_DISCONN } state = S_IDLE;
  static char szBuf[1024];
  static uint16_t idxBuf = 0;
  static WiFiClient client;
  static uint32_t timeStart;

  switch (state)
  {
    case S_IDLE:   // khởi tạo
      idxBuf = 0;
      state = S_WAIT_CONN;
      break;

    case S_WAIT_CONN:   //chờ cho việc kết nối
      {
        client = server.available();
        if (!client) break;
        if (!client.connected()) break;
        timeStart = millis();
        state = S_READ;
      }
      break;

    case S_READ: //đọc dữ liệu

      while (client.available())
      {
        char c = client.read();

        if ((c == '\r') || (c == '\n'))
        {
          szBuf[idxBuf] = '\0';
          client.flush();
          state = S_EXTRACT;
        }
        else
          szBuf[idxBuf++] = (char)c;
      }
      if (millis() - timeStart > 1000)
      {
        state = S_DISCONN;
      }
      break;

    case S_EXTRACT: // trích xuất dữ liệu  
      //trích xuất chuỗi từ tin nhắn nếu có
    getData(szBuf, BUF_SIZE); // lấy dữ liêu từ phần khai báo ở trên
      state = S_RESPONSE;
      break;

    case S_RESPONSE: // gửi phản hồi tới client
      getDateWeb();
      client.print(WebResponse);
      client.print(WebPage);  
      sendXMLFile(client); // truyền file
      state = S_DISCONN;
      break;

    case S_DISCONN: // ngắt kết nối client
      client.flush();
      client.stop();
      state = S_IDLE;
      break;

    default:  state = S_IDLE;
  }
}



void setup() {

// CHO CẢM BIẾN BỤI
  pinMode(iled, OUTPUT); // cấu hình chân output
  digitalWrite(iled, LOW);// tín hiệu ouput ở mức thấp
  Serial.begin(115200);//khai bao UART , baudrate 115200  
  display.begin(16);


  display.setFastUpdate(true); // tốc độ update dữ liệu lên webserver
  display.setRotation(0); 
  display.clearDisplay();
  display.setTextColor(myCYAN);
  display.setCursor(2, 0);
  display.println("Connecting");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.print(".");
  }
  display.setTextWrap(false);
  display.clearDisplay();
  display.println("");
  display.print("WiFi OK"); // hiển thi dòng chữ này lên sau khi kết nối thành công
  display_update_enable(true);
  delay(3000);
  getDate();
  getWeather();
  // Start the server
  server.begin();
  newMessage[0] = '\0'; // '\0' tương đương Null

  Serial.println(WiFi.localIP()); // hiển thị địa chỉ web lên serial
}

void loop() { // vòng lặp chính để hiển thị độ ẩm , nhiệt độ, và dòng chữ chạy ở dưới


/*
  get adcvalue
  */
  digitalWrite(iled, HIGH); //  chân iled của cảm biến ở mức high
  delayMicroseconds(280); // giá trị delay của cảm biến
  adcvalue = analogRead(vout);   // doc gia tri adc
  digitalWrite(iled, LOW); //   iled low
  
  adcvalue = Filter(adcvalue);  //  loc adc
  
  /*
  chuyen muc dien ap (mv)
  */
  voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11; // phục vụ cho công thức tính nồng độ bụi
  
  /*
chuyen tu dien ap sang do bui
  */
  if(voltage >= NO_DUST_VOLTAGE) // khi mà điện áp trả về từ sensor lớn hơn giá trị điện áp sẵn có của nó là 400mv thì mới bắt đầu tính nồng độ bụi 
  {
    voltage -= NO_DUST_VOLTAGE;
    
    density = voltage * COV_RATIO;
  }
  else
    density = 0;
 
  Serial.print("The current dust concentration is: "); // hiển thị nồng độ bụi nên serial
  Serial.print(density);
  Serial.print(" ug/m3\n"); 
  delay(1000);


 level = pollution_level (density);
  switch(level) {                 
    case 0: {   // mức tốt
  display.setCursor(0, 8); 
  display.setFont(&kongtext4pt7b);
  display.setTextColor(myYELLOW);
 // display.color565(0, 0, 0);
  display.fillRect(0, 8, 40, 7, display.color565(0, 0, 0)); 
  display.print("TOT");
  display.setFont();
  break;
    }
       case 1: {  // mức bụi
  display.setCursor(0, 8); 
  display.setFont(&kongtext4pt7b);
  display.setTextColor(myGREEN);
  display.fillRect(0, 8, 40, 7, display.color565(0, 0, 0)); 
  display.print("TOT");
  display.setFont();
  break;
    }
       case 2: {  // mức xấu
  display.setCursor(0, 8);  
  display.setFont(&kongtext4pt7b);
  display.setTextColor(myBLUE);
  display.fillRect(0, 8, 40, 7, display.color565(0, 0, 0));  
  display.print("TOT"); 
  display.setFont();
  break;
    }
       case 3: { // mức xấu 1                        
  display.setCursor(0, 8);  
  display.setFont(&kongtext4pt7b);
  display.setTextColor(myRED);
display.fillRect(0, 8, 40, 7, display.color565(0, 0, 0));  
  display.print("TOT"); 
  display.setFont();
  break;
    }
       case 4: { // mức xấu 2                        
  display.setCursor(0, 8);  
  display.setFont(&kongtext4pt7b);
  display.setTextColor(myYELLOW);
  display.fillRect(0, 8, 40, 7, display.color565(0, 0, 0));  
  display.print("TOT"); 
  display.setFont();
  break;
    }
       case 5: { // mức xấu 3 
  display.setCursor(0, 8);  
  display.setFont(&kongtext4pt7b);
  display.setTextColor(myYELLOW);
display.fillRect(0, 8, 40, 7, display.color565(0, 0, 0));  
  display.print("TOT"); 
  display.setFont();
  break;
    }    
  }
  

  if (newMessageAvailable)
  {
   strcpy(curMessage, newMessage);
    newMessageAvailable = false;
  }
  if (Mode)
  {
    getDowe();
    scroll_text(23, 12, WeatherT); // cuộn dòng chữ và nhiệt độ độ ẩm ở ô thứ 23 dòng thứ 12 
    scroll_text(23, 12, WeatherH);
    getDate();
    Mode = false;
  }
 scroll_text(23, frameDelay, curMessage); 
}

//LẤY DỮ LIỆU ĐANG CHẠY Ở DƯỚI
void getTime()
{

  configTime(timezone * 25200, dst, "pool.ntp.org", "time.nist.gov"); // lấy dữ liệu ngày và tháng từ hai web này // thay 3600 bằng 25200(múi giờ +7 của Việt Nam)

  while (!time(nullptr)) {
    display.print(".");
  }
}

void updateTime() // cài đặt cho thời gian chạy tiến dần nên
{
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  dow = p_tm->tm_wday + 1;
  day = p_tm->tm_mday;
  month = p_tm->tm_mon + 1;
  year = p_tm->tm_year + 1900;
  h = p_tm->tm_hour;
  m = p_tm->tm_min;
  s = p_tm->tm_sec;
}

void sendXMLFile(WiFiClient cl){ /// gửi dữ liệu nhiệt độ, độ ẩm lên webserver
     cl.print("<td style=\"width:40%\">");
     cl.print(density);
     cl.print(" u/mg</td><td style=\"width:40%\">");
     cl.print(t);
     cl.print(".0 *C</td><td style=\"width:28%\">");
     cl.print(rh);
     cl.print(".0 %</td></tr></table></div><br>");
//     cl.print("<div class=\"submit\">");
//     cl.print("<input type=\"button\" value=\"Actualiser\"");
//     cl.print(" onClick=\"javascript:window.location.reload()\" class=\"button-blue\">");// XUẤT RA KHI TRANG WEB SẴN SÀNG
     cl.print("</div><div class=\"submit\">");
     cl.print("<input type=\"button\" value=\"More informations\" onClick=\"Javascript:window.open('https://www.youtube.com/channel/UCCC8DuqicBtP3A_aC53HYDQ/videos');\"");
     cl.print(" class=\"button-blue\"></div>");
     cl.print("</td></tr></table></div><br>");
//     cl.print("<div class=\"submit\" style=\"width:100%\"><h1>");
//     cl.print(date);
//     cl.print("</h1></div>");
     cl.print("</div></div></body></html>");
}

 
//Màu sắc là sự chuyển đổi r - g - b - trở lại r. 
uint16_t Wheel(byte WheelPos) {
  if (WheelPos < 2) {
    return display.color565(255, 0, 0);
  } else if (WheelPos < 5) {
    WheelPos -= 2;
    return display.color565(0, 0, 255);
  } else {
    WheelPos -= 5;
    return display.color565(255, 0, 0);
  }
}
 
uint16_t Whel(byte WheelPos) {
  if (WheelPos < 1) {
    return display.color565(255, 0, 0);
  } else if (WheelPos < 2) {
    WheelPos -= 1;
    return display.color565(0, 255, 0);
  } else if (WheelPos < 3) {
    WheelPos -= 2;
    return display.color565(255, 255, 0);
  } else if (WheelPos < 4) {
    WheelPos -= 3;
    return display.color565(255, 0, 255);
  } else if (WheelPos < 5) {
    WheelPos -= 4;
    return display.color565(0, 0, 255);
  } else if (WheelPos < 6) {
    WheelPos -= 5;
    return display.color565(0, 255, 255);
  } else if (WheelPos < 7) {
    WheelPos -= 6;
    return display.color565(255, 0, 255);
  } else if (WheelPos < 8) {
    WheelPos -= 7;
    return display.color565(255, 255, 0);
  } else if (WheelPos < 9) {
    WheelPos -= 8;
    return display.color565(255, 0, 0);
  } else {
    WheelPos -= 9;
    return display.color565(255, 255, 255);
 }
}

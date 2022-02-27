#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// inisialisasi sensor garis dan motor yang digunakan
const int sensor_ir[] = {7, 6, 5, 4, 3};
const int IN_1 = 13;
const int IN_2 = 12;
const int enable_A = 11;
const int enable_B = 10;
const int IN_3 = 9;
const int IN_4 = 8;

// variabel untuk kontrol pid
float nilai_tujuan = 0; //ini adalah nilai utama saya dan tujuan dari pid.
float nilai_tujuan_sebelum;
float PID_nilai_kesalahan; //berapa nilai kesalahan dari pembacaan sensor garis
float PID_nilai_kesalahan_sebelum;
float waktu, waktu_selisih, waktu_sebelum;
float PID_total;
// variabel pid saya
//////////////////////////////
int kp = 80;
int ki = 6;
int kd = 2;
//////////////////////////////
// nilai p, i, dan d, dan variabel pid lainnya
int nilai_p = 0;
int nilai_i = 0;
int nilai_d = 0;
float nilai_kp_sebelum;
float nilai_ki_sebelum;
float nilai_kd_sebelum;
int nilai_PID_tetap = 0;
// nilai pid sebelum

// ini adalah variabel sensornya
// gunakan sebaik mungkin
int nilai_sensor[] = {0, 0, 0, 0, 0};
int nilai_beban_sensor[] = {-2, -1, 0, 1, 2};
int nilai_simpan[] = {0, 0, 0, 0, 0};
int sum_nilai_sensor = 0;
// nilai akselerasi pada lintasan lurus
int akselerasi = 0;

// inisialisasi lcd
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup()
{
    lcd.init();
    lcd.backlight();
    Serial.begin(9600);
    for (size_t i = 8; i <= 13; i++)
        pinMode(i, OUTPUT);

    for (size_t i = 0; i <= 4; i++)
        pinMode(sensor_ir[i], INPUT);

    waktu = millis();
}

void loop()
{
    for (size_t i = 0; i <= 4; i++)
        nilai_sensor[i] = digitalRead(sensor_ir[i]);

    for (size_t i = 0; i <= 4; i++)
    {
        if (nilai_sensor[i] == 0)
            nilai_simpan[i] = nilai_beban_sensor[i];
        else if (nilai_sensor[i] == 1)
            nilai_simpan[i] = 0;
    }
    // jumlahkan semua nilai sensor
    // sum nilai sensor adalah nilai sensor utama yang akan dijadikan bahan acuan nilai pid
    sum_nilai_sensor = nilai_simpan[0] + nilai_simpan[1] + nilai_simpan[2] + nilai_simpan[3] + nilai_simpan[4];
    // kode puntuk nilai p
    PID_nilai_kesalahan = nilai_tujuan - sum_nilai_sensor;
    Serial.print("nil_kslhn:");
    Serial.print(PID_nilai_kesalahan);
    nilai_p = kp * PID_nilai_kesalahan;
    Serial.print(" kp:");
    Serial.print(nilai_p);
    // kode untuk nilai i dan batasannya
    nilai_i = nilai_i + (ki * PID_nilai_kesalahan);
    if (nilai_i >= 255)
        nilai_i = 255;
    else if (nilai_i <= -255)
        nilai_i = -255;
    Serial.print(" ki:");
    Serial.print(nilai_i);
    // dan ini adalah kode untuk nilai d dan batasannya
    waktu_sebelum = waktu;
    waktu = millis();
    waktu_selisih = (waktu - waktu_sebelum) / 1000;
    nilai_d = kd * ((PID_nilai_kesalahan - PID_nilai_kesalahan_sebelum) / waktu_selisih);
    if (nilai_d >= 255)
        nilai_d = 255;
    else if (nilai_d <= -255)
        nilai_d = -255;
    Serial.print(" kd:");
    Serial.println(nilai_d);
    // jumlahkan semua nilai pid
    PID_total = nilai_p + nilai_i;

    // kode untuk membatasi nilai pid
    if (PID_total >= 255)
        PID_total = 255;
    else if (PID_total <= -255)
        PID_total = -255;

    // ini kode untuk kontrol motor drivernya
    if (sum_nilai_sensor > 0)
    {
        analogWrite(enable_A, PID_total * -1 * 0.5);
        analogWrite(enable_B, PID_total * -1);
        digitalWrite(IN_1, HIGH);
        digitalWrite(IN_2, LOW);
        digitalWrite(IN_3, LOW);
        digitalWrite(IN_4, HIGH);
        akselerasi = 100;
    }
    else if (sum_nilai_sensor < 0)
    {
        analogWrite(enable_A, PID_total);
        analogWrite(enable_B, PID_total * 0.5);
        digitalWrite(IN_1, LOW);
        digitalWrite(IN_2, HIGH);
        digitalWrite(IN_3, HIGH);
        digitalWrite(IN_4, LOW);
        akselerasi = 100;
    }
    else
    {
        digitalWrite(IN_1, HIGH);
        digitalWrite(IN_2, LOW);
        digitalWrite(IN_3, HIGH);
        digitalWrite(IN_4, LOW);
        analogWrite(enable_A, akselerasi);
        analogWrite(enable_B, akselerasi);
        nilai_i = 0;
    }
    akselerasi = akselerasi + 2;
    if (akselerasi >= 255)
        akselerasi = 255;

    // ini kode untuk menampilkan nilai sensor
    lcd.setCursor(0, 0);
    lcd.print("s:");
    for (size_t i = 0; i <= 4; i++)
        lcd.print(nilai_sensor[i]);
    lcd.print(" pid:");
    lcd.print(PID_total);
    lcd.print("      ");

    lcd.setCursor(0, 1);
    lcd.print("sum:");
    lcd.print(sum_nilai_sensor);
    lcd.print("      ");
}
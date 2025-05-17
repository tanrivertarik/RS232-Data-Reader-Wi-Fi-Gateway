# STM32 RS232 Veri Okuyucu ve Wi-Fi Aktarıcı (RS232 Data Reader & Wi-Fi Gateway)

## Proje Özeti

Bu proje, STM32F103C8T6 mikrodenetleyicisi tabanlı bir kart kullanarak RS232 seri portundan gelen verileri okumak, bu verileri işlemek ve ardından bir ESP32 Wi-Fi modülü aracılığıyla kablosuz olarak bir ağa göndermek için tasarlanmıştır. Kart üzerinde MAX232 entegresi ile RS232 seviye dönüşümü, STM32 ile veri işleme ve ESP32 ile Wi-Fi iletişimi bulunmaktadır.

Proje, endüstriyel makinelerden (CNC tezgahları vb.) veya diğer RS232 arayüzlü cihazlardan veri toplamak ve bu verileri uzaktan izleme, analiz veya kayıt amacıyla bir sunucuya veya uygulamaya iletmek için kullanılabilir.

## Donanım Bileşenleri (Şemaya Göre)

*   **Mikrodenetleyici (MCU):** STMicroelectronics STM32F103C8T6
*   **Wi-Fi Modülü:** ESP32-WROOM-32 (veya benzeri bir ESP32 modülü)
*   **RS232 Seviye Dönüştürücü:** MAX232DR (veya benzeri)
*   **DC-DC Dönüştürücüler:**
    *   12V -> 5V (Şemada U4)
    *   5V -> 3.3V (Şemada U5)
*   **Konektörler:**
    *   DB25 Dişi (J1 - RS232 Girişi)
    *   USB Mini-B (USB1 - Sadece programlama/debug için kullanılmıyor, harici güç gerekli)
    *   DC Güç Giriş Jakı (J2 - 12V DC)
    *   SWD Debugger Header (J3 - Programlama/Hata Ayıklama için)
*   **Pasif Bileşenler:** Dirençler, kapasitörler, LED'ler, kristaller (16MHz MCU için, ESP32 için kendi kristali)

## Temel İşlevsellik

1.  **RS232 Veri Alımı:** DB25 konektörü üzerinden gelen RS232 sinyalleri MAX232 entegresi ile TTL seviyesine dönüştürülür.
2.  **STM32 UART Haberleşmesi (RS232):** TTL seviyesindeki veriler STM32'nin USART1 arayüzü (PA9 - TX, PA10 - RX) tarafından okunur ve STM32'den RS232 cihazına veri gönderilebilir.
3.  **Wi-Fi Modülü Kontrolü:**
    *   RS232 üzerinden veri akışı başladığında, STM32 (PB0 pini üzerinden) ESP32 modülünü aktif hale getirir (Enable pinini HIGH yapar).
    *   Belirli bir süre RS232'den veri gelmediğinde veya özel bir "bitiş" komutu alındığında Wi-Fi modülü devre dışı bırakılır.
4.  **STM32 Veri İşleme:** STM32, RS232'den gelen ham verileri alır. Bu aşamada, gelen veriler üzerinde çeşitli işlemler yapılabilir (örneğin, verim hesaplamaları, PID kontrolünden gelen verilerin analizi, veri filtreleme, formatlama vb.).
5.  **STM32 - ESP32 UART Haberleşmesi:** İşlenen veya ham veriler, STM32'nin USART2 arayüzü (PA2 - TX, PA3 - RX) üzerinden ESP32 modülüne gönderilir.
6.  **Wi-Fi İletimi:** ESP32 modülü, STM32'den aldığı verileri Wi-Fi üzerinden (TCP, UDP, HTTP, MQTT vb. protokoller kullanılarak) bir sunucuya, veritabanına veya başka bir ağ cihazına iletir. *Bu kısım ESP32 firmware'i tarafından yönetilir ve bu STM32 projesinin kapsamı dışındadır, ancak STM32 ESP32'ye veriyi sağlar.*
7.  **Durum LED'leri:**
    *   MCU RX LED (D4): MAX232'den STM32'ye veri geldiğinde yanıp söner.
    *   MCU TX LED (D5): STM32'den MAX232'ye veri gittiğinde yanıp söner.
    *   Wi-Fi Out LED (D6): STM32'den ESP32'ye veri gönderildiğinde yanar (STM32 PA1 pini ile kontrol edilir).

## Yazılım ve Geliştirme Ortamı

*   **STM32 Firmware:**
    *   **IDE:** STM32CubeIDE
    *   **Kütüphaneler:** STM32Cube HAL (Hardware Abstraction Layer)
    *   **Programlama Dili:** C
*   **ESP32 Firmware:** (Bu STM32 projesinin bir parçası değildir, ancak tamamlayıcıdır)
    *   **IDE:** Arduino IDE, PlatformIO, ESP-IDF
    *   **Programlama Dili:** C/C++ (Arduino/ESP-IDF)

## Kurulum ve Kullanım (STM32 Firmware)

1.  **Gerekli Yazılımlar:**
    *   [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
    *   ST-Link sürücüleri (STM32CubeIDE ile birlikte gelir veya ayrıca yüklenebilir)
2.  **Projeyi Klonlama/İndirme:**
    ```bash
    git clone https://github.com/KULLANICI_ADINIZ/PROJE_ADINIZ.git
    cd PROJE_ADINIZ
    ```
3.  **Projeyi STM32CubeIDE'de Açma:**
    *   STM32CubeIDE'yi başlatın.
    *   "File" -> "Import..." -> "General" -> "Existing Projects into Workspace" seçin.
    *   "Select root directory" olarak klonladığınız proje klasörünü gösterin.
    *   Projenin listede göründüğünden ve işaretli olduğundan emin olun. "Finish" tıklayın.
4.  **Yapılandırma (Gerekirse):**
    *   Proje, STM32F103C8T6 için yapılandırılmıştır. Farklı bir STM32 hedefi kullanıyorsanız, `.ioc` dosyasını (STM32CubeMX konfigürasyon dosyası) açıp hedefi değiştirmeniz ve kodu yeniden üretmeniz gerekebilir.
    *   **Önemli:** `Core/Inc/main.h` dosyasındaki pin tanımlarının (`WIFI_EN_PIN_Pin`, `WIFI_OUT_LED_PIN_Pin` vb.) donanımınızla eşleştiğinden emin olun. Bu proje `.ioc` dosyasıyla birlikte geliyorsa, bu tanımlar CubeMX tarafından doğru şekilde üretilmiş olmalıdır.
5.  **Derleme:**
    *   Proje üzerinde sağ tıklayın ve "Build Project" seçin veya araç çubuğundaki çekiç ikonuna tıklayın.
6.  **Yükleme:**
    *   ST-Link debugger'ını bilgisayarınıza ve kartınızdaki J3 (SWD) konektörüne bağlayın.
    *   Kartınıza harici 12V güç (J2) uygulayın. **USB üzerinden güç yeterli değildir.**
    *   Proje üzerinde sağ tıklayın ve "Run As" -> "STM32 MCU C/C++ Application" seçin. Alternatif olarak, araç çubuğundaki yeşil "Play" ikonuna tıklayın.
7.  **Test Etme:**
    *   Bir RS232 kaynağını (örneğin, bir PC'deki USB-RS232 adaptörü veya RS232 çıkışı olan bir cihaz) kartın DB25 (J1) portuna bağlayın.
        *   DB25 Pin 2 (RXD - Kartın Alıcısı) -> RS232 Kaynağının TXD Pini
        *   DB25 Pin 3 (TXD - Kartın Vericisi) -> RS232 Kaynağının RXD Pini
        *   DB25 Pin 7 (GND) -> RS232 Kaynağının GND Pini
    *   Bir seri terminal programı (PuTTY, TeraTerm, RealTerm vb.) kullanarak STM32'nin USART1'i üzerinden (MAX232 aracılığıyla) veri gönderin ve alın. Baud rate genellikle 9600 olarak ayarlanmıştır (koddaki `MX_USART1_UART_Init()` fonksiyonunu kontrol edin).
    *   Veri akışı başladığında, ESP32 modülünün aktifleştiğini (Wi-Fi ağına bağlanmaya çalıştığını veya kendi AP'sini oluşturduğunu - ESP32 firmware'ine bağlı olarak) gözlemleyin.
    *   STM32'nin USART2 (PA2 - TX, PA3 - RX) pinlerini bir logic analyzer veya başka bir UART adaptörü ile izleyerek ESP32'ye giden veriyi kontrol edebilirsiniz.

## Kod Yapısı (STM32 Firmware)

*   **`Core/Inc/main.h`**: Ana başlık dosyası. HAL kütüphanesi, pin tanımları ve global fonksiyon prototiplerini içerir.
*   **`Core/Src/main.c`**: Ana program dosyasını içerir. `main()` fonksiyonu, başlangıç ayarları, ana döngü ve kullanıcı fonksiyonları (veri işleme, Wi-Fi modülü kontrolü vb.) buradadır.
*   **`Core/Src/stm32f1xx_hal_msp.c`**: MCU'ya özgü çevre birimi başlatma kodlarını içerir (GPIO, UART vb. için düşük seviye pin ayarları).
*   **`Core/Src/stm32f1xx_it.c`**: Kesme (interrupt) işleyici fonksiyonlarını içerir (örneğin, `USART1_IRQHandler`).
*   **`Drivers/`**: STM32Cube HAL sürücülerini ve CMSIS (Cortex Microcontroller Software Interface Standard) dosyalarını içerir.
*   **`.ioc` dosyası**: STM32CubeMX proje yapılandırma dosyası. Bu dosya ile CubeMX'te projenin pin ve çevre birimi ayarları düzenlenebilir.

## Gelecekteki Geliştirmeler ve İyileştirmeler

*   **ESP32 Firmware Entegrasyonu:** ESP32 için örnek bir firmware eklenerek tam bir uçtan uca çözüm sunulabilir.
*   **Daha Gelişmiş Veri İşleme:** STM32 tarafında daha karmaşık filtreleme, analiz veya şifreleme algoritmaları eklenebilir.
*   **Yapılandırılabilir Parametreler:** Baud rate, Wi-Fi ağ bilgileri gibi parametrelerin UART veya başka bir arayüz üzerinden çalışma zamanında ayarlanabilmesi.
*   **Hata Yönetimi ve Loglama:** Daha kapsamlı hata tespiti ve loglama mekanizmaları.
*   **OTA Güncellemeleri:** STM32 veya ESP32 için kablosuz firmware güncelleme yeteneği.
*   **Güç Yönetimi:** Daha düşük güç tüketimi için uyku modları ve optimizasyonlar.


---

# BSM316_Odev3_21640310016_buse_guvez
Bu proje STM32F103C8 (Blue Pill) mikrodenetleyicisi üzerinde Timer Kesmeleri, Flash Bellek Yönetimi ve GPIO Kontrolü uygulamasıdır. ( BSM316 3. Ödev)

Özellikler:
- Timer Kesmesi: TIM2 saniyede 1 kez kesme üreterek LED'i kontrol eder.  
- Flash Bellek: blink_count değeri enerji kesilse dahi Flash üzerinde saklanır. 
- Modlar: 4-7 arası yanıp sönme ve 5 saniye bekleme döngüsü.
- Reset: Açılışta 3 saniye basılı tutulduğunda fabrika ayarlarına (4) döner.

Derleyici: arm-none-eabi-gcc / Makefile
Editör: VS Code
Hazırlayan: Buse Güvez 21640310016


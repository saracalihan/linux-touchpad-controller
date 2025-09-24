# linux-touchpad-controller

Linux sistem event'leri üzerinden touchpad olaylarını dinler.
Kullanıcılar kontrol panelinden parmak sayısı, konumu ve
süresine göre istedikleri touchpad kullanımına göre 
macro(keyboard, mouse, exec) komutları atayabilir.

## Örnekler

- Touchpad üzerinde neredeyse aynı noktaya bir saniye içinde
  iki kere dokunulursa click macrosu tetiklenir.
- Touchpad'in sağ üstüne 3 saniye boyunca parmak değdirilirse
  google-chrome açılır
- Touchpad'in sol üstüne 3 saniye basılı tutulduktan sonra
  sağındaki 50px'lik alanda parmak yukarı ve aşağı yönde
  haraket ettirilerek ses düzeyi değiştirilir

## Sistem
- [Driver]: Yazılan kernel modülü, touchpad'in dinlenmesinden,
            tcp üzerinden konuşulmasından ve komut çalıştırılmasından
            sorumludur.
- [Sunucu]: Driver ile arayüz arasında aracılık yapar.
            Tcp isteğini WebSocket'e dönüştürür.
- [Panel]: Touchpad'in mevcut durumunu gösterir.
           Kullanıcı mevcut macroları görüntüler ve yeni akışları oluşturabilir.

# License
This project is licensed under the [MIT License](./LICENSE).

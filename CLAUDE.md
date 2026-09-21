# S.E.R.A. — Statie de senzori sera (Arduino/ESP32)

## Ce este proiectul
Firmware PlatformIO pentru un ESP32 care citeste periodic senzori de mediu
(temperatura, umiditate, presiune, lumina, umiditate sol) pentru monitorizarea
unei sere, si afiseaza valorile prin Serial.

Repo GitHub: https://github.com/RaduMatei04/S.E.R.A.---Arduino-Code

## Stack
- Platforma: PlatformIO, `env:esp32dev`, board `esp32dev`, framework `arduino`
- Limbaj: C++ (Arduino framework)
- Librarii (`lib_deps` in `platformio.ini`):
  - `adafruit/Adafruit BME280 Library` — temperatura/umiditate/presiune
  - `adafruit/Adafruit Unified Sensor` — dependenta pentru BME280
  - `claws/BH1750` — senzor de lumina (lux)

## Structura
- `src/main.cpp` — tot firmware-ul (setup/loop, citire senzori, formatare output)
- `include/`, `lib/`, `test/` — foldere standard PlatformIO, goale (doar README-uri placeholder)
- `platformio.ini` — configurare board, porturi, dependinte

## Hardware / pini (vezi `src/main.cpp`)
- I2C: SDA=GPIO21, SCL=GPIO22 (BME280 pe adresa `0x76`, BH1750 pe `0x23`)
- Senzori de sol (analogici): pin1=GPIO32, pin2=GPIO33
- Calibrare sol: `SOIL_DRY_RAW=3000`, `SOIL_WET_RAW=1200` (mapate 0-100%)
- Interval de esantionare: 1000 ms (`SAMPLE_INTERVAL_MS`)
- Port serial: COM3 @ 115200 baud (hardcodat in `platformio.ini` — ajusteaza daca placa e pe alt port)

## Build / upload / monitor
Comenzi PlatformIO CLI standard (necesita `pio` in PATH, sau extensia PlatformIO in VS Code):
```
pio run                # build
pio run -t upload      # upload pe placa (COM3)
pio device monitor      # serial monitor la 115200 baud
```

## Conventii de cod observate
- Cod simplu, procedural, intr-un singur fisier (`main.cpp`); nu s-a introdus
  inca o structura pe mai multe fisiere/clase — pastreaza acest stil pana
  cand complexitatea justifica separarea (ex. adaugarea mai multor senzori
  sau a conectivitatii WiFi).
- Mesajele afisate operatorului (Serial output, etichete) sunt in limba romana
  — pastreaza aceasta conventie pentru orice text nou orientat spre utilizator.
- Flag-uri `*Ok` (`bmeOk`, `lightOk`) marcheaza daca un senzor a fost gasit la
  `begin()`; citirile senzorilor lipsa sunt sarite silentios, nu genereaza erori.

## Git
- Branch principal: `main`
- Remote: `origin` -> repo-ul GitHub de mai sus

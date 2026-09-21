# S.E.R.A. — Statie Electronica de monitorizare a unei Sere Automatizate

Firmware PlatformIO pentru ESP32, care citeste periodic senzori de mediu
(temperatura, umiditate, presiune atmosferica, lumina si umiditate sol) si
afiseaza valorile prin portul serial, pentru monitorizarea conditiilor
dintr-o sera.

## Cuprins
- [Arhitectura](#arhitectura)
- [Hardware](#hardware)
- [Schema de conectare](#schema-de-conectare)
- [Structura proiectului](#structura-proiectului)
- [Cerinte software](#cerinte-software)
- [Instalare si configurare](#instalare-si-configurare)
- [Build, upload, monitor](#build-upload-monitor)
- [Calibrare senzori de sol](#calibrare-senzori-de-sol)
- [Exemplu de output](#exemplu-de-output)
- [Roadmap](#roadmap)

## Arhitectura

Proiectul este un firmware single-board, cu o bucla principala (`loop()`) care
esantioneaza toti senzorii la un interval fix si trimite datele pe serial.
Nu exista (inca) conectivitate retea, persistenta sau logica de actuare —
este in prezent o statie de citire/monitorizare.

```
                        +---------------------+
                        |        ESP32        |
                        |   (env:esp32dev)     |
                        +----------+----------+
                                   |
                 +-----------------+-----------------+
                 |                 |                 |
              I2C Bus         Analog (ADC)       Serial (USB)
           (SDA=21, SCL=22)   GPIO32 / GPIO33      115200 baud
                 |                 |                 |
        +--------+--------+   +----+----+       +----+----+
        |                 |   | Soil #1 |       |  PC /   |
   BME280 (0x76)     BH1750 (0x23) | Soil #2 |       | Monitor |
  Temp/Umid/Presiune   Lumina (lux) +---------+       +---------+
```

Fluxul logic din `src/main.cpp`:

1. **`setup()`** — initializeaza Serial, magistrala I2C si senzorii
   (`initSensors()`); marcheaza fiecare senzor ca disponibil sau nu.
2. **`loop()`** — la fiecare `SAMPLE_INTERVAL_MS` (implicit 1000 ms):
   - citeste BME280 (`readBME`), BH1750 (`readLight`) si senzorii de sol
     (`readSoil`), populand un `struct SensorData`;
   - converteste valorile brute ale senzorilor de sol in procente
     (`soilPercent`) si intr-o eticheta descriptiva (`soilLabel`);
   - afiseaza toate citirile formatate pe Serial (`printAllReadings`).
3. Senzorii care nu au fost gasiti la `begin()` sunt sariti silentios in
   citiri (fara erori repetate), dar semnalati o singura data la boot.

Design-ul este intentionat simplu si intr-un singur fisier, potrivit pentru
stadiul actual al proiectului (citire si afisare). Vezi [Roadmap](#roadmap)
pentru directii de extindere (WiFi/MQTT, actuatori, persistenta).

## Hardware

| Componenta            | Interfata | Detalii                                   |
|------------------------|-----------|--------------------------------------------|
| ESP32 DevKit           | —         | placa principala (`board = esp32dev`)      |
| BME280                 | I2C       | temperatura, umiditate, presiune (adresa `0x76`) |
| BH1750                 | I2C       | intensitate luminoasa in lux (adresa `0x23`) |
| Senzor umiditate sol #1| Analog    | GPIO32                                     |
| Senzor umiditate sol #2| Analog    | GPIO33                                     |

## Schema de conectare

| Semnal      | Pin ESP32 |
|-------------|-----------|
| I2C SDA     | GPIO21    |
| I2C SCL     | GPIO22    |
| Soil sensor 1 (AO) | GPIO32 |
| Soil sensor 2 (AO) | GPIO33 |

BME280 si BH1750 se conecteaza in paralel pe aceeasi magistrala I2C (SDA/SCL),
fiecare cu adresa lui proprie.

## Structura proiectului

```
sera-code/
├── platformio.ini        # configurare board, porturi, dependinte de librarii
├── src/
│   └── main.cpp           # tot firmware-ul: setup/loop, citire senzori, output
├── include/                # headere proprii (gol momentan)
├── lib/                    # librarii private ale proiectului (gol momentan)
├── test/                   # teste PlatformIO (gol momentan)
└── CLAUDE.md               # note de context pentru Claude Code
```

## Cerinte software

- [PlatformIO](https://platformio.org/) (CLI sau extensia pentru VS Code)
- Driver USB-Serial pentru placa ESP32 (ex. CP210x / CH340, in functie de placa)

Dependinte de librarii (instalate automat de PlatformIO din `platformio.ini`):

- `adafruit/Adafruit BME280 Library@^2.2.4`
- `adafruit/Adafruit Unified Sensor@^1.1.14`
- `claws/BH1750@^1.3.0`

## Instalare si configurare

1. Cloneaza repo-ul:
   ```
   git clone https://github.com/RaduMatei04/S.E.R.A.---Arduino-Code.git
   cd S.E.R.A.---Arduino-Code
   ```
2. Deschide folderul in VS Code cu extensia PlatformIO, sau foloseste CLI-ul `pio`.
3. Verifica/ajusteaza portul serial in `platformio.ini` (`upload_port`,
   `monitor_port`) astfel incat sa corespunda portului la care este conectata placa
   (implicit `COM3`).

## Build, upload, monitor

```
pio run                 # compileaza firmware-ul
pio run -t upload       # incarca firmware-ul pe placa
pio device monitor      # deschide monitorul serial (115200 baud)
```

Sau, din VS Code cu extensia PlatformIO, foloseste butoanele din bara de jos
(Build / Upload / Monitor).

## Calibrare senzori de sol

Valorile brute ADC ale senzorilor de sol sunt mapate procentual folosind doua
constante in `src/main.cpp`:

```cpp
const int SOIL_DRY_RAW = 3000; // valoare citita in sol/aer uscat
const int SOIL_WET_RAW = 1200; // valoare citita in sol/apa umeda
```

Pentru calibrare corecta pe senzorii tai:

1. Incarca firmware-ul si citeste valorile raw afisate pe Serial cu senzorul
   complet uscat (in aer) — noteaza valoarea ca `SOIL_DRY_RAW`.
2. Cufunda senzorul in apa sau sol foarte umed — noteaza valoarea ca
   `SOIL_WET_RAW`.
3. Actualizeaza constantele si reincarca firmware-ul.

Procentul rezultat este impartit in patru categorii (`soilLabel`):

| Procent   | Eticheta                |
|-----------|--------------------------|
| < 20%     | USCAT - uda planta       |
| 20-39%    | Umiditate scazuta        |
| 40-69%    | Optim                    |
| >= 70%    | Umed - nu mai uda        |

## Exemplu de output

```
Statie senzori sera - pornita

=== t=1.0s ===
BME280   : 24.3C  55.2%  1012.8hPa
BH1750   : 320.5 lux
Sol P1   : 42% [Optim] (raw 2100)
Sol P2   : 15% [USCAT - uda planta] (raw 2950)
```

## Roadmap

Idei pentru extinderea proiectului (neimplementate momentan):

- Conectivitate WiFi si publicare date (MQTT / HTTP / dashboard)
- Persistenta locala (SD card / SPIFFS) pentru istoricul citirilor
- Actuatori (pompa de udare, ventilatie) controlati pe baza citirilor
- Alerte (ex. sol prea uscat, temperatura prea mare)
- Separarea codului pe module/clase pe masura ce creste complexitatea

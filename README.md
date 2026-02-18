# ESP32-S3 High-Quality Audio Recorder with PCM1809

A professional audio recording system using the ESP32-S3 microcontroller and PCM1809 ADC (Analog-to-Digital Converter) for high-fidelity audio capture to SD card. Features 192kHz sampling rate, 32-bit depth, and automatic gain control (AGC) processing.

## Hardware Requirements

- **Microcontroller**: ESP32-S3
- **ADC**: PCM1809 (Audio Analog-to-Digital Converter)
- **Storage**: MicroSD card module
- **Interface**: I2S for audio, SPI for SD card
- **Power**: 3.3V/5V depending on modules

## Features

- ✨ **High-Quality Audio Recording**
  - 192 kHz sample rate
  - 32-bit depth
  - Mono channel recording
  - WAV file format output

- 🎚️ **Automatic Gain Control (AGC)**
  - Generates 4 versions of each recording with different gain levels:
    - -33 dB (very low gain)
    - -10 dB (low gain)
    - 0 dB (original)
    - +10 dB (boosted)

- ⏱️ **Fixed Recording Duration**
  - Records for 10 seconds automatically
  - Auto-stops when duration reached

- 💾 **SD Card Storage**
  - Fast 80 MHz SPI speed
  - Timestamped filenames
  - Multiple file output

## Pin Configuration

### I2S Connections (PCM1809 ↔ ESP32-S3)

| ESP32-S3 Pin | Function | PCM1809 Pin |
|--------------|----------|-------------|
| GPIO 7       | I2S_SCK  | BCK (Bit Clock) |
| GPIO 15      | I2S_WS   | LRCK (Word Select) |
| GPIO 6       | I2S_DIN  | DOUT (Data Out) |
| 3.3V         | Power    | VCC |
| GND          | Ground   | GND |

### SD Card Module Connections

| ESP32-S3 Pin | Function | SD Card Pin |
|--------------|----------|-------------|
| GPIO 10      | CS       | CS |
| Default MOSI | MOSI     | MOSI |
| Default MISO | MISO     | MISO |
| Default SCK  | SCK      | SCK |
| GPIO 21      | SD_EN    | Enable (if required) |
| 3.3V/5V      | Power    | VCC |
| GND          | Ground   | GND |

## Required Libraries

Install these libraries through the Arduino Library Manager:

- **ESP_I2S** (ESP32 I2S library)
- **SD** (SD card library - included with Arduino)
- **SPI** (SPI communication - included with Arduino)

## Installation

1. **Clone this repository:**
   ```bash
   git clone https://github.com/yourusername/esp32-s3-pcm1809-recorder.git
   ```

2. **Open in Arduino IDE:**
   - Open `pcm1809_to_esp32_s3_esp_i2s_sd_card5.ino`

3. **Install required libraries:**
   - Go to **Sketch** → **Include Library** → **Manage Libraries**
   - Search for and install "ESP_I2S"

4. **Select your board:**
   - **Tools** → **Board** → **ESP32 Arduino** → **ESP32S3 Dev Module**

5. **Configure board settings:**
   - Ensure PSRAM is enabled if your ESP32-S3 has it

6. **Select COM port:**
   - **Tools** → **Port** → Select your ESP32-S3 port

7. **Upload the sketch**

## How It Works

### Recording Process

1. **Initialization**: On startup, the system initializes:
   - SD card (80 MHz SPI)
   - I2S interface (192 kHz, 32-bit, mono)

2. **Recording**: Automatically starts recording:
   - Captures 10 seconds of audio
   - Saves to SD card as `AUDIO_[timestamp].wav`
   - Uses 32-sample buffer for efficient data transfer

3. **AGC Processing**: After recording completes:
   - Reads the original WAV file
   - Applies 4 different gain levels
   - Saves 4 additional files:
     - `AGC_-33dB.wav`
     - `AGC_-10dB.wav`
     - `AGC_0dB.wav`
     - `AGC_10dB.wav`

### File Output

After one recording cycle, you'll have 5 files on the SD card:
- 1 original recording
- 4 AGC-processed versions

## Audio Specifications

| Parameter | Value |
|-----------|-------|
| Sample Rate | 192,000 Hz (192 kHz) |
| Bit Depth | 32-bit |
| Channels | 1 (Mono) |
| Format | WAV (PCM) |
| Recording Time | 10 seconds |
| File Size (approx.) | ~7.3 MB per file |

## Customization

### Change Recording Duration

Modify the `TOTAL_SAMPLES` calculation:
```cpp
const uint32_t TOTAL_SAMPLES = SAMPLE_RATE * 20; // 20 seconds
```

### Change Sample Rate

Adjust the `SAMPLE_RATE` constant:
```cpp
const int SAMPLE_RATE = 48000; // 48 kHz (CD quality)
```
**Note**: PCM1809 supports: 32kHz, 44.1kHz, 48kHz, 96kHz, 192kHz

### Modify AGC Gain Levels

Edit the gain factors in `processAGC()`:
```cpp
float gainLevel1 = pow(10, -20.0 / 20.0);  // -20 dB
float gainLevel2 = pow(10,  -5.0 / 20.0);  //  -5 dB
float gainLevel3 = pow(10,   5.0 / 20.0);  //  +5 dB
float gainLevel4 = pow(10,  15.0 / 20.0);  // +15 dB
```

### Change Buffer Size

Modify for performance tuning:
```cpp
const int BUFFER_SIZE = 64; // Larger = more efficient, more memory
```

### Enable Stereo Recording

```cpp
const int CHANNELS = 2; // Stereo
// Update I2S initialization:
I2S_SLOT_MODE_STEREO
```

## Understanding AGC (Automatic Gain Control)

The AGC feature applies gain to the recorded audio:

**Gain Formula**: `Gain (linear) = 10^(dB / 20)`

**Why AGC is useful:**
- Normalizes volume levels
- Compensates for quiet recordings
- Provides multiple listening options
- Useful for analyzing audio at different levels

## Wiring Diagram

```
PCM1809 ADC          ESP32-S3
-----------          --------
   BCK      -------> GPIO 7  (I2S_SCK)
   LRCK     -------> GPIO 15 (I2S_WS)
   DOUT     -------> GPIO 6  (I2S_DIN)
   VCC      -------> 3.3V
   GND      -------> GND

SD Card Module       ESP32-S3
--------------       --------
   CS       -------> GPIO 10
   MOSI     -------> Default MOSI
   MISO     -------> Default MISO
   SCK      -------> Default SCK
   VCC      -------> 3.3V or 5V
   GND      -------> GND
```

## Troubleshooting

### SD Card Initialization Failed
- Check wiring connections
- Ensure SD card is formatted as FAT32
- Try a different/slower SD card
- Reduce SPI speed: `SD.begin(SD_CS, SPI, 40000000)`

### I2S Initialization Failed
- Verify PCM1809 connections
- Check power supply to PCM1809
- Ensure correct pin configuration
- Verify I2S pins are not used elsewhere

### No Audio / Silent Recording
- Check PCM1809 analog input connections
- Verify input signal level
- Test with AGC boosted files (+10dB)
- Check PCM1809 mode pins (MD0, MD1)

### File Won't Play
- Ensure WAV header is properly written
- Check file size isn't 0 bytes
- Verify sample rate compatibility with player
- Try different audio player software

### Recording Stops Early
- Check SD card free space
- Monitor Serial output for errors
- Verify power supply stability
- Check for SD card write speed issues

## Serial Monitor Output

Expected output during operation:
```
SD card initialized.
I2S initialized.
Recording started: /AUDIO_12345.wav
7680000
7680000
Recording stopped
Processing AGC
AGC applied and saved: /AGC_-33dB.wav
AGC applied and saved: /AGC_-10dB.wav
AGC applied and saved: /AGC_0dB.wav
AGC applied and saved: /AGC_10dB.wav
```

## Technical Details

### WAV File Format
- **Header Size**: 44 bytes
- **Format**: PCM (uncompressed)
- **Endianness**: Little-endian
- **Byte Rate**: 768,000 bytes/second (192kHz × 1 channel × 4 bytes)

### Memory Usage
- **Buffer**: 128 bytes (32 samples × 4 bytes)
- **DMA**: Handled by ESP32-S3 I2S peripheral
- **Minimal RAM usage** - streams directly to SD

## Performance Notes

- **Recording**: Real-time streaming to SD card
- **AGC Processing**: Sequential file processing
- **Total Time**: ~10 seconds recording + ~40 seconds AGC processing
- **SD Card Speed**: Critical for 192kHz recording

## PCM1809 Mode Configuration

The PCM1809 has mode pins (MD0, MD1) that control:
- Sample rate selection
- System clock mode
- Master/Slave mode

Refer to PCM1809 datasheet for proper configuration.

## License

This project is open source and available under the MIT License.

## Contributing

Pull requests are welcome! For major changes, please open an issue first.

## Author

Your Name

## Resources

- [PCM1809 Datasheet](https://www.ti.com/product/PCM1809)
- [ESP32-S3 I2S Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/i2s.html)
- [WAV File Format Specification](http://soundfile.sapp.org/doc/WaveFormat/)
- [ESP32-S3 Datasheet](https://www.espressif.com/en/products/socs/esp32-s3)

## Acknowledgments

- Texas Instruments for the PCM1809 ADC
- Espressif for ESP32-S3 and I2S library
- Arduino community for SD and SPI libraries

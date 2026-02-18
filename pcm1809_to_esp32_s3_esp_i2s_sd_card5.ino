#include <ESP_I2S.h>
#include <SD.h>
#include <SPI.h>

I2SClass i2S;

// I2S pins
const uint8_t I2S_SCK = 7;
const uint8_t I2S_WS = 15;
const uint8_t I2S_DIN = 6;

const uint8_t SD_EN = 21;
// SD card pin
const int SD_CS = 10;

// Audio parameters
const int SAMPLE_RATE = 192000; // 192 kHz
const int BITS_PER_SAMPLE = 32;
const int CHANNELS = 1;

// Buffer settings
const int BUFFER_SIZE = 32; 
int32_t sampleBuffer[BUFFER_SIZE];
String recordedFilename;

// WAV header structure
struct WAVHeader {
    char riffHeader[4] = {'R', 'I', 'F', 'F'};
    uint32_t wavSize = 0;
    char waveHeader[4] = {'W', 'A', 'V', 'E'};
    char fmtHeader[4] = {'f', 'm', 't', ' '};
    uint32_t fmtChunkSize = 16;
    uint16_t audioFormat = 1; // PCM
    uint16_t numChannels = CHANNELS;
    uint32_t sampleRate = SAMPLE_RATE;
    uint32_t byteRate = SAMPLE_RATE * CHANNELS * (BITS_PER_SAMPLE / 8);
    uint16_t blockAlign = CHANNELS * (BITS_PER_SAMPLE / 8);
    uint16_t bitsPerSample = BITS_PER_SAMPLE;
    char dataHeader[4] = {'d', 'a', 't', 'a'};
    uint32_t dataSize = 0;
};

// Recording state
bool isRecording = false;
File wavFile;
WAVHeader header;

// Total samples to record for 10 seconds
const uint32_t TOTAL_SAMPLES = SAMPLE_RATE * 10; // 10 seconds
uint32_t recordedSamples = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) {;} // Wait for Serial
    
    pinMode(SD_EN, OUTPUT);
   digitalWrite(SD_EN, HIGH);  // turn the LED on (HIGH is the voltage level)
    // Initialize SD card
    SPI.begin();
    if (!SD.begin(SD_CS, SPI, 80000000)) { // 80 MHz SPI speed
        Serial.println("SD card initialization failed!");
        return;
    }
    Serial.println("SD card initialized.");

    // Initialize I2S with DMA buffer for faster performance
    i2S.setPins(I2S_SCK, I2S_WS, -1, I2S_DIN);
    if (!i2S.begin(I2S_MODE_STD, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
        Serial.println("Failed to initialize I2S!");
        return;
    }
    Serial.println("I2S initialized.");
    // Start recording
    startRecording();
}

void startRecording() {
    recordedFilename = "/AUDIO_" + String(millis()) + ".wav";
    wavFile = SD.open(recordedFilename, FILE_WRITE);
    if (!wavFile) {
        Serial.println("Could not create file!");
        return;
    }
    // Write placeholder WAV header
    wavFile.write((const uint8_t *)&header, sizeof(WAVHeader));
    isRecording = true;
    recordedSamples = 0;
    Serial.println("Recording started: " + recordedFilename);
}


void stopRecording() {
    if (!isRecording) return;


    uint32_t dataSize = recordedSamples*4;
    // Update WAV header
    header.dataSize = dataSize;
    Serial.println(dataSize);
    Serial.println(TOTAL_SAMPLES*4);
    header.wavSize = dataSize + sizeof(WAVHeader) - 8;

    // Seek back to start and write updated header
    wavFile.seek(0);
    wavFile.write((const uint8_t *)&header, sizeof(WAVHeader));

    wavFile.close();
    isRecording = false;
    Serial.println("Recording stopped");
    processAGC();
    Serial.println("Processing AGC ");
}

void loop() {
    if (isRecording) {
        int bytesRead = i2S.readBytes((char*)sampleBuffer, sizeof(sampleBuffer));
        if (bytesRead > 0) {
            wavFile.write((const uint8_t*)sampleBuffer, bytesRead);
            recordedSamples += (bytesRead / sizeof(int32_t));
        }

        // Stop after recording enough samples
        if (recordedSamples >= TOTAL_SAMPLES) {
            stopRecording();

        }
    }
}

void applyAGC(const char* filename, float gainFactor, const char* outputFilename) {
    File inputFile = SD.open(filename, FILE_READ);
    if (!inputFile) {
        Serial.println("Error: Failed to open input WAV file!");
        return;
    }

    // Read WAV header
    WAVHeader header;
    inputFile.read((uint8_t*)&header, sizeof(WAVHeader));

    // Open output file
    File outputFile = SD.open(outputFilename, FILE_WRITE);
    if (!outputFile) {
        Serial.println("Error: Failed to create output WAV file!");
        inputFile.close();
        return;
    }

    // Write the same header to output file
    outputFile.write((uint8_t*)&header, sizeof(WAVHeader));

    // Process audio samples
    int32_t sample;
    while (inputFile.read((uint8_t*)&sample, sizeof(sample)) == sizeof(sample)) {
        // Apply gain
        float adjustedSample = sample * gainFactor;

        // Clip the sample to 32-bit signed range
        if (adjustedSample > INT32_MAX) adjustedSample = INT32_MAX;
        if (adjustedSample < INT32_MIN) adjustedSample = INT32_MIN;

        int32_t outputSample = (int32_t)adjustedSample;
        outputFile.write((uint8_t*)&outputSample, sizeof(outputSample));
    }

    inputFile.close();
    outputFile.close();

    Serial.println(String("AGC applied and saved: ") + outputFilename);
}


void processAGC() {
    const char* inputFile = recordedFilename.c_str();  // Convert to C-string

    // Gain factors for different levels
    float gainLevel1 = pow(10, -33.0 / 20.0);  // -33 dB
    float gainLevel2 = pow(10, -10.0 / 20.0);  // -10 dB
    float gainLevel3 = pow(10,  0.0  / 20.0);  //  0 dB
    float gainLevel4 = pow(10, 10.0  / 20.0);  // 10 dB

    /*
    "10" is used because dB calculations in electronics are based on log base 10.
    "20" is used because decibels for voltage and amplitude signals use the formula 20 log10(V_out / V_in), not 10 log10(P_out / P_in), which is used for power.
    */

    // Apply AGC and save as separate files
    applyAGC(inputFile, gainLevel1, "/AGC_-33dB.wav");
    applyAGC(inputFile, gainLevel2, "/AGC_-10dB.wav");
    applyAGC(inputFile, gainLevel3, "/AGC_0dB.wav");
    applyAGC(inputFile, gainLevel4, "/AGC_10dB.wav");

}




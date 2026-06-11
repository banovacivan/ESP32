#include <driver/i2s.h>
//#include "Constants2.h"
#include "TrainingData.h"
#include "ModelData.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

#define I2S_WS 15
#define I2S_SD 16
#define I2S_SCK 17
#define I2S_PORT I2S_NUM_0
#define visina 64
#define sirina 128


const int SAMPLE_RATE = 16000;
const int BUFFER_SIZE = 16384;
float audioBuffer[BUFFER_SIZE];

class Complex {
public:
    float real;
    float imaginary;

    Complex(float r = 0.0, float i = 0.0) {
        real = r;
        imaginary = i;
    }

    Complex Add(Complex c) { return Complex(this->real + c.real, this->imaginary + c.imaginary); }
    Complex Subtract(Complex c) { return Complex(this->real - c.real, this->imaginary - c.imaginary); }
    
    Complex Multiply(Complex c) {
        return Complex((this->real * c.real) - (this->imaginary * c.imaginary),
                       (this->real * c.imaginary) + (this->imaginary * c.real));
    }
};

class FFT {
public:
    Complex* Samples = nullptr;
    int Count = 512;

    void transform(Complex* s) {
        Samples = s;
        if (Count % 2 != 0) return;
        
        decimation_in_time(Samples, Count);
        int twiddle_step = Count / 2;

        for (int block_size = 2; block_size <= Count; block_size *= 2) {
            int half = block_size / 2;
            for (int start = 0; start < Count; start += block_size) {
                for (int jump = 0; jump < half; jump++) {
                    int t_idx = jump * twiddle_step;
                    Complex w(twiddle_real[t_idx], twiddle_imag[t_idx]);
                    
                    Complex second = Samples[start + jump + half].Multiply(w);
                    Complex first = Samples[start + jump];
                    Samples[start + jump + half] = first.Subtract(second);
                    Samples[start + jump] = first.Add(second);
                }
            }
            twiddle_step /= 2;
        }
    }

    void decimation_in_time(Complex* s, int N) {
        int carry = 0;
        int msb = N / 2;
        for (int i = 0; i < N - 1; i++) {
            if (i < carry) {
                Complex temp = s[i];
                s[i] = s[carry];
                s[carry] = temp;
            }
            int power = msb;
            while (carry >= power) {
                carry -= power;
                power /= 2;
            }
            carry += power;
        }
    }
};

class Processing {
public:
    Complex* input_samples;
    int Count = 512;
    int step = 256;
    bool isPadded = false;
    int original_size = 0;

    Processing() { input_samples = new Complex[Count]; }
    ~Processing() { delete[] input_samples; }

    void ZeroPadding(float* s, int orig_size) {
        original_size = orig_size;
        for (int i = 0; i < Count; i++) {
            if (i < original_size) input_samples[i] = Complex(s[i], 0.0f);
            else input_samples[i] = Complex(0.0f, 0.0f);
        }
        isPadded = (original_size < Count);
    }

    void ZCR(float* s, float& zcr) {
        float sum = 0.0f, crosses = 0.0f, max_val = 0.0f;
        for (int i = 0; i < original_size; i++) sum += s[i];
        float mean_val = sum / (original_size + 1e-6);
        for (int i = 0; i < original_size; i++) {
            float val = fabs(s[i] - mean_val);
            if (val > max_val) max_val = val;
        }
        float threshold = 0.05f * max_val;
        bool high_state = (s[0] - mean_val) > threshold;
        for (int i = 1; i < original_size; i++) {
            float current = s[i] - mean_val;
            if (high_state && current < -threshold) { high_state = false; crosses++; } 
            else if (!high_state && current > threshold) { high_state = true; crosses++; }
        }
        zcr = crosses / (float)original_size;
    }

    void Energy(float* s, float& energy) {
        energy = 0.0f;
        float sum = 0.0f;
        for (int i = 0; i < original_size; i++) sum += s[i];
        float mean_val = sum / (original_size + 1e-6);
        for (int i = 0; i < original_size; i++) {
            float val = s[i] - mean_val;
            energy += (val * val);
        }
    }
void SpectralFeatures(Complex* s, int length, float& centroid, float& variance) {
    float num = 0, den = 0;
    
    for (int i = 2; i < length / 2; i++) {
        float mag = (s[i].real * s[i].real) + (s[i].imaginary * s[i].imaginary);
        num += mag * i;
        den += mag;
    }

    if (den < 0.05f) { 
        centroid = 0.0f;
        variance = 0.0f;
        return;
    }

    centroid = num / den;

    num = 0;
    for (int i = 2; i < length / 2; i++) {
        float mag = (s[i].real * s[i].real) + (s[i].imaginary * s[i].imaginary);
        num += mag * fabs(i - centroid);
    }
    variance = num / den;
    }

    void ExtractFeatures(float* s, int s_length, FFT& fft, float& zm, float& zv, float& em, float& ev, float& cm, float& cv, float& vm, float& vv) {
        int max_frames = (s_length / step) + 1;
        float* f_zcr = new float[max_frames];
        float* f_en  = new float[max_frames];
        float* f_cent = new float[max_frames];
        float* f_var = new float[max_frames];
        float* buffer = new float[Count];
        int frame_count = 0;

        for (int i = 0; i < s_length; i += step) {
            int cur_size = 0;
            for (int j = 0; j < Count; j++) {
                if ((i + j) < s_length) { buffer[j] = s[i + j]; cur_size++; }
                else buffer[j] = 0;
            }
            ZeroPadding(buffer, cur_size);
            for (int k = 0; k < Count; k++) input_samples[k].real *= hamming_coeff[k];
            fft.transform(input_samples);

            ZCR(buffer, f_zcr[frame_count]);
            Energy(buffer, f_en[frame_count]);
            SpectralFeatures(input_samples, Count, f_cent[frame_count], f_var[frame_count]);
            frame_count++;
            if (isPadded) break;
        }

    auto calc_mv_active = [&](float* arr, int n, float& m, float& v) {
    float sum = 0;
    int active_count = 0;
    
    for (int i = 0; i < n; i++) {
        if (arr[i] > 0.0001f) { 
            sum += arr[i];
            active_count++;
        }
    }
    
    if (active_count == 0) {
        m = 0; v = 0;
        return;
    }
    
    m = sum / active_count;
    
    float vsum = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] > 0.0001f) {
            vsum += (arr[i] - m) * (arr[i] - m);
        }
    }
    v = vsum / active_count;
};

calc_mv_active(f_zcr, frame_count, zm, zv);
calc_mv_active(f_en, frame_count, em, ev);
calc_mv_active(f_cent, frame_count, cm, cv);
calc_mv_active(f_var, frame_count, vm, vv);

        delete[] f_zcr; delete[] f_en; delete[] f_cent; delete[] f_var; delete[] buffer;
    }
};

class NewSample {
public:
    float features[8];
    float features_scaled[8];
    float lda_features[2];

    void AddFeatures(float zm, float zv, float em, float ev, float cm, float cv, float vm, float vv) {
        features[0] = zm; features[1] = zv;
        features[2] = em; features[3] = ev;
        features[4] = cm; features[5] = cv;
        features[6] = vm; features[7] = vv;
    }

    void LDA() {
        for (int i = 0; i < 8; i++) {
            float divisor = (feature_std[i] < 1e-6) ? 1.0f : feature_std[i];
            features_scaled[i] = (features[i] - feature_mean[i]) / divisor;
        }

        for (int i = 0; i < 2; i++) {
            float sum = 0.0f;
            for (int j = 0; j < 8; j++) {
                sum += features_scaled[j] * lda_weights[j][i];
            }
            lda_features[i] = sum;
        }
    }

const char* Classify(float ev, float zm) {
    auto dist = [](float x1, float y1, const float* center) {
        return sqrt(pow(x1 - center[0], 2) + pow(y1 - center[1], 2));
    };

    float d_clap    = dist(lda_features[0], lda_features[1], center_clap);
    float d_speech  = dist(lda_features[0], lda_features[1], center_speech);
    float d_whistle = dist(lda_features[0], lda_features[1], center_whistle);

    if (d_whistle < d_clap && d_whistle < d_speech) {
        return "Zvizduk";
    }

    if (ev < 0.5f) { 
        return "Govor"; 
    }

    if (d_clap < d_speech) return "Pljesak";
    return "Govor";
}
};

Processing inputProc;
FFT fft;

void setupI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
    i2s_zero_dma_buffer(I2S_PORT);
}

Adafruit_SSD1306 display(sirina, visina, &Wire, -1);
void setup() {
    Serial.begin(115200);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 FAIL"));
    for(;;);
    }
    delay(3000);
    setupI2S();
    Serial.println("Spremno...");
    Serial.println("constants");
    for (int i = 0; i < 8; i++)
    Serial.printf("feature_mean[%d]=%.8f  feature_std[%d]=%.8f\n", 
                   i, feature_mean[i], i, feature_std[i]);
                   
                   
}
float z1=0;

void loop() {
    display.clearDisplay();
    display.setTextSize(1); 
    display.setTextColor(WHITE);
    display.setCursor(5, 5);
    display.println("Nema zvuka..."); 
    display.display(); 

    size_t bytesIn = 0;
    int32_t rawSample = 0;
    float threshold = 0.06f; 

    while (true) {
        i2s_read(I2S_PORT, &rawSample, sizeof(int32_t), &bytesIn, portMAX_DELAY);
        float val = (float)(rawSample >> 8) / 8388608.0f;
        if (fabs(val) > threshold) break;
    }

    float average = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        i2s_read(I2S_PORT, &rawSample, sizeof(int32_t), &bytesIn, portMAX_DELAY);
        audioBuffer[i] = (float)(rawSample >> 8) / 8388608.0f;
        average += audioBuffer[i];
    }

    average /= BUFFER_SIZE;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        audioBuffer[i] -= average;
    }

    float zm=0, zv=0, em=0, ev=0, cm=0, cv=0, vm=0, vv=0;
    inputProc.ExtractFeatures(audioBuffer, BUFFER_SIZE, fft, zm, zv, em, ev, cm, cv, vm, vv);

    NewSample test;
    test.AddFeatures(zm, zv, em, ev, cm, cv, vm, vv);
    test.LDA();

    Serial.println("\n--- Classification ---");
    Serial.print("RESULTAT: "); Serial.println(test.Classify(ev,zm));
    Serial.print("LDA X: "); Serial.println(test.lda_features[0], 2);
    Serial.print("LDA Y: "); Serial.println(test.lda_features[1], 2);
    Serial.print("energija "); Serial.println(ev,4);
    Serial.print("Centroid : "); Serial.println(cm, 2);

    display.clearDisplay(); 
    display.setTextSize(2); 
    display.setTextColor(WHITE);
    display.setCursor(5, 5);
    display.println("Rezultat:");
    display.setCursor(5, 25);
    display.println(test.Classify(ev,zm));
    display.display(); 
    
    delay(2000); 
}
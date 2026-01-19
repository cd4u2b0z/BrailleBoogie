// cavacore FFT processing
// Derived from cava's cavacore.c

#include "cavacore.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.1415926535897932385
#endif

struct cava_plan *cava_init(int number_of_bars, unsigned int rate, int channels, int autosens,
                            double noise_reduction, int low_cut_off, int high_cut_off) {
    struct cava_plan *p = malloc(sizeof(struct cava_plan));
    p->status = 0;

    // Sanity checks
    if (channels < 1 || channels > 2) {
        snprintf(p->error_message, 1024,
                 "cava_init called with illegal number of channels: %d", channels);
        p->status = -1;
        return p;
    }

    if (number_of_bars < 1) {
        snprintf(p->error_message, 1024,
                 "cava_init called with illegal number of bars: %d", number_of_bars);
        p->status = -1;
        return p;
    }

    if (number_of_bars % channels != 0) {
        snprintf(p->error_message, 1024,
                 "number of bars must be divisible by number of channels");
        p->status = -1;
        return p;
    }

    if ((int)rate < high_cut_off * 2) {
        snprintf(p->error_message, 1024,
                 "sample rate must be at least twice the high cutoff frequency");
        p->status = -1;
        return p;
    }

    p->number_of_bars = number_of_bars;
    p->audio_channels = channels;
    p->rate = rate;
    p->autosens = autosens;
    p->noise_reduction = noise_reduction;
    p->sens = 1.0;
    p->sens_init = 1;
    p->framerate = 0;
    p->frame_skip = 1;

    // Calculate FFT buffer sizes based on sample rate
    // Bass needs longer FFT for better frequency resolution at low frequencies
    p->FFTbassbufferSize = rate / 20;  // ~50ms window for bass
    p->FFTbufferSize = rate / 80;      // ~12.5ms window for mids/treble

    // Make FFT sizes power of 2 for efficiency
    int power = 1;
    while (power < p->FFTbassbufferSize)
        power *= 2;
    p->FFTbassbufferSize = power;

    power = 1;
    while (power < p->FFTbufferSize)
        power *= 2;
    p->FFTbufferSize = power;

    // Input buffer holds enough samples for bass FFT
    p->input_buffer_size = p->FFTbassbufferSize * channels;
    p->input_buffer = (double *)calloc(p->input_buffer_size, sizeof(double));

    // Calculate frequency bands
    int bars_per_channel = number_of_bars / channels;
    p->cut_off_frequency = (double *)calloc(bars_per_channel + 1, sizeof(double));
    p->FFTbuffer_lower_cut_off = (int *)calloc(bars_per_channel, sizeof(int));
    p->FFTbuffer_upper_cut_off = (int *)calloc(bars_per_channel, sizeof(int));
    p->eq = (double *)calloc(bars_per_channel, sizeof(double));
    p->cava_fall = (double *)calloc(number_of_bars, sizeof(double));
    p->cava_mem = (double *)calloc(number_of_bars, sizeof(double));
    p->cava_peak = (double *)calloc(number_of_bars, sizeof(double));
    p->prev_cava_out = (double *)calloc(number_of_bars, sizeof(double));

    // Calculate cutoff frequencies using logarithmic scale
    double freq_const = log10((double)low_cut_off / (double)high_cut_off) /
                        ((1.0 / (bars_per_channel + 1)) - 1);

    for (int n = 0; n <= bars_per_channel; n++) {
        double relative_cut_off = high_cut_off * pow(10, freq_const * (-1) +
                                  ((((double)n + 1) / (bars_per_channel + 1)) * freq_const));
        p->cut_off_frequency[n] = relative_cut_off;
    }

    // Find where bass ends (around 300Hz)
    p->bass_cut_off_bar = 0;
    for (int n = 0; n < bars_per_channel; n++) {
        if (p->cut_off_frequency[n] < 300) {
            p->bass_cut_off_bar = n;
            break;
        }
    }

    // Map frequency bands to FFT bins
    for (int n = 0; n < bars_per_channel; n++) {
        int fft_size = (n < p->bass_cut_off_bar) ? p->FFTbassbufferSize : p->FFTbufferSize;

        p->FFTbuffer_lower_cut_off[n] = (int)(p->cut_off_frequency[n] * fft_size / rate);
        p->FFTbuffer_upper_cut_off[n] = (int)(p->cut_off_frequency[n + 1] * fft_size / rate);

        if (p->FFTbuffer_lower_cut_off[n] < 1)
            p->FFTbuffer_lower_cut_off[n] = 1;
        if (p->FFTbuffer_upper_cut_off[n] < 1)
            p->FFTbuffer_upper_cut_off[n] = 1;

        // Ensure at least one bin per bar
        if (p->FFTbuffer_lower_cut_off[n] > p->FFTbuffer_upper_cut_off[n])
            p->FFTbuffer_upper_cut_off[n] = p->FFTbuffer_lower_cut_off[n];
    }

    // Calculate EQ curve
    for (int n = 0; n < bars_per_channel; n++) {
        p->eq[n] = pow(p->cut_off_frequency[n + 1], 0.85);
        if (n < p->bass_cut_off_bar) {
            p->eq[n] /= log2(p->FFTbassbufferSize);
        } else {
            p->eq[n] /= log2(p->FFTbufferSize);
        }
        p->eq[n] /= p->FFTbuffer_upper_cut_off[n] - p->FFTbuffer_lower_cut_off[n] + 1;
    }

    // Create window functions
    p->bass_multiplier = (double *)calloc(p->FFTbassbufferSize, sizeof(double));
    p->multiplier = (double *)calloc(p->FFTbufferSize, sizeof(double));

    // Hann window
    for (int i = 0; i < p->FFTbassbufferSize; i++) {
        p->bass_multiplier[i] = 0.5 * (1 - cos(2 * M_PI * i / (p->FFTbassbufferSize - 1)));
    }
    for (int i = 0; i < p->FFTbufferSize; i++) {
        p->multiplier[i] = 0.5 * (1 - cos(2 * M_PI * i / (p->FFTbufferSize - 1)));
    }

    // Allocate FFTW buffers and create plans
    int fftw_flag = FFTW_MEASURE;

    // Left channel (or mono)
    p->in_bass_l = fftw_alloc_real(p->FFTbassbufferSize);
    p->in_bass_l_raw = fftw_alloc_real(p->FFTbassbufferSize);
    p->out_bass_l = fftw_alloc_complex(p->FFTbassbufferSize / 2 + 1);
    p->p_bass_l = fftw_plan_dft_r2c_1d(p->FFTbassbufferSize, p->in_bass_l, p->out_bass_l, fftw_flag);

    p->in_l = fftw_alloc_real(p->FFTbufferSize);
    p->in_l_raw = fftw_alloc_real(p->FFTbufferSize);
    p->out_l = fftw_alloc_complex(p->FFTbufferSize / 2 + 1);
    p->p_l = fftw_plan_dft_r2c_1d(p->FFTbufferSize, p->in_l, p->out_l, fftw_flag);

    memset(p->in_bass_l, 0, sizeof(double) * p->FFTbassbufferSize);
    memset(p->in_l, 0, sizeof(double) * p->FFTbufferSize);

    // Right channel (stereo only)
    if (channels == 2) {
        p->in_bass_r = fftw_alloc_real(p->FFTbassbufferSize);
        p->in_bass_r_raw = fftw_alloc_real(p->FFTbassbufferSize);
        p->out_bass_r = fftw_alloc_complex(p->FFTbassbufferSize / 2 + 1);
        p->p_bass_r = fftw_plan_dft_r2c_1d(p->FFTbassbufferSize, p->in_bass_r, p->out_bass_r, fftw_flag);

        p->in_r = fftw_alloc_real(p->FFTbufferSize);
        p->in_r_raw = fftw_alloc_real(p->FFTbufferSize);
        p->out_r = fftw_alloc_complex(p->FFTbufferSize / 2 + 1);
        p->p_r = fftw_plan_dft_r2c_1d(p->FFTbufferSize, p->in_r, p->out_r, fftw_flag);

        memset(p->in_bass_r, 0, sizeof(double) * p->FFTbassbufferSize);
        memset(p->in_r, 0, sizeof(double) * p->FFTbufferSize);
    }

    return p;
}

void cava_execute(double *cava_in, int new_samples, double *cava_out, struct cava_plan *p) {
    // Handle overflow
    if (new_samples > p->input_buffer_size) {
        new_samples = p->input_buffer_size;
    }

    int silence = 1;

    if (new_samples > 0) {
        p->framerate -= p->framerate / 64;
        p->framerate += (double)((p->rate * p->audio_channels * p->frame_skip) / new_samples) / 64;
        p->frame_skip = 1;

        // Shift input buffer
        for (int n = p->input_buffer_size - 1; n >= new_samples; n--) {
            p->input_buffer[n] = p->input_buffer[n - new_samples];
        }

        // Fill input buffer with new samples
        for (int n = 0; n < new_samples; n++) {
            p->input_buffer[new_samples - n - 1] = cava_in[n];
            if (cava_in[n])
                silence = 0;
        }
    } else {
        p->frame_skip++;
    }

    // Fill FFT buffers from input buffer
    for (int n = 0; n < p->FFTbassbufferSize; n++) {
        if (p->audio_channels == 2) {
            p->in_bass_r_raw[n] = p->input_buffer[n * 2];
            p->in_bass_l_raw[n] = p->input_buffer[n * 2 + 1];
        } else {
            p->in_bass_l_raw[n] = p->input_buffer[n];
        }
    }

    for (int n = 0; n < p->FFTbufferSize; n++) {
        if (p->audio_channels == 2) {
            p->in_r_raw[n] = p->input_buffer[n * 2];
            p->in_l_raw[n] = p->input_buffer[n * 2 + 1];
        } else {
            p->in_l_raw[n] = p->input_buffer[n];
        }
    }

    // Apply window function
    for (int i = 0; i < p->FFTbassbufferSize; i++) {
        p->in_bass_l[i] = p->bass_multiplier[i] * p->in_bass_l_raw[i];
        if (p->audio_channels == 2)
            p->in_bass_r[i] = p->bass_multiplier[i] * p->in_bass_r_raw[i];
    }
    for (int i = 0; i < p->FFTbufferSize; i++) {
        p->in_l[i] = p->multiplier[i] * p->in_l_raw[i];
        if (p->audio_channels == 2)
            p->in_r[i] = p->multiplier[i] * p->in_r_raw[i];
    }

    // Execute FFT
    fftw_execute(p->p_bass_l);
    fftw_execute(p->p_l);
    if (p->audio_channels == 2) {
        fftw_execute(p->p_bass_r);
        fftw_execute(p->p_r);
    }

    // Process frequency bands
    int bars_per_channel = p->number_of_bars / p->audio_channels;

    for (int n = 0; n < bars_per_channel; n++) {
        double temp_l = 0;
        double temp_r = 0;

        // Sum FFT bins within each band
        for (int i = p->FFTbuffer_lower_cut_off[n]; i <= p->FFTbuffer_upper_cut_off[n]; i++) {
            if (n < p->bass_cut_off_bar) {
                // Bass uses bass FFT
                temp_l += hypot(p->out_bass_l[i][0], p->out_bass_l[i][1]);
                if (p->audio_channels == 2)
                    temp_r += hypot(p->out_bass_r[i][0], p->out_bass_r[i][1]);
            } else {
                // Mids and treble use regular FFT
                temp_l += hypot(p->out_l[i][0], p->out_l[i][1]);
                if (p->audio_channels == 2)
                    temp_r += hypot(p->out_r[i][0], p->out_r[i][1]);
            }
        }

        // Apply EQ and scaling
        temp_l *= p->eq[n];
        cava_out[n] = temp_l;

        if (p->audio_channels == 2) {
            temp_r *= p->eq[n];
            cava_out[n + bars_per_channel] = temp_r;
        }
    }

    // Apply smoothing (noise reduction)
    for (int n = 0; n < p->number_of_bars; n++) {
        // Smoothing with fall-off
        if (cava_out[n] < p->prev_cava_out[n] * p->noise_reduction) {
            cava_out[n] = p->prev_cava_out[n] * p->noise_reduction;
        }
        p->prev_cava_out[n] = cava_out[n];

        // Normalize to 0-1 range (adjusted scaling for better sensitivity)
        cava_out[n] = cava_out[n] / 100000.0 * p->sens;
        if (cava_out[n] > 1.0)
            cava_out[n] = 1.0;
        if (cava_out[n] < 0.0)
            cava_out[n] = 0.0;
    }

    // Auto sensitivity adjustment
    if (p->autosens) {
        double max_val = 0;
        for (int n = 0; n < p->number_of_bars; n++) {
            if (cava_out[n] > max_val)
                max_val = cava_out[n];
        }

        if (max_val > 1.0) {
            p->sens = p->sens * 0.985;
            p->sens_init = 0;
        } else {
            if (!silence) {
                p->sens = p->sens * 1.001;
                if (p->sens_init)
                    p->sens = p->sens * 1.1;
            }
        }
    }
}

void cava_destroy(struct cava_plan *p) {
    free(p->input_buffer);
    free(p->bass_multiplier);
    free(p->multiplier);
    free(p->eq);
    free(p->cut_off_frequency);
    free(p->FFTbuffer_lower_cut_off);
    free(p->FFTbuffer_upper_cut_off);
    free(p->cava_fall);
    free(p->cava_mem);
    free(p->cava_peak);
    free(p->prev_cava_out);

    fftw_free(p->in_bass_l);
    fftw_free(p->in_bass_l_raw);
    fftw_free(p->out_bass_l);
    fftw_destroy_plan(p->p_bass_l);

    fftw_free(p->in_l);
    fftw_free(p->in_l_raw);
    fftw_free(p->out_l);
    fftw_destroy_plan(p->p_l);

    if (p->audio_channels == 2) {
        fftw_free(p->in_bass_r);
        fftw_free(p->in_bass_r_raw);
        fftw_free(p->out_bass_r);
        fftw_destroy_plan(p->p_bass_r);

        fftw_free(p->in_r);
        fftw_free(p->in_r_raw);
        fftw_free(p->out_r);
        fftw_destroy_plan(p->p_r);
    }

    free(p);
}

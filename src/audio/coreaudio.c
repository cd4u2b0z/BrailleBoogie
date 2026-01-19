// CoreAudio audio input for macOS
// Implements the same interface as pulse.c and pipewire.c

#include "audio.h"

#ifdef __APPLE__

#include <AudioToolbox/AudioQueue.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUM_BUFFERS 3
#define FRAMES_PER_BUFFER 512

typedef struct {
    AudioQueueRef queue;
    AudioQueueBufferRef buffers[NUM_BUFFERS];
    struct audio_data *audio_data;
    int is_running;
} CoreAudioContext;

// AudioQueue callback - called when buffer is filled
static void audioQueueCallback(void *userData, AudioQueueRef queue,
                              AudioQueueBufferRef buffer) {
    CoreAudioContext *ctx = (CoreAudioContext *)userData;
    struct audio_data *audio = ctx->audio_data;

    if (audio->terminate || !ctx->is_running) {
        return;
    }

    // Process the audio buffer
    unsigned char *data = (unsigned char *)buffer->mAudioData;
    uint16_t size = buffer->mAudioDataByteSize / (audio->format / 8);
    
    write_to_cava_input_buffers(size, data, audio);

    // Re-enqueue the buffer
    if (ctx->is_running) {
        AudioQueueEnqueueBuffer(queue, buffer, 0, NULL);
    }
}

// Get default input device
static AudioDeviceID getDefaultInputDevice(void) {
    AudioDeviceID deviceID = kAudioObjectUnknown;
    UInt32 size = sizeof(deviceID);

    AudioObjectPropertyAddress propertyAddress = {
        .mSelector = kAudioHardwarePropertyDefaultInputDevice,
        .mScope = kAudioObjectPropertyScopeGlobal,
        .mElement = kAudioObjectPropertyElementMain
    };

    OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                                 &propertyAddress,
                                                 0, NULL,
                                                 &size, &deviceID);

    if (status != noErr) {
        fprintf(stderr, "Failed to get default input device\n");
        return kAudioObjectUnknown;
    }

    return deviceID;
}

// Get device name for error messages
static void getDeviceName(AudioDeviceID deviceID, char *name, size_t maxLen) {
    CFStringRef deviceName = NULL;
    UInt32 size = sizeof(deviceName);

    AudioObjectPropertyAddress propertyAddress = {
        .mSelector = kAudioDevicePropertyDeviceNameCFString,
        .mScope = kAudioObjectPropertyScopeGlobal,
        .mElement = kAudioObjectPropertyElementMain
    };

    OSStatus status = AudioObjectGetPropertyData(deviceID, &propertyAddress,
                                                 0, NULL, &size, &deviceName);

    if (status == noErr && deviceName != NULL) {
        CFStringGetCString(deviceName, name, maxLen, kCFStringEncodingUTF8);
        CFRelease(deviceName);
    } else {
        snprintf(name, maxLen, "Unknown Device");
    }
}

void *input_coreaudio(void *data) {
    struct audio_data *audio = (struct audio_data *)data;
    CoreAudioContext ctx = {0};
    ctx.audio_data = audio;
    ctx.is_running = 0;

    // Configure audio format (16-bit stereo at 44.1kHz)
    AudioStreamBasicDescription format = {0};
    format.mSampleRate = 44100.0;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    format.mBitsPerChannel = 16;
    format.mChannelsPerFrame = 2;
    format.mBytesPerFrame = format.mChannelsPerFrame * (format.mBitsPerChannel / 8);
    format.mFramesPerPacket = 1;
    format.mBytesPerPacket = format.mBytesPerFrame * format.mFramesPerPacket;

    // Create audio queue
    OSStatus status = AudioQueueNewInput(&format,
                                        audioQueueCallback,
                                        &ctx,
                                        NULL,
                                        kCFRunLoopCommonModes,
                                        0,
                                        &ctx.queue);

    if (status != noErr) {
        sprintf(audio->error_message,
                "Failed to create CoreAudio input queue (error %d)\n", (int)status);
        audio->terminate = 1;
        pthread_exit(NULL);
        return 0;
    }

    // Get and display the input device being used
    AudioDeviceID deviceID = getDefaultInputDevice();
    if (deviceID != kAudioObjectUnknown) {
        char deviceName[256];
        getDeviceName(deviceID, deviceName, sizeof(deviceName));
        fprintf(stderr, "CoreAudio: Using input device: %s\n", deviceName);
    }

    // Allocate and enqueue buffers
    uint32_t bufferByteSize = FRAMES_PER_BUFFER * format.mBytesPerFrame;
    for (int i = 0; i < NUM_BUFFERS; i++) {
        status = AudioQueueAllocateBuffer(ctx.queue, bufferByteSize, &ctx.buffers[i]);
        if (status != noErr) {
            sprintf(audio->error_message,
                    "Failed to allocate CoreAudio buffer %d (error %d)\n", i, (int)status);
            audio->terminate = 1;
            AudioQueueDispose(ctx.queue, true);
            pthread_exit(NULL);
            return 0;
        }
        AudioQueueEnqueueBuffer(ctx.queue, ctx.buffers[i], 0, NULL);
    }

    // Start the audio queue
    ctx.is_running = 1;
    status = AudioQueueStart(ctx.queue, NULL);
    if (status != noErr) {
        sprintf(audio->error_message,
                "Failed to start CoreAudio queue (error %d)\n", (int)status);
        audio->terminate = 1;
        ctx.is_running = 0;
        AudioQueueDispose(ctx.queue, true);
        pthread_exit(NULL);
        return 0;
    }

    fprintf(stderr, "CoreAudio: Audio capture started\n");

    // Wait until termination is requested
    while (!audio->terminate) {
        usleep(100000); // 100ms
    }

    // Cleanup
    ctx.is_running = 0;
    AudioQueueStop(ctx.queue, true);
    AudioQueueDispose(ctx.queue, true);

    fprintf(stderr, "CoreAudio: Audio capture stopped\n");

    pthread_exit(NULL);
    return 0;
}

#endif // __APPLE__

// PulseAudio audio input
// Derived from cava's input/pulse.c

#include "audio.h"

#include <pulse/error.h>
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

static pa_mainloop *m_pulseaudio_mainloop;

static void cb(pa_context *pulseaudio_context, const pa_server_info *i, void *userdata) {
    struct audio_data *audio = (struct audio_data *)userdata;

    pthread_mutex_lock(&audio->lock);
    free(audio->source);
    audio->source = malloc(sizeof(char) * 1024);

    strcpy(audio->source, i->default_sink_name);
    audio->source = strcat(audio->source, ".monitor");

    pthread_mutex_unlock(&audio->lock);

    pa_context_disconnect(pulseaudio_context);
    pa_context_unref(pulseaudio_context);
    pa_mainloop_quit(m_pulseaudio_mainloop, 0);
}

static void pulseaudio_context_state_callback(pa_context *pulseaudio_context, void *userdata) {
    switch (pa_context_get_state(pulseaudio_context)) {
    case PA_CONTEXT_UNCONNECTED:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        break;
    case PA_CONTEXT_READY:
        pa_operation_unref(pa_context_get_server_info(pulseaudio_context, cb, userdata));
        break;
    case PA_CONTEXT_FAILED:
        fprintf(stderr, "Failed to connect to PulseAudio server\n");
        exit(EXIT_FAILURE);
        break;
    case PA_CONTEXT_TERMINATED:
        pa_mainloop_quit(m_pulseaudio_mainloop, 0);
        break;
    }
}

void getPulseDefaultSink(void *data) {
    struct audio_data *audio = (struct audio_data *)data;
    pa_mainloop_api *mainloop_api;
    pa_context *pulseaudio_context;
    int ret;

    m_pulseaudio_mainloop = pa_mainloop_new();
    mainloop_api = pa_mainloop_get_api(m_pulseaudio_mainloop);
    pulseaudio_context = pa_context_new(mainloop_api, "asciidancer device list");

    pa_context_connect(pulseaudio_context, NULL, PA_CONTEXT_NOFLAGS, NULL);
    pa_context_set_state_callback(pulseaudio_context, pulseaudio_context_state_callback,
                                  (void *)audio);

    if (!(ret = pa_mainloop_iterate(m_pulseaudio_mainloop, 0, &ret))) {
        fprintf(stderr,
                "Could not open PulseAudio mainloop to find default device name: %d\n"
                "Check if PulseAudio is running\n",
                ret);
        exit(EXIT_FAILURE);
    }

    pa_mainloop_run(m_pulseaudio_mainloop, &ret);
    pa_mainloop_free(m_pulseaudio_mainloop);
}

void *input_pulse(void *data) {
    struct audio_data *audio = (struct audio_data *)data;
    uint16_t buffer_size = audio->input_buffer_size * audio->format / 8;
    unsigned char buf[buffer_size];

    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
    };

    pa_buffer_attr pb = {
        .maxlength = (uint32_t)-1,
        .fragsize = buffer_size
    };

    pa_simple *s = NULL;
    int error;

    if (!(s = pa_simple_new(NULL, "asciidancer", PA_STREAM_RECORD, audio->source,
                            "audio for asciidancer", &ss, NULL, &pb, &error))) {
        sprintf(audio->error_message,
                "Could not open PulseAudio source: %s, %s.\n"
                "To find a list of sources run 'pacmd list-sources'\n",
                audio->source, pa_strerror(error));

        audio->terminate = 1;
        pthread_exit(NULL);
        return 0;
    }

    while (!audio->terminate) {
        if (pa_simple_read(s, buf, sizeof(buf), &error) < 0) {
            sprintf(audio->error_message, "pa_simple_read() failed: %s\n", pa_strerror(error));
            audio->terminate = 1;
        }

        write_to_cava_input_buffers(audio->input_buffer_size, buf, audio);
    }

    pa_simple_free(s);
    pthread_exit(NULL);
    return 0;
}

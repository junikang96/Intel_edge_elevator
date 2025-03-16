#include "../incs/define.h"

void *audio_capture_thread(void *arg) {
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;
    int err;
    void (*callback)(const char *) = (void (*)(const char *))arg;

    if ((err = snd_pcm_open(&capture_handle, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf(stderr, "Cannot open audio device: %s\n", snd_strerror(err));
        pthread_exit(NULL);
    }

    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(capture_handle, hw_params);
    snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(capture_handle, hw_params, FORMAT);
    snd_pcm_hw_params_set_channels(capture_handle, hw_params, CHANNELS);
    snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &(unsigned int){SAMPLE_RATE}, 0);
    snd_pcm_hw_params(capture_handle, hw_params);

    printf("Voice recognition has begun.\n");

    void *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        snd_pcm_close(capture_handle);
        pthread_exit(NULL);
    }

    while (recording_flag) {
        int frames_read = snd_pcm_readi(capture_handle, buffer, FRAMES_PER_CHUNK);
        
        if (frames_read < 0) {
            fprintf(stderr, "Error Reading Audio: %s\n", snd_strerror(frames_read));
            snd_pcm_recover(capture_handle, frames_read, 0);
            continue;
        }

        if (frames_read > 0) {
            // 음량 검사 - 무음이 아닐 경우에만 STT 요청
            short *samples = (short *)buffer;
            int total_samples = frames_read * CHANNELS;
            int silent = 1;
            
            for (int i = 0; i < total_samples; i++) {
                if (abs(samples[i]) > 500) {  // 임계값 조정 가능
                    silent = 0;
                    break;
                }
            }
            
            if (!silent) {
                pthread_t stt_thread;
                struct STTThreadArg *arg = (struct STTThreadArg *)malloc(sizeof(struct STTThreadArg));
                
                if (arg) {
                    size_t data_size = frames_read * CHANNELS * BYTES_PER_SAMPLE;
                    arg->audio_data = malloc(data_size);
                    
                    if (arg->audio_data) {
                        memcpy(arg->audio_data, buffer, data_size);
                        arg->audio_size = data_size;
                        arg->callback = callback;
                        
                        if (pthread_create(&stt_thread, NULL, STT_thread, arg) != 0) {
                            fprintf(stderr, "Failed to create STT Thread.\n");
                            free(arg->audio_data);
                            free(arg);
                        } else {
                            pthread_detach(stt_thread);
                        }
                    } else {
                        free(arg);
                    }
                }
            } else {
                printf("Skip STT processing due to silence.\n");
            }
        }
    }

    free(buffer);
    snd_pcm_close(capture_handle);
    pthread_exit(NULL);
}

void handle_signal(int sig) {
    recording_flag = 0;
    printf("\nStop recording...\n");
}

pthread_t start_realtime_stt(void (*callback)(const char *)) {
    pthread_t thread_id;
    
    // Ctrl+C 시그널 핸들러 등록
    signal(SIGINT, handle_signal);
    
    if (pthread_create(&thread_id, NULL, audio_capture_thread, (void *)callback) != 0) {
        fprintf(stderr, "Creating thread failed.\n");
        return 0;
    }
    
    return thread_id;
}


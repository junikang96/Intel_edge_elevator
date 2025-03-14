#include "../incs/STT_client.h"

// 오디오 캡처 및 STT 처리를 위한 스레드 함수
void *audio_capture_thread(void *arg) {
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;
    int err;
    void (*callback)(const char *) = (void (*)(const char *))arg;

    // ALSA 초기화
    if ((err = snd_pcm_open(&capture_handle, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf(stderr, "오디오 장치를 열 수 없습니다: %s\n", snd_strerror(err));
        pthread_exit(NULL);
    }

    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(capture_handle, hw_params);
    snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(capture_handle, hw_params, FORMAT);
    snd_pcm_hw_params_set_channels(capture_handle, hw_params, CHANNELS);
    snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &(unsigned int){SAMPLE_RATE}, 0);
    snd_pcm_hw_params(capture_handle, hw_params);

    printf("실시간 음성 인식이 시작되었습니다. (Ctrl+C로 종료)\n");

    // 오디오 버퍼 할당
    void *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        fprintf(stderr, "메모리 할당 실패\n");
        snd_pcm_close(capture_handle);
        pthread_exit(NULL);
    }

    while (recording_flag) {
        // 3초 분량의 오디오 캡처
        int frames_read = snd_pcm_readi(capture_handle, buffer, FRAMES_PER_CHUNK);
        
        if (frames_read < 0) {
            fprintf(stderr, "오디오 읽기 오류: %s\n", snd_strerror(frames_read));
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
                // STT 처리를 위한 스레드 시작
                pthread_t stt_thread;
                struct STTThreadArg *arg = (struct STTThreadArg *)malloc(sizeof(struct STTThreadArg));
                
                if (arg) {
                    // 오디오 데이터 복사
                    size_t data_size = frames_read * CHANNELS * BYTES_PER_SAMPLE;
                    arg->audio_data = malloc(data_size);
                    
                    if (arg->audio_data) {
                        memcpy(arg->audio_data, buffer, data_size);
                        arg->audio_size = data_size;
                        arg->callback = callback;
                        
                        if (pthread_create(&stt_thread, NULL, thread_STT, arg) != 0) {
                            fprintf(stderr, "STT 스레드 생성 실패\n");
                            free(arg->audio_data);
                            free(arg);
                        } else {
                            // 스레드를 분리하여 자동으로 리소스가 정리되도록 함
                            pthread_detach(stt_thread);
                        }
                    } else {
                        free(arg);
                    }
                }
            } else {
                printf("무음 감지됨, STT 처리 생략\n");
            }
        }
    }

    free(buffer);
    snd_pcm_close(capture_handle);
    pthread_exit(NULL);
}

// 시그널 핸들러 - Ctrl+C로 녹음 중지
void handle_signal(int sig) {
    recording_flag = 0;
    printf("\n녹음을 중지합니다...\n");
}

// 실시간 음성 인식 시작
pthread_t start_realtime_stt(void (*callback)(const char *)) {
    pthread_t thread_id;
    
    // Ctrl+C 시그널 핸들러 등록
    signal(SIGINT, handle_signal);
    
    // 오디오 캡처 스레드 시작
    if (pthread_create(&thread_id, NULL, audio_capture_thread, (void *)callback) != 0) {
        fprintf(stderr, "스레드 생성 실패\n");
        return 0;
    }
    
    return thread_id;
}


#include <opentok.h>

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <signal.h>

#include "config.h"
#include "otk_thread.h"
#include <stdio.h>
#include <string.h>
#include <alsa/asoundlib.h>

#include <time.h>
#define PCM_DEVICE "default"


static std::atomic<bool> g_is_connected(false);

//some Alsa Vars
bool pcm_settings_set = false;
unsigned int pcm, tmp, dir, rate;
int  channels, seconds;
snd_pcm_t *pcm_handle;
snd_pcm_hw_params_t *params;
snd_pcm_uframes_t frames;


static void on_subscriber_connected(otc_subscriber *subscriber,
                                    void *user_data,
                                    const otc_stream *stream) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
  //otc_subscriber_set_subscribe_to_video(subscriber,0);
}

static void on_subscriber_error(otc_subscriber* subscriber,
                                void *user_data,
                                const char* error_string,
                                enum otc_subscriber_error_code error) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
  std::cout << "Subscriber error. Error code: " << error_string << std::endl;
}

//this is where we get the audio data stuff
static void on_subscriber_audio_data(otc_subscriber* subscriber,
                        void* user_data,
                        const struct otc_audio_data* audio_data){


  std::cout << __FUNCTION__ << " Audio IN" << std::endl;

	//Initialize PCM if it's not yet initialized.
  //Technically you can do it on main, but I want to dynamically set the values
  //for pcm through whatever audio_data is providing
  if (!pcm_settings_set){

    rate 	 = audio_data->sample_rate;
	  channels = audio_data->number_of_channels;

    /* Open the PCM device in playback mode */
	  if (pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE,
					SND_PCM_STREAM_PLAYBACK, 0) < 0) 
		  printf("ERROR: Can't open \"%s\" PCM device. %s\n",
					PCM_DEVICE, snd_strerror(pcm));

    /* Allocate parameters object and fill it with default values*/
    snd_pcm_hw_params_alloca(&params);

    snd_pcm_hw_params_any(pcm_handle, params);


    /* Set parameters */
    if (pcm = snd_pcm_hw_params_set_access(pcm_handle, params,
            SND_PCM_ACCESS_RW_INTERLEAVED) < 0) 
      printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(pcm));

    if (pcm = snd_pcm_hw_params_set_format(pcm_handle, params,
              SND_PCM_FORMAT_S16_LE) < 0) 
      printf("ERROR: Can't set format. %s\n", snd_strerror(pcm));

    if (pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, channels) < 0) 
      printf("ERROR: Can't set channels number. %s\n", snd_strerror(pcm));

    if (pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0) < 0) 
      printf("ERROR: Can't set rate. %s\n", snd_strerror(pcm));

    /* Write parameters */
    if (pcm = snd_pcm_hw_params(pcm_handle, params) < 0)
      printf("ERROR: Can't set harware parameters. %s\n", snd_strerror(pcm));

    //mark pcm settings as initialized  
    pcm_settings_set = true;
  }
  
  //if it;s initialized, then we can write frames
  else{
  frames =  audio_data->number_of_samples;

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params, &frames,0);

  if (pcm = snd_pcm_writei(pcm_handle, audio_data->sample_buffer, frames) == -EPIPE) {
    printf("XRUN.\n");
    snd_pcm_prepare(pcm_handle);
  } else if (pcm < 0) {
    printf("ERROR. Can't write to PCM device. %s\n", snd_strerror(pcm));
  }
  }
  

}

static void on_session_connected(otc_session *session, void *user_data) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;

  g_is_connected = true;

}

static void on_session_connection_created(otc_session *session,
                                          void *user_data,
                                          const otc_connection *connection) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
}

static void on_session_connection_dropped(otc_session *session,
                                          void *user_data,
                                          const otc_connection *connection) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
}

static void on_session_stream_received(otc_session *session,
                                       void *user_data,
                                       const otc_stream *stream) {


  std::cout << __FUNCTION__ << " callback function" << std::endl;
  struct otc_subscriber_callbacks subscriber_callbacks = {0};
  subscriber_callbacks.user_data = user_data;
  subscriber_callbacks.on_connected = on_subscriber_connected;
  subscriber_callbacks.on_error = on_subscriber_error;
  subscriber_callbacks.on_audio_data = on_subscriber_audio_data;

  otc_subscriber *subscriber = otc_subscriber_new(stream,&subscriber_callbacks);
  //otc_subscriber_set_subscribe_to_video(subscriber,1);
 
  if (otc_session_subscribe(session, subscriber) == OTC_SUCCESS) {
    printf("subscribed successfully\n");
    return;
  }
  else{
    printf("Error during subscribe\n");
  }
}

static void on_session_stream_dropped(otc_session *session,
                                      void *user_data,
                                      const otc_stream *stream) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
}

static void on_session_disconnected(otc_session *session, void *user_data) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
}

static void on_session_error(otc_session *session,
                             void *user_data,
                             const char *error_string,
                             enum otc_session_error_code error) {
  std::cout << __FUNCTION__ << " callback function" << std::endl;
  std::cout << "Session error. Error : " << error_string << std::endl;
}

static void on_otc_log_message(const char* message) {
  std::cout <<  __FUNCTION__ << ":" << message << std::endl;
}

void sigfun(int sig)
{
        printf("You have presses Ctrl-C , please press again to exit\n");
	(void) signal(SIGINT, SIG_DFL);
}

int main(int argc, char** argv) {
 
  if (otc_init(nullptr) != OTC_SUCCESS) {
    std::cout << "Could not init OpenTok library" << std::endl;
    return EXIT_FAILURE;
  }
 (void) signal(SIGINT, sigfun);
#ifdef CONSOLE_LOGGING
  otc_log_set_logger_callback(on_otc_log_message);
  otc_log_enable(OTC_LOG_LEVEL_ALL);
#endif


  struct otc_session_callbacks session_callbacks = {0};
  session_callbacks.on_connected = on_session_connected;
  session_callbacks.on_connection_created = on_session_connection_created;
  session_callbacks.on_connection_dropped = on_session_connection_dropped;
  session_callbacks.on_stream_received = on_session_stream_received;
  session_callbacks.on_stream_dropped = on_session_stream_dropped;
  session_callbacks.on_disconnected = on_session_disconnected;
  session_callbacks.on_error = on_session_error;
  

  otc_session *session = nullptr;
  session = otc_session_new(API_KEY, SESSION_ID, &session_callbacks);

  if (session == nullptr) {
    std::cout << "Could not create OpenTok session successfully" << std::endl;
    return EXIT_FAILURE;
  }

  otc_session_connect(session, TOKEN);



  while(1){
	  sleep(1);
  }

  if ((session != nullptr) && g_is_connected.load()) {
    otc_session_disconnect(session);
  }

  if (session != nullptr) {
    otc_session_delete(session);
  }

  otc_destroy();

  return EXIT_SUCCESS;
}

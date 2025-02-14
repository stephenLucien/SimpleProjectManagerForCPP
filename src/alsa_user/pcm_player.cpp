#include "pcm_player.h"

//
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <cstdint>
#include <memory>
#include <string>
#include "utils/os_tools_log.h"


namespace
{
class AlsaPcmPlayer
{
   private:
    //
    snd_pcm_t *pcm_handle = NULL;
    //
    std::string device = "default";
    //
    unsigned int samplerate = 16000;
    //
    int chn = 1;
    //
    int _deinit()
    {
        if (pcm_handle)
        {
            snd_pcm_drain(pcm_handle);
            snd_pcm_close(pcm_handle);
            pcm_handle = NULL;
        }
        return 0;
    }

    int _init()
    {
        int ret = -1;
        //
        if (pcm_handle)
        {
            return 0;
        }
        //
        ret = snd_pcm_open(&pcm_handle, device.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
        if (ret < 0)
        {
            OS_LOGE("%s", snd_strerror(ret));
            return ret;
        }
        //
        snd_pcm_hw_params_t *hw_params = NULL;
        //
        snd_pcm_hw_params_alloca(&hw_params);
        snd_pcm_hw_params_any(pcm_handle, hw_params);

        //
        ret = snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
        if (ret < 0)
        {
            OS_LOGE("%s", snd_strerror(ret));
            return ret;
        }
        //
        ret = snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE);
        if (ret < 0)
        {
            OS_LOGE("%s", snd_strerror(ret));
            return ret;
        }
        //
        ret = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, chn);
        if (ret < 0)
        {
            OS_LOGE("%s", snd_strerror(ret));
            return ret;
        }
        //
        ret = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &samplerate, NULL);
        if (ret < 0)
        {
            OS_LOGE("%s", snd_strerror(ret));
            return ret;
        }
        /* Write parameters */
        ret = snd_pcm_hw_params(pcm_handle, hw_params);
        if (ret < 0)
        {
            OS_LOGE("%s", snd_strerror(ret));
            return ret;
        }
        return ret;
    }

    bool _ready()
    {
        return pcm_handle ? true : false;
    }


    int _write(const uint8_t *data, int len)
    {
        int ret = -1;
        //
        if (!pcm_handle)
        {
            return ret;
        }
        //
        if (!data || len <= 0)
        {
            return ret;
        }
        snd_pcm_uframes_t fc = len / chn / sizeof(short);
        //
        ret = snd_pcm_writei(pcm_handle, data, fc);
        if (ret < 0)
        {
            OS_LOGE("%s", snd_strerror(ret));
            return ret;
        }
        return ret;
    }

   public:
    AlsaPcmPlayer(std::string device = "default", unsigned int samplerate = 16000, int chn = 1)
        : device(std::move(device)), samplerate(samplerate), chn(chn)
    {
        _init();
    }
    ~AlsaPcmPlayer()
    {
        _deinit();
    }

    int init()
    {
        int ret = -1;
        //
        ret = _init();
        if (ret < 0)
        {
            _deinit();
        }
        return ret;
    }

    int deinit()
    {
        return _deinit();
    }

    bool ready()
    {
        return _ready();
    }

    int write(const uint8_t *data, int len)
    {
        return _write(data, len);
    }
};

static std::shared_ptr<AlsaPcmPlayer> m_player(nullptr);

}  // namespace

int simple_alsa_pcm_player_start(const char *device, unsigned int samplerate, int chn)
{
    m_player = std::make_shared<AlsaPcmPlayer>(device, samplerate, chn);
    //
    if (!m_player)
    {
        return -1;
    }
    if (!m_player->ready())
    {
        return -1;
    }
    return 0;
}

int simple_alsa_pcm_player_stop()
{
    m_player = nullptr;
    return 0;
}

int simple_alsa_pcm_player_write(const uint8_t *data, int len)
{
    if (!m_player)
    {
        return -1;
    }
    if (!m_player->ready())
    {
        return -1;
    }
    return m_player->write(data, len);
}

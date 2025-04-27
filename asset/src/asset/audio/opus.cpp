#include "muon/asset/audio/opus.hpp"

#include <opus/opus.h>
#include <opus/opusfile.h>

namespace muon::asset {

    std::optional<Audio> decodeOpus(const std::vector<uint8_t> &data) {
        OggOpusFile *opusFile = op_open_memory(data.data(), data.size(), nullptr);
        if (opusFile == nullptr) {
            return {};
        }

        Audio audio;
        audio.sampleRate = 48000;
        audio.channels = op_channel_count(opusFile, -1);

        std::vector<float> samples{};
        const int32_t bufferSize = 4096;
        float temp[bufferSize * 2];

        int32_t samplesRead{0};
        while ((samplesRead = op_read_float(opusFile, temp, bufferSize * audio.channels, nullptr)) > 0) {
            samples.insert(samples.end(), temp, temp + samplesRead * audio.channels);
        }

        op_free(opusFile);

        audio.samples = std::move(samples);
        return audio;
    }

}

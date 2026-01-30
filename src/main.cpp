#include <lv2/core/lv2.h>
#include <cmath>
#include <algorithm>
#include <numbers>

#define SHAPING_FUZZ_URI "https://github.com/YimRakHee/ShapingFuzzForBass"

class ShapingFuzz {
public:
    enum PortIndex { IN = 0, OUT = 1, DRIVE = 2, TONE = 3, DRY_LVL = 4, WET_LVL = 5 };

    struct LowPassFilter {
        float y_prev = 0.0f;
        float alpha = 0.1f;
        float one_minus_alpha = 0.9f; 

        void update_coeff(float cutoff, double sample_rate) {
            double rc = 1.0 / (2.0 * std::numbers::pi * cutoff);
            double dt = 1.0 / sample_rate;
            alpha = static_cast<float>(dt / (rc + dt));
            one_minus_alpha = 1.0f - alpha;
        }

        inline float process(float sample) {
            float y = (alpha * sample) + (one_minus_alpha * y_prev);

            if (std::abs(y) < 1e-15f) y = 0.0f;

            y_prev = y;
            return y;
        }
    };

    float* ports[6];
    double sample_rate;
    LowPassFilter tone_filter;
    float last_tone = -1.0f;

    static inline float fast_tanh(float x) {
        float x_clamped = std::clamp(x, -3.0f, 3.0f);
        float x2 = x_clamped * x_clamped;
        return x_clamped * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    static LV2_Handle instantiate(const LV2_Descriptor*, double rate, const char*, const LV2_Feature* const*) {
        auto* self = new ShapingFuzz();
        self->sample_rate = rate;
        return static_cast<LV2_Handle>(self);
    }

    static void connect_port(LV2_Handle instance, uint32_t port, void* data) {
        static_cast<ShapingFuzz*>(instance)->ports[port] = static_cast<float*>(data);
    }

    static void run(LV2_Handle instance, uint32_t sample_count) {
        auto* self = static_cast<ShapingFuzz*>(instance);

        const float* __restrict__ input = self->ports[IN];
        float* __restrict__ output      = self->ports[OUT];
        const float drive  = *self->ports[DRIVE];
        const float tone   = *self->ports[TONE];
        const float dry    = *self->ports[DRY_LVL];
        const float wet    = *self->ports[WET_LVL];

        if (tone != self->last_tone) {
            self->tone_filter.update_coeff(tone, self->sample_rate);
            self->last_tone = tone;
        }

        for (uint32_t i = 0; i < sample_count; ++i) {
            const float in_sample = input[i];

            float wet_sample = fast_tanh(in_sample * drive);

            wet_sample = self->tone_filter.process(wet_sample);

            output[i] = (in_sample * dry) + (wet_sample * wet);
        }
    }

    static void cleanup(LV2_Handle instance) {
        delete static_cast<ShapingFuzz*>(instance);
    }
};

static const LV2_Descriptor descriptor = {
    SHAPING_FUZZ_URI, ShapingFuzz::instantiate, ShapingFuzz::connect_port,
    nullptr, ShapingFuzz::run, nullptr, ShapingFuzz::cleanup, nullptr
};

LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index) {
    return (index == 0) ? &descriptor : nullptr;
}

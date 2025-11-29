#include <Arduino.h>
#include <cstdint>
#include <array>
#include <esp_random.h>

#include "common_types.h"
#include "animations.h"
#include "inputs.h"
#include "outputs.h"
#include "knobs.h"
#include "configuration.h"

namespace animations {

    constexpr const unsigned max_stages = 16;
    constexpr const unsigned max_next_stage = 16;

    struct AnimationStage;

    using StagesArray = std::array<AnimationStage, max_stages>;

    struct AnimationTransition {
        long int start_ms;
        long int end_ms;
        long int next_ms;
        ColorChannels start_color;
        ColorChannels end_color;
        AnimationStage* next_stage;
        void setColor();
    };

    struct AnimationStage {
        unsigned fade_in_ms;
        unsigned fade_in_randomness;
        unsigned period_ms;
        unsigned period_randomness;
        char colorspace[4];
        ColorChannels base_color;
        ColorChannels color_randomness;
        std::array<std::int8_t, max_next_stage> next_stages;
        std::uint8_t next_stages_size;
        bool use_white;
        void generateTransition(AnimationTransition* transition) const;
    };


    StagesArray loaded_stages;
    AnimationTransition current_transition;


    void checkTimer() {
        if (inputs::source_control == inputs::scAnimation) {
            current_transition.setColor();
        }
    }


    void startAnimations(unsigned id) {
        JsonDocument animations = configuration::getAnimations();
        unsigned size = animations.size();
        if (id >= size) return;
        const auto& stages = animations[id];
        unsigned stages_count = stages.size();
        if (stages_count > max_stages) stages_count = max_stages;
        if (!stages_count) return;
        for (unsigned i=0;i<stages_count;i++) {
            AnimationStage& stage = loaded_stages[i];
            const auto& stage_json = stages[i];
            stage.fade_in_ms = stage_json["fade_in_ms"].as<unsigned>();
            stage.fade_in_randomness = stage_json["fade_in_randomness"].as<unsigned>();
            stage.period_ms = stage_json["period_ms"].as<unsigned>();
            stage.period_randomness = stage_json["period_randomness"].as<unsigned>();
            String colorspace = stage_json["colorspace"].as<String>();
            for (unsigned j=0;j<3;j++)
                stage.colorspace[j] = colorspace[j];
            stage.colorspace[3] = 0;
            const auto& base_color = stage_json["base_color"];
            for (unsigned j=0;j<4;j++)
                stage.base_color[j] = base_color[j].as<fixed32_c>();
            const auto& color_randomness = stage_json["color_randomness"];
            for (unsigned j=0;j<4;j++)
                stage.color_randomness[j] = color_randomness[j].as<fixed32_c>();
            const auto& next_stages = stage_json["next_stages"];
            stage.next_stages_size = next_stages.size();
            if (stage.next_stages_size > max_next_stage) stage.next_stages_size = max_next_stage;
            for (unsigned j=0;j<stage.next_stages_size;j++) {
                unsigned val = next_stages[j].as<unsigned>();
                if (val >= stages_count) val %= stages_count;
                stage.next_stages[j] = val;
            }
            stage.use_white = stage_json.as<bool>();
        }
        inputs::source_control = inputs::scAnimation;
    }


    void AnimationTransition::setColor() {
        unsigned long current_time = millis();
        if (current_time >= end_ms) {
            outputs::setColor(end_color);
        } else {
            fixed32_c frac = fixed32_c::fraction(current_time - start_ms, end_ms - start_ms);
            fixed32_c rest = 1 - frac;
            ColorChannels out_color;
            for (int i=0;i<4;i++)
                out_color[i] = start_color[i] * rest + end_color[i] * frac;
            outputs::setColor(out_color);
        }
        if (current_time >= next_ms)
            next_stage->generateTransition(this);
        loaded_stages[0].generateTransition(&current_transition);
    }


    void AnimationStage::generateTransition(AnimationTransition* transition) const {
        transition->start_ms = millis();
        transition->end_ms = transition->start_ms + fade_in_ms + (esp_random() % fade_in_randomness);
        transition->next_ms = transition->end_ms + period_ms + (esp_random() % period_randomness);
        transition->start_color = outputs::getColor();
        ColorChannels input_color;
        for (int i=0;i<4;i++)
            input_color[i] = base_color[i] + fixed32_c::buf_cast((esp_random() % color_randomness[i].getBuf())) - (color_randomness[i] >> 1);
        if (!use_white)
            input_color[3] = base_color[3];
        transition->end_color = inputs::prepareAuto(colorspace, input_color);
        if (transition->end_color[0] - transition->start_color[0] > fixed32_c::fraction(1, 2))
            transition->start_color[0] += 1;
        else if (transition->start_color[0] - transition->end_color[0] > fixed32_c::fraction(1, 2))
            transition->end_color[0] += 1;
        if (next_stages_size)
            transition->next_stage = &loaded_stages[next_stages[esp_random() % next_stages_size]];
    }


}


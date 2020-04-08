/*
 * Copyright (C) 2020 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "DacHalControl.h"

#include <android-base/logging.h>
#include <android-base/strings.h>
#include <cutils/properties.h>

#include <fstream>

namespace vendor {
namespace lge {
namespace hardware {
namespace audio {
namespace dac {
namespace control {
namespace V1_0 {
namespace implementation {

static std::vector<KeyValue> quaddac_states = {{"Off", "0"}, {"On", "1"}};

static std::vector<KeyValue> sound_presets = {{"Normal", "0"},
                                              {"Enhanced", "1"},
                                              {"Detailed", "2"},
                                              {"Live", "3"},
                                              {"Bass", "4"}};

static std::vector<KeyValue> digital_filters = {{"Short", "0"},
                                                {"Sharp", "1"},
                                                {"Slow", "2"}};

DacHalControl::DacHalControl() {
    mAudioClient = IDevice::getService();

    mAudioClient->getParameters(hidl_vec<hidl_string> {DAC_COMMAND}, [this](Result a, const std::vector<ParameterValue>& b) {
        if(a != Result::OK) {
            LOG(ERROR) << "DacHalControl: DAC not supported by HAL, exiting...";
            return;
        }
        std::vector<KeyValue> fstates;

        for(auto pv : b) {
            fstates.push_back(KeyValue {
                                pv.key.c_str(), 
                                pv.value.c_str()
                                });
        }
        
        FeatureStates states;
        states.states = hidl_vec<KeyValue> {fstates};
        mSupportedStates.emplace(HalFeature::QuadDAC, states);
        mSupportedHalFeatures.push_back(HalFeature::QuadDAC);

        setFeatureValue(HalFeature::QuadDAC, getFeatureValue(HalFeature::QuadDAC));
    });
    
    mAudioClient->getParameters(hidl_vec<hidl_string> {SET_DIGITAL_FILTER_COMMAND}, [this](Result a, const std::vector<ParameterValue>& b) {
        if(a != Result::OK) {
            LOG(ERROR) << "DacHalControl: ESS_FILTER not supported by HAL, exiting...";
            return;
        }
        std::vector<KeyValue> fstates;

        for(auto pv : b) {
            for(auto kv : digital_filters) {
                if(std::string(pv.value).compare(std::string(kv.value)) == 0) {
                    fstates.push_back(kv);
                }
            }
        }

        FeatureStates states;
        states.states = hidl_vec<KeyValue> {fstates};
        mSupportedStates.emplace(HalFeature::DigitalFilter, states);
        mSupportedHalFeatures.push_back(HalFeature::DigitalFilter);

        setFeatureValue(HalFeature::DigitalFilter, getFeatureValue(HalFeature::DigitalFilter));
    });

    mAudioClient->getParameters(hidl_vec<hidl_string> {SET_SOUND_PRESET_COMMAND}, [this](Result a, const std::vector<ParameterValue>& b) {
        if(a != Result::OK) {
            LOG(ERROR) << "DacHalControl: SOUND_PRESET not supported by HAL, exiting...";
            return;
        }
        std::vector<KeyValue> fstates;

        for(auto pv : b) {
            for(auto kv : sound_presets) {
                if(std::string(pv.value).compare(std::string(kv.value)) == 0) {
                    fstates.push_back(kv);
                }
            }
        }

        FeatureStates states;
        states.states = hidl_vec<KeyValue> {fstates};
        mSupportedStates.emplace(HalFeature::SoundPreset, states);
        mSupportedHalFeatures.push_back(HalFeature::SoundPreset);

        setFeatureValue(HalFeature::SoundPreset, getFeatureValue(HalFeature::SoundPreset));
    });

    mAudioClient->getParameters(hidl_vec<hidl_string> {SET_LEFT_BALANCE_COMMAND}, [this](Result a, const std::vector<ParameterValue>& b) {
        if(a != Result::OK) {
            LOG(ERROR) << "DacHalControl: LEFT_BALANCE not supported by HAL, exiting...";
            return;
        }
        FeatureStates states;
        Range range;
        range.min = std::stof(b[0].value);
        range.max = std::stof(b[0].value);

        for(int i = 1; i < b.size(); i++) {
            float val = std::stof(b[i].value);
            if(val < range.min) {
                range.min = val;
            } else if(val > range.max) {
                range.max = val;
            }
        }
        range.step = 1;
        states.range = range;
        mSupportedStates.emplace(HalFeature::BalanceLeft, states);
        mSupportedHalFeatures.push_back(HalFeature::BalanceLeft);

        setFeatureValue(HalFeature::BalanceLeft, getFeatureValue(HalFeature::BalanceLeft));
    });

    mAudioClient->getParameters(hidl_vec<hidl_string> {SET_RIGHT_BALANCE_COMMAND}, [this](Result a, const std::vector<ParameterValue>& b) {
        if(a != Result::OK) {
            LOG(ERROR) << "DacHalControl: LEFT_BALANCE not supported by HAL, exiting...";
            return;
        }
        FeatureStates states;
        Range range;
        range.min = std::stof(b[0].value);
        range.max = std::stof(b[0].value);

        for(int i = 1; i < b.size(); i++) {
            float val = std::stof(b[i].value);
            if(val < range.min) {
                range.min = val;
            } else if(val > range.max) {
                range.max = val;
            }
        }
        range.step = 1;
        states.range = range;
        mSupportedStates.emplace(HalFeature::BalanceRight, states);
        mSupportedHalFeatures.push_back(HalFeature::BalanceRight);

        setFeatureValue(HalFeature::BalanceRight, getFeatureValue(HalFeature::BalanceRight));
    });
}

Return<void> DacHalControl::getSupportedHalFeatures(getSupportedHalFeatures_cb _hidl_cb) {
    
    _hidl_cb(mSupportedHalFeatures);
    
    return Void();
}

Return<void> DacHalControl::getSupportedHalFeatureValues(HalFeature feature, getSupportedHalFeatureValues_cb _hidl_cb) {
    std::map<HalFeature, FeatureStates>::iterator it;
    it = mSupportedStates.find(feature);
    if (it != mSupportedStates.end()) {
        _hidl_cb(it->second);
    } else {
        LOG(ERROR) << "DacAdvancedControl::getSupportedAdvancedFeatureValues: tried to get values for unsupported Feature...";
    }
    
    return Void();
}

Return<bool> DacHalControl::setFeatureValue(HalFeature feature, int32_t value) {
    
    if(std::find(mSupportedHalFeatures.begin(), mSupportedHalFeatures.end(), feature) == mSupportedHalFeatures.end()) {
        LOG(ERROR) << "DacHalControl::setFeatureValue: tried to set value for unsupported Feature...";
        return false;
    }

    ParameterValue pv;
    std::string property;
    if(feature == HalFeature::QuadDAC) {
        pv.key = DAC_COMMAND;
        property = PROPERTY_HIFI_DAC_ENABLED;
        if(value == 0) {
            pv.value = SET_DAC_OFF_COMMAND;
        } else if(value == 1) {
            pv.value = SET_DAC_ON_COMMAND;
        }
    } else {
        switch(feature) {
            case HalFeature::DigitalFilter: {
                pv.key = SET_DIGITAL_FILTER_COMMAND;
                property = PROPERTY_DIGITAL_FILTER;
                break;
            }
            case HalFeature::SoundPreset: {
                pv.key = SET_SOUND_PRESET_COMMAND;
                property = PROPERTY_SOUND_PRESET;
                break;
            }
            case HalFeature::BalanceLeft: {
                pv.key = SET_LEFT_BALANCE_COMMAND;
                property = PROPERTY_LEFT_BALANCE;
                break;
            }
            case HalFeature::BalanceRight: {
                pv.key = SET_RIGHT_BALANCE_COMMAND;
                property = PROPERTY_RIGHT_BALANCE;
                break;
            }
            default: return false;
        }
        pv.value = std::to_string(value);
    }
    
    auto ret = mAudioClient->setParameters(hidl_vec<ParameterValue> {pv});

    if(ret == Result::OK) {
        property_set(property.c_str(), pv.value.c_str());
        return true;
    } else {
        return false;
    }
}

Return<int32_t> DacHalControl::getFeatureValue(HalFeature feature) {
    if(std::find(mSupportedHalFeatures.begin(), mSupportedHalFeatures.end(), feature) == mSupportedHalFeatures.end()) {
        LOG(ERROR) << "DacHalControl::getFeatureValue: tried to set value for unsupported Feature...";
        return -1;
    }
    int32_t ret;
    std::string property;
    char value[PROPERTY_VALUE_MAX];
    if(feature == HalFeature::QuadDAC) {
        property = PROPERTY_HIFI_DAC_ENABLED;
        property_get(property.c_str(), value, "off");
        if(strcmp(value, "off") == 0) {
            ret = 0;
        } else if(strcmp(value, "on") == 0) {
            ret = 1;
        } else {
            ret = 0;
        }
    } else {
        switch(feature) {
            case HalFeature::DigitalFilter: {
                property = PROPERTY_DIGITAL_FILTER;
                break;
            }
            case HalFeature::SoundPreset: {
                property = PROPERTY_SOUND_PRESET;
                break;
            }
            case HalFeature::BalanceLeft: {
                property = PROPERTY_LEFT_BALANCE;
                break;
            }
            case HalFeature::BalanceRight: {
                property = PROPERTY_RIGHT_BALANCE;
                break;
            }
            default: return false;
        }
        property_get(property.c_str(), value, "0");
        ret = std::stoi(value);
    }
    return ret;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace control
}  // namespace dac
}  // namespace audio
}  // namespace hardware
}  // namespace lge
}  // namespace vendor

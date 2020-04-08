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

#include "DacAdvancedControl.h"

#include <android-base/logging.h>
#include <android-base/file.h>
#include <cutils/properties.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>

using android::base::ReadFileToString;
using android::base::Trim;
using android::base::WriteStringToFile;

namespace vendor {
namespace lge {
namespace hardware {
namespace audio {
namespace dac {
namespace control {
namespace V1_0 {
namespace implementation {

/*
 * Write value to path and close file.
 */
template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value;
}

template <typename T>
static T get(const std::string& path, const T& def) {
    std::ifstream file(path);
    T result;

    file >> result;
    return file.fail() ? def : result;
}



DacAdvancedControl::DacAdvancedControl() {
    
    for(const auto & entry : std::filesystem::directory_iterator(COMMON_ES9218_PATH)) {
        // There should only be 1 subdirectory, but check anyway
        if(entry.is_directory()) {
            mDacBasePath = entry.path();
            if(mDacBasePath.find("0048") != std::string::npos) {
                break;
            }
        }
    }
    std::ofstream avc(mDacBasePath + AVC_VOLUME);
    std::ofstream hifi(mDacBasePath + HIFI_MODE);
    
    if(mDacBasePath.empty()) {
        LOG(ERROR) << "DacAdvancedControl: No ES9218 path found, exiting...";
        return;
    }
    
    if (avc) {
        mSupportedAdvancedFeatures.push_back(AdvancedFeature::AVCVolume);
    }
    
    if (hifi) {
        mSupportedAdvancedFeatures.push_back(AdvancedFeature::HifiMode);
    }

}

Return<void> DacAdvancedControl::getSupportedAdvancedFeatures(getSupportedAdvancedFeatures_cb _hidl_cb) {
    _hidl_cb(mSupportedAdvancedFeatures);

    return Void();
}

FeatureStates DacAdvancedControl::getAvcVolumeStates() {
    FeatureStates states;
    
    states.range.min = -24;
    states.range.max = 0;
    states.range.step = 1;
    
    return states;
}

static std::vector<KeyValue> hifi_modes = {{"Normal", "0"},
                                           {"High Impedance", "1"},
                                           {"AUX", "2"}};

FeatureStates DacAdvancedControl::getHifiModeStates() {
    FeatureStates states;
    
    states.states = hidl_vec<KeyValue> {hifi_modes};
    
    return states;
}

Return<void> DacAdvancedControl::getSupportedAdvancedFeatureValues(AdvancedFeature feature, getSupportedAdvancedFeatureValues_cb _hidl_cb) {

    if(std::find(mSupportedAdvancedFeatures.begin(), mSupportedAdvancedFeatures.end(), feature) != mSupportedAdvancedFeatures.end()) {
        switch(feature) {
            case AdvancedFeature::AVCVolume: {
                    _hidl_cb(getAvcVolumeStates());
                    break;
                }
            case AdvancedFeature::HifiMode: {
                    _hidl_cb(getHifiModeStates());
                    break;
                }
            default: break;
        }
    } else {
        LOG(ERROR) << "DacAdvancedControl::getSupportedAdvancedFeatureValues: tried to get values for unsupported Feature...";
    }
    
    return Void();
}

bool DacAdvancedControl::writeAvcVolumeState(int32_t value) {
    set(mDacBasePath + AVC_VOLUME, value);
    return (bool)property_set(PROPERTY_HIFI_DAC_AVC_VOLUME, std::to_string(value).c_str());
}

bool DacAdvancedControl::writeHifiModeState(int32_t value) {
    set(mDacBasePath + HIFI_MODE, value);
    return (bool)property_set(PROPERTY_HIFI_DAC_MODE, std::to_string(value).c_str());
}

Return<bool> DacAdvancedControl::setFeatureValue(AdvancedFeature feature, int32_t value) {
    
    if(std::find(mSupportedAdvancedFeatures.begin(), mSupportedAdvancedFeatures.end(), feature) == mSupportedAdvancedFeatures.end()) {
        LOG(ERROR) << "DacAdvancedControl::setFeatureValue: tried to set value for unsupported Feature...";
        return false;
    }

    switch(feature) {
        case AdvancedFeature::AVCVolume: {
                return writeAvcVolumeState(value);
            }
        case AdvancedFeature::HifiMode: {
                return writeHifiModeState(value);
            }
        default: break;
    }
    return false;
}

Return<int32_t> DacAdvancedControl::getFeatureValue(AdvancedFeature feature) {
    int32_t ret;
    
    if(std::find(mSupportedAdvancedFeatures.begin(), mSupportedAdvancedFeatures.end(), feature) == mSupportedAdvancedFeatures.end()) {
        LOG(ERROR) << "DacAdvancedControl::getFeatureValue: tried to get value for unsupported Feature...";
        return -1;
    }

    switch(feature) {
        case AdvancedFeature::AVCVolume: {
                ret = get(mDacBasePath + AVC_VOLUME, AVC_VOLUME_DEFAULT);
                break;
            }
        case AdvancedFeature::HifiMode: {
                int32_t val = get(mDacBasePath + HIFI_MODE, HIFI_MODE_DEFAULT);
                for(const KeyValue kv : hifi_modes) {
                    if(val == std::stoi(kv.value)) {
                        ret = val;
                        break;
                    }
                }
                break;
            }
        default: break;
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

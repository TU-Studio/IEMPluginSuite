/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at

 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <https://www.gnu.org/licenses/>.
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StereoEncoderAudioProcessor::StereoEncoderAudioProcessor()
    : AudioProcessorBase(
#ifndef JucePlugin_PreferredChannelConfigurations
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::discreteChannels(64), true)
#endif
              ,
#endif
          createParameterLayout()),
      posC(1.0f, 0.0f, 0.0f),
      posL(1.0f, 0.0f, 0.0f),
      posR(1.0f, 0.0f, 0.0f),
      updatedPositionData(true)
{
    parameters.addParameterListener("qw", this);
    parameters.addParameterListener("qx", this);
    parameters.addParameterListener("qy", this);
    parameters.addParameterListener("qz", this);
    parameters.addParameterListener("azimuth", this);
    parameters.addParameterListener("elevation", this);
    parameters.addParameterListener("roll", this);
    parameters.addParameterListener("width", this);

    parameters.addParameterListener("deltaTime", this);
    parameters.addParameterListener("grainLength", this);

    parameters.addParameterListener("orderSetting", this);
    parameters.addParameterListener("useSN3D", this);

    orderSetting = parameters.getRawParameterValue("orderSetting");
    useSN3D = parameters.getRawParameterValue("useSN3D");
    qw = parameters.getRawParameterValue("qw");
    qx = parameters.getRawParameterValue("qx");
    qy = parameters.getRawParameterValue("qy");
    qz = parameters.getRawParameterValue("qz");
    azimuth = parameters.getRawParameterValue("azimuth");
    elevation = parameters.getRawParameterValue("elevation");

    shape = parameters.getRawParameterValue("shape");
    size = parameters.getRawParameterValue("size");

    roll = parameters.getRawParameterValue("roll");
    width = parameters.getRawParameterValue("width");

    deltaTime = parameters.getRawParameterValue("deltaTime");
    deltaTimeMod = parameters.getRawParameterValue("deltaTimeMod");

    grainLength = parameters.getRawParameterValue("grainLength");
    grainLengthMod = parameters.getRawParameterValue("grainLengthMod");

    position = parameters.getRawParameterValue("position");
    positionMod = parameters.getRawParameterValue("positionMod");

    pitch = parameters.getRawParameterValue("pitch");
    pitchMod = parameters.getRawParameterValue("pitchMod");

    windowAttack = parameters.getRawParameterValue("windowAttack");
    windowAttackMod = parameters.getRawParameterValue("windowAttackMod");

    windowDecay = parameters.getRawParameterValue("windowDecay");
    windowDecayMod = parameters.getRawParameterValue("windowDecayMod");

    freeze = parameters.getRawParameterValue("freeze");

    highQuality = parameters.getRawParameterValue("highQuality");

    processorUpdatingParams = false;

    sphericalInput = true; // input from ypr

    juce::FloatVectorOperations::clear(SHL, 64);
    juce::FloatVectorOperations::clear(SHR, 64);

    _hannWindowBuffer.setSize(1, windowResolution);
    _rectWindowBuffer.setSize(1, windowResolution);

    float *_hannWindowBufferWritePtr = _hannWindowBuffer.getWritePointer(0);
    float *_rectWindowBufferWritePtr = _rectWindowBuffer.getWritePointer(0);

    for (int i = 0; i < windowResolution; i++)
    {
        _hannWindowBufferWritePtr[i] = std::pow(std::sin(i * juce::MathConstants<float>::pi / windowResolution), 2);
        _rectWindowBufferWritePtr[i] = 1.0f;
    }
    _currentWindow = &_rectWindowBuffer;
}

StereoEncoderAudioProcessor::~StereoEncoderAudioProcessor() = default;

//==============================================================================

int StereoEncoderAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int StereoEncoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StereoEncoderAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String StereoEncoderAudioProcessor::getProgramName(int index)
{
    return juce::String();
}

void StereoEncoderAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
}

//==============================================================================
void StereoEncoderAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, 2, *orderSetting, true);

    bufferCopy.setSize(2, samplesPerBlock);
    grainOutBuffer.setSize(1, samplesPerBlock);

    if (mode != OperationMode::Freeze)
    {
        circularBuffer.setSize(2, juce::roundToInt(sampleRate * CIRC_BUFFER_SECONDS)); // seconds long circular buffer
        circularBufferWriteHead = 0;
        circularBufferLength = circularBuffer.getNumSamples();
        circularBuffer.clear();
    }

    writeGainCircBuffer.reset(sampleRate, 0.1f);
    writeGainCircBuffer.setCurrentAndTargetValue(1.0f);

    lastSampleRate = sampleRate;
    deltaTimeSamples = 0;

    for (int g = 0; g < maxNumGrains; g++)
    {
        grains[g].setBlockSize(samplesPerBlock);
    }
    // maxNumGrains = (size_t) grains;

    smoothAzimuthL.setCurrentAndTargetValue(*azimuth / 180.0f * juce::MathConstants<float>::pi);
    smoothElevationL.setCurrentAndTargetValue(*elevation / 180.0f * juce::MathConstants<float>::pi);

    smoothAzimuthR.setCurrentAndTargetValue(*azimuth / 180.0f * juce::MathConstants<float>::pi);
    smoothElevationR.setCurrentAndTargetValue(*elevation / 180.0f * juce::MathConstants<float>::pi);

    smoothAzimuthL.reset(1, samplesPerBlock);
    smoothElevationL.reset(1, samplesPerBlock);
    smoothAzimuthR.reset(1, samplesPerBlock);
    smoothElevationR.reset(1, samplesPerBlock);

    const float widthInRadiansQuarter{Conversions<float>::degreesToRadians(*width) / 4.0f};
    const iem::Quaternion<float> quatLRot{iem::Quaternion<float>(cos(widthInRadiansQuarter), 0.0f, 0.0f, sin(widthInRadiansQuarter))};
    const iem::Quaternion<float> quatL = quaternionDirection * quatLRot;
    const iem::Quaternion<float> quatR = quaternionDirection * quatLRot.getConjugate();

    const auto left = quatL.getCartesian();
    const auto right = quatR.getCartesian();

    SHEval(7, left, _SHL);
    SHEval(7, right, _SHR);

    positionHasChanged = true; // just to be sure
}

void StereoEncoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

inline void StereoEncoderAudioProcessor::updateQuaternions()
{
    float ypr[3];
    ypr[0] = Conversions<float>::degreesToRadians(*azimuth);
    ypr[1] = -Conversions<float>::degreesToRadians(*elevation); // pitch
    ypr[2] = Conversions<float>::degreesToRadians(*roll);

    // updating not active params
    quaternionDirection.fromYPR(ypr);
    processorUpdatingParams = true;
    parameters.getParameter("qw")->setValueNotifyingHost(parameters.getParameterRange("qw").convertTo0to1(quaternionDirection.w));
    parameters.getParameter("qx")->setValueNotifyingHost(parameters.getParameterRange("qx").convertTo0to1(quaternionDirection.x));
    parameters.getParameter("qy")->setValueNotifyingHost(parameters.getParameterRange("qy").convertTo0to1(quaternionDirection.y));
    parameters.getParameter("qz")->setValueNotifyingHost(parameters.getParameterRange("qz").convertTo0to1(quaternionDirection.z));
    processorUpdatingParams = false;
}

void StereoEncoderAudioProcessor::updateEuler()
{
    float ypr[3];
    quaternionDirection = iem::Quaternion<float>(*qw, *qx, *qy, *qz);
    quaternionDirection.normalize();
    quaternionDirection.toYPR(ypr);

    // updating not active params
    processorUpdatingParams = true;
    parameters.getParameter("azimuth")->setValueNotifyingHost(parameters.getParameterRange("azimuth").convertTo0to1(Conversions<float>::radiansToDegrees(ypr[0])));
    parameters.getParameter("elevation")->setValueNotifyingHost(parameters.getParameterRange("elevation").convertTo0to1(-Conversions<float>::radiansToDegrees(ypr[1])));
    parameters.getParameter("roll")->setValueNotifyingHost(parameters.getParameterRange("roll").convertTo0to1(Conversions<float>::radiansToDegrees(ypr[2])));
    processorUpdatingParams = false;
}

juce::Vector3D<float> StereoEncoderAudioProcessor::getRandomGrainDirection()
{
    float shape_abs = std::abs(*shape);
    float shape_param = std::pow(2, shape_abs);
    float size_param = *size;
    juce::Vector3D<float> centerDir = quaternionDirection.getCartesian();

    float azi_center;
    float ele_center;
    Conversions<float>::cartesianToSpherical(centerDir, azi_center, ele_center);
    float zen_center = (juce::MathConstants<float>::pi / 2.0f) - ele_center;

    // Uniform random distribution in [0,2*pi]
    float rand_phi = juce::Random::getSystemRandom().nextFloat() * 2.0f * juce::MathConstants<float>::pi;

    // Beta distribution to control shape of rotationally symmetric distribution around centerDir
    jassert(shape_param >= 1.0f);
    // beta_distribution<float> dist(shape, shape);
    std::gamma_distribution<float> dist1(shape_param, 1.0f);
    std::gamma_distribution<float> dist2(shape_param, 1.0f);
    float gamma1 = dist1(rng);
    float gamma2 = dist2(rng);
    float eps = 1e-16f;
    float beta_val = (gamma1 + eps) / (gamma1 + gamma2 + eps);

    // float sym_beta_sample = abs(dist(rng) - 0.5f) * 2.0f;
    float sym_beta_sample;
    if (*shape < 0.0f)
        sym_beta_sample = 1.0f - abs(beta_val - 0.5f) * 2.0f;
    else
        sym_beta_sample = abs(beta_val - 0.5f) * 2.0f;

    // Input parameter size defines opening angle of distribution cap
    jassert(size_param >= 0.0f && size_param <= 360.0f);
    float size_factor = (size_param / 2.0f) / 180.0f;
    float beta_sized = sym_beta_sample * size_factor;

    // To achieve uniform distribution on a sphere
    float rand_theta = acos(2 * (1 - beta_sized) - 1);

    float sinZenith = sin(rand_theta);
    float x = cos(rand_phi) * sinZenith;
    float y = sin(rand_phi) * sinZenith;
    float z = cos(rand_theta);

    // Rotate random direction (on 'polar cap') to target center direction
    float cosAzi = cos(azi_center);
    float sinAzi = sin(azi_center);
    float cosZen = cos(zen_center);
    float sinZen = sin(zen_center);

    float target_x = cosAzi * cosZen * x - sinAzi * y + cosAzi * sinZen * z;
    float target_y = sinAzi * cosZen * x + cosAzi * y + sinAzi * sinZen * z;
    float target_z = -sinZen * x + cosZen * z;

    juce::Vector3D<float> vec(target_x, target_y, target_z);
    return vec;
}

juce::AudioBuffer<float> StereoEncoderAudioProcessor::getWindowBuffer()
{
    const float attackPercentage = *windowAttack;
    const float decayPercentage = *windowDecay;
    const int windowNumSamples = windowResolution;
    const int windowHalfNumSamples = windowResolution / 2;

    const int windowAttackSamples =  attackPercentage / 100.0f * windowNumSamples;
    const int windowDecaySamples =  decayPercentage / 100.0f * windowNumSamples;

    juce::AudioBuffer<float> windowBuffer;
    windowBuffer.setSize(1, windowResolution);

    float *windowBufferWritePtr = windowBuffer.getWritePointer(0);

    float pi_over_two = (juce::MathConstants<float>::pi / 2.0f);

    for (int i = 0; i < windowAttackSamples; i++)
    {
        // sine-squared fade-in
        windowBufferWritePtr[i] = std::pow(std::sin(static_cast<float>(i) / static_cast<float>(windowAttackSamples) * pi_over_two), 2);
    }
    for (int i = windowAttackSamples; i < (windowNumSamples - windowDecaySamples); i++)
    {
        // rectangular part
        windowBufferWritePtr[i] = 1.0f;
    }
    int c = 0;
    for (int i = (windowNumSamples - windowDecaySamples); i < windowNumSamples; i++)
    {
        // cosine-squared fade-out
        windowBufferWritePtr[i] = std::pow(std::cos(static_cast<float>(c) / static_cast<float>(windowDecaySamples) * pi_over_two), 2);
        c++;
    }

    return windowBuffer;
}

int StereoEncoderAudioProcessor::getStartPositionCircBuffer() const
{
    int startPositionCircBuffer = circularBufferWriteHead - juce::roundToInt(*position * lastSampleRate);
    int startMod = juce::roundToInt(*positionMod / 100.0f * juce::Random::getSystemRandom().nextFloat() * (CIRC_BUFFER_SECONDS / 2.0f) * lastSampleRate);
    // Unidirectional modulation of position (due to hard real-time constraint)
    startPositionCircBuffer -= startMod;
    if (startPositionCircBuffer < 0)
    {
        startPositionCircBuffer += circularBufferLength;
    }

    return startPositionCircBuffer;
}

std::pair<int, float> StereoEncoderAudioProcessor::getGrainLengthAndPitchFactor() const
{
    // Bidirectional modulation of grain length
    float grainLengthModSeconds = *grainLengthMod / 100.0f * (*grainLength) * 2 * (juce::Random::getSystemRandom().nextFloat() - 0.5f);
    float newGrainLengthSeconds = *grainLength + grainLengthModSeconds;
    newGrainLengthSeconds = std::min(newGrainLengthSeconds, 0.5f);
    newGrainLengthSeconds = std::max(newGrainLengthSeconds, 0.001f);

    // jassert(newGrainLengthSeconds >= 0.001f && newGrainLengthSeconds =< 0.5f);
    float grainLengthSamplesFloat = newGrainLengthSeconds * lastSampleRate;

    // Unidirectional modulation of pitch (due to hard real-time constraint)
    const float maxPitchModulation = 0.125f;
    float pitchModSemitones = *pitchMod / 100.0f * maxPitchModulation * juce::Random::getSystemRandom().nextFloat();
    float pitchReadFactor = std::pow(2.0f, (*pitch - pitchModSemitones) / 12.0f);

    // Updated length of grain in samples
    int grainLengthSamples = static_cast<int>(grainLengthSamplesFloat * (1 / pitchReadFactor));

    return std::make_pair(grainLengthSamples, pitchReadFactor);
}

int StereoEncoderAudioProcessor::getDeltaTimeSamples()
{
    // Bidirectional modulation of deltaTime between grains
    float deltaTimeModSeconds = *deltaTimeMod / 100.0f * (*deltaTime) * 2.0f * (juce::Random::getSystemRandom().nextFloat() - 0.5f);
    float newDeltaTime = *deltaTime + deltaTimeModSeconds;
    newDeltaTime = std::min(newDeltaTime, 0.5f);
    newDeltaTime = std::max(newDeltaTime, 0.001f);
    jassert(newDeltaTime >= 0.001f && newDeltaTime <= 0.5f);
    int deltaTimeSamples = juce::roundToInt(lastSampleRate * newDeltaTime);
    return deltaTimeSamples;
}

void StereoEncoderAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    checkInputAndOutput(this, 2, *orderSetting);

    const int L = buffer.getNumSamples();
    const int totalNumInputChannels = getTotalNumInputChannels() < 2 ? 1 : 2;

    const int ambisonicOrder = *orderSetting < 0.5f ? output.getOrder() : juce::roundToInt(orderSetting->load()) - 1;
    const int nChOut = juce::jmin(buffer.getNumChannels(), juce::square(ambisonicOrder + 1));

    for (int i = 0; i < totalNumInputChannels; ++i)
        bufferCopy.copyFrom(i, 0, buffer.getReadPointer(i), buffer.getNumSamples());
    buffer.clear();

    // SPATIAL PROCESSING
    const float widthInRadiansQuarter{Conversions<float>::degreesToRadians(*width) / 4.0f};
    const iem::Quaternion<float> quatLRot{iem::Quaternion<float>(cos(widthInRadiansQuarter), 0.0f, 0.0f, sin(widthInRadiansQuarter))};
    const iem::Quaternion<float> quatL = quaternionDirection * quatLRot;
    const iem::Quaternion<float> quatR = quaternionDirection * quatLRot.getConjugate();

    const auto left = quatL.getCartesian();
    const auto right = quatR.getCartesian();

    // conversion to spherical for high-quality mode
    float azimuthL, azimuthR, elevationL, elevationR;
    Conversions<float>::cartesianToSpherical(left, azimuthL, elevationL);
    Conversions<float>::cartesianToSpherical(right, azimuthR, elevationR);

    // SH eval and mix between grains and direct encoding of audio
    if (positionHasChanged.compareAndSetBool(false, true))
    {
        smoothAzimuthL.setCurrentAndTargetValue(azimuthL);
        smoothElevationL.setCurrentAndTargetValue(elevationL);
        smoothAzimuthR.setCurrentAndTargetValue(azimuthR);
        smoothElevationR.setCurrentAndTargetValue(elevationR);

        SHEval(ambisonicOrder, left.x, left.y, left.z, SHL);
        SHEval(ambisonicOrder, right.x, right.y, right.z, SHR);

        if (*useSN3D > 0.5f)
        {
            juce::FloatVectorOperations::multiply(SHL, SHL, n3d2sn3d, nChOut);
            juce::FloatVectorOperations::multiply(SHR, SHR, n3d2sn3d, nChOut);
        }
    }

    // DRY PROCESSING
    const float *leftIn = bufferCopy.getReadPointer(0);
    const float *rightIn = bufferCopy.getReadPointer(1);
    float dryAmount = (1 - mixAmount);
    for (int i = 0; i < nChOut; ++i)
    {
        // buffer.copyFromWithRamp(i, 0, leftIn, buffer.getNumSamples(), _SHL[i]* dryAmount, SHL[i] * dryAmount);
        // buffer.addFromWithRamp(i, 0, rightIn, buffer.getNumSamples(), _SHR[i] * dryAmount, SHR[i] * dryAmount);
        buffer.copyFrom(i, 0, leftIn, buffer.getNumSamples(), SHL[i] * dryAmount);
        buffer.addFrom(i, 0, rightIn, buffer.getNumSamples(), SHR[i] * dryAmount);
    }

    // TEMPORAL PROCESSING
    // Fill circular buffer with audio input
    // Try to start grains in loop
    const float *leftInput = bufferCopy.getReadPointer(0);
    const float *rightInput = bufferCopy.getReadPointer(1);

    float gainFactor;
    if (*positionMod > 0.0f)
    {
        gainFactor = juce::jmin(std::sqrt(*deltaTime / *grainLength), 1.0f);
    }
    else
    {
        gainFactor = juce::jmin(*deltaTime / *grainLength, 1.0f);
    }

    switch (_currentWindowType)
    {
    default:
        break;
    case WindowType::hann:
        _currentWindow = &_hannWindowBuffer;
        break;
    case WindowType::rectangular:
        _currentWindow = &_rectWindowBuffer;
        break;
    }

    bool freeze_bool;
    if (*freeze < 0.5f)
        freeze_bool = false;
    else
        freeze_bool = true;

    if (mode == OperationMode::Realtime && freeze_bool)
    {
        mode = OperationMode::ToFreeze;
        writeGainCircBuffer.setTargetValue(0.0f);
    }
    if (mode == OperationMode::Freeze && !freeze_bool)
    {
        mode = OperationMode::ToRealtime;
        writeGainCircBuffer.setTargetValue(1.0f);
    }

    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
        float nextCircGain = writeGainCircBuffer.getNextValue();
        if (mode == OperationMode::ToFreeze && nextCircGain < 0.001f) // Reached Freeze State
        {
            mode = OperationMode::Freeze;
        }
        if (mode == OperationMode::ToRealtime && nextCircGain > 0.999f) // Reached Realtime State
        {
            mode = OperationMode::Realtime;
        }

        if (mode != OperationMode::Freeze)
        {
            circularBuffer.setSample(0, circularBufferWriteHead, leftInput[i] * nextCircGain);
            circularBuffer.setSample(1, circularBufferWriteHead, rightInput[i] * nextCircGain);
        }

        if (grainTimeCounter >= deltaTimeSamples)
        {
            grainTimeCounter = 0;
            // reset (possibly modulated) deltaTime after a grain is started
            deltaTimeSamples = getDeltaTimeSamples();
            // start a grain at this sample time stamp
            for (int g = 0; g < maxNumGrains; g++)
            {
                if (!grains[g].isActive())
                {
                    juce::Vector3D<float> grainDir = getRandomGrainDirection();
                    SHEval(ambisonicOrder, grainDir.x, grainDir.y, grainDir.z, _grainSH[g]);

                    std::array<float, 64> channelWeights;
                    std::copy(_grainSH[g], _grainSH[g] + 64, channelWeights.begin());
                    Grain::GrainJobParameters params;
                    params.startPositionCircBuffer = getStartPositionCircBuffer();
                    auto grainLengthAndPitch = getGrainLengthAndPitchFactor();
                    params.grainLengthSamples = grainLengthAndPitch.first;
                    params.pitchReadFactor = grainLengthAndPitch.second;
                    params.startOffsetInBlock = i;
                    params.channelWeights = channelWeights;
                    params.gainFactor = gainFactor;
                    params.mix = mixAmount;

                    params.windowBuffer = getWindowBuffer();
                    grains[g].startGrain(params);
                    break;
                }
            }
        }
        else
        {
            grainTimeCounter++;
        }

        /*for (int g = 0; g < maxNumGrains; g++)
        {
            grains[g].processSample(buffer, circLeftCh, circRightCh, circularBufferLength, SHL, mixAmount, gainFactor, i);
        }*/

        // increment circular buffer write head
        if (mode != OperationMode::Freeze)
        {
            circularBufferWriteHead++;
            if (circularBufferWriteHead >= circularBufferLength)
            {
                circularBufferWriteHead = 0;
            }
        }
    }

    int numActiveGrains = 0;
    for (int g = 0; g < maxNumGrains; g++)
    {
        if (grains[g].isActive())
            numActiveGrains++;
    }

    for (int g = 0; g < maxNumGrains; g++)
    {
        grains[g].processBlock(buffer, circularBuffer);
    }

    juce::FloatVectorOperations::copy(_SHL, SHL, nChOut);
    juce::FloatVectorOperations::copy(_SHR, SHR, nChOut);
}

//==============================================================================
bool StereoEncoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *StereoEncoderAudioProcessor::createEditor()
{
    return new StereoEncoderAudioProcessorEditor(*this, parameters);
}

void StereoEncoderAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    if (!processorUpdatingParams)
    {
        if (parameterID == "qw" || parameterID == "qx" || parameterID == "qy" || parameterID == "qz")
        {
            sphericalInput = false;
            updateEuler();
            updatedPositionData = true;
            positionHasChanged = true;
        }
        else if (parameterID == "azimuth" || parameterID == "elevation" || parameterID == "roll")
        {
            sphericalInput = true;
            updateQuaternions();
            updatedPositionData = true;
            positionHasChanged = true;
        }
        else if (parameterID == "width")
        {
            updatedPositionData = true;
            positionHasChanged = true;
        }
    }
    if (parameterID == "orderSetting")
    {
        userChangedIOSettings = true;
        positionHasChanged = true;
    }
    else if (parameterID == "useSN3D")
    {
        positionHasChanged = true;
    }
}

//==============================================================================
void StereoEncoderAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    auto state = parameters.copyState();

    auto oscConfig = state.getOrCreateChildWithName("OSCConfig", nullptr);
    oscConfig.copyPropertiesFrom(oscParameterInterface.getConfig(), nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void StereoEncoderAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
            if (parameters.state.hasProperty("OSCPort")) // legacy
            {
                oscParameterInterface.getOSCReceiver().connect(parameters.state.getProperty("OSCPort", juce::var(-1)));
                parameters.state.removeProperty("OSCPort", nullptr);
            }

            auto oscConfig = parameters.state.getChildWithName("OSCConfig");

            if (oscConfig.isValid())
                oscParameterInterface.setConfig(oscConfig);
        }
}

//==============================================================================

const bool StereoEncoderAudioProcessor::processNotYetConsumedOSCMessage(const juce::OSCMessage &message)
{
    juce::String prefix("/" + juce::String(JucePlugin_Name));
    if (!message.getAddressPattern().toString().startsWith(prefix))
        return false;

    juce::OSCMessage msg(message);
    msg.setAddressPattern(message.getAddressPattern().toString().substring(juce::String(JucePlugin_Name).length() + 1));

    if (msg.getAddressPattern().toString().equalsIgnoreCase("/quaternions") && msg.size() == 4)
    {
        float qs[4];
        for (int i = 0; i < 4; ++i)
            if (msg[i].isFloat32())
                qs[i] = msg[i].getFloat32();
            else if (msg[i].isInt32())
                qs[i] = msg[i].getInt32();

        oscParameterInterface.setValue("qw", qs[0]);
        oscParameterInterface.setValue("qx", qs[1]);
        oscParameterInterface.setValue("qy", qs[2]);
        oscParameterInterface.setValue("qz", qs[3]);

        return true;
    }

    return false;
}

//==============================================================================
std::vector<std::unique_ptr<juce::RangedAudioParameter>> StereoEncoderAudioProcessor::createParameterLayout()
{
    // add your audio parameters here
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "orderSetting", "Ambisonics Order", "",
        juce::NormalisableRange<float>(0.0f, 8.0f, 1.0f), 0.0f,
        [](float value)
        {
            if (value >= 0.5f && value < 1.5f)
                return "0th";
            else if (value >= 1.5f && value < 2.5f)
                return "1st";
            else if (value >= 2.5f && value < 3.5f)
                return "2nd";
            else if (value >= 3.5f && value < 4.5f)
                return "3rd";
            else if (value >= 4.5f && value < 5.5f)
                return "4th";
            else if (value >= 5.5f && value < 6.5f)
                return "5th";
            else if (value >= 6.5f && value < 7.5f)
                return "6th";
            else if (value >= 7.5f)
                return "7th";
            else
                return "Auto";
        },
        nullptr));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "useSN3D", "Normalization", "",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f,
        [](float value)
        {
            if (value >= 0.5f)
                return "SN3D";
            else
                return "N3D";
        },
        nullptr));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "qw", "Quaternion W", "",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 1.0,
        [](float value)
        { return juce::String(value, 2); },
        nullptr, true));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "qx", "Quaternion X", "",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0,
        [](float value)
        { return juce::String(value, 2); },
        nullptr, true));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "qy", "Quaternion Y", "",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0,
        [](float value)
        { return juce::String(value, 2); },
        nullptr, true));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "qz", "Quaternion Z", "",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0,
        [](float value)
        { return juce::String(value, 2); },
        nullptr, true));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "azimuth", "Azimuth Angle", juce::CharPointer_UTF8(R"(°)"),
        juce::NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
        [](float value)
        { return juce::String(value, 2); },
        nullptr, true));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "elevation", "Elevation Angle", juce::CharPointer_UTF8(R"(°)"),
        juce::NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
        [](float value)
        { return juce::String(value, 2); },
        nullptr, true));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "shape", "Shape", juce::CharPointer_UTF8(R"()"),
        juce::NormalisableRange<float>(-10.0f, 10.0f, 0.1f), 0.0,
        [](float value)
        { return juce::String(value, 1); },
        nullptr, true));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "size", "Size", juce::CharPointer_UTF8(R"(°)"),
        juce::NormalisableRange<float>(0.0f, 360.0f, 0.01f), 0.0,
        [](float value)
        { return juce::String(value, 2); },
        nullptr));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "roll", "Roll Angle", juce::CharPointer_UTF8(R"(°)"),
        juce::NormalisableRange<float>(-180.0f, 180.0f, 0.01f), 0.0,
        [](float value)
        { return juce::String(value, 2); },
        nullptr, true));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "width", "Stereo Width", juce::CharPointer_UTF8(R"(°)"),
        juce::NormalisableRange<float>(-360.0f, 360.0f, 0.01f), 0.0,
        [](float value)
        { return juce::String(value, 2); },
        nullptr));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "deltaTime", "Delta Time", juce::CharPointer_UTF8(R"(s)"),
        juce::NormalisableRange<float>(0.001f, 0.5f, 0.0001f), 0.05f,
        [](float value)
        { return juce::String(value, 3); },
        nullptr));
    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "deltaTimeMod", "Delta Time Mod", juce::CharPointer_UTF8(R"(%)"),
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f,
        [](float value)
        { return juce::String(value, 1); },
        nullptr));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "grainLength", "Grain Length", juce::CharPointer_UTF8(R"(s)"),
        juce::NormalisableRange<float>(0.001f, 0.500f, 0.0001f), 0.1f,
        [](float value)
        { return juce::String(value, 3); },
        nullptr));
    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "grainLengthMod", "Grain Length Mod", juce::CharPointer_UTF8(R"(%)"),
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f,
        [](float value)
        { return juce::String(value, 1); },
        nullptr));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "position", "Position", juce::CharPointer_UTF8(R"(s)"),
        juce::NormalisableRange<float>(0.0f, CIRC_BUFFER_SECONDS / 2, 0.001f), 0.0f,
        [](float value)
        { return juce::String(value, 3); },
        nullptr));
    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "positionMod", "Position Mod", juce::CharPointer_UTF8(R"(%)"),
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f,
        [](float value)
        { return juce::String(value, 1); },
        nullptr));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "pitch", "Pitch", juce::CharPointer_UTF8(R"(st)"),
        juce::NormalisableRange<float>(-12.0f, 0.0f, 0.1f), 0.0f,
        [](float value)
        { return juce::String(value, 1); },
        nullptr));
    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "pitchMod", "Pitch Mod", juce::CharPointer_UTF8(R"(%)"),
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f,
        [](float value)
        { return juce::String(value, 1); },
        nullptr));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "windowAttack", "Window Attack", juce::CharPointer_UTF8(R"(%)"),
        juce::NormalisableRange<float>(0.0f, 50.0f, 0.1f), 50.0f,
        [](float value)
        { return juce::String(value, 1); },
        nullptr));
    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "windowAttackMod", "Window Attack Mod", juce::CharPointer_UTF8(R"(%)"),
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f,
        [](float value)
        { return juce::String(value, 1); },
        nullptr));
    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "windowDecay", "Window Decay", juce::CharPointer_UTF8(R"(%)"),
        juce::NormalisableRange<float>(0.0f, 50.0f, 0.1f), 50.0f,
        [](float value)
        { return juce::String(value, 1); },
        nullptr));
    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "windowDecayMod", "Window Decay Mod", juce::CharPointer_UTF8(R"(%)"),
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f,
        [](float value)
        { return juce::String(value, 1); },
        nullptr));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "freeze", "Freeze Mode", "",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f,
        [](float value)
        {
            if (value >= 0.5f)
                return "Yes";
            else
                return "No";
        },
        nullptr));

    params.push_back(OSCParameterInterface::createParameterTheOldWay(
        "highQuality", "Sample-wise Panning", "",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f,
        [](float value)
        { return value < 0.5f ? "OFF" : "ON"; },
        nullptr));

    return params;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new StereoEncoderAudioProcessor();
}

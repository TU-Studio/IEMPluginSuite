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
RoomEncoderAudioProcessor::RoomEncoderAudioProcessor()
: AudioProcessorBase (
#ifndef JucePlugin_PreferredChannelConfigurations
                  BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  AudioChannelSet::discreteChannels(64), true)
#endif
                  .withOutput ("Output", AudioChannelSet::discreteChannels(64), true)
#endif
                  ,
#endif
createParameterLayout())
{
    directivityOrderSetting = parameters.getRawParameterValue ("directivityOrderSetting");
    inputIsSN3D = parameters.getRawParameterValue ("inputIsSN3D");
    orderSetting = parameters.getRawParameterValue ("orderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");

    roomX = parameters.getRawParameterValue ("roomX");
    roomY = parameters.getRawParameterValue ("roomY");
    roomZ = parameters.getRawParameterValue ("roomZ");

    sourceX = parameters.getRawParameterValue ("sourceX");
    sourceY = parameters.getRawParameterValue ("sourceY");
    sourceZ = parameters.getRawParameterValue ("sourceZ");
    listenerX = parameters.getRawParameterValue ("listenerX");
    listenerY = parameters.getRawParameterValue ("listenerY");
    listenerZ = parameters.getRawParameterValue ("listenerZ");
    numRefl = parameters.getRawParameterValue ("numRefl");
    reflCoeff = parameters.getRawParameterValue ("reflCoeff");

    syncChannel = parameters.getRawParameterValue ("syncChannel");
    syncRoomSize = parameters.getRawParameterValue ("syncRoomSize");
    syncReflection = parameters.getRawParameterValue ("syncReflection");
    syncListener = parameters.getRawParameterValue ("syncListener");

    renderDirectPath = parameters.getRawParameterValue ("renderDirectPath");
    directPathZeroDelay = parameters.getRawParameterValue ("directPathZeroDelay");
    directPathUnityGain = parameters.getRawParameterValue ("directPathUnityGain");

    lowShelfFreq = parameters.getRawParameterValue ("lowShelfFreq");
    lowShelfGain = parameters.getRawParameterValue ("lowShelfGain");
    highShelfFreq = parameters.getRawParameterValue ("highShelfFreq");
    highShelfGain = parameters.getRawParameterValue ("highShelfGain");

    parameters.addParameterListener ("directivityOrderSetting", this);
    parameters.addParameterListener ("orderSetting", this);
    parameters.addParameterListener ("lowShelfFreq", this);
    parameters.addParameterListener ("lowShelfGain", this);
    parameters.addParameterListener ("highShelfFreq", this);
    parameters.addParameterListener ("highShelfGain", this);
    parameters.addParameterListener ("reflCoeff", this);
    parameters.addParameterListener ("numRefl", this);
    parameters.addParameterListener ("listenerX", this);
    parameters.addParameterListener ("listenerY", this);
    parameters.addParameterListener ("listenerZ", this);
    parameters.addParameterListener ("sourceX", this);
    parameters.addParameterListener ("sourceY", this);
    parameters.addParameterListener ("sourceZ", this);
    parameters.addParameterListener ("roomX", this);
    parameters.addParameterListener ("roomY", this);
    parameters.addParameterListener ("roomZ", this);


    _numRefl = 0;

    sourcePos = Vector3D<float>(*sourceX, *sourceY, *sourceZ);
    listenerPos = Vector3D<float>(*listenerX, *listenerY, *listenerZ);

    for (int i = 0; i<nImgSrc;++i) {
        oldDelay[i] = 44100/343.2f*interpMult; //init oldRadius
        allGains[i] = 0.0f;
        FloatVectorOperations::clear(SHcoeffsOld[i], 64);
        FloatVectorOperations::clear((float *) &SHsampleOld[i], 64);
    }

    lowShelfCoefficients = IIR::Coefficients<float>::makeLowShelf(48000, *lowShelfFreq, 0.707f, Decibels::decibelsToGain(*lowShelfGain));
    highShelfCoefficients = IIR::Coefficients<float>::makeHighShelf(48000, *highShelfFreq, 0.707f, Decibels::decibelsToGain(*highShelfGain));


    lowShelfArray.clear();
    highShelfArray.clear();
    lowShelfArray2.clear();
    highShelfArray2.clear();
    for (int i = 0; i<16; ++i)
    {
        lowShelfArray.add(new IIR::Filter<IIRfloat>(lowShelfCoefficients));
        highShelfArray.add(new IIR::Filter<IIRfloat>(highShelfCoefficients));
        lowShelfArray2.add(new IIR::Filter<IIRfloat>(lowShelfCoefficients));
        highShelfArray2.add(new IIR::Filter<IIRfloat>(highShelfCoefficients));
    }

    startTimer(50);

    oscReceiver.addListener (this);
}

RoomEncoderAudioProcessor::~RoomEncoderAudioProcessor()
{
}

//==============================================================================
int RoomEncoderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int RoomEncoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RoomEncoderAudioProcessor::setCurrentProgram (int index)
{
}

const String RoomEncoderAudioProcessor::getProgramName (int index)
{
    return {};
}

void RoomEncoderAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void RoomEncoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    dist2smpls = sampleRate/343.2f*interpMult; //factor 128 is a small hack for Lagrange lookuptable

    checkInputAndOutput(this, *directivityOrderSetting, *orderSetting, true);

    readOffset = 0;
    bufferReadIdx = 0;


    interleavedData.clear();

    for (int i = 0; i<16; ++i)
    {
        lowShelfArray[i]->reset(IIRfloat(0.0f));
        highShelfArray[i]->reset(IIRfloat(0.0f));
        lowShelfArray2[i]->reset(IIRfloat(0.0f));
        highShelfArray2[i]->reset(IIRfloat(0.0f));

        interleavedData.add(new AudioBlock<IIRfloat> (interleavedBlockData[i], 1, samplesPerBlock));
        interleavedData.getLast()->clear();
    }

    zero = AudioBlock<float> (zeroData, IIRfloat_elements(), samplesPerBlock);
    zero.clear();

    updateFv = true;
}

void RoomEncoderAudioProcessor::releaseResources()
{
}


void RoomEncoderAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "orderSetting" || parameterID == "directivityOrderSetting") userChangedIOSettings = true;
    else if (parameterID == "reflCoeff") {
        updateFv = true;
    }
    else if (parameterID == "lowShelfFreq" || parameterID == "lowShelfGain" ||
        parameterID == "highShelfFreq" || parameterID == "highShelfGain") userChangedFilterSettings = true;
    else if (parameterID.startsWith("source") || parameterID.startsWith("listener"))
    {
        repaintPositionPlanes = true;
    }

    if (*syncChannel >= 0.5f && !readingSharedParams)
    {
        int ch = (int) *syncChannel;
        RoomParams& roomParam = sharedParams.get().rooms.getReference(ch);

        bool sRoom(*syncRoomSize>=0.5f);
        bool sListener(*syncListener>=0.5f);
        bool sReflections(*syncReflection>=0.5f);

        if (sRoom && (parameterID == "roomX" || parameterID == "roomY" || parameterID == "roomZ"))
        {
            roomParam.roomX = *roomX;
            roomParam.roomY = *roomY;
            roomParam.roomZ = *roomZ;
            roomParam.validRoomData = true;
        }
        else if (sListener && (parameterID == "listenerX" || parameterID == "listenerY" || parameterID == "listenerZ"))
        {
            roomParam.listenerX = *listenerX;
            roomParam.listenerY = *listenerY;
            roomParam.listenerZ = *listenerZ;
            roomParam.validListenerData = true;
        }
        else if (sReflections && (parameterID == "reflCoeff" || parameterID == "numRefl" ||
                                  parameterID == "lowShelfFreq" || parameterID == "lowShelfGain" ||
                                  parameterID == "highShelfFreq" || parameterID == "highShelfGain"))
        {
            roomParam.reflCoeff = *reflCoeff;
            roomParam.numRefl = *numRefl;
            roomParam.lowShelfFreq = *lowShelfFreq;
            roomParam.lowShelfGain = *lowShelfGain;
            roomParam.highShelfFreq = *highShelfFreq;
            roomParam.highShelfGain = *highShelfGain;

            roomParam.validReflectionData = true;
        }

    }
}

void RoomEncoderAudioProcessor::updateFilterCoefficients(double sampleRate) {
    *lowShelfCoefficients = *IIR::Coefficients<float>::makeLowShelf(sampleRate, *lowShelfFreq, 0.707f, Decibels::decibelsToGain(*lowShelfGain));
    *highShelfCoefficients = *IIR::Coefficients<float>::makeHighShelf(sampleRate, *highShelfFreq, 0.707f, Decibels::decibelsToGain(*highShelfGain));
    userChangedFilterSettings = false;

    updateFv = true;
}

void RoomEncoderAudioProcessor::calculateImageSourcePositions()
{
    const float t = *roomX;
    const float b = *roomY;
    const float h = *roomZ;

    for (int q = 0; q < nImgSrc; ++q)
    {
        int m = reflList[q][0];
        int n = reflList[q][1];
        int o = reflList[q][2];
        mx[q] = m * t + mSig[m&1] * sourcePos.x - listenerPos.x;
        my[q] = n * b + mSig[n&1] * sourcePos.y - listenerPos.y;
        mz[q] = o * h + mSig[o&1] * sourcePos.z - listenerPos.z;

        mRadius[q] = sqrt (mx[q] * mx[q] + my[q] * my[q]+ mz[q] * mz[q]);
        mx[q] /= mRadius[q];
        my[q] /= mRadius[q];
        mz[q] /= mRadius[q];

        smx[q] = -mSig[m & 1] * mx[q];
        smy[q] = -mSig[n & 1] * my[q];
        smz[q] = -mSig[o & 1] * mz[q];
    }
}

void RoomEncoderAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    checkInputAndOutput(this, *directivityOrderSetting, *orderSetting);

    // =============================== settings and parameters
    const int maxNChIn = jmin(buffer.getNumChannels(), input.getNumberOfChannels());
    const int maxNChOut = jmin(buffer.getNumChannels(), output.getNumberOfChannels());
    const int directivityOrder = input.getOrder();
    const int ambisonicOrder = output.getOrder();

    const int sampleRate = getSampleRate();
    const int L = buffer.getNumSamples();
    const float oneOverL = 1.0/((double) L);

    const bool doInputSn3dToN3dConversion = *inputIsSN3D > 0.5f;

    float* pBufferWrite = buffer.getWritePointer(0);
    const float* pBufferRead = buffer.getReadPointer(0);

    const int nSIMDFilters = 1 + (maxNChIn-1)/IIRfloat_elements();

    // update iir filter coefficients
    if (userChangedFilterSettings) updateFilterCoefficients(sampleRate);


    //interleave input data
    int partial = maxNChIn%IIRfloat_elements();
    if (partial == 0)
    {
        for (int i = 0; i<nSIMDFilters; ++i)
        {
            AudioDataConverters::interleaveSamples(buffer.getArrayOfReadPointers() + i*IIRfloat_elements(),
                                                   reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)), L,
                                                   static_cast<int> (IIRfloat_elements()));
        }
    }
    else
    {
        int i;
        for (i = 0; i<nSIMDFilters-1; ++i)
        {
            AudioDataConverters::interleaveSamples(buffer.getArrayOfReadPointers() + i*IIRfloat_elements(),
                                                   reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)), L,
                                                   static_cast<int> (IIRfloat_elements()));
        }

        const float* addr[IIRfloat_elements()];
        size_t ch;
        for (ch = 0; ch < partial; ++ch)
        {
            addr[ch] = buffer.getReadPointer(i * static_cast<int> (IIRfloat_elements() + ch));
        }
        for (; ch < IIRfloat_elements(); ++ch)
        {
            addr[ch] = zero.getChannelPointer(ch);
        }
        AudioDataConverters::interleaveSamples(addr,
                                               reinterpret_cast<float*> (interleavedData[i]->getChannelPointer (0)), L,
                                               static_cast<int> (IIRfloat_elements()));
    }

    int currNumRefl = roundToInt(*numRefl);
    int workingNumRefl = (currNumRefl < _numRefl) ? _numRefl : currNumRefl;


    // calculating reflection coefficients (only if parameter changed)
    float reflCoeffGain = Decibels::decibelsToGain(*reflCoeff);
    if (powReflCoeff[1] != reflCoeffGain)
    {
        powReflCoeff[0] = 1;
        powReflCoeff[1] = reflCoeffGain;
        for (int i = 2; i<=maxOrderImgSrc; ++i )
        {
            powReflCoeff[i] = powReflCoeff[i-1] * reflCoeffGain;
        }
    }

    //===== LIMIT MOVING SPEED OF SOURCE AND LISTENER ===============================
    const float maxDist = 30.0f / sampleRate * L; // 30 meters per second
    {
        const Vector3D<float> targetSourcePos (jlimit (-*roomX / 2, *roomX / 2, *sourceX),
                                               jlimit (-*roomY / 2, *roomY / 2, *sourceY),
                                               jlimit (-*roomZ / 2, *roomZ / 2, *sourceZ));
        const auto sourcePosDiff = targetSourcePos - sourcePos;
        const float sourcePosDiffLength = sourcePosDiff.length();

        if (sourcePosDiffLength > maxDist)
            sourcePos += sourcePosDiff * maxDist / sourcePosDiffLength;
        else
            sourcePos = targetSourcePos;

        const Vector3D<float> listenerSourcePos (jlimit (-*roomX / 2, *roomX / 2, *listenerX),
                                                 jlimit (-*roomY / 2, *roomY / 2, *listenerY),
                                                 jlimit (-*roomZ / 2, *roomZ / 2, *listenerZ));
        const auto listenerPosDiff = listenerSourcePos - listenerPos;
        const float listenerPosDiffLength = listenerPosDiff.length();

        if (listenerPosDiffLength > maxDist)
            listenerPos += listenerPosDiff * maxDist / listenerPosDiffLength;
        else
            listenerPos = listenerSourcePos;
    }


    const bool doRenderDirectPath = *renderDirectPath > 0.5f;
    if (doRenderDirectPath)
    {
        // prevent division by zero when source is as listener's position
        if ((listenerPos - sourcePos).lengthSquared() < 0.01)
            sourcePos = listenerPos + Vector3D<float> (0.1f, 0.0f, 0.0f);
    }

    float* pMonoBufferWrite = monoBuffer.getWritePointer(0);

    calculateImageSourcePositions();


    for (int q=0; q<workingNumRefl+1; ++q) {
        if (q == 1) {
            for (int i = 0; i<nSIMDFilters; ++i)
            {
                ProcessContextReplacing<IIRfloat> context (*interleavedData[i]);
                lowShelfArray[i]->process(context);
                highShelfArray[i]->process(context);
            }
        }
        if (q == 7) {
            for (int i = 0; i<nSIMDFilters; ++i)
            {
                lowShelfArray2[i]->process(ProcessContextReplacing<IIRfloat> (*interleavedData[i]));
                highShelfArray2[i]->process(ProcessContextReplacing<IIRfloat> (*interleavedData[i]));
            }
        }


        // ========================================   CALCULATE SAMPLED MONO SIGNALS
        /* JMZ:
         * the following section is broken, as it hardcodes asumptions about how
         * many floats can be stored in IIRfloat
         */
        IIRfloat SHsample[16]; //TODO: can be smaller: (N+1)^2/IIRfloat_elements()
        IIRfloat SHsampleStep[16];
#if JUCE_USE_SIMD
        FloatVectorOperations::clear((float *) &SHsample->value,
                                     IIRfloat_elements() * sizeof(SHsample) / sizeof(*SHsample));
        SHEval(directivityOrder, smx[q], smy[q], smz[q],(float *) &SHsample->value, false); // deoding -> false
#else  /* !JUCE_USE_SIMD */
        FloatVectorOperations::clear((float *) SHsample,
                                     IIRfloat_elements() * sizeof(SHsample) / sizeof(*SHsample));
        SHEval(directivityOrder, smx[q], smy[q], smz[q],(float *) SHsample, false); // deoding -> false
#endif /* JUCE_USE_SIMD */

        if (doInputSn3dToN3dConversion)
            FloatVectorOperations::multiply((float *) SHsample, sn3d2n3d, maxNChIn);

        Array<IIRfloat*> interleavedDataPtr;
        interleavedDataPtr.resize(nSIMDFilters);
        IIRfloat** intrlvdDataArrayPtr = interleavedDataPtr.getRawDataPointer();

        for (int i = 0; i<nSIMDFilters; ++i)
        {
            intrlvdDataArrayPtr[i] = reinterpret_cast<IIRfloat*> (interleavedData[i]->getChannelPointer (0));
            SHsampleStep[i] = SHsample[i]-SHsampleOld[q][i];
            SHsampleStep[i] *= oneOverL;
            SHsample[i] = SHsampleOld[q][i];
        }

        for (int smpl = 0; smpl < L; ++smpl)
        {
            IIRfloat SIMDTemp;
            SIMDTemp = 0.0f;

            for (int i = 0; i<nSIMDFilters; ++i)
            {
                SIMDTemp += SHsample[i] * *(intrlvdDataArrayPtr[i]++);
                SHsample[i] += SHsampleStep[i];
            }
#if JUCE_USE_SIMD
            pBufferWrite[smpl] = SIMDTemp.sum();
#else /* !JUCE_USE_SIMD */
            pBufferWrite[smpl] = SIMDTemp;
#endif /* JUCE_USE_SIMD */
        }

        // ============================================
        double delayOffset = *directPathZeroDelay > 0.5f ? mRadius[0] * dist2smpls : 0.0;
        double delay, delayStep;
        int firstIdx, copyL;
        delay = mRadius[q]*dist2smpls - delayOffset; // dist2smpls also contains factor 128 for LUT
        delayStep = (delay - oldDelay[q])*oneOverL;

        //calculate firstIdx and copyL
        int startIdx = ((int)oldDelay[q])>>interpShift;
        int stopIdx = L-1 + (((int)(oldDelay[q] + delayStep * L-1))>>interpShift); // ((int)(startIdx + delayStep * L-1))>>7
        firstIdx = jmin (startIdx, stopIdx);
        copyL = abs(stopIdx-startIdx) + interpLength;

        monoBuffer.clear(0,firstIdx, copyL); //TODO: optimization idea: resample input to match delay stretching

        float* tempWritePtr = pMonoBufferWrite; //reset writePtr as it gets increased during the next for loop
        const float* readPtr = pBufferRead;

        double tempDelay = oldDelay[q]; //start from oldDelay and add delayStep after each interation;

        //int interpCoeffIdx;
        for (int smplIdx = 0; smplIdx < L; ++smplIdx) {
            //int delayInt; = truncatePositiveToUnsignedInt(tempDelay); //(int)tempDelay;
            float integer;
            float fraction = modff(tempDelay, &integer);
            int delayInt = (int) integer;

            int interpCoeffIdx = delayInt&interpMask;
            delayInt = delayInt>>interpShift;
            int idx = delayInt-interpOffset;


            float* dest = tempWritePtr++ + idx;

#if JUCE_USE_SSE_INTRINSICS
            __m128 destSamples = _mm_loadu_ps(dest);
            __m128 srcSample = _mm_set1_ps(*readPtr++);
            __m128 interp = getInterpolatedLagrangeWeights(interpCoeffIdx, fraction);

            destSamples = _mm_add_ps(destSamples, _mm_mul_ps(interp, srcSample));
            _mm_storeu_ps(dest, destSamples);
#else /* !JUCE_USE_SSE_INTRINSICS */
            float interp[4];
            getInterpolatedLagrangeWeights(interpCoeffIdx, fraction, interp);
            float src = *readPtr++;
            dest[0] += interp[0] * src;
            dest[1] += interp[1] * src;
            dest[2] += interp[2] * src;
            dest[3] += interp[3] * src;
#endif /* JUCE_USE_SSE_INTRINSICS */
            tempDelay += delayStep;
        }

        const float* monoBufferReadPtrWithOffset = monoBuffer.getReadPointer(0) + firstIdx;
        firstIdx = firstIdx + readOffset;
        if (firstIdx >= bufferSize)
            firstIdx -= bufferSize;

        float SHcoeffs[64];
        float SHcoeffsStep[64];

        if (q<=currNumRefl)
        {
            SHEval(ambisonicOrder, mx[q], my[q], mz[q], SHcoeffs, true); // encoding -> true
            if (*useSN3D > 0.5f)
            {
                FloatVectorOperations::multiply(SHcoeffs, SHcoeffs, n3d2sn3d, maxNChOut);
            }
        }
        else
            FloatVectorOperations::clear(SHcoeffs, 64);

        float gain = powReflCoeff[reflList[q][3]] / mRadius[q];
        if (*directPathUnityGain > 0.5f)
            gain *= mRadius[0];

        // direct path rendering
        if (q == 0 && ! doRenderDirectPath)
            gain = 0.0f;

        allGains[q] = gain; // for reflectionVisualizer

        FloatVectorOperations::multiply(SHcoeffs, gain, maxNChOut);
        //FloatVectorOperations::multiply(SHcoeffs, mSig[reflList[q][3]&1]*gain, maxNChOut);
        FloatVectorOperations::subtract(SHcoeffsStep, SHcoeffs, SHcoeffsOld[q], maxNChOut);
        FloatVectorOperations::multiply(SHcoeffsStep, 1.0f/copyL, maxNChOut);

        if (firstIdx + copyL - 1 >= bufferSize)
        {
            int firstNumCopy = bufferSize - firstIdx;
            int secondNumCopy = copyL-firstNumCopy;

            for (int channel = 0; channel < maxNChOut; ++channel)
            {
                if (SHcoeffsOld[q][channel] != SHcoeffs[channel])
                {
#if defined(JUCE_USE_VDSP_FRAMEWORK) && defined(JUCE_MAC)
                    vDSP_vrampmuladd(monoBufferReadPtrWithOffset, 1, //input vector with stride
                                     &SHcoeffsOld[q][channel], //ramp start value (gets increased)
                                     &SHcoeffsStep[channel], //step value
                                     delayBufferWritePtrArray[channel] + firstIdx, 1,// output with stride
                                     (size_t) firstNumCopy //num
                                     );
                    vDSP_vrampmuladd(monoBufferReadPtrWithOffset+firstNumCopy, 1, //input vector with stride
                                     &SHcoeffsOld[q][channel], //ramp start value (gets increased)
                                     &SHcoeffsStep[channel], //step value
                                     delayBufferWritePtrArray[channel], 1,// output with stride
                                     (size_t) secondNumCopy //num
                                     );
#else
                    delayBuffer.addFromWithRamp(channel, firstIdx, monoBufferReadPtrWithOffset, firstNumCopy,
                                                SHcoeffsOld[q][channel], SHcoeffsOld[q][channel] + SHcoeffsStep[channel]*firstNumCopy);
                    delayBuffer.addFromWithRamp(channel, 0,        monoBufferReadPtrWithOffset+firstNumCopy, secondNumCopy,
                                                SHcoeffsOld[q][channel] + SHcoeffsStep[channel]*firstNumCopy, SHcoeffs[channel]);
#endif
                }
                else
                {
                    FloatVectorOperations::addWithMultiply(delayBufferWritePtrArray[channel] + firstIdx,
                                                           monoBufferReadPtrWithOffset,
                                                           SHcoeffs[channel], firstNumCopy);
                    FloatVectorOperations::addWithMultiply(delayBufferWritePtrArray[channel],
                                                           monoBufferReadPtrWithOffset + firstNumCopy,
                                                           SHcoeffs[channel], secondNumCopy);
                }
            }
        }
        else
        {
            for (int channel = 0; channel < maxNChOut; ++channel)
            {
                if (SHcoeffsOld[q][channel] != SHcoeffs[channel])
                {
#if defined(JUCE_USE_VDSP_FRAMEWORK) && defined(JUCE_MAC)
                    vDSP_vrampmuladd(monoBufferReadPtrWithOffset, 1, //input vector with stride
                                     &SHcoeffsOld[q][channel], //ramp start value (gets increased)
                                     &SHcoeffsStep[channel], //step value
                                     delayBufferWritePtrArray[channel] + firstIdx, 1,// output with stride
                                     (size_t) copyL //num
                                     );
#else
                    delayBuffer.addFromWithRamp(channel, firstIdx, monoBufferReadPtrWithOffset, copyL, SHcoeffsOld[q][channel], SHcoeffs[channel]);
#endif
                }
                else
                {
                    FloatVectorOperations::addWithMultiply(delayBufferWritePtrArray[channel] + firstIdx,
                                                           monoBufferReadPtrWithOffset,
                                                           SHcoeffs[channel], copyL);
                }

            }
        }

        FloatVectorOperations::copy(SHcoeffsOld[q], SHcoeffs, maxNChOut);
#if JUCE_USE_SIMD
        FloatVectorOperations::copy((float *) &SHsampleOld[q]->value, (float *) &SHsample->value, maxNChIn);
#else  /* !JUCE_USE_SIMD */
        FloatVectorOperations::copy((float *) SHsampleOld[q], (float *) SHsample, maxNChIn);
#endif /* JUCE_USE_SIMD */
        //oldDelay[q] = delay;
        oldDelay[q] = tempDelay;
    }

    //updating the remaining oldDelay values
    for (int q = workingNumRefl + 1; q < nImgSrc; ++q)
        oldDelay[q] = mRadius[q]*dist2smpls;

    // ======= Read from delayBuffer and clear read content ==============
    buffer.clear();

    const int blockSize1 = jmin (bufferSize - readOffset, L);

    for (int channel = 0; channel < maxNChOut; ++channel)
    {
        buffer.copyFrom(channel, 0, delayBufferWritePtrArray[channel] + readOffset, blockSize1);
        delayBuffer.clear(channel, readOffset, blockSize1);
    }

    const int numLeft = L - blockSize1;
    if (numLeft > 0)
    {
        for (int channel = 0; channel < maxNChOut; ++channel)
        {
            buffer.copyFrom(channel, 0, delayBufferWritePtrArray[channel], numLeft);
            delayBuffer.clear(channel, 0, numLeft);
        }
    }

    _numRefl = currNumRefl;

    readOffset += L;
    if (readOffset >= bufferSize) readOffset -= bufferSize;

}

//==============================================================================
bool RoomEncoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* RoomEncoderAudioProcessor::createEditor()
{
    return new RoomEncoderAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void RoomEncoderAudioProcessor::getStateInformation (MemoryBlock &destData)
{
    auto state = parameters.copyState();
    state.setProperty ("OSCPort", var(oscReceiver.getPortNumber()), nullptr);
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void RoomEncoderAudioProcessor::setStateInformation (const void *data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (ValueTree::fromXml (*xmlState));
            if (parameters.state.hasProperty ("OSCPort"))
            {
                oscReceiver.connect (parameters.state.getProperty ("OSCPort", var (-1)));
            }
        }
}

void RoomEncoderAudioProcessor::timerCallback()
{
    if (*syncChannel > 0.5f)
    {
        int ch = (int) *syncChannel;
        bool sRoom(*syncRoomSize>=0.5f);
        bool sListener(*syncListener>=0.5f);
        bool sReflections(*syncReflection>=0.5f);

        RoomParams& roomParam = sharedParams.get().rooms.getReference(ch);


        if (sRoom)
        {
            if (roomParam.validRoomData) {
                readingSharedParams = true;
                parameters.getParameter ("roomX")->setValueNotifyingHost (parameters.getParameterRange ("roomX").convertTo0to1 (roomParam.roomX));
                parameters.getParameter ("roomY")->setValueNotifyingHost (parameters.getParameterRange ("roomY").convertTo0to1 (roomParam.roomY));
                parameters.getParameter ("roomZ")->setValueNotifyingHost (parameters.getParameterRange ("roomZ").convertTo0to1 (roomParam.roomZ));
                readingSharedParams = false;
            }
            else
            {
                roomParam.roomX = *roomX;
                roomParam.roomY = *roomY;
                roomParam.roomZ = *roomZ;
                roomParam.validRoomData = true;
            }
        }
        if (sListener)
        {
            if (roomParam.validListenerData) {
                readingSharedParams = true;
                parameters.getParameter ("listenerX")->setValueNotifyingHost (parameters.getParameterRange ("listenerX").convertTo0to1 (roomParam.listenerX));
                parameters.getParameter ("listenerY")->setValueNotifyingHost (parameters.getParameterRange ("listenerY").convertTo0to1 (roomParam.listenerY));
                parameters.getParameter ("listenerZ")->setValueNotifyingHost (parameters.getParameterRange ("listenerZ").convertTo0to1 (roomParam.listenerZ));
                readingSharedParams = false;
            }
            else
            {
                roomParam.listenerX = *listenerX;
                roomParam.listenerY = *listenerY;
                roomParam.listenerZ = *listenerZ;
                roomParam.validListenerData = true;
            }
        }
        if (sReflections)
        {
            if (roomParam.validReflectionData) {
                readingSharedParams = true;
                parameters.getParameter ("reflCoeff")->setValueNotifyingHost (parameters.getParameterRange ("reflCoeff").convertTo0to1 (roomParam.reflCoeff));
                parameters.getParameter ("numRefl")->setValueNotifyingHost (parameters.getParameterRange ("numRefl").convertTo0to1 (roomParam.numRefl));
                parameters.getParameter ("lowShelfFreq")->setValueNotifyingHost (parameters.getParameterRange ("lowShelfFreq").convertTo0to1 (roomParam.lowShelfFreq));
                parameters.getParameter ("lowShelfGain")->setValueNotifyingHost (parameters.getParameterRange ("lowShelfGain").convertTo0to1 (roomParam.lowShelfGain));
                parameters.getParameter ("highShelfFreq")->setValueNotifyingHost (parameters.getParameterRange ("highShelfFreq").convertTo0to1 (roomParam.highShelfFreq));
                parameters.getParameter ("highShelfGain")->setValueNotifyingHost (parameters.getParameterRange ("highShelfGain").convertTo0to1 (roomParam.highShelfGain));
                readingSharedParams = false;
            }
            else
            {
                roomParam.reflCoeff = *reflCoeff;
                roomParam.numRefl = *numRefl;
                roomParam.lowShelfFreq = *lowShelfFreq;
                roomParam.lowShelfGain = *lowShelfGain;
                roomParam.highShelfFreq = *highShelfFreq;
                roomParam.highShelfGain = *highShelfGain;
                roomParam.validReflectionData = true;
            }
        }
    }
}


void RoomEncoderAudioProcessor::updateBuffers()
{
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());

    const int nChOut = output.getNumberOfChannels();
    const int samplesPerBlock = getBlockSize();

    bufferSize = round(180.0/343.2* getSampleRate()) + samplesPerBlock + 100;
    bufferSize += samplesPerBlock - bufferSize%samplesPerBlock;

    monoBuffer.setSize(1, bufferSize);
    monoBuffer.clear();

    delayBuffer.setSize(nChOut, bufferSize);
    delayBuffer.clear();
    delayBufferWritePtrArray = delayBuffer.getArrayOfWritePointers();

    if (input.getSize() != input.getPreviousSize())
    {
        for (int i = 0; i<interleavedData.size(); ++i)
        {
            interleavedData[i]->clear();
        }
    }
}

//==============================================================================
std::vector<std::unique_ptr<RangedAudioParameter>> RoomEncoderAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("directivityOrderSetting", "Input Directivity Order", "",
                                     NormalisableRange<float> (0.0f, 8.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f && value < 1.5f) return "0th";
                                         else if (value >= 1.5f && value < 2.5f) return "1st";
                                         else if (value >= 2.5f && value < 3.5f) return "2nd";
                                         else if (value >= 3.5f && value < 4.5f) return "3rd";
                                         else if (value >= 4.5f && value < 5.5f) return "4th";
                                         else if (value >= 5.5f && value < 6.5f) return "5th";
                                         else if (value >= 6.5f && value < 7.5f) return "6th";
                                         else if (value >= 7.5f) return "7th";
                                         else return "Auto"; },
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("inputIsSN3D", "Input Directivity Normalization", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) { if (value >= 0.5f ) return "SN3D";
                                         else return "N3D"; },
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("orderSetting", "Output Ambisonics Order", "",
                                     NormalisableRange<float> (0.0f, 8.0f, 1.0f), 0.0f,
                                     [](float value) {
                                         if (value >= 0.5f && value < 1.5f) return "0th";
                                         else if (value >= 1.5f && value < 2.5f) return "1st";
                                         else if (value >= 2.5f && value < 3.5f) return "2nd";
                                         else if (value >= 3.5f && value < 4.5f) return "3rd";
                                         else if (value >= 4.5f && value < 5.5f) return "4th";
                                         else if (value >= 5.5f && value < 6.5f) return "5th";
                                         else if (value >= 6.5f && value < 7.5f) return "6th";
                                         else if (value >= 7.5f) return "7th";
                                         else return "Auto"; },
                                     nullptr));
    params.push_back (OSCParameterInterface::createParameterTheOldWay ("useSN3D", "Normalization", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) { if (value >= 0.5f ) return "SN3D";
                                         else return "N3D"; },
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("roomX", "room size x", "m",
                                     NormalisableRange<float> (1.0f, 30.0f, 0.01f), 10.0f,
                                     [](float value) { return String(value, 2); }, nullptr));
    params.push_back (OSCParameterInterface::createParameterTheOldWay ("roomY", "room size y", "m",
                                     NormalisableRange<float> (1.0f, 30.0f, 0.01f), 11.0f,
                                     [](float value) { return String(value, 2); }, nullptr));
    params.push_back (OSCParameterInterface::createParameterTheOldWay ("roomZ", "room size z", "m",
                                     NormalisableRange<float> (1.0f, 20.0f, 0.01f), 7.0f,
                                     [](float value) { return String(value, 2); }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("sourceX", "source position x", "m",
                                     NormalisableRange<float> (-15.0f, 15.0f, 0.001f), 1.0f,
                                     [](float value) { return String(value, 3); }, nullptr));
    params.push_back (OSCParameterInterface::createParameterTheOldWay ("sourceY", "source position y", "m",
                                     NormalisableRange<float> (-15.0f, 15.0f, 0.001f), 1.0f,
                                     [](float value) { return String(value, 3); }, nullptr));
    params.push_back (OSCParameterInterface::createParameterTheOldWay ("sourceZ", "source position z", "m",
                                     NormalisableRange<float> (-10.0f, 10.0f, 0.001f), -1.0f,
                                     [](float value) { return String(value, 3); }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("listenerX", "listener position x", "m",
                                     NormalisableRange<float> (-15.0f, 15.0f, 0.001f), -1.0f,
                                     [](float value) { return String(value, 3); }, nullptr));
    params.push_back (OSCParameterInterface::createParameterTheOldWay ("listenerY", "listener position y", "m",
                                     NormalisableRange<float> (-15.0f, 15.0f, 0.001f), -1.0f,
                                     [](float value) { return String(value, 3); }, nullptr));
    params.push_back (OSCParameterInterface::createParameterTheOldWay ("listenerZ", "listener position z", "m",
                                     NormalisableRange<float> (-10.0f, 10.0f, 0.001f), -1.0f,
                                     [](float value) { return String(value, 3); }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("numRefl", "number of reflections", "",
                                     NormalisableRange<float> (0.0f, nImgSrc-1, 1.0f), 33.0f,
                                     [](float value) { return String((int) value); }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("lowShelfFreq", "LowShelf Frequency", "Hz",
                                     NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.2f), 100.0,
                                     [](float value) { return String((int) value); }, nullptr));
    params.push_back (OSCParameterInterface::createParameterTheOldWay ("lowShelfGain", "LowShelf Gain", "dB",
                                     NormalisableRange<float> (-15.0f, 5.0f, 0.1f), -5.0f,
                                     [](float value) { return String(value, 1); }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("highShelfFreq", "HighShelf Frequency", "Hz",
                                     NormalisableRange<float> (20., 20000.0f, 1.0f, 0.2f), 8000.0,
                                     [](float value) { return String((int) value); }, nullptr));
    params.push_back (OSCParameterInterface::createParameterTheOldWay ("highShelfGain", "HighShelf Gain", "dB",
                                     NormalisableRange<float> (-15.0f, 5.0f, 0.1f), -5.0f,
                                     [](float value) { return String(value, 1); }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("reflCoeff", "Reflection Coefficient", "dB",
                                     NormalisableRange<float> (-15.0f, 0.0f, 0.01f), -1.0f,
                                     [](float value) { return String(value, 2); }, nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("syncChannel", "Synchronize to Channel", "",
                                     NormalisableRange<float> (0.0f, 4.0f, 1.0f), 0.0f,
                                     [](float value) {
                                         if (value >= 0.5f && value < 1.5f) return "Channel 1";
                                         else if (value >= 1.5f && value < 2.5f) return "Channel 2";
                                         else if (value >= 2.5f && value < 3.5f) return "Channel 3";
                                         else if (value >= 3.5f) return "Channel 4";
                                         else return "None"; },
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("syncRoomSize", "Synchronize Room Dimensions", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "YES";
                                         else return "NO"; },
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("syncReflection", "Synchronize Reflection Properties", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "YES";
                                         else return "NO"; },
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("syncListener", "Synchronize Listener Position", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "YES";
                                         else return "NO"; },
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("renderDirectPath", "Render Direct Path", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "YES";
                                         else return "NO"; },
                                     nullptr));

    params.push_back (OSCParameterInterface::createParameterTheOldWay ("directPathZeroDelay", "Zero-Delay for Direct Path", "",
                                                                       NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
                                                                       [](float value) {
                                                                           if (value >= 0.5f) return "ON";
                                                                           else return "OFF"; },
                                                                       nullptr));


    params.push_back (OSCParameterInterface::createParameterTheOldWay ("directPathUnityGain", "Unity-Gain for Direct Path", "",
                                                                       NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
                                                                       [](float value) {
                                                                           if (value >= 0.5f) return "ON";
                                                                           else return "OFF"; },
                                                                       nullptr));

    return params;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RoomEncoderAudioProcessor();
}

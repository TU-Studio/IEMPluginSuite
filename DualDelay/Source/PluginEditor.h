/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 http://www.iem.at
 
 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/customComponents/DoubleSlider.h"
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/lookAndFeel/IEM_LaF.h"

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

//==============================================================================
/**
*/
class DualDelayAudioProcessorEditor  : public AudioProcessorEditor,
private Timer
{
public:
    DualDelayAudioProcessorEditor (DualDelayAudioProcessor&, AudioProcessorValueTreeState&);
    ~DualDelayAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    void timerCallback() override;
    
    LaF globalLaF;
    TitleBar<AmbisonicIOWidget, NoIOWidget> title;
    Footer footer;
    
    ScopedPointer<ComboBoxAttachment> cbNormalizationAtachement;
    ScopedPointer<ComboBoxAttachment> cbOrderAtachement;
    int maxPossibleOrder;
    
    Slider SlDryGain;
    ScopedPointer<SliderAttachment> SlDryGainAttachment;
    
    // elements for left side
    DoubleSlider dblSlLeftFilter;
    ReverseSlider SlLeftRot;
    Slider SlLeftDelay, SlLeftLfoRate, SlLeftLfoDepth, SlLeftFb, SlLeftCrossFb, SlLeftGain;
    
    ScopedPointer<SliderAttachment> dblSlLeftFilterHpAttachment,dblSlLeftFilterLpAttachment;
    ScopedPointer<SliderAttachment> SlLeftRotAttachment;
    ScopedPointer<SliderAttachment> SlLeftDelayAttachment, SlLeftLfoRateAttachment, SlLeftLfoDepthAttachment, SlLeftFbAttachment, SlLeftCrossFbAttachment, SlLeftGainAttachment;
    
    // elements for right side
    DoubleSlider dblSlRightFilter;
    ReverseSlider SlRightRot;
    Slider SlRightDelay, SlRightLfoRate, SlRightLfoDepth, SlRightFb, SlRightCrossFb, SlRightGain;
    
    ScopedPointer<SliderAttachment> dblSlRightFilterHpAttachment,dblSlRightFilterLpAttachment;
    ScopedPointer<SliderAttachment> SlRightRotAttachment;
    ScopedPointer<SliderAttachment> SlRightDelayAttachment, SlRightLfoRateAttachment, SlRightLfoDepthAttachment, SlRightFbAttachment, SlRightCrossFbAttachment, SlRightGainAttachment;
    
    // lables and groups
    SimpleLabel lbRotL, lbDelL, lbFbL, lbXFbL;
    SimpleLabel lbRotR, lbDelR, lbFbR, lbXFbR;
    SimpleLabel lbGainL, lbGainR, lbGainDry;
    TripleLabel lbLfoL, lbLfoR, lbFilterL, lbFilterR;
    
    GroupComponent gcRotDelL, gcRotDelR, gcFiltL, gcFiltR, gcFbL, gcFbR, gcOutput;
    
    
    
    DualDelayAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DualDelayAudioProcessorEditor)
};

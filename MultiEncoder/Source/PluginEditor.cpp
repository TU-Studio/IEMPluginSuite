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

#include "PluginProcessor.h"
#include "PluginEditor.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//==============================================================================
MultiEncoderAudioProcessorEditor::MultiEncoderAudioProcessorEditor (MultiEncoderAudioProcessor& p, AudioProcessorValueTreeState& vts)
: AudioProcessorEditor (&p), processor (p), valueTreeState(vts), encoderList(p, sphere, &vts)//, sphere_opengl(nullptr)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600,450);
    setLookAndFeel (&globalLaF);
    
    // ==== SPHERE AND ELEMENTS ===============
    addAndMakeVisible(&sphere);
    //sphere.addListener(this);
    
    
    sphere.addElement(&masterElement);
    masterElement.setColour(Colours::black);
    masterElement.setTextColour(Colours::white);
    masterElement.setLabel("M");
    masterElement.setID("master");
    masterElement.setSliders(&slMasterYaw, &slMasterPitch);
    
    // ======================================
    
    addAndMakeVisible(&title);
    title.setTitle(String("Multi"),String("Encoder"));
    title.setFont(globalLaF.robotoBold,globalLaF.robotoLight);
    
    addAndMakeVisible(&footer);
    
    toolTipWin.setMillisecondsBeforeTipAppears(500);
    
    addAndMakeVisible(&viewport);
    viewport.setViewedComponent(&encoderList);

    cbNumInputChannelsAttachment = new ComboBoxAttachment(valueTreeState,"inputSetting",*title.getInputWidgetPtr()->getChannelsCbPointer());
    cbNormalizationAtachment = new ComboBoxAttachment(valueTreeState,"useSN3D",*title.getOutputWidgetPtr()->getNormCbPointer());
    cbOrderAtachment = new ComboBoxAttachment(valueTreeState,"orderSetting",*title.getOutputWidgetPtr()->getOrderCbPointer());
    
    // ======================== YAW PITCH ROLL GROUP
    ypGroup.setText("Encoder settings");
    ypGroup.setTextLabelPosition (Justification::centredLeft);
    ypGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    ypGroup.setColour (GroupComponent::textColourId, Colours::white);
    addAndMakeVisible(&ypGroup);
    ypGroup.setVisible(true);
    
    addAndMakeVisible(&slMasterYaw);
    slMasterYawAttachment = new SliderAttachment(valueTreeState,"masterYaw", slMasterYaw);
    slMasterYaw.setSliderStyle (Slider::Rotary);
    slMasterYaw.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slMasterYaw.setReverse(true);
    slMasterYaw.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slMasterYaw.setRotaryParameters(M_PI, 3*M_PI, false);
    slMasterYaw.setTooltip("Master yaw angle");
    slMasterYaw.setTextValueSuffix(CharPointer_UTF8 ("\xc2\xb0"));
    
    addAndMakeVisible(&slMasterPitch);
    slMasterPitchAttachment = new SliderAttachment(valueTreeState,"masterPitch", slMasterPitch);
    slMasterPitch.setSliderStyle (Slider::Rotary);
    slMasterPitch.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slMasterPitch.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slMasterPitch.setReverse(true);
    slMasterPitch.setRotaryParameters(0.5*M_PI, 2.5*M_PI, false);
    slMasterPitch.setTooltip("Master pitch angle");
    slMasterPitch.setTextValueSuffix(CharPointer_UTF8 ("\xc2\xb0"));
    
    addAndMakeVisible(&slMasterRoll);
    slMasterRollAttachment = new SliderAttachment(valueTreeState,"masterRoll", slMasterRoll);
    slMasterRoll.setSliderStyle (Slider::Rotary);
    slMasterRoll.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slMasterRoll.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slMasterRoll.setRotaryParameters(0.5*M_PI, 2.5*M_PI, false);
    slMasterRoll.setTooltip("Master roll angle");
    slMasterRoll.setTextValueSuffix(CharPointer_UTF8 ("\xc2\xb0"));
    
    addAndMakeVisible(&tbLockedToMaster);
    tbLockedToMasterAttachment = new ButtonAttachment(valueTreeState,"lockedToMaster", tbLockedToMaster);
    tbLockedToMaster.setName("locking");
    tbLockedToMaster.setButtonText("Lock Directions");
    
    
    // ====================== GRAB GROUP
    quatGroup.setText("Master");
    quatGroup.setTextLabelPosition (Justification::centredLeft);
    quatGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    quatGroup.setColour (GroupComponent::textColourId, Colours::white);
    addAndMakeVisible(&quatGroup);
    quatGroup.setVisible(true);
    
    
    
    // ================ LABELS ===================
    addAndMakeVisible(&lbNum);
    lbNum.setText("#");
    
    addAndMakeVisible(&lbYaw);
    lbYaw.setText("Yaw");
    
    addAndMakeVisible(&lbPitch);
    lbPitch.setText("Pitch");

    addAndMakeVisible(&lbGain);
    lbGain.setText("Gain");
    
    startTimer(10);
}

void MultiEncoderAudioProcessorEditor::IEMSphereElementChanged (IEMSphere* sphere, IEMSphereElement* element) {
    
    Vector3D<float> pos = element->getPosition();
    float hypxy = sqrt(pos.x*pos.x+pos.y*pos.y);
    
    float yaw = atan2f(pos.y,pos.x);
    float pitch = atan2f(hypxy,pos.z)-M_PI/2;
    
    if (element->getID() == "grabber") {
        valueTreeState.getParameter("masterYaw")->setValue(valueTreeState.getParameterRange("masterYaw").convertTo0to1(yaw/M_PI*180.0f));
        valueTreeState.getParameter("masterPitch")->setValue(valueTreeState.getParameterRange("masterPitch").convertTo0to1(pitch/M_PI*180.0f));
    }
}

MultiEncoderAudioProcessorEditor::~MultiEncoderAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void MultiEncoderAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void MultiEncoderAudioProcessorEditor::timerCallback()
{
    // check max possible order and update combobox in title
    if (processor.maxPossibleOrder != maxPossibleOrder)
    {
        maxPossibleOrder = processor.maxPossibleOrder;
        title.getOutputWidgetPtr()->updateOrderCb(maxPossibleOrder);
    }
    
    if (processor.maxNumInputs != maxNumInputs)
    {
        maxNumInputs = processor.maxNumInputs;
        title.getInputWidgetPtr()->updateMxPossibleChannelNumber(maxNumInputs); //TODO
    }
    
    if (processor.nChIn != lastSetNumChIn)
    {
        encoderList.setNumberOfChannels(processor.nChIn);
        lastSetNumChIn = processor.nChIn;
        processor.editorNChIn = lastSetNumChIn;
    }

    
    if (!processor.soloMask.isZero()) {
        for (int i = 0; i<lastSetNumChIn; ++i)
        {
            encoderList.sphereElementArray[i]->setActive(processor.soloMask[i]);
        }
    }
    else
    {
        for (int i = 0; i<lastSetNumChIn; ++i)
        {
            encoderList.sphereElementArray[i]->setActive(!processor.muteMask[i]);
        }
    }
    
    if (processor.updateColours)
    {
        processor.updateColours = false;
        encoderList.updateColours();
    }
    
    //masterElement.setPosition(Vector3D<float>(processor.xyzGrab[0], processor.xyzGrab[1], processor.xyzGrab[2]));
    sphere.repaint();
}

void MultiEncoderAudioProcessorEditor::resized()
{
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds(footerArea);
    
    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop    (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop(10);
    
    Rectangle<int> sliderRow;
    
    // ============== SIDEBAR RIGHT ====================
    // =================================================
    Rectangle<int> sideBarArea (area.removeFromRight(220));
    //const int sliderHeight = 15;
    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 10;
    //const int sliderSpacing = 3;
    const int rotSliderWidth = 40;
    //const int labelHeight = 15;
    //const int labelWidth = 20;


    // -------------- Yaw Pitch Roll Labels ------------------
    Rectangle<int> yprArea (sideBarArea);
    ypGroup.setBounds (yprArea);
    yprArea.removeFromTop(25); //for box headline


    sliderRow = (yprArea.removeFromTop(15));
    lbNum.setBounds(sliderRow.removeFromLeft(22));
    sliderRow.removeFromLeft(5);
    lbYaw.setBounds(sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    lbPitch.setBounds(sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    lbGain.setBounds(sliderRow.removeFromLeft(rotSliderWidth));

    viewport.setBounds(yprArea);

    

    // ============== SIDEBAR LEFT ====================

    area.removeFromRight(10); // spacing
    
    // ------------- Grabber ------------------------
    Rectangle<int> grabberArea (area.removeFromBottom(70));
    quatGroup.setBounds (grabberArea);
    grabberArea.removeFromTop(25); //for box headline
    
    sliderRow = (grabberArea.removeFromTop(rotSliderHeight));
    slMasterYaw.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    slMasterPitch.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    slMasterRoll.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    tbLockedToMaster.setBounds (sliderRow.removeFromLeft(100));
    
    
    Rectangle<int> sphereArea (area);
    
    if ((float)sphereArea.getWidth()/sphereArea.getHeight() > 1)
        sphereArea.setWidth(sphereArea.getHeight());
    else
        sphereArea.setHeight(sphereArea.getWidth());
    sphere.setBounds(sphereArea);
    
}


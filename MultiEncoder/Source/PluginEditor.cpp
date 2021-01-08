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
MultiEncoderAudioProcessorEditor::MultiEncoderAudioProcessorEditor (MultiEncoderAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
: juce::AudioProcessorEditor (&p), footer (p.getOSCParameterInterface()), processor (p), valueTreeState(vts),
masterElement(*valueTreeState.getParameter("masterAzimuth"), valueTreeState.getParameterRange("masterAzimuth"),
              *valueTreeState.getParameter("masterElevation"), valueTreeState.getParameterRange("masterElevation")),
encoderList (p, sphere, &vts),
lbAzimuth (encoderList.getAzimuthArray()),
lbElevation (encoderList.getElevationArray()),
lbGain (encoderList.getGainArray())
{
    setLookAndFeel (&globalLaF);

    // ==== SPHERE AND ELEMENTS ===============
    addAndMakeVisible(&sphere);
    sphere.addListener(this);

    sphere.addElement(&masterElement);
    masterElement.setColour(juce::Colours::black);
    masterElement.setTextColour(juce::Colours::white);
    masterElement.setLabel("M");

    // ======================================

    addAndMakeVisible(&title);
    title.setTitle(juce::String("Multi"),juce::String("Encoder"));
    title.setFont(globalLaF.robotoBold,globalLaF.robotoLight);

    addAndMakeVisible(&footer);

    tooltipWin.setLookAndFeel (&globalLaF);
    tooltipWin.setMillisecondsBeforeTipAppears(500);
    tooltipWin.setOpaque (false);



    cbNumInputChannelsAttachment.reset (new ComboBoxAttachment (valueTreeState,"inputSetting",*title.getInputWidgetPtr()->getChannelsCbPointer()));
    cbNormalizationAtachment.reset (new ComboBoxAttachment (valueTreeState,"useSN3D",*title.getOutputWidgetPtr()->getNormCbPointer()));
    cbOrderAtachment.reset (new ComboBoxAttachment (valueTreeState,"orderSetting",*title.getOutputWidgetPtr()->getOrderCbPointer()));

    // ======================== Encoder group
    encoderGroup.setText("Encoder settings");
    encoderGroup.setTextLabelPosition (juce::Justification::centredLeft);
    encoderGroup.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    encoderGroup.setColour (juce::GroupComponent::textColourId, juce::Colours::white);
    addAndMakeVisible(&encoderGroup);
    encoderGroup.setVisible(true);

    addAndMakeVisible(tbImport);
    tbImport.setButtonText ("IMPORT");
    tbImport.setColour (juce::TextButton::buttonColourId, juce::Colours::orange);
    tbImport.setTooltip ("Imports sources from a configuration file.");
    tbImport.onClick = [&] () { importLayout(); };

    addAndMakeVisible(&viewport);
    viewport.setViewedComponent(&encoderList);

    // ====================== GRAB GROUP
    masterGroup.setText("Master");
    masterGroup.setTextLabelPosition (juce::Justification::centredLeft);
    masterGroup.setColour (juce::GroupComponent::outlineColourId, globalLaF.ClSeperator);
    masterGroup.setColour (juce::GroupComponent::textColourId, juce::Colours::white);
    addAndMakeVisible(&masterGroup);
    masterGroup.setVisible(true);

    addAndMakeVisible(&slMasterAzimuth);
    slMasterAzimuthAttachment.reset (new SliderAttachment (valueTreeState, "masterAzimuth", slMasterAzimuth));
    slMasterAzimuth.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slMasterAzimuth.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slMasterAzimuth.setReverse(true);
    slMasterAzimuth.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slMasterAzimuth.setRotaryParameters(juce::MathConstants<float>::pi, 3*juce::MathConstants<float>::pi, false);
    slMasterAzimuth.setTooltip("Master azimuth angle");

    addAndMakeVisible(&slMasterElevation);
    slMasterElevationAttachment.reset (new SliderAttachment (valueTreeState, "masterElevation", slMasterElevation));
    slMasterElevation.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slMasterElevation.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slMasterElevation.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slMasterElevation.setRotaryParameters(0.5*juce::MathConstants<float>::pi, 2.5*juce::MathConstants<float>::pi, false);
    slMasterElevation.setTooltip("Master elevation angle");

    addAndMakeVisible(&slMasterRoll);
    slMasterRollAttachment.reset (new SliderAttachment (valueTreeState, "masterRoll", slMasterRoll));
    slMasterRoll.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slMasterRoll.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 15);
    slMasterRoll.setColour (juce::Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slMasterRoll.setRotaryParameters(juce::MathConstants<float>::pi, 3*juce::MathConstants<float>::pi, false);
    slMasterRoll.setTooltip("Master roll angle");

    addAndMakeVisible(&tbLockedToMaster);
    tbLockedToMasterAttachment.reset (new ButtonAttachment(valueTreeState, "lockedToMaster", tbLockedToMaster));
    tbLockedToMaster.setName("locking");
    tbLockedToMaster.setButtonText("Lock Directions");


    // ================ LABELS ===================
    addAndMakeVisible(&lbNum);
    lbNum.setText("#");

    addAndMakeVisible(&lbAzimuth);
    lbAzimuth.setText("Azimuth");

    addAndMakeVisible(&lbElevation);
    lbElevation.setText("Elevation");

    addAndMakeVisible(&lbGain);
    lbGain.setText("Gain");

    addAndMakeVisible(&lbMasterAzimuth);
    lbMasterAzimuth.setText("Azimuth");

    addAndMakeVisible(&lbMasterElevation);
    lbMasterElevation.setText("Elevation");

    addAndMakeVisible(&lbMasterRoll);
    lbMasterRoll.setText("Roll");

    setResizeLimits(590, 455, 800, 1200);
    startTimer (40);
}


MultiEncoderAudioProcessorEditor::~MultiEncoderAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void MultiEncoderAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void MultiEncoderAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    const int nChIn = processor.input.getSize();
    if (nChIn != lastSetNumChIn)
    {
        encoderList.setNumberOfChannels(nChIn);
        lastSetNumChIn = nChIn;
        sphere.repaint();
    }

    if (processor.soloMuteChanged)
    {
        if (! processor.soloMask.isZero()) {
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
        processor.soloMuteChanged = false;
        sphere.repaint();
    }

    if (processor.updateColours)
    {
        processor.updateColours = false;
        encoderList.updateColours();
        sphere.repaint();
    }
    if (processor.updateSphere)
    {
        processor.updateSphere = false;
        sphere.repaint();
    }
}

void MultiEncoderAudioProcessorEditor::mouseWheelOnSpherePannerMoved (SpherePanner* sphere, const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
{
    if (event.mods.isCommandDown() && event.mods.isAltDown())
        slMasterRoll.mouseWheelMove(event, wheel);
    else if (event.mods.isAltDown())
        slMasterElevation.mouseWheelMove(event, wheel);
    else if (event.mods.isCommandDown())
        slMasterAzimuth.mouseWheelMove(event, wheel);
}

void MultiEncoderAudioProcessorEditor::resized()
{
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    juce::Rectangle<int> area (getLocalBounds());

    juce::Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds(footerArea);

    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    juce::Rectangle<int> headerArea = area.removeFromTop    (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop(10);
    area.removeFromBottom(5);

    juce::Rectangle<int> sliderRow;

    // ============== SIDEBAR RIGHT ====================
    // =================================================
    juce::Rectangle<int> sideBarArea (area.removeFromRight(220));
    //const int sliderHeight = 15;
    const int rotSliderSpacing = 10;
    //const int sliderSpacing = 3;
    const int rotSliderWidth = 40;
    //const int labelHeight = 15;
    //const int labelWidth = 20;


    // -------------- Azimuth Elevation Roll juce::Labels ------------------
    juce::Rectangle<int> yprArea (sideBarArea);
    encoderGroup.setBounds (yprArea);
    auto headlineArea = yprArea.removeFromTop (25); //for box headline
    tbImport.setBounds (headlineArea.removeFromRight (60).removeFromTop (15));


    sliderRow = yprArea.removeFromTop (15);
    lbNum.setBounds (sliderRow.removeFromLeft (15));
    sliderRow.removeFromLeft (3);
    lbAzimuth.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 10));
    sliderRow.removeFromLeft (rotSliderSpacing - 7);
    lbElevation.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 13));
    sliderRow.removeFromLeft (rotSliderSpacing - 5);
    lbGain.setBounds(sliderRow.removeFromLeft(rotSliderWidth));

    viewport.setBounds(yprArea);


    // ============== SIDEBAR LEFT ====================

    const int masterAreaHeight = 90;
    area.removeFromRight(10); // spacing

    juce::Rectangle<int> sphereArea (area);
    sphereArea.removeFromBottom(masterAreaHeight);

    if ((float)sphereArea.getWidth()/sphereArea.getHeight() > 1)
        sphereArea.setWidth(sphereArea.getHeight());
    else
        sphereArea.setHeight(sphereArea.getWidth());
    sphere.setBounds(sphereArea);

    area.removeFromTop(sphereArea.getHeight());

    // ------------- Master Grabber ------------------------
    juce::Rectangle<int> masterArea (area.removeFromTop(masterAreaHeight));
    masterGroup.setBounds (masterArea);
    masterArea.removeFromTop(25); //for box headline


    sliderRow = (masterArea.removeFromBottom(12));
    lbMasterAzimuth.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing - 5);
    lbMasterElevation.setBounds (sliderRow.removeFromLeft(rotSliderWidth + 10));
    sliderRow.removeFromLeft(rotSliderSpacing - 5);
    lbMasterRoll.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);

    sliderRow = masterArea;
    slMasterAzimuth.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    slMasterElevation.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    slMasterRoll.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    tbLockedToMaster.setBounds (sliderRow.removeFromLeft(100));
}

void MultiEncoderAudioProcessorEditor::importLayout()
{
    juce::FileChooser myChooser ("Load configuration...",
                           processor.getLastDir().exists() ? processor.getLastDir() : juce::File::getSpecialLocation (juce::File::userHomeDirectory),
                           "*.json");
    if (myChooser.browseForFileToOpen())
    {
        juce::File configFile (myChooser.getResult());
        processor.setLastDir (configFile.getParentDirectory());
        auto result = processor.loadConfiguration (configFile);

        if (! result.wasOk())
        {
            auto component = std::make_unique<juce::TextEditor> ();
            component->setMultiLine (true, true);
            component->setReadOnly (true);
            component->setText (result.getErrorMessage());
            component->setSize (200, 110);

            auto& myBox = juce::CallOutBox::launchAsynchronously (std::move (component), tbImport.getBounds(), this);
            myBox.setLookAndFeel (&getLookAndFeel());
        }
    }
}

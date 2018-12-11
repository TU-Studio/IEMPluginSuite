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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//==============================================================================
ProbeDecoderAudioProcessorEditor::ProbeDecoderAudioProcessorEditor (ProbeDecoderAudioProcessor& p, AudioProcessorValueTreeState& vts)
: AudioProcessorEditor (&p), footer (p.getOSCReceiver()), processor (p), valueTreeState(vts),
probe(*valueTreeState.getParameter("azimuth"), valueTreeState.getParameterRange("azimuth"),
      *valueTreeState.getParameter("elevation"), valueTreeState.getParameterRange("elevation"))
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 325);
    setLookAndFeel (&globalLaF);

    // ==== SPHERE AND ELEMENTS ===============
    addAndMakeVisible(&sphere);
    //sphere.addListener(this);

    probe.setColour(Colours::aqua);
    sphere.addElement(&probe);


    // ======================================


    addAndMakeVisible(&title);
    title.setTitle(String("Probe"),String("Decoder"));
    title.setFont(globalLaF.robotoBold,globalLaF.robotoLight);

    addAndMakeVisible(&footer);

    toolTipWin.setMillisecondsBeforeTipAppears(500);
    toolTipWin.setOpaque (false);


    cbNormalizationAtachement = new ComboBoxAttachment(valueTreeState,"useSN3D", *title.getInputWidgetPtr()->getNormCbPointer());
    cbOrderAtachement = new ComboBoxAttachment(valueTreeState,"orderSetting", *title.getInputWidgetPtr()->getOrderCbPointer());


    // ======================== YAW PITCH ROLL GROUP
    ypGroup.setText("Azimuth & Elevation");
    ypGroup.setTextLabelPosition (Justification::centredLeft);
    ypGroup.setColour (GroupComponent::outlineColourId, globalLaF.ClSeperator);
    ypGroup.setColour (GroupComponent::textColourId, Colours::white);
    addAndMakeVisible(&ypGroup);
    ypGroup.setVisible(true);

    addAndMakeVisible(&slAzimuth);
    slAzimuthAttachment = new SliderAttachment(valueTreeState,"azimuth", slAzimuth);
    slAzimuth.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slAzimuth.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slAzimuth.setReverse(true);
    slAzimuth.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slAzimuth.setRotaryParameters(M_PI, 3*M_PI, false);
    slAzimuth.setTooltip("Azimuth angle");
    slAzimuth.setTextValueSuffix(CharPointer_UTF8 (R"(°)"));

    addAndMakeVisible(&slElevation);
    slElevationAttachment = new SliderAttachment(valueTreeState,"elevation", slElevation);
    slElevation.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slElevation.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slElevation.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slElevation.setRotaryParameters(0.5*M_PI, 2.5*M_PI, false);
    slElevation.setTooltip("Elevation angle");


    // ================ LABELS ===================
    addAndMakeVisible(&lbAzimuth);
    lbAzimuth.setText("Azimuth");

    addAndMakeVisible(&lbElevation);
    lbElevation.setText("Elevation");

    startTimer(10);
}


ProbeDecoderAudioProcessorEditor::~ProbeDecoderAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void ProbeDecoderAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void ProbeDecoderAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    int maxInSize, maxOutSize;
    processor.getMaxSize(maxInSize, maxOutSize);
    title.setMaxSize(maxInSize, maxOutSize);
    // ==========================================

    if (processor.updatedPositionData.get())
    {
        processor.updatedPositionData = false;
        sphere.repaint();
    }
}

void ProbeDecoderAudioProcessorEditor::resized()
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
    Rectangle<int> sideBarArea (area.removeFromRight(190));

    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 10;

    const int rotSliderWidth = 40;
    const int labelHeight = 15;


    // -------------- Yaw Pitch Roll ------------------
    Rectangle<int> yprArea (sideBarArea.removeFromTop(25 + rotSliderHeight + labelHeight));
    ypGroup.setBounds (yprArea);
    yprArea.removeFromTop(25); //for box headline

    sliderRow = (yprArea.removeFromTop(rotSliderHeight));
    slAzimuth.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    slElevation.setBounds (sliderRow.removeFromLeft(rotSliderWidth));

    lbAzimuth.setBounds(yprArea.removeFromLeft(rotSliderWidth));
    yprArea.removeFromLeft(rotSliderSpacing - 5);
    lbElevation.setBounds(yprArea.removeFromLeft(rotSliderWidth + 10));

    sideBarArea.removeFromTop(20);


    // ============== SIDEBAR LEFT ====================

    area.removeFromRight(10); // spacing
    sphere.setBounds(area.getX(), area.getY(),area.getWidth()-20,area.getWidth()-20);


}

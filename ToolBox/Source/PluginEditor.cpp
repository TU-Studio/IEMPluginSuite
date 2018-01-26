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
ToolBoxAudioProcessorEditor::ToolBoxAudioProcessorEditor (ToolBoxAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts)
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    //setSize(500, 300); // use this to create a fixed-size GUI
    setResizeLimits(400, 200, 800, 500); // use this to create a resizeable GUI
    setLookAndFeel (&globalLaF);
    
    // make title and footer visible, and set the PluginName
    addAndMakeVisible(&title);
    title.setTitle(String("Tool"),String("Box"));
    title.setFont(globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================
    
    
    // create the connection between title component's comboBoxes and parameters
    cbInputOrderSettingAttachment = new ComboBoxAttachment(valueTreeState, "inputOrderSetting", *title.getInputWidgetPtr()->getOrderCbPointer());
    cbInputNormalizationSettingAttachment = new ComboBoxAttachment(valueTreeState, "useSn3dInput", *title.getInputWidgetPtr()->getNormCbPointer());
    
    cbOutputOrderSettingAttachment = new ComboBoxAttachment(valueTreeState, "outputOrderSetting", *title.getOutputWidgetPtr()->getOrderCbPointer());
    cbOutputNormalizationSettingAttachment = new ComboBoxAttachment(valueTreeState, "useSn3dOutput", *title.getOutputWidgetPtr()->getNormCbPointer());
    
    
    addAndMakeVisible(gcFlip);
    gcFlip.setText("Flip");
    
    addAndMakeVisible(tbFlipX);
    tbFlipXAttachment = new ButtonAttachment(valueTreeState, "flipX", tbFlipX);
    tbFlipX.setButtonText("Flip X (front/back)");
    tbFlipX.setColour(ToggleButton::tickColourId, globalLaF.ClWidgetColours[2]);
    
    addAndMakeVisible(tbFlipY);
    tbFlipYAttachment = new ButtonAttachment(valueTreeState, "flipY", tbFlipY);
    tbFlipY.setButtonText("Flip Y (left/right)");
    tbFlipY.setColour(ToggleButton::tickColourId, globalLaF.ClWidgetColours[1]);
    
    addAndMakeVisible(tbFlipZ);
    tbFlipZAttachment = new ButtonAttachment(valueTreeState, "flipZ", tbFlipZ);
    tbFlipZ.setButtonText("Flip Z (bottom/top)");
    tbFlipZ.setColour(ToggleButton::tickColourId, globalLaF.ClWidgetColours[0]);
    
    // start timer after everything is set up properly
    startTimer(20);
}

ToolBoxAudioProcessorEditor::~ToolBoxAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void ToolBoxAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void ToolBoxAudioProcessorEditor::resized()
{
    // ============ BEGIN: header and footer ============
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom(footerHeight));
    footer.setBounds(footerArea);

    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop(headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop(10);
    area.removeFromBottom(5);
    // =========== END: header and footer =================
    
    

    Rectangle<int> sliderRow = area.removeFromTop(85);
    
    {
        Rectangle<int> flipArea = sliderRow.removeFromLeft(150);
        gcFlip.setBounds(flipArea);
        flipArea.removeFromTop(25);
        tbFlipX.setBounds(flipArea.removeFromTop(20));
        tbFlipY.setBounds(flipArea.removeFromTop(20));
        tbFlipZ.setBounds(flipArea.removeFromTop(20));
    }

    
    
    
}

void ToolBoxAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    int maxInSize, maxOutSize;
    processor.getMaxSize(maxInSize, maxOutSize);
    title.setMaxSize(maxInSize, maxOutSize);
    // ==========================================
    
    // insert stuff you want to do be done at every timer callback
}

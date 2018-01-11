/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://www.iem.at
 
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

#pragma once
#include "ReferenceCountedMatrix.h"
#include "ambisonicTools.h"
#include "MaxRE.h"

class ReferenceCountedDecoder : public ReferenceCountedMatrix
{
public:
    typedef ReferenceCountedObjectPtr<ReferenceCountedDecoder> Ptr;
    
    enum Normalization
    {
        n3d,
        sn3d
    };
    
    enum Weights
    {
        none,
        maxrE,
        inPhase
    };
    
    struct Settings {
        Normalization expectedNormalization = sn3d;
        Weights weights = none;
        bool weightsAlreadyApplied = false;
    };
    
    
    ReferenceCountedDecoder (const String& nameToUse, const String& descriptionToUse, int rows, int columns)
    :   ReferenceCountedMatrix(nameToUse, descriptionToUse, rows, columns), order(isqrt(columns)-1)
    {}
    
    ~ReferenceCountedDecoder()
    {}
    
    virtual String getConstructorMessage() override
    {
        return "Decoder named '" + name + "' constructed. Size: " + String(matrix.rows()) + "x" + String(matrix.cols());
    }
    
    virtual String getDeconstructorMessage() override
    {
        return "Decoder named '" + name + "' destroyed.";
    }
    
    Eigen::MatrixXf* getMatrix()
    {
        return &matrix;
    }
    const String getName()
    {
        return name;
    }
    
    const String getDescription()
    {
        return description;
    }
    
    void setSettings(Settings newSettings)
    {
        settings = newSettings;
    }
    
    const Settings getSettings()
    {
        return settings;
    }

    const String getSettingsAsString()
    {

        return "Decoder expects Ambisonic input up to " + getOrderString(order) + " order with " + String(settings.expectedNormalization == Normalization::n3d ? "N3D" : "SN3D") + " normalization. The weights are '" + getWeightsString() + "' and are " + String(settings.weightsAlreadyApplied ? "already applied." : "not aplied yet.");
    }
    
    const String getWeightsString()
    {
        switch(settings.weights)
        {
            case 1: return String("maxrE");
            case 2: return String("inPhase");
            default: return String("none");
        }
    }
    void processAppliedWeights()
    {
        if (settings.weightsAlreadyApplied && settings.weights != Weights::none)
        {
            if (settings.weights == Weights::maxrE)
                for (int i = 0; i < matrix.cols(); ++i)
                    matrix.col(i) /= getMaxRELUT(order)[i];
            settings.weightsAlreadyApplied = false;
        }
    }
    
    int getOrder()
    {
        return order;
    }

private:
    Settings settings;
    const int order;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReferenceCountedDecoder)
};


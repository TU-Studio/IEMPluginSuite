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


//==============================================================================
/*
*/
class VisualizerComponent    : public Component, public OpenGLRenderer, private Timer
{
public:
    VisualizerComponent() 
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.

        
        openGLContext.setComponentPaintingEnabled(true);
        openGLContext.setContinuousRepainting(false);
        openGLContext.setRenderer(this);
        openGLContext.attachTo(*this);
        
        addAndMakeVisible(&hammerAitovGrid);
        
        startTimer(20);
    }

    ~VisualizerComponent()
    {
        openGLContext.detach();
        openGLContext.setRenderer(nullptr);
    }

    void timerCallback() override {
        openGLContext.triggerRepaint();
    }
    void setRmsDataPtr(Array<float>* newRms) {
        pRMS = newRms;
    }
    
    void newOpenGLContextCreated() override
    {
        createShaders();
    }
    
    void renderOpenGL() override
    {
        jassert (OpenGLHelpers::isContextActive());
        
        OpenGLHelpers::clear (Colour(0xFF2D2D2D));
        
        const float desktopScale = (float) openGLContext.getRenderingScale();
        glViewport (-5, -5, roundToInt (desktopScale * getWidth()+10), roundToInt (desktopScale * getHeight()+10));
        
        glEnable (GL_DEPTH_TEST);
        glDepthFunc (GL_LESS);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        openGLContext.extensions.glActiveTexture (GL_TEXTURE0);
        glEnable (GL_TEXTURE_2D);
        
        texture.bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // linear interpolation when too small
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // linear interpolation when too bi
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT);
        
        shader->use();
        
        if (firstRun)
        {
            PixelARGB colormapData[8];
            colormapData[0] = Colours::skyblue.withMultipliedAlpha(0.0f).getPixelARGB();
            colormapData[1] = Colours::skyblue.withMultipliedAlpha(0.2f).getPixelARGB();
            colormapData[2] = Colours::skyblue.withMultipliedAlpha(0.3f).getPixelARGB();
            colormapData[3] = Colour::fromFloatRGBA(0.167f, 0.620f, 0.077f, 6.0f).getPixelARGB();
            colormapData[4] = Colour::fromFloatRGBA(0.167f, 0.620f, 0.077f, 7.0f).getPixelARGB();
            colormapData[5] = Colour::fromFloatRGBA(0.8f, 0.620f, 0.077f, 0.8f).getPixelARGB();
            colormapData[6] = Colour::fromFloatRGBA(0.8f, 0.620f, 0.077f, 1.0f).getPixelARGB();
            colormapData[7] = Colours::red.getPixelARGB();
            texture.loadARGB(colormapData,8,1);
            
            firstRun = false;
            openGLContext.extensions.glGenBuffers(1, &vertexBuffer);
            openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
            openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, sizeof(hammerAitovSampleVertices), hammerAitovSampleVertices, GL_STATIC_DRAW);
            
            openGLContext.extensions.glGenBuffers(1, &indexBuffer);
            openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            openGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(hammerAitovSampleIndices), hammerAitovSampleIndices, GL_STATIC_DRAW);
        }

        static GLfloat g_colorMap_data[nSamplePoints];
        for (int i = 0; i < nSamplePoints; i++){
            //g_colorMap_data[i] = (float) rand() / RAND_MAX;
            g_colorMap_data[i] = pRMS->getUnchecked(i);
            //g_colorMap_data[i] = (float) i / tDesignN;
        }
        
        GLuint colorBuffer;
        openGLContext.extensions.glGenBuffers(1, &colorBuffer);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, sizeof(g_colorMap_data), g_colorMap_data, GL_STATIC_DRAW);
        
        
        GLint attributeID;
        GLint programID = shader->getProgramID();
        
        // 1st attribute buffer : vertices
        attributeID = openGLContext.extensions.glGetAttribLocation(programID,"position");
        openGLContext.extensions.glEnableVertexAttribArray(attributeID);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        
        
        openGLContext.extensions.glVertexAttribPointer(
                              attributeID,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                              2,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        // 2nd attribute buffer : colors
        attributeID = openGLContext.extensions.glGetAttribLocation(programID,"colormapDepthIn");
        
        openGLContext.extensions.glEnableVertexAttribArray(attributeID);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        
        openGLContext.extensions.glVertexAttribPointer(
                              attributeID,                                // attribute. No particular reason for 1, but must match the layout in the shader.
                              1,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        
        //glDrawElements (GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);  // Draw triangles!
        //glDrawArrays(GL_TRIANGLES, 0, 3);
        
//        glPointSize(20.0);
//        glDrawElements (GL_POINTS, 100, GL_UNSIGNED_INT, 0);  // Draw triangles!
        glDrawElements(
                       GL_TRIANGLES,      // mode
                       sizeof(hammerAitovSampleIndices),    // count
                       GL_UNSIGNED_INT,   // type
                       (void*)0           // element array buffer offset
                       );
        
        openGLContext.extensions.glDisableVertexAttribArray(0);
        openGLContext.extensions.glDisableVertexAttribArray(1);
        
        openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
        openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
        
        openGLContext.extensions.glDeleteBuffers (1, &colorBuffer);
        
    }
    void openGLContextClosing() override {
        openGLContext.extensions.glDeleteBuffers (1, &vertexBuffer);
        openGLContext.extensions.glDeleteBuffers (1, &indexBuffer);
        texture.release();
    }
    
    void paint (Graphics& g) override
    {

    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..
        hammerAitovGrid.setBounds(getLocalBounds());
    }
    
    void createShaders()
    {
//        vertexShader =
//        "attribute vec3 position;\n"
//        "void main()\n"
//        "{\n"
//        "   gl_Position.xyz = position;\n"
//        "   gl_Position.w = 1.0;\n"
//        "}";
//

//        fragmentShader =
//        "void main()\n"
//        "{\n"
//        "      gl_FragColor = vec4(1,0,0,0.5);\n"
//        "}";
        
        vertexShader =
        "attribute vec2 position;\n"
        "attribute float colormapDepthIn;\n"
        "varying float colormapDepthOut;\n"
        "void main()\n"
        "{\n"
        "   gl_Position.xy = position;\n"
        "   gl_Position.z = 0.0;\n"
        "   gl_Position.w = 1.0;\n"
        "   colormapDepthOut = colormapDepthIn;\n"
        "}";
        
        fragmentShader =
        "varying float colormapDepthOut;\n"
        "uniform sampler2D tex0;\n"
        "void main()\n"
        "{\n"
        //"      gl_FragColor = fragmentColor;\n"
        "      gl_FragColor = texture2D(tex0, vec2(colormapDepthOut, 0));\n"
        "}";
        
        ScopedPointer<OpenGLShaderProgram> newShader (new OpenGLShaderProgram (openGLContext));
        String statusText;
        
        if (newShader->addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (vertexShader))
            && newShader->addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (fragmentShader))
            && newShader->link())
        {
            shader = newShader;
            shader->use();
            statusText = "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2);
        }
        else
        {
            statusText = newShader->getLastError();
        }
        
    }
    

private:
    HammerAitovGrid hammerAitovGrid;
    GLuint vertexBuffer, indexBuffer;
    const char* vertexShader;
    const char* fragmentShader;
    ScopedPointer<OpenGLShaderProgram> shader;
    
    
    OpenGLTexture texture;
    
    bool firstRun = true;
    
    Array<float>* pRMS;
    
    OpenGLContext openGLContext;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VisualizerComponent)
};

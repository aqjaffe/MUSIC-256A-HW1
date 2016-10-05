// Music 256a / CS 476a | fall 2016
// CCRMA, Stanford University
//
// Author: Adam Jaffe
// Description: A simple keyboard piano. This program connects
//              keyboard buttons with notes on a piano, and
//              allows users to play them in real time.

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Sine.h"
#include <string>

#define NUM_KEYS 17

class MainContentComponent :
    public AudioAppComponent,
    private Slider::Listener,
    private ToggleButton::Listener,
    private KeyListener
{
public:
    MainContentComponent() : gain (0.0), samplingRate(0)
    {   
        for(int i = 0; i < NUM_KEYS; i++)
        {
            // configure key buttons and add them to the main window
            addAndMakeVisible(keyButtons[i]);
            keyButtons[i].addListener(this);
            keyStates[i] = 0;
            
            // configure gain label and add it to the main window
            addAndMakeVisible(keyLabels[i]);
            keyLabels[i].setText(std::string(1, keyNames[i]), dontSendNotification);
            keyLabels[i].attachToComponent(&keyButtons[i], false);
        }
        
        
        // configure gain slider and add it to the main window
        addAndMakeVisible(gainSlider);
        gainSlider.setRange(0.0, 1.0);
        gainSlider.setValue(0.5); // this will set the default gain of the sine osc
        gainSlider.addListener(this);
        
        // configure gain label and add it to the main window
        addAndMakeVisible(gainLabel);
        gainLabel.setText("Volume", dontSendNotification);
        gainLabel.attachToComponent(&gainSlider, true);
        
        // configure keyboard listeners
        getTopLevelComponent()->addKeyListener(this);
        
        //set default window size
        setSize (800, 100);
        nChans = 2;
        setAudioChannels (0, nChans); // no inputs, two outputs
    }
    
    ~MainContentComponent()
    {
        shutdownAudio();
    }
    
    void resized() override
    {
        const int margin = 80;
        gainSlider.setBounds (margin, 20, getWidth() - margin - 20, 20);
        for(int i = 0; i < NUM_KEYS; i++)
        {
            keyButtons[i].setBounds(margin + 40 + i * 30, 70, 20, 20);
        }
    }
    
    void sliderValueChanged (Slider* slider) override
    {
        if (slider == &gainSlider) gain = gainSlider.getValue();
        if (samplingRate > 0.0) {
            for(int i = 0; i < NUM_KEYS; i++) {
                keySines[i].setFrequency(toneFrequencies[i]);
            }
        }
    }
    
    void buttonClicked (Button* button) override
    {
        for(int i = 0; i < NUM_KEYS; i++) {
            if(button == &keyButtons[i]) {
                if((*button).getToggleState()) keyStates[i] = 1;
                else keyStates[i] = 0;
            }
        }
    }
    
    bool keyPressed(const KeyPress &key, Component* /* originatingComponent */) override
    {
        for(int i = 0; i < NUM_KEYS; i++) {
            if(key.getTextCharacter() == keyChars[i]) {
                keyStates[i] = 1;
                keyButtons[i].setToggleState(1, false);
            }
        }
        return true;
    }
    
    bool keyStateChanged(bool isKeyDown, Component* /* originatingComponent */)
    {
        for(int i = 0; i < NUM_KEYS; i++) {
            if(keyStates[i] == 1 && !isKeyDown) {
                keyStates[i] = 0;
                keyButtons[i].setToggleState(0, false);
            }
        }
        return true;
    }
    
    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        samplingRate = (int) sampleRate;
        for(int i = 0; i < NUM_KEYS; i++) {
            keySines[i].setSamplingRate(samplingRate);
        }
    }
    
    void releaseResources() override
    {
    }
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // getting the audio output buffers to be filled
        float* const bufferLeft = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
        float* const bufferRight = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);
        
        // computing one block
        for (int sample = 0; sample < bufferToFill.numSamples; ++sample) {
            float total = 0.0;
            int nOscsActive = 0;
            for(int i = 0; i < NUM_KEYS; i++) {
                if(keyStates[i] == 1) {
                    total += (float) keySines[i].tick();
                    nOscsActive++;
                }
            }
            bufferLeft[sample] = (float) (total * gain / (float) nOscsActive);
            bufferRight[sample] = (float) (total * gain / (float) nOscsActive);
        }
    }
    
    
private:
    Slider gainSlider;
    // set the frequencies for one octave of notes
    static constexpr double toneFrequencies[] = {523.251, 554.365, 587.330, 622.254,
                                                 659.485, 698.456, 739.989, 783.991,
                                                 830.609, 880.000, 932.328, 987.767,
                                                 1046.500, 1108.73, 1174.66, 1244.51,
                                                 1318.51};
    // define the keys connected to each tone
    static constexpr char keyChars[] = {'a', 'w', 's', 'e', 'd', 'f', 't', 'g', 'y',
                                        'h', 'u', 'j', 'k', 'o', 'l', 'p', ';'};
    
    static constexpr char keyNames[] = {'C', ' ', 'D', ' ', 'E', 'F', ' ', 'G', ' ',
                                        'A', ' ', 'B', 'C', ' ', 'D', ' ', 'E'};
                                             
    Label gainLabel;
    Label keyLabels[NUM_KEYS];

    double gain;
    
    ToggleButton keyButtons[NUM_KEYS];
    Sine keySines[NUM_KEYS]; // the sine wave oscillators
    int keyStates[NUM_KEYS]; //keeps track of which tones are playing
    
    int samplingRate, nChans;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <cmath>

//==============================================================================
/**
*/
class InputNameHereAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Slider::Listener, private juce::Timer
{
public:
    InputNameHereAudioProcessorEditor (InputNameHereAudioProcessor&);
    ~InputNameHereAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    juce::AudioFormatManager formatManager;
    juce::TextButton openFileButton;
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.

    juce::Slider grainStartSlider;
    juce::Label grainStartLabel;
    juce::Slider grainLengthSlider;
    juce::Label grainLengthLabel;
    juce::Label debug;
    juce::Slider grainCloudSizeSlider;
    juce::Label grainCloudSizeLabel;
    juce::Slider grainRandomizerSlider;
    juce::Label grainRandomizerLabel;
    juce::Slider grainMinGainSlider;
    juce::Label grainMinGainLabel;
    juce::Slider grainMaxGainSlider;
    juce::Label grainMaxGainLabel;
    juce::Slider outputGainSlider;
    juce::Label outputGainLabel;
    juce::Slider overdriveSlider;
    juce::Label overdriveLabel;

    InputNameHereAudioProcessor& audioProcessor;
    void openFile();
    void setAudioSource(juce::AudioFormatReader& audioFile);
    void setSlider(juce::Slider* slider, float base, float start, float stop, float step, juce::String suffix = "");
    void setLabel(juce::Slider* slider, juce::Label* label, juce::String labelValue);
    void sliderValueChanged(juce::Slider* slider) override;
    void setSliderValues();

    void timerCallback() override
    {
        keyboardComponent.grabKeyboardFocus();
        stopTimer();
    }
    juce::MidiKeyboardComponent keyboardComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InputNameHereAudioProcessorEditor)
};

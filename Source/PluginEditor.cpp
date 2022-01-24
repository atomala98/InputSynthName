/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
InputNameHereAudioProcessorEditor::InputNameHereAudioProcessorEditor (InputNameHereAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), keyboardComponent(audioProcessor.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(keyboardComponent);
    setSize (600, 600);
    startTimer(400);

    addAndMakeVisible(&openFileButton);
    openFileButton.setButtonText("Add Audio Sample");
    openFileButton.onClick = [this] { openFile(); };

    setSlider(&grainStartSlider, 1, 1, 44100, 1, " sample");
    setLabel(&grainStartSlider, &grainStartLabel, "Grain Start");

    setSlider(&grainLengthSlider, 1, 10, 200, 0.2, " ms");
    setLabel(&grainLengthSlider, &grainLengthLabel, "Grain Length");

    setSlider(&grainCloudSizeSlider, 1, 1, 20, 1);
    setLabel(&grainCloudSizeSlider, &grainCloudSizeLabel, "Grain Cloud Size");

    setSlider(&grainRandomizerSlider, 100, 100, 100000, 10);
    setLabel(&grainRandomizerSlider, &grainRandomizerLabel, "Grain Randomization");

    setSlider(&grainMinGainSlider, 100, 0, 100, 1, " %");
    setLabel(&grainMinGainSlider, &grainMinGainLabel, "Grain Min Gain");

    setSlider(&grainMaxGainSlider, 100, 0, 100, 1, " %");
    setLabel(&grainMaxGainSlider, &grainMaxGainLabel, "Grain Max Gain");

    setSlider(&outputGainSlider, 1.0, 0.0, 4.0, 0.01, "");
    setLabel(&outputGainSlider, &outputGainLabel, "Output Gain");

    setSlider(&overdriveSlider, 0, 0, 100, 1, "%");
    setLabel(&overdriveSlider, &overdriveLabel, "Overdrive");

    addAndMakeVisible(debug);

    formatManager.registerBasicFormats();
}

InputNameHereAudioProcessorEditor::~InputNameHereAudioProcessorEditor()
{
}

//==============================================================================
void InputNameHereAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    grainStartSlider.setValue(audioProcessor.synthAudioSource.getGrainStart());
}

void InputNameHereAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    int sliderLeft = 120;
    int keyboardY = 330;
    openFileButton.setBounds(30, 30, getWidth() - 60, 20);
    grainStartSlider.setBounds(sliderLeft, 60, getWidth() - (sliderLeft + 30), 20);
    grainLengthSlider.setBounds(sliderLeft, 90, getWidth() - (sliderLeft + 30), 20);
    grainCloudSizeSlider.setBounds(sliderLeft, 120, getWidth() - (sliderLeft + 30), 20);
    grainRandomizerSlider.setBounds(sliderLeft, 150, getWidth() - (sliderLeft + 30), 20);
    grainMinGainSlider.setBounds(sliderLeft, 180, getWidth() - (sliderLeft + 30), 20);
    grainMaxGainSlider.setBounds(sliderLeft, 210, getWidth() - (sliderLeft + 30), 20);
    outputGainSlider.setBounds(sliderLeft, 240, getWidth() - (sliderLeft + 30), 20);
    overdriveSlider.setBounds(sliderLeft, 270, getWidth() - (sliderLeft + 30), 20);
    keyboardComponent.setBounds(10, keyboardY, getWidth() - 20, getHeight() - keyboardY - 10);

}

void InputNameHereAudioProcessorEditor::openFile() 
{
    juce::FileChooser chooseFile("Select a WAV file to play...", {}, "*.wav;*.flac;*.mp3");
    if (chooseFile.browseForFileToOpen())
    {
        juce::File file = chooseFile.getResult();
        juce::AudioFormatReader* reader = formatManager.createReaderFor(file);
        if (reader != nullptr) {
            setAudioSource(*reader);
        }
    }
}

void InputNameHereAudioProcessorEditor::setAudioSource(juce::AudioFormatReader& audioFile) {
    int wavSize = (int)(audioFile.lengthInSamples);
    int sampleRate = (int)(audioFile.sampleRate);

    grainStartSlider.setRange(1, wavSize / 2, 1);

    grainStartSlider.setValue(1);
    grainLengthSlider.setValue(1);

    audioProcessor.synthAudioSource.loadFile();

    audioProcessor.wavFileBuffer = juce::AudioSampleBuffer(audioFile.numChannels, static_cast<int>(audioFile.lengthInSamples));
    audioFile.read(&audioProcessor.wavFileBuffer, 0, wavSize, 0, true, true);

    setSliderValues();
}

void InputNameHereAudioProcessorEditor::setSlider(juce::Slider* slider, float base, float start, float stop, float step, juce::String suffix) {
    addAndMakeVisible(slider);
    slider->setRange(start, stop, step);
    slider->setValue(base);
    slider->setTextValueSuffix(suffix);
    slider->addListener(this);
}

void InputNameHereAudioProcessorEditor::setLabel(juce::Slider* slider, juce::Label* label, juce::String labelValue) {
    addAndMakeVisible(label);
    juce::Font labelFont(juce::Typeface::createSystemTypefaceFor(BinaryData::MontserratExtraBoldItalic_ttf,
        BinaryData::MontserratExtraBoldItalic_ttfSize));
    label->setText(labelValue, juce::dontSendNotification);
    label->setFont(labelFont);
    label->attachToComponent(slider, true);
}


void InputNameHereAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    setSliderValues();
}

void InputNameHereAudioProcessorEditor::setSliderValues() {
    if (grainMinGainSlider.getValue() > grainMaxGainSlider.getValue()) {
        grainMaxGainSlider.setValue(grainMinGainSlider.getValue());
    }
    audioProcessor.synthAudioSource.setGrainStart(grainStartSlider.getValue());
    audioProcessor.synthAudioSource.setGrainLength((int)(grainLengthSlider.getValue() * 44.1));
    audioProcessor.synthAudioSource.setGrainCloudSize(grainCloudSizeSlider.getValue());
    audioProcessor.synthAudioSource.setRandomGrain(grainRandomizerSlider.getValue());
    audioProcessor.synthAudioSource.setMinGain(grainMinGainSlider.getValue());
    audioProcessor.synthAudioSource.setMaxGain(grainMaxGainSlider.getValue());
    audioProcessor.synthAudioSource.setOutputGain(outputGainSlider.getValue() * outputGainSlider.getValue());
    audioProcessor.synthAudioSource.setOverdriveThreshold((125 - overdriveSlider.getValue()) / 125.f);
}
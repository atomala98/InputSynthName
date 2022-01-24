#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
//==============================================================================
struct GrainSound : public juce::SynthesiserSound
{
	GrainSound() {}
	bool appliesToNote(int) override { return true; }
	bool appliesToChannel(int) override { return true; }
};
//==============================================================================
struct GrainCloudVoice : public juce::SynthesiserVoice
{
	// ***
	GrainCloudVoice(const juce::AudioSampleBuffer& wavetableToUse, unsigned int voices) : wavetable(wavetableToUse)
	{
		globalLevel = 0.5 / (float)voices;
		jassert(wavetable.getNumChannels() == 1);
	}
	void setFrequency(float frequency, float sampleRate)
	{
		auto tableSizeOverSampleRate = wavetable.getNumSamples() / sampleRate;
		tableDelta = frequency / 264.0f;
	}
	bool canPlaySound(juce::SynthesiserSound* sound) override
	{
		return dynamic_cast<GrainSound*> (sound) != nullptr;
	}
	void startNote(int midiNoteNumber, float velocity,
		juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
	{
		level = velocity * globalLevel;
		tailOff = 0.0;
		// *** auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
		// *** auto cyclesPerSample = cyclesPerSecond / getSampleRate();
		setFrequency(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber), getSampleRate()); // ***
		// *** angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
	}
	void stopNote(float /*velocity*/, bool allowTailOff) override
	{
		if (allowTailOff)
		{
			if (tailOff == 0.0)
				tailOff = 1.0;
		}
		else
		{
			clearCurrentNote();
			tableDelta = 0.0f; // *** angleDelta = 0.0;
		}
	}
	void pitchWheelMoved(int) override {}
	void controllerMoved(int, int) override {}
	void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
	{
		if (tableDelta != 0.0) // ***
		{
			if (tailOff > 0.0)
			{
				while (--numSamples >= 0)
				{  // ***
					for (auto i = outputBuffer.getNumChannels(); --i >= 0;) {
						const float* wavetableBuffer = wavetable.getReadPointer(i);
						outputBuffer.addSample(i, startSample, wavetableBuffer[(int)currentSample] * level * tailOff);
					}
					if (currentSample >= wavetable.getNumSamples() - 1) {
						currentSample = 0;
					}
					currentSample += tableDelta;
					// *** currentAngle += angleDelta;
					++startSample;
					tailOff *= 0.999;
					if (tailOff <= 0.005)
					{
						clearCurrentNote();
						tableDelta = 0.0f; // *** angleDelta = 0.0;
						break;
					}
				}
			}
			else
			{
				while (--numSamples >= 0)
				{
					for (auto i = outputBuffer.getNumChannels(); --i >= 0;) {
						const float* wavetableBuffer = wavetable.getReadPointer(i);
						outputBuffer.addSample(i, startSample, wavetableBuffer[(int)currentSample] * level);
					}
					if (currentSample >= wavetable.getNumSamples() - 1) {
						currentSample = 0;
					}
					currentSample += tableDelta;
					// *** currentAngle += angleDelta;
					++startSample;
				}
			}
		}
	}
private:
	// *** double currentAngle = 0.0, angleDelta = 0.0, level = 0.0,;
	float currentSample = 1;
	float tailOff = 0.0; // ***
	const juce::AudioSampleBuffer& wavetable;
	float currentIndex = 0.0f, tableDelta = 0.0f, level = 0.0f, globalLevel = 0.0f;
};

//==============================================================================
class GrainSynthAudioSource : public juce::AudioSource
{
public:
	GrainSynthAudioSource(juce::MidiKeyboardState& keyState) : keyboardState(keyState)
	{
		for (auto i = 0; i < 4; ++i)
			synth.addVoice(new GrainCloudVoice(outputGrainCloud, numVoices));
		synth.addSound(new GrainSound());
	}
	void setUsingSineWaveSound()
	{
		synth.clearSounds();
	}
	void prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate) override
	{
		synth.setCurrentPlaybackSampleRate(sampleRate);
	}
	void releaseResources() override {}
	void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
	{
		setGrainCloud();
		bufferToFill.clearActiveBufferRegion();
		juce::MidiBuffer incomingMidi;
		midiCollector.removeNextBlockOfMessages(incomingMidi, bufferToFill.numSamples);
		keyboardState.processNextMidiBuffer(incomingMidi, bufferToFill.startSample,
			bufferToFill.numSamples, true);
		synth.renderNextBlock(*bufferToFill.buffer, incomingMidi,
			bufferToFill.startSample, bufferToFill.numSamples);
	}
	juce::MidiMessageCollector* getMidiCollector()
	{
		return &midiCollector;
	}
	juce::MidiMessageCollector midiCollector;

	void setGrainStart(int grainStart) { this->grainStart = grainStart; }
	void setGrainLength(int grainLength) { this->grainLength = grainLength; }
	void setGrainCloudSize(int grainCloudSize) { this->grainCloudSize = grainCloudSize; }
	void setRandomGrain(int randomGrain) { this->randomGrain = randomGrain; }
	void setMinGain(int grainMinGain) { this->grainMinGain = grainMinGain; }
	void setMaxGain(int grainMaxGain) { this->grainMaxGain = grainMaxGain; }
	void setOutputGain(float outputGain) { this->outputGain = outputGain; }
	void setOverdriveThreshold(float overdriveThreshold) { this->overdriveThreshold = overdriveThreshold; }
	void loadFile() { fileLoaded = true; }
	
	int getGrainStart() { return this->grainStart; }
	int getDrive() { return this->overdriveThreshold; }

	void setWavetable(juce::AudioBuffer<float> wavFileBuffer) {
		wavTable.setSize(wavFileBuffer.getNumChannels(), wavFileBuffer.getNumSamples());
		for (int channel = 0; channel < wavFileBuffer.getNumChannels(); channel++) {
			const float* newSamples = wavFileBuffer.getReadPointer(channel);
			float* samples = wavTable.getWritePointer(channel);
			for (int j = 0; j < wavFileBuffer.getNumSamples(); j++) {
				samples[j] = newSamples[j];
			}
		}
	}
private:
	juce::MidiKeyboardState& keyboardState;
	juce::Synthesiser synth;

	bool fileLoaded = false;
	int grainStart = 1;
	int grainLength = 450;
	int grainCloudSize = 1;
	int randomGrain = 100;
	float overdriveThreshold = 1.f;
	float grainMinGain = 100;
	float grainMaxGain = 100;
	float outputGain = 1.f;
	float softClipThreshold = 2.f / 3.f;
	// ***
	int numVoices = 4;
	juce::AudioBuffer<float> wavTable;
	juce::AudioBuffer<float> outputGrainCloud;

	float overdrive(float sample) {
		if (sample < -overdriveThreshold) {
			return -overdriveThreshold * softClipThreshold;
		}
		else if (sample > overdriveThreshold) {
			return overdriveThreshold * softClipThreshold;
		}
		else {
			return sample - ((sample * sample * sample) / 3.f);
		}
	}


	void setGrainCloud() {
		outputGrainCloud.setSize(wavTable.getNumChannels(), grainLength * grainCloudSize);
		for (int channel = 0; channel < wavTable.getNumChannels(); channel++) {
			for (int j = 0; j < grainCloudSize; j++) {
				int randomInt = juce::Random::getSystemRandom().nextInt(2 * randomGrain) - randomGrain;
				randomInt = grainStart + randomInt < 1 ? 1 : randomInt;
				const float* wavtableBuffer = wavTable.getReadPointer(channel, grainStart + randomInt);
				float randomGain = (juce::Random::getSystemRandom().nextInt(grainMaxGain - grainMinGain) + grainMinGain) / 100.0f;
				outputGrainCloud.copyFromWithRamp(channel, j * grainLength, wavtableBuffer, grainLength, randomGain * outputGain, randomGain * outputGain);
			}
		}
		if (overdriveThreshold != 1.f)
			for (int channel = 0; channel < outputGrainCloud.getNumChannels(); channel++) {
				float* outputBuffer = outputGrainCloud.getWritePointer(channel);
				for (int j = 0; j < outputGrainCloud.getNumSamples(); j++) {
					outputBuffer[j] = overdrive(outputBuffer[j]) * (2 - overdriveThreshold) * (2 - overdriveThreshold) * (2 - overdriveThreshold);
				}
			}
	}
};
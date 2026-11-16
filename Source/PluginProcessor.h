#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <vector>

class TalkTronProcessor : public juce::AudioProcessor {
public:
 TalkTronProcessor();
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override { return true; }
 const juce::String getName() const override { return "TALK-TRON"; }
 bool acceptsMidi() const override { return true; }
 bool producesMidi() const override { return false; }
 bool isMidiEffect() const override { return false; }
 double getTailLengthSeconds() const override { return 0.5; }
 int getNumPrograms() override { return 1; }
 int getCurrentProgram() override { return 0; }
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override { return {}; }
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState parameters;
 juce::MidiKeyboardState keyboardState;
 void setLyrics(const juce::String&);
 juce::String getLyrics() const;
 void setSelectedSyllable(int);
 int getSelectedSyllable() const { return selected.load(); }
 int getActiveSyllable() const { return activeSyllable.load(); }
 std::vector<juce::String> getSyllables() const;
 static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();
private:
 struct Token { juce::String text; std::vector<int> phonemes; };
 struct Voice {
  bool held=false; int note=-1, token=0; double phase=0, modPhase=0, age=0, release=0, env=0; float lp=0; std::array<float,6> band{};
 };
 std::array<Voice,16> voices{};
 std::shared_ptr<const std::vector<Token>> tokens;
 mutable juce::CriticalSection lyricLock;
 juce::String lyricText="I want you tonight";
 std::atomic<int> selected{0}, activeSyllable{-1};
 int cursor=0; double sampleRate=44100;
 static std::vector<Token> tokenize(const juce::String&);
 static std::vector<int> phonemize(const juce::String&);
 float synthSample(Voice&,float,float,float,float,float,int);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TalkTronProcessor)
};

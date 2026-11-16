#pragma once
#include "PluginProcessor.h"
class TalkTronEditor : public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit TalkTronEditor(TalkTronProcessor&);
 ~TalkTronEditor() override;
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 TalkTronProcessor& processor;
 juce::TextEditor lyrics;
 juce::Label signature;
 juce::ComboBox mode;
 juce::ComboBox syllablePicker;
 juce::String cachedLyrics;
 juce::ToggleButton hold{"HOLD SYLLABLE"},single{"SINGLE SYLLABLE"};
 juce::Slider volume;
 std::array<juce::Slider,6> knobs;
 std::array<juce::Label,6> labels;
 juce::MidiKeyboardComponent keyboard{processor.keyboardState,juce::MidiKeyboardComponent::horizontalKeyboard};
 juce::TextButton minus{"OCT -"},plus{"OCT +"};
 juce::Label active;
 std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttach;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,6> knobAttach;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttach;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> holdAttach,singleAttach;
 int octave=3;
 void timerCallback() override;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TalkTronEditor)
};

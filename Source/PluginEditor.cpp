#include "PluginEditor.h"
namespace {const juce::Colour metal(0xff20232c),pink(0xffff5896),lcd(0xffffa719),ink(0xff211509);}
TalkTronEditor::TalkTronEditor(TalkTronProcessor& p):AudioProcessorEditor(&p),processor(p){
 setSize(1180,630);
 auto& ap=processor.parameters;
 lyrics.setMultiLine(true);lyrics.setReturnKeyStartsNewLine(true);lyrics.setScrollbarsShown(true);lyrics.setColour(juce::TextEditor::backgroundColourId,lcd);lyrics.setColour(juce::TextEditor::textColourId,ink);lyrics.setColour(juce::TextEditor::highlightColourId,juce::Colours::white.withAlpha(.5f));lyrics.setFont(juce::Font(juce::FontOptions(23.f)));lyrics.setText(p.getLyrics(),false);
 lyrics.onTextChange=[this]{processor.setLyrics(lyrics.getText());cachedLyrics.clear();};addAndMakeVisible(lyrics);
 syllablePicker.onChange=[this]{if(syllablePicker.getSelectedId()>0)processor.setSelectedSyllable(syllablePicker.getSelectedId()-1);};addAndMakeVisible(syllablePicker);
 active.setColour(juce::Label::textColourId,lcd);active.setJustificationType(juce::Justification::centredLeft);addAndMakeVisible(active);
 mode.addItemList({"TALKBOX","VOCODER","ROBOT","SYNTH VOCAL","CHORUS VOCAL"},1);addAndMakeVisible(mode);modeAttach=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(ap,"mode",mode);
 for(auto* b:{&hold,&single}){b->setColour(juce::ToggleButton::tickColourId,pink);b->setColour(juce::ToggleButton::textColourId,juce::Colours::whitesmoke);addAndMakeVisible(*b);}
 holdAttach=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(ap,"hold",hold);singleAttach=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(ap,"single",single);
 const char* names[]={"CHARACTER","BREATHE","GLIDE","VIBRATO","FORMANT","OUTPUT"};const char* ids[]={"character","breathe","glide","vibrato","formant","output"};
 for(int i=0;i<6;++i){auto& k=knobs[(size_t)i];k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);k.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);k.setColour(juce::Slider::rotarySliderFillColourId,pink);k.setColour(juce::Slider::rotarySliderOutlineColourId,juce::Colour(0xff55545e));k.setColour(juce::Slider::thumbColourId,juce::Colours::ivory);addAndMakeVisible(k);knobAttach[(size_t)i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(ap,ids[i],k);labels[(size_t)i].setText(names[i],juce::dontSendNotification);labels[(size_t)i].setJustificationType(juce::Justification::centred);labels[(size_t)i].setColour(juce::Label::textColourId,juce::Colours::whitesmoke);addAndMakeVisible(labels[(size_t)i]);}
 volume.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);volume.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);volume.setColour(juce::Slider::rotarySliderFillColourId,pink);addAndMakeVisible(volume);volumeAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(ap,"volume",volume);
 keyboard.setAvailableRange(24,96);keyboard.setKeyWidth(38.f);keyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId,juce::Colour(0xffffebd6));addAndMakeVisible(keyboard);
 addAndMakeVisible(minus);addAndMakeVisible(plus);
 minus.onClick=[this]{octave=juce::jmax(2,octave-1);keyboard.setLowestVisibleKey(octave*12);};plus.onClick=[this]{octave=juce::jmin(6,octave+1);keyboard.setLowestVisibleKey(octave*12);};keyboard.setLowestVisibleKey(48);
 // Keyboard clicks produce notes through the same MIDI input path via an injected state buffer.
 startTimerHz(15);
}
TalkTronEditor::~TalkTronEditor(){stopTimer();}
void TalkTronEditor::timerCallback(){if(cachedLyrics!=processor.getLyrics()){cachedLyrics=processor.getLyrics();auto items=processor.getSyllables();syllablePicker.clear(juce::dontSendNotification);for(int i=0;i<(int)items.size();++i)syllablePicker.addItem(juce::String(i+1)+": "+items[(size_t)i],i+1);syllablePicker.setSelectedId(processor.getSelectedSyllable()+1,juce::dontSendNotification);}auto syll=processor.getSyllables();int idx=processor.getActiveSyllable();active.setText(idx>=0&&idx<(int)syll.size()?"PLAYING: "+syll[(size_t)idx].toUpperCase():"READY  •  Use hyphens to split syllables (to-night)",juce::dontSendNotification);}
void TalkTronEditor::paint(juce::Graphics& g){
 g.fillAll(juce::Colour(0xff111219));auto r=getLocalBounds().reduced(8).toFloat();g.setColour(metal);g.fillRoundedRectangle(r,13);g.setColour(juce::Colour(0xff88818a));g.drawRoundedRectangle(r,13,2);
 g.setColour(juce::Colour(0xffeee7e1));g.setFont(juce::Font(juce::FontOptions(48.f,juce::Font::bold)));g.drawText("TALK-TRON",45,16,410,70,juce::Justification::centredLeft);
 g.setColour(pink);g.setFont(juce::Font(juce::FontOptions("URW Chancery L",37.f,juce::Font::italic)));g.drawText("by Zonium",465,20,280,60,juce::Justification::centredLeft);
 g.setColour(juce::Colour(0xff50515a));g.drawLine(20.f,95.f,1160.f,95.f,2.f);g.drawLine(20.f,378.f,1160.f,378.f,2.f);g.drawLine(20.f,495.f,1160.f,495.f,2.f);
 g.setColour(juce::Colours::ivory);g.setFont(17.f);g.drawText("VOLUME",28,125,140,24,juce::Justification::centred);g.drawText("VOICE MODE",960,124,200,28,juce::Justification::centred);
 g.setColour(juce::Colour(0xff0d0e13));g.fillRoundedRectangle(192.f,112.f,750.f,255.f,14.f);g.setColour(juce::Colour(0xff74717c));g.drawRoundedRectangle(192.f,112.f,750.f,255.f,14.f,2.f);
 g.setColour(juce::Colour(0xffe3d3c7));g.setFont(13.f);g.drawText("LYRICS / SYLLABLE EDITOR",218,118,340,22,juce::Justification::centredLeft);
 g.setColour(juce::Colour(0xff55515b));g.drawRoundedRectangle(35.f,396.f,335.f,78.f,7.f,1.f);
 g.setColour(juce::Colours::whitesmoke);g.drawText("MIDI KEYBOARD",25,505,150,25,juce::Justification::centred);
 for(auto x:{26.f,1154.f})for(auto y:{26.f,604.f}){g.setColour(juce::Colour(0xff9a939a));g.fillEllipse(x-5,y-5,10,10);g.setColour(juce::Colour(0xff27262b));g.drawLine(x-3,y+3,x+3,y-3,1.5f);}
}
void TalkTronEditor::resized(){
 volume.setBounds(48,165,100,100);lyrics.setBounds(211,145,711,190);active.setBounds(216,338,700,24);mode.setBounds(974,166,172,35);
 hold.setBounds(47,400,153,42);single.setBounds(202,400,170,42);syllablePicker.setBounds(48,445,320,31);
 for(int i=0;i<6;++i){int x=405+i*125;knobs[(size_t)i].setBounds(x,390,91,77);labels[(size_t)i].setBounds(x-10,465,110,23);}
 minus.setBounds(25,545,65,44);keyboard.setBounds(106,523,968,86);plus.setBounds(1090,545,65,44);
}

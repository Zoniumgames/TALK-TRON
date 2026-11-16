#include "PluginEditor.h"
#include <cmath>
namespace {
 const juce::Colour metal(0xff202128), pink(0xfffa4d91), lcd(0xffffa31a), ink(0xff21140b), ivory(0xfff6e9d9);
 void panel(juce::Graphics& g,juce::Rectangle<float> r,float radius=10.f){
  g.setColour(juce::Colour(0xff0c0d12));g.fillRoundedRectangle(r.expanded(3),radius+2);
  g.setGradientFill(juce::ColourGradient(juce::Colour(0xff47434b),r.getX(),r.getY(),juce::Colour(0xff14151a),r.getRight(),r.getBottom(),false));
  g.fillRoundedRectangle(r,radius);g.setColour(juce::Colour(0xff9b9295));g.drawRoundedRectangle(r,radius,1.4f);
 }
}
void TalkTronEditor::VintageLook::drawRotarySlider(juce::Graphics& g,int x,int y,int width,int height,float pos,float start,float end,juce::Slider&){
 auto r=juce::Rectangle<float>((float)x,(float)y,(float)width,(float)height).reduced(8.f);
 float size=juce::jmin(r.getWidth(),r.getHeight());auto c=r.getCentre();float rad=size*.46f;
 juce::Path track;track.addCentredArc(c.x,c.y,rad,rad,0.f,start,end,true);
 g.setColour(juce::Colour(0xff54505a));g.strokePath(track,juce::PathStrokeType(5.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
 juce::Path progress;progress.addCentredArc(c.x,c.y,rad,rad,0.f,start,start+(end-start)*pos,true);
 g.setColour(pink);g.strokePath(progress,juce::PathStrokeType(5.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
 auto face=juce::Rectangle<float>(size*.76f,size*.76f).withCentre(c);
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff686069),face.getX(),face.getY(),juce::Colour(0xff17181e),face.getRight(),face.getBottom(),false));g.fillEllipse(face);
 g.setColour(juce::Colour(0xffc3b3ad));g.drawEllipse(face,1.5f);
 float angle=start+(end-start)*pos;
 auto a=c+juce::Point<float>(std::sin(angle),-std::cos(angle))*size*.24f;
 auto b=c+juce::Point<float>(std::sin(angle),-std::cos(angle))*size*.36f;
 g.setColour(ivory);g.drawLine(a.x,a.y,b.x,b.y,3.f);
}
void TalkTronEditor::VintageLook::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool over,bool down){
 auto r=b.getLocalBounds().toFloat().reduced(1);
 g.setGradientFill(juce::ColourGradient(juce::Colour(down?0xff9e365b:0xff4b4650),0,0,juce::Colour(0xff111219),0,r.getBottom(),false));g.fillRoundedRectangle(r,5.f);
 g.setColour(over?pink:juce::Colour(0xff8e858c));g.drawRoundedRectangle(r,5.f,1.f);
}
TalkTronEditor::TalkTronEditor(TalkTronProcessor& p):AudioProcessorEditor(&p),processor(p){
 setSize(1400,750);setResizable(true,true);setResizeLimits(1100,600,1900,1100);
 auto& ap=processor.parameters;
 lyrics.setMultiLine(true);lyrics.setReturnKeyStartsNewLine(true);lyrics.setScrollbarsShown(true);
 lyrics.setColour(juce::TextEditor::backgroundColourId,lcd);lyrics.setColour(juce::TextEditor::textColourId,ink);
 lyrics.setColour(juce::TextEditor::outlineColourId,juce::Colours::transparentBlack);
 lyrics.setColour(juce::TextEditor::focusedOutlineColourId,juce::Colours::transparentBlack);
 lyrics.setColour(juce::TextEditor::highlightColourId,juce::Colour(0xfff7df76));
 lyrics.setFont(juce::Font(juce::FontOptions(27.f)));lyrics.setText(p.getLyrics(),false);
 lyrics.onTextChange=[this]{processor.setLyrics(lyrics.getText());cachedLyrics.clear();};addAndMakeVisible(lyrics);
 syllablePicker.onChange=[this]{if(syllablePicker.getSelectedId()>0)processor.setSelectedSyllable(syllablePicker.getSelectedId()-1);};addAndMakeVisible(syllablePicker);
 active.setColour(juce::Label::textColourId,ink);active.setFont(juce::Font(juce::FontOptions(17.f,juce::Font::bold)));addAndMakeVisible(active);
 const char* modeNames[]={"TALKBOX","VOCODER","ROBOT","SYNTH VOCAL","CHORUS VOCAL"};
 for(int i=0;i<5;++i){modes[(size_t)i].setButtonText(modeNames[i]);modes[(size_t)i].setLookAndFeel(&look);modes[(size_t)i].onClick=[this,i]{if(auto* param=processor.parameters.getParameter("mode")){param->beginChangeGesture();param->setValueNotifyingHost((float)i/4.f);param->endChangeGesture();}};addAndMakeVisible(modes[(size_t)i]);}
 for(auto* b:{&hold,&single}){b->setColour(juce::ToggleButton::tickColourId,pink);b->setColour(juce::ToggleButton::textColourId,ivory);addAndMakeVisible(*b);}
 holdAttach=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(ap,"hold",hold);
 singleAttach=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(ap,"single",single);
 const char* names[]={"CHARACTER","BREATHE","GLIDE","VIBRATO","FORMANT","OUTPUT"};
 const char* ids[]={"character","breathe","glide","vibrato","formant","output"};
 for(int i=0;i<6;++i){auto& k=knobs[(size_t)i];k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);k.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);k.setLookAndFeel(&look);addAndMakeVisible(k);knobAttach[(size_t)i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(ap,ids[i],k);labels[(size_t)i].setText(names[i],juce::dontSendNotification);labels[(size_t)i].setJustificationType(juce::Justification::centred);labels[(size_t)i].setColour(juce::Label::textColourId,ivory);addAndMakeVisible(labels[(size_t)i]);}
 volume.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);volume.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);volume.setLookAndFeel(&look);addAndMakeVisible(volume);volumeAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(ap,"volume",volume);
 keyboard.setAvailableRange(24,96);keyboard.setKeyWidth(42.f);keyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId,juce::Colour(0xfff4e6d7));addAndMakeVisible(keyboard);
 for(auto* b:{&minus,&plus}){b->setLookAndFeel(&look);addAndMakeVisible(*b);}
 minus.onClick=[this]{octave=juce::jmax(2,octave-1);keyboard.setLowestVisibleKey(octave*12);};
 plus.onClick=[this]{octave=juce::jmin(6,octave+1);keyboard.setLowestVisibleKey(octave*12);};keyboard.setLowestVisibleKey(48);
 startTimerHz(15);
}
TalkTronEditor::~TalkTronEditor(){stopTimer();volume.setLookAndFeel(nullptr);for(auto& k:knobs)k.setLookAndFeel(nullptr);for(auto& m:modes)m.setLookAndFeel(nullptr);minus.setLookAndFeel(nullptr);plus.setLookAndFeel(nullptr);}
void TalkTronEditor::timerCallback(){
 if(cachedLyrics!=processor.getLyrics()){
  cachedLyrics=processor.getLyrics();auto items=processor.getSyllables();syllablePicker.clear(juce::dontSendNotification);
  for(int i=0;i<(int)items.size();++i)syllablePicker.addItem(juce::String(i+1)+": "+items[(size_t)i],i+1);
  syllablePicker.setSelectedId(processor.getSelectedSyllable()+1,juce::dontSendNotification);
 }
 auto syll=processor.getSyllables();int idx=processor.getActiveSyllable();
 active.setText(idx>=0&&idx<(int)syll.size()?"SINGING  ["+juce::String(idx+1)+"]  "+syll[(size_t)idx].toUpperCase():"READY  •  SEPARATE SYLLABLES WITH HYPHENS: TO-NIGHT",juce::dontSendNotification);
 int current=(int)processor.parameters.getRawParameterValue("mode")->load();
 for(int i=0;i<5;++i){auto& b=modes[(size_t)i];bool on=i==current;b.setColour(juce::TextButton::textColourOffId,on?pink:ivory);b.setColour(juce::TextButton::textColourOnId,pink);b.setToggleState(on,juce::dontSendNotification);}
}
void TalkTronEditor::paint(juce::Graphics& g){
 auto W=(float)getWidth(),H=(float)getHeight();g.fillAll(juce::Colour(0xff0b0a10));
 auto body=getLocalBounds().reduced(9).toFloat();g.setGradientFill(juce::ColourGradient(juce::Colour(0xff36323a),0,0,juce::Colour(0xff17181f),W,H,false));g.fillRoundedRectangle(body,18.f);g.setColour(juce::Colour(0xffb1a3a3));g.drawRoundedRectangle(body,18.f,2.f);
 g.setColour(ivory);g.setFont(juce::Font(juce::FontOptions(W*.046f,juce::Font::bold)));g.drawText("TALK-TRON",W*.067f,H*.014f,W*.43f,H*.13f,juce::Justification::centredLeft);
 g.setColour(pink);g.setFont(juce::Font(juce::FontOptions("URW Chancery L",W*.031f,juce::Font::italic)));g.drawText("by Zonium",W*.53f,H*.023f,W*.3f,H*.11f,juce::Justification::centredLeft);
 g.setColour(pink);g.fillRect(W*.86f,H*.067f,W*.09f,2.f);g.fillRect(W*.88f,H*.079f,W*.07f,1.f);
 g.setColour(juce::Colour(0xff9b8c91));g.drawLine(W*.015f,H*.145f,W*.985f,H*.145f,2.f);g.drawLine(W*.015f,H*.59f,W*.985f,H*.59f,2.f);g.drawLine(W*.015f,H*.77f,W*.985f,H*.77f,2.f);
 panel(g,{W*.15f,H*.172f,W*.66f,H*.39f},14.f);
 g.setColour(juce::Colour(0xffe5c36b));g.fillRoundedRectangle(W*.166f,H*.192f,W*.63f,H*.343f,7.f);
 g.setColour(lcd);g.fillRoundedRectangle(W*.168f,H*.195f,W*.626f,H*.338f,5.f);
 g.setColour(ink.withAlpha(.18f));for(float y=H*.2f;y<H*.53f;y+=4.f)g.drawHorizontalLine((int)y,W*.17f,W*.79f);
 g.setColour(ivory);g.setFont(juce::Font(juce::FontOptions(W*.014f,juce::Font::bold)));g.drawText("VOLUME",W*.035f,H*.20f,W*.10f,H*.04f,juce::Justification::centred);
 g.drawText("VOICE MODE",W*.82f,H*.185f,W*.16f,H*.04f,juce::Justification::centred);
 g.setColour(juce::Colour(0xff8e8088));g.drawRoundedRectangle(W*.82f,H*.225f,W*.16f,H*.315f,8.f,1.f);
 g.setColour(ivory);g.setFont(juce::Font(juce::FontOptions(W*.012f)));g.drawText("HOLD / SINGLE SYLLABLE",W*.03f,H*.604f,W*.27f,H*.04f,juce::Justification::centredLeft);
 g.setColour(juce::Colour(0xff8b8187));g.drawRoundedRectangle(W*.025f,H*.645f,W*.28f,H*.1f,6.f,1.f);
 g.setColour(ivory);g.drawText("MIDI KEYBOARD",W*.032f,H*.78f,W*.20f,H*.035f,juce::Justification::centredLeft);
 g.setColour(juce::Colour(0xff8b8187));g.drawRoundedRectangle(W*.135f,H*.803f,W*.73f,H*.16f,8.f,1.f);
 for(float x:{W*.02f,W*.98f})for(float y:{H*.03f,H*.97f}){g.setColour(juce::Colour(0xffd5c4b6));g.fillEllipse(x-6,y-6,12,12);g.setColour(juce::Colour(0xff3b3438));g.drawLine(x-3,y+3,x+3,y-3,2.f);}
}
void TalkTronEditor::resized(){
 auto W=getWidth(),H=getHeight();auto rect=[&](float x,float y,float w,float h){return juce::Rectangle<int>((int)(x*W),(int)(y*H),(int)(w*W),(int)(h*H));};
 volume.setBounds(rect(.035f,.25f,.10f,.16f));
 lyrics.setBounds(rect(.175f,.21f,.613f,.245f));active.setBounds(rect(.177f,.465f,.60f,.048f));
 for(int i=0;i<5;++i)modes[(size_t)i].setBounds(rect(.832f,.232f+i*.060f,.137f,.048f));
 hold.setBounds(rect(.035f,.656f,.135f,.038f));single.setBounds(rect(.17f,.656f,.13f,.038f));syllablePicker.setBounds(rect(.04f,.699f,.255f,.039f));
 for(int i=0;i<6;++i){float x=.34f+i*.107f;knobs[(size_t)i].setBounds(rect(x,.61f,.085f,.105f));labels[(size_t)i].setBounds(rect(x-.01f,.715f,.105f,.033f));}
 minus.setBounds(rect(.035f,.85f,.06f,.07f));keyboard.setBounds(rect(.145f,.82f,.71f,.135f));plus.setBounds(rect(.9f,.85f,.06f,.07f));
}

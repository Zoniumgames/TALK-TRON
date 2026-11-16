#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <algorithm>
#include <map>
namespace {
 constexpr float pi=juce::MathConstants<float>::pi;
 // Approximate English vowel formants (F1,F2,F3). The consonant engine is deliberately experimental.
 constexpr float vowelF[9][3]={{800,1150,2900},{500,1700,2500},{300,2200,3000},{400,900,2200},{650,1100,2400},{350,1900,2700},{600,1500,2600},{450,1300,2600},{350,800,2200}};
 float clampf(float v,float lo,float hi){return juce::jlimit(lo,hi,v);}
}
TalkTronProcessor::TalkTronProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"PARAMS",makeLayout()) { tokens=std::make_shared<const std::vector<Token>>(tokenize(lyricText)); }
juce::AudioProcessorValueTreeState::ParameterLayout TalkTronProcessor::makeLayout(){
 juce::AudioProcessorValueTreeState::ParameterLayout l;
 auto f=[&](const char* id,const char* name,float d){l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(0.f,1.f),d));};
 f("character","Character",.5f); f("breathe","Breathe",.18f); f("glide","Glide",.05f); f("vibrato","Vibrato",.1f); f("formant","Formant",.5f); f("output","Output",.65f); f("volume","Volume",.75f);
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"mode",1},"Mode",juce::StringArray{"Talkbox","Vocoder","Robot","Synth Vocal","Chorus Vocal"},0));
 l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"hold",1},"Hold Syllable",true));
 l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"single",1},"Single Syllable",false));
 return l;
}
std::vector<int> TalkTronProcessor::phonemize(const juce::String& word){
 // Small English-pronunciation lexicon. Vowel IDs 0..8, consonant classes 9..11.
 // This is an intelligibility experiment, not a complete English G2P engine.
 const std::map<juce::String,std::vector<int>> dictionary={
  {"i",{0,2}}, {"you",{11,8}}, {"want",{11,0,11,10}}, {"to",{10,8}},
  {"night",{11,0,2,10}}, {"tonight",{10,7,11,0,2,10}},
  {"love",{11,7,9}}, {"me",{11,2}}, {"my",{11,0,2}}, {"we",{11,2}},
  {"are",{0}}, {"the",{9,7}}, {"this",{9,5,9}}, {"is",{5,9}},
  {"on",{0,11}}, {"in",{5,11}}, {"and",{6,11,10}}, {"a",{7}},
  {"hello",{9,1,11,4}}, {"world",{11,7,11,10}}, {"baby",{10,1,10,2}},
  {"yeah",{11,6}}, {"oh",{4}}, {"hey",{9,1}}, {"go",{10,4}},
  {"stay",{9,10,1}}, {"with",{11,5,9}}, {"your",{11,3}},
  {"heart",{9,0,11,10}}, {"feel",{9,2,11}}, {"ton",{10,7,11}},
  {"ferdi",{9,1,11,10,2}}, {"sax",{9,6,10,9}}, {"saxophone",{9,6,10,9,7,9,4,11}}
 };
 auto w=word.toLowerCase().trim();auto found=dictionary.find(w);if(found!=dictionary.end())return found->second;
 std::vector<int> out;auto str=w.toStdString();
 for(size_t i=0;i<str.size();){
  if(i+1<str.size()){
   auto pair=str.substr(i,2);
   if(pair=="ee"||pair=="ea"||pair=="ie"){out.push_back(2);i+=2;continue;}
   if(pair=="oo"||pair=="ou"||pair=="ue"){out.push_back(8);i+=2;continue;}
   if(pair=="sh"||pair=="th"||pair=="ch"||pair=="ph"){out.push_back(9);i+=2;continue;}
   if(pair=="ai"||pair=="ay"){out.insert(out.end(),{0,2});i+=2;continue;}
   if(pair=="ow"||pair=="oa"){out.push_back(4);i+=2;continue;}
  }
  char c=str[i++];int ph=-1;
  switch(c){case 'a':ph=6;break;case 'e':ph=1;break;case 'i':case 'y':ph=5;break;case 'o':ph=4;break;case 'u':ph=7;break;case 'w':ph=8;break;case 's':case 'f':case 'h':case 'z':case 'v':case 'x':ph=9;break;case 't':case 'k':case 'p':case 'd':case 'g':case 'b':case 'c':case 'q':ph=10;break;case 'm':case 'n':case 'l':case 'r':case 'j':ph=11;break;default:break;}
  if(ph>=0)out.push_back(ph);
 }
 if(out.empty())out.push_back(0);return out;
}
std::vector<TalkTronProcessor::Token> TalkTronProcessor::tokenize(const juce::String& lyrics){
 std::vector<Token> result;juce::StringArray words;
 words.addTokens(lyrics, " \t\r\n,.;:!?()[]{}\"", "'"); words.removeEmptyStrings();
 // A small explicit syllable lexicon, plus optional manual '-' boundaries.
 const std::map<juce::String,juce::StringArray> syllables={
  {"tonight",{"to","night"}}, {"hello",{"hel","lo"}},
  {"baby",{"ba","by"}}, {"ferdi",{"fer","di"}},
  {"saxophone",{"sax","o","phone"}}, {"forever",{"for","ev","er"}},
  {"together",{"to","geth","er"}}, {"beautiful",{"beau","ti","ful"}},
  {"remember",{"re","mem","ber"}}, {"music",{"mu","sic"}}
 };
 for(auto word:words){
  auto w=word.toLowerCase().trim();juce::StringArray parts;
  if(w.containsChar('-')){parts.addTokens(w,"-","");parts.removeEmptyStrings();}
  else {auto it=syllables.find(w);if(it!=syllables.end())parts=it->second;else parts.add(w);}
  for(auto part:parts){if(result.size()>=256)break;result.push_back({part,phonemize(part)});}
  if(result.size()>=256)break;
 }
 if(result.empty())result.push_back({"ah",{0}});return result;
}
void TalkTronProcessor::setLyrics(const juce::String& s){juce::String safe=s.substring(0,2048);auto fresh=std::make_shared<const std::vector<Token>>(tokenize(safe));{const juce::ScopedLock lock(lyricLock);lyricText=safe;}std::atomic_store(&tokens,fresh);cursor=0;selected.store(0);activeSyllable.store(-1);}
juce::String TalkTronProcessor::getLyrics() const{const juce::ScopedLock lock(lyricLock);return lyricText;}
std::vector<juce::String> TalkTronProcessor::getSyllables() const{auto p=std::atomic_load(&tokens);std::vector<juce::String> r;for(auto& t:*p)r.push_back(t.text);return r;}
void TalkTronProcessor::setSelectedSyllable(int i){auto p=std::atomic_load(&tokens);selected.store(juce::jlimit(0,(int)p->size()-1,i));}
void TalkTronProcessor::prepareToPlay(double sr,int){sampleRate=sr;for(auto& v:voices)v=Voice{};cursor=0;}
bool TalkTronProcessor::isBusesLayoutSupported(const BusesLayout& l) const{return l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo()||l.getMainOutputChannelSet()==juce::AudioChannelSet::mono();}
float TalkTronProcessor::synthSample(Voice& v,float freq,float character,float breath,float vibrato,float formant,int mode){
 auto tok=v.tokenSnapshot;if(!tok || tok->empty())return 0;
 const auto& ph=(*tok)[(size_t)juce::jlimit(0,(int)tok->size()-1,v.token)].phonemes;
 double segment=std::max(0.075,0.34/(double)std::max<size_t>(1,ph.size()));
 int idx=juce::jlimit(0,(int)ph.size()-1,(int)(v.age/segment));int p=ph[(size_t)idx];
 float vowel=(p<9)?(float)p:((idx>0&&ph[(size_t)(idx-1)]<9)?(float)ph[(size_t)(idx-1)]:0.f);
 float fm=std::pow(2.f,(formant-.5f)*1.5f);float f1=vowelF[(int)vowel][0]*fm,f2=vowelF[(int)vowel][1]*fm,f3=vowelF[(int)vowel][2]*fm;
 float vib=1.f+vibrato*.022f*std::sin(2*pi*5.5*v.age);
 float hz=freq*vib;
 v.phase+=hz/sampleRate;if(v.phase>=1)v.phase-=1;
 float saw=2.f*(float)v.phase-1.f;
 float excitation=(mode==2?std::sin(2*pi*v.phase):mode==3?(.7f*std::sin(2*pi*v.phase)+.3f*saw):saw);
 if(mode==1)excitation=.5f*saw+.5f*std::sin(2*pi*v.phase);
 if(mode==4)excitation=.7f*saw+.3f*std::sin(2*pi*(v.phase+0.012f*std::sin(2*pi*.6*v.age)));
 v.modPhase=std::fmod(v.modPhase+0.6180339887,1.0);float noise=(float)(2*v.modPhase-1);
 if(p==9)excitation=.15f*excitation+.85f*noise;else if(p==10 && std::fmod(v.age,segment)<.035)excitation=.65f*noise+.35f*excitation;else if(p==11)excitation*=.55f;
 excitation=(1.f-breath*.5f)*excitation+breath*.24f*noise;
 // Parallel resonant biquad bandpass filters, coefficient update at audio rate to allow moving formants.
 float freqs[3]={f1,f2,f3};float sum=0;
 for(int i=0;i<3;++i){
  float center=clampf(freqs[i],100.f,(float)sampleRate*.42f);
  float q=2.5f+character*8.f;
  float g=std::tan(pi*center/(float)sampleRate),k=1.f/q,a=1.f/(1.f+g*(g+k));
  float& ic1=v.band[(size_t)i*2],&ic2=v.band[(size_t)i*2+1];
  float v3=excitation-ic2;float bp=a*(ic1+g*v3);float lp=ic2+g*bp;
  ic1=2*bp-ic1;ic2=2*lp-ic2;
  sum+=bp*(i==0?1.2f:i==1?.85f:.55f);
 }
 float attack=1.f-std::exp(-1.f/(float)(sampleRate*.012));
 float release=1.f-std::exp(-1.f/(float)(sampleRate*.14));
 v.env+=(v.held?attack:release)*((v.held?1.f:0.f)-v.env);
 v.age+=1.0/sampleRate;
 return std::tanh(sum*(1.4f+character*2.f))*v.env*.24f;
}
void TalkTronProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals noDenormals;buffer.clear();keyboardState.processNextMidiBuffer(midi,0,buffer.getNumSamples(),true);
 auto param=[this](const char* id){return parameters.getRawParameterValue(id)->load();};
 float character=param("character"),breath=param("breathe"),glide=param("glide"),vibrato=param("vibrato"),formant=param("formant"),gain=param("output")*param("volume");
 int mode=(int)param("mode");bool single=param("single")>.5f;bool hold=param("hold")>.5f;
 auto tk=std::atomic_load(&tokens);
 auto it=midi.cbegin();
 for(int i=0;i<buffer.getNumSamples();++i){
  while(it!=midi.cend()&&(*it).samplePosition<=i){auto msg=(*it).getMessage();
   if(msg.isNoteOn()){
    int index=single?selected.load():cursor;
    if(!single)cursor=(cursor+1)%(int)tk->size();
    Voice* voice=nullptr;for(auto& v:voices)if(!v.held&&v.env<.005f){voice=&v;break;}
    if(!voice)voice=&voices[0];*voice=Voice{};voice->held=true;voice->note=msg.getNoteNumber();voice->token=index;voice->tokenSnapshot=tk;voice->smoothFreq=juce::MidiMessage::getMidiNoteInHertz(msg.getNoteNumber());activeSyllable.store(index);
   }else if(msg.isNoteOff()){
    for(auto& v:voices)if(v.held&&v.note==msg.getNoteNumber())v.held=false;
   }else if(msg.isAllNotesOff()||msg.isAllSoundOff())for(auto& v:voices)v.held=false;
   ++it;
  }
  float s=0;
  if(!hold)for(auto& v:voices)if(v.held && v.age>0.42)v.held=false;
  for(auto& v:voices){if(v.held||v.env>.0005f){float freq=juce::MidiMessage::getMidiNoteInHertz(v.note);float glideSeconds=.001f+glide*.7f;v.smoothFreq+=(freq-v.smoothFreq)*(1.f-std::exp(-1.f/(float)(sampleRate*glideSeconds)));s+=synthSample(v,(float)v.smoothFreq,character,breath,vibrato,formant,mode);}}
  s=std::tanh(s*gain*1.8f);
  for(int ch=0;ch<buffer.getNumChannels();++ch)buffer.setSample(ch,i,s);
 }
 midi.clear();
}
void TalkTronProcessor::getStateInformation(juce::MemoryBlock& dest){
 auto tree=parameters.copyState();tree.setProperty("lyrics",getLyrics(),nullptr);tree.setProperty("selected",selected.load(),nullptr);
 auto xml=tree.createXml();copyXmlToBinary(*xml,dest);
}
void TalkTronProcessor::setStateInformation(const void* data,int bytes){
 auto xml=getXmlFromBinary(data,bytes);if(!xml)return;
 auto tree=juce::ValueTree::fromXml(*xml);if(!tree.isValid())return;
 auto lyrics=tree.getProperty("lyrics","I want you tonight").toString();int sel=(int)tree.getProperty("selected",0);
 parameters.replaceState(tree);setLyrics(lyrics);setSelectedSyllable(sel);
}
juce::AudioProcessorEditor* TalkTronProcessor::createEditor(){return new TalkTronEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new TalkTronProcessor();}

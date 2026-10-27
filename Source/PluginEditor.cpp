#include "PluginEditor.h"
#include <cmath>

namespace { const juce::Colour bg(0xff171923), silver(0xffb9bdc7), gold(0xffd5b94b); }

TimeFreezeAudioProcessorEditor::TimeFreezeAudioProcessorEditor(TimeFreezeAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(695, 480);
    titleLabel.setText("TIMEFREEZE", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(22.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(titleLabel);
    presetLabel.setText("Default Preset", juce::dontSendNotification);
    presetLabel.setColour(juce::Label::textColourId, gold);
    presetLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(presetLabel);
    auto styleButton=[](juce::TextButton& b, juce::Colour c) {
        b.setColour(juce::TextButton::buttonColourId,c);
        b.setColour(juce::TextButton::textColourOffId,juce::Colours::white);
        b.setColour(juce::TextButton::textColourOnId,juce::Colours::white);
    };
    styleButton(freezeButton,juce::Colour(0xff777b83)); styleButton(bypassButton,juce::Colour(0xff303440));
    styleButton(limiterButton,juce::Colour(0xff303440)); styleButton(resetButton,juce::Colour(0xff303440));
    addAndMakeVisible(freezeButton); addAndMakeVisible(bypassButton); addAndMakeVisible(limiterButton); addAndMakeVisible(resetButton);
    freezeAttachment=std::make_unique<ButtonAttachment>(p.parameters,"freeze",freezeButton);
    bypassAttachment=std::make_unique<ButtonAttachment>(p.parameters,"bypass",bypassButton);
    limiterAttachment=std::make_unique<ButtonAttachment>(p.parameters,"limiter",limiterButton);
    resetButton.onClick=[this]{ audioProcessor.resetFreeze(); };
    modeBox.addItemList(juce::StringArray{"Normal","2 Bars","Loop","Ping Pong"},1);
    addAndMakeVisible(modeBox);
    modeAttachment=std::make_unique<ComboAttachment>(p.parameters,"mode",modeBox);
    configureSlider(input,inputLabel,"Input Gain"); configureSlider(output,outputLabel,"Output Gain");
    configureSlider(overlap,overlapLabel,"Overlap"); configureSlider(grain,grainLabel,"Grain Size");
    configureSlider(speed,speedLabel,"Speed"); configureSlider(mix,mixLabel,"Spectral Mix");
    configureSlider(smear,smearLabel,"Smear"); configureSlider(volume,volumeLabel,"Volume");
    configureSlider(pitch,pitchLabel,"Pitch"); configureSlider(reducer,reducerLabel,"Reducer");
    const std::pair<juce::Slider*,const char*> attachments[]={{&input,"input"},{&output,"output"},{&overlap,"overlap"},{&grain,"grain"},{&speed,"speed"},{&mix,"mix"},{&smear,"smear"},{&volume,"volume"},{&pitch,"pitch"},{&reducer,"reducer"}};
    for(auto item:attachments) sliderAttachments.push_back(std::make_unique<SliderAttachment>(p.parameters,item.second,*item.first));
    startTimerHz(20);
}
void TimeFreezeAudioProcessorEditor::configureSlider(juce::Slider& s,juce::Label& l,const juce::String& name)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,18);
    s.setColour(juce::Slider::rotarySliderFillColourId,gold); s.setColour(juce::Slider::rotarySliderOutlineColourId,juce::Colour(0xff454957));
    s.setColour(juce::Slider::thumbColourId,silver); s.setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);
    s.setColour(juce::Slider::textBoxBackgroundColourId,bg); addAndMakeVisible(s);
    l.setText(name,juce::dontSendNotification); l.setFont(juce::Font(11.0f)); l.setColour(juce::Label::textColourId,silver);
    l.setJustificationType(juce::Justification::centred); addAndMakeVisible(l);
}
void TimeFreezeAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    juce::ColourGradient metal(juce::Colour(0xff555a64),0,0,juce::Colour(0xff151720),0,(float)getHeight(),false);
    g.setGradientFill(metal); g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(2.0f),16.0f);
    g.setColour(juce::Colour(0xff08090d)); g.fillRoundedRectangle(115,64,465,158,12.0f);
    g.setColour(juce::Colour(0xff242939)); g.fillRoundedRectangle(121,70,453,146,8.0f);
    g.setColour(juce::Colour(0xff3b4050));
    for(int i=0;i<8;++i) g.drawVerticalLine(125+i*63,76.0f,211.0f);
    for(int i=0;i<5;++i) g.drawHorizontalLine(78+i*32,124.0f,570.0f);
    juce::Path curve,fill; curve.startNewSubPath(137.0f,92.0f); fill.startNewSubPath(137.0f,92.0f);
    for(int i=1;i<=100;++i){ float x=137.0f+i*4.15f; float y=92.0f+120.0f*(1.0f-std::exp(-i/31.0f)); curve.lineTo(x,y); fill.lineTo(x,y); }
    fill.lineTo(552.0f,211.0f); fill.lineTo(137.0f,211.0f); fill.closeSubPath();
    g.setColour(juce::Colour(0x44d5b94b)); g.fillPath(fill); g.setColour(gold); g.strokePath(curve,juce::PathStrokeType(2.0f));
    g.setColour(juce::Colours::white.withAlpha(0.75f)); g.setFont(10.0f);
    g.drawText("Seq     Time     Pitch     Misc 1     Misc 2     Misc 3",135,68,420,16,juce::Justification::centredLeft);
    for(int i=0;i<4;++i) g.drawText(juce::StringArray{"Normal","2 Bars","Loop","Ping Pong"}[i],508,84+i*18,55,15,juce::Justification::centred);
    for(int side:{18,604}) {
        g.setColour(juce::Colour(0xff090a0f)); g.fillRoundedRectangle((float)side,70.0f,69.0f,170.0f,6.0f);
        g.setColour(juce::Colour(0xff343947)); g.fillRoundedRectangle((float)side+5,76.0f,59.0f,158.0f,4.0f);
        g.setColour(juce::Colour(0xff656b79)); for(int i=0;i<14;++i) g.drawHorizontalLine(80+i*11,(float)side+8,(float)side+61);
    }
    juce::Rectangle<float> knob(286,237,122,122);
    g.setColour(juce::Colour(0xff0a0b10)); g.fillEllipse(knob.expanded(3));
    juce::ColourGradient knobGrad(juce::Colour(0xfff1f2f3),knob.getX(),knob.getY(),juce::Colour(0xff5d626c),knob.getRight(),knob.getBottom(),false);
    g.setGradientFill(knobGrad); g.fillEllipse(knob); g.setColour(juce::Colour(0xff171923)); g.drawEllipse(knob.reduced(7),2.0f);
    g.setColour(juce::Colour(0xff22242b)); g.setFont(juce::Font(22.0f,juce::Font::bold)); g.drawText("FREEZE",knob.toNearestInt(),juce::Justification::centred);
    g.setColour(juce::Colour(0xff0a0b10)); g.fillRect(0,0,getWidth(),43); g.setColour(gold); g.drawRect(58,9,230,24,1);
    g.setColour(juce::Colours::white.withAlpha(0.65f)); g.setFont(10.0f); g.drawText("PRESET",10,12,43,16,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff0a0b10)); g.fillRect(0,456,getWidth(),24); g.setColour(silver);
    g.setFont(11.0f); g.drawText("TIMEFREEZE  •  GRANULAR AUDIO EFFECT",18,459,340,16,juce::Justification::centredLeft);
    g.setColour(gold); g.drawText("TF",638,457,38,20,juce::Justification::centred);
}
void TimeFreezeAudioProcessorEditor::resized()
{
    titleLabel.setBounds(420,7,255,28); presetLabel.setBounds(65,9,220,24);
    bypassButton.setBounds(300,8,68,26); limiterButton.setBounds(372,8,66,26); modeBox.setBounds(495,84,73,75);
    freezeButton.setBounds(304,278,88,38); resetButton.setBounds(309,386,76,24);
    input.setBounds(16,238,92,84); inputLabel.setBounds(12,320,100,20); output.setBounds(587,238,92,84); outputLabel.setBounds(583,320,100,20);
    overlap.setBounds(135,231,62,62); overlapLabel.setBounds(126,291,80,18); grain.setBounds(207,235,66,66); grainLabel.setBounds(200,298,80,18);
    speed.setBounds(203,322,66,62); speedLabel.setBounds(197,383,80,18); mix.setBounds(430,231,66,62); mixLabel.setBounds(423,291,80,18);
    smear.setBounds(500,235,66,66); smearLabel.setBounds(493,298,80,18); volume.setBounds(420,322,66,62); volumeLabel.setBounds(413,383,80,18);
    pitch.setBounds(132,352,66,62); pitchLabel.setBounds(126,413,80,18); reducer.setBounds(499,352,66,62); reducerLabel.setBounds(493,413,80,18);
}
void TimeFreezeAudioProcessorEditor::timerCallback()
{
    elapsed = audioProcessor.parameters.getRawParameterValue("freeze")->load() > 0.5f ? elapsed + 0.05 : 0.0;
    repaint();
}

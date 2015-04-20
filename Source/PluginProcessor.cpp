/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ulib_pluginhost.h"



//==============================================================================
ChuckPluginTest4AudioProcessor::ChuckPluginTest4AudioProcessor()
{
    fprintf(stderr, "ChuckPluginTest4AudioProcessor::ChuckPluginTest4AudioProcessor\n");
    chuck_options options;
    libchuck_options_reset(&options);
//    options.buffer_size = getBlockSize();
//    options.adaptive_buffer_size = 0;
//    options.num_channels = getNumOutputChannels();
//    options.sample_rate = getSampleRate();
    options.slave = true;
    // hardcode (HACK!)
    options.buffer_size = 512;
    options.adaptive_buffer_size = 0;
    options.num_channels = 2;
    options.sample_rate = 44100;

    
    ck = libchuck_create(&options);
    
    libchuck_add_module(ck, (void*)pluginhost_query);
    
    input_buffer = new float[options.buffer_size*options.num_channels];
    output_buffer = new float[options.buffer_size*options.num_channels];
    
    libchuck_vm_start(ck);
    
    codeEditorDemo = new CodeEditorDemo();
    
    wasPlaying=false;
    current16th=-1;
}

ChuckPluginTest4AudioProcessor::~ChuckPluginTest4AudioProcessor()
{
    fprintf(stderr, "ChuckPluginTest4AudioProcessor::~ChuckPluginTest4AudioProcessor\n");

    libchuck_destroy(ck);
    ck = NULL;
    
    if(input_buffer) { delete[] input_buffer; input_buffer = NULL; }
    if(output_buffer) { delete[] output_buffer; output_buffer = NULL; }
}

//==============================================================================
const String ChuckPluginTest4AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int ChuckPluginTest4AudioProcessor::getNumParameters()
{
    return 0;
}

float ChuckPluginTest4AudioProcessor::getParameter (int index)
{
    return 0.0f;
}

void ChuckPluginTest4AudioProcessor::setParameter (int index, float newValue)
{
}

const String ChuckPluginTest4AudioProcessor::getParameterName (int index)
{
    return String::empty;
}

const String ChuckPluginTest4AudioProcessor::getParameterText (int index)
{
    return String::empty;
}

const String ChuckPluginTest4AudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String ChuckPluginTest4AudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool ChuckPluginTest4AudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool ChuckPluginTest4AudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool ChuckPluginTest4AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ChuckPluginTest4AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ChuckPluginTest4AudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double ChuckPluginTest4AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ChuckPluginTest4AudioProcessor::getNumPrograms()
{
    return 1;
}

int ChuckPluginTest4AudioProcessor::getCurrentProgram()
{
    return 0;
}

void ChuckPluginTest4AudioProcessor::setCurrentProgram (int index)
{
}

const String ChuckPluginTest4AudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void ChuckPluginTest4AudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void ChuckPluginTest4AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ChuckPluginTest4AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void ChuckPluginTest4AudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{    
    // Get current position/time info from host, otherwise set to some default
    AudioPlayHead::CurrentPositionInfo pos;
    if (getPlayHead() != nullptr && getPlayHead()->getCurrentPosition(pos)) {
        lastPosInfo = pos;
    }else{
        lastPosInfo.resetToDefault();
    }
    
    if (lastPosInfo.bpm != previousTempo)
    {
        g_hostInfo->setTempo(lastPosInfo.bpm);
    }
    
    if(lastPosInfo.isPlaying&&!wasPlaying)
    {
        g_hostInfo->broadcastPlayEvent();
    }
    
    
    //DBG(pos.ppqPosition);

    positionInBeat=fmod(pos.ppqPosition,1);
    DBG(positionInBeat);
    if (positionInBeat>0.749) //sixteenth
    {
        if (current16th!=3)
        {
            DBG("16th3");
            g_hostInfo->broadcast16thHit();
            current16th=3;
        }
    }
    else if (positionInBeat>0.499)
    {
        if (current16th!=2)
        {
            DBG("16th2");
            g_hostInfo->broadcast16thHit();
            current16th=2;
        }
    }
    else if (positionInBeat>0.249)
    {
        if (current16th!=1)
        {
            DBG("16th1");
            g_hostInfo->broadcast16thHit();
            current16th=1;
        }
    }
    else if (positionInBeat>0.0)
    {
        if (current16th!=0)
        {
            DBG("16th0");
            g_hostInfo->broadcast16thHit();
            g_hostInfo->broadcastBeatStartEvent();

            current16th=0;
        }
    }
        // DBG(pos.ppqPosition);
       // DBG("16th!");
    
    previousTempo=lastPosInfo.bpm;
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // I've added this to avoid people getting screaming feedback
    // when they first compile the plugin, but obviously you don't need to
    // this code if your algorithm already fills all the output channels.
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // copy input
    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);
        
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            input_buffer[i*2+channel] = channelData[i];
        }
    }
    
    libchuck_slave_process(ck, input_buffer, output_buffer, buffer.getNumSamples());
    
    // copy output
    for (int channel = 0; channel < getNumOutputChannels(); ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);

        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            channelData[i] = output_buffer[i*2+channel];
        }
    }

    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
//    for (int channel = 0; channel < getNumInputChannels(); ++channel)
//    {
//        float* channelData = buffer.getWritePointer (channel);
//
//        // ..do something to the data...
//    }
}

//==============================================================================
bool ChuckPluginTest4AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ChuckPluginTest4AudioProcessor::createEditor()
{
    return new ChuckPluginTest4AudioProcessorEditor (this);
}

//==============================================================================
void ChuckPluginTest4AudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ChuckPluginTest4AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChuckPluginTest4AudioProcessor();
}

void ChuckPluginTest4AudioProcessor::testAddShred()
{
    libchuck_add_shred(ck, "SqrOsc", "SqrOsc osc => dac; 1::day => now;");
}

void ChuckPluginTest4AudioProcessor::addShred()
{
    //libchuck_add_shred(ck, fileManager.filePath, NULL);
    //libchuck_add_shred(ck, fileManager.filePath.toRawUTF8(), NULL);
    
    //libchuck_add_shred(ck, codeEditorDemo->fileChooser.getResult().getFullPathName().toRawUTF8(),
                       //codeEditorDemo->codeDocument.getAllContent().toRawUTF8());
    
    
    libchuck_add_shred(ck, codeEditorDemo->fileChooser.getCurrentFile().getFullPathName().toRawUTF8(),
                       codeEditorDemo->codeDocument.getAllContent().toRawUTF8());
    //String name="SqrOsc";
    //libchuck_add_shred(ck, name, "SqrOsc osc => dac; 1::day => now;");
    

}




void ChuckPluginTest4AudioProcessor::removeShred()
{
    libchuck_remove_shred(ck, 1);
}
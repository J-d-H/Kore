#include "pch.h"
#include "Audio.h"
#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#include <stdio.h>

using namespace Kore;

#define kOutputBus 0

namespace {
	const int samplesPerSecond = 44100;
	
	void affirm(OSStatus err) {
		if (err) {
		//	fprintf(stderr, "Error: %i\n", err);
		}
	}
	
	bool initialized;
	bool soundPlaying;
	AudioStreamBasicDescription deviceFormat;
	AudioComponentInstance audioUnit;
	
	void copySample(void* buffer) {
		float value = *(float*)&Audio::buffer.data[Audio::buffer.readLocation];
		Audio::buffer.readLocation += 4;
		if (Audio::buffer.readLocation >= Audio::buffer.dataSize) Audio::buffer.readLocation = 0;
		*(s16*)buffer = static_cast<s16>(value * 32767);
	}
	
	OSStatus renderInput(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList* outOutputData) {
		Audio::audioCallback(inNumberFrames * 2);
		s16* out = (s16*)outOutputData->mBuffers[0].mData;
		for (int i = 0; i < inNumberFrames; ++i) {
			copySample(out++); //left
			copySample(out++); //right
		}
		return noErr;
	}
}

void Audio::init() {
	buffer.readLocation = 0;
	buffer.writeLocation = 0;
	buffer.dataSize = 128 * 1024;
	buffer.data = new u8[buffer.dataSize];
	
	initialized = false;
	
	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_RemoteIO;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	
	AudioComponent comp = AudioComponentFindNext(nullptr, &desc);
	
	// Get audio units
	affirm(AudioComponentInstanceNew(comp, &audioUnit));
	UInt32 flag = 1;
	affirm(AudioUnitSetProperty(audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, kOutputBus, &flag, sizeof(UInt32)));
	
	UInt32 size = sizeof(AudioStreamBasicDescription);
	affirm(AudioUnitGetProperty(audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &deviceFormat, &size));
	
	if (deviceFormat.mFormatID != kAudioFormatLinearPCM) {
		fprintf(stderr, "mFormatID !=  kAudioFormatLinearPCM\n");
		return;
	}

	if (!(deviceFormat.mFormatFlags & kLinearPCMFormatFlagIsSignedInteger)) {
		fprintf(stderr, "Only works with signed integer format.\n");
		return;
	}

	initialized = true;

	fprintf(stderr, "mSampleRate = %g\n", deviceFormat.mSampleRate);
	fprintf(stderr, "mFormatFlags = %08X\n", (unsigned int)deviceFormat.mFormatFlags);
	fprintf(stderr, "mBytesPerPacket = %d\n", (unsigned int)deviceFormat.mBytesPerPacket);
	fprintf(stderr, "mFramesPerPacket = %d\n", (unsigned int)deviceFormat.mFramesPerPacket);
	fprintf(stderr, "mChannelsPerFrame = %d\n", (unsigned int)deviceFormat.mChannelsPerFrame);
	fprintf(stderr, "mBytesPerFrame = %d\n", (unsigned int)deviceFormat.mBytesPerFrame);
	fprintf(stderr, "mBitsPerChannel = %d\n", (unsigned int)deviceFormat.mBitsPerChannel);
	
	if (soundPlaying) return;

	AURenderCallbackStruct callbackStruct;
	callbackStruct.inputProc = renderInput;
	callbackStruct.inputProcRefCon = nullptr;
	affirm(AudioUnitSetProperty(audioUnit,
								  kAudioUnitProperty_SetRenderCallback,
								  kAudioUnitScope_Global,
								  kOutputBus,
								  &callbackStruct,
								  sizeof(callbackStruct)));
	
	affirm(AudioOutputUnitStart(audioUnit));
	
	soundPlaying = true;
}

void Audio::update() {
	
}

void Audio::shutdown() {
	if (!initialized) return;
	if (!soundPlaying) return;

	affirm(AudioOutputUnitStop(audioUnit));
	
	soundPlaying = false;
}
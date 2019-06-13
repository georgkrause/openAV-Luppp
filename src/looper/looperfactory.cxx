#include "looperfactory.hxx"
#include "looper.hxx"
#include "looperclip.hxx"
#include "audiobuffer.hxx"

Looper *LooperFactory::buildLooper ( int t ) {
	LooperClip *clips[NSCENES];
	for ( int i = 0; i < NSCENES;  i++) {
		AudioBuffer *buffer =
			new AudioBuffer ( LOOPER_SAMPLES_UPDATE_SIZE );
		clips[i] = new LooperClip ( t, i, buffer );
    }
		return new Looper ( t, clips );
}
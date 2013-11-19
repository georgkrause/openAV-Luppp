
#include "diskreader.hxx"

#include <sstream>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <libgen.h>
#include <sys/stat.h>

#include "config.hxx"
#include "gui.hxx"
#include "event.hxx"
#include "audiobuffer.hxx"
#include "eventhandler.hxx"
#include "gmastertrack.hxx"

#include "controller/genericmidi.hxx"

#include <sndfile.hh>
#include <samplerate.h>

extern Gui* gui;

using namespace std;

DiskReader::DiskReader()
{
}

int DiskReader::loadPreferences()
{
  stringstream s;
  s << getenv("HOME") << "/.config/openAV/luppp/luppp.prfs";
  std::ifstream sampleFile( s.str().c_str(), std::ios_base::in|std::ios_base::ate);
  
  long file_length = sampleFile.tellg();
  if ( file_length > 0 )
  {
    sampleFile.seekg(0, std::ios_base::beg);
    sampleFile.clear();
    char *sampleString = new char[file_length];
    sampleFile.read(sampleString, file_length);
    
    cJSON* preferencesJson = cJSON_Parse( sampleString );
    if (!preferencesJson)
    {
      return LUPPP_RETURN_ERROR;
    }
    
    cJSON* ctlrs = cJSON_GetObjectItem( preferencesJson, "defaultControllers" );
    if ( ctlrs )
    {
      //cout << ".ctlr HAS items." << endl;
      int nCtlrs = cJSON_GetArraySize( ctlrs );
      for(int i = 0; i < nCtlrs; i++ )
      {
        cJSON* ctlr = cJSON_GetArrayItem( ctlrs, i );
        if( ctlr )
        {
          LUPPP_NOTE("Loading controller %s", ctlr->valuestring);
          stringstream s;
          s << getenv("HOME") << "/.config/openAV/luppp/controllers/" << ctlr->valuestring;
          GenericMIDI* c = new GenericMIDI( s.str().c_str() );
          EventControllerInstance e(c);
          writeToDspRingbuffer( &e );
        }
      }
    }
    else
    {
      LUPPP_NOTE("No default controllers active.");
    }
    
    cJSON_Delete( preferencesJson );
    delete[] sampleString;
  }
  else
  {
    // empty file / file no exists:
    return LUPPP_RETURN_ERROR;
  }
  
  return LUPPP_RETURN_OK;
}

int DiskReader::showAudioEditor(AudioBuffer* ab)
{
  while ( ab->getBeats() == 0 )
  {
    // FIXME: Cancel = no load sample?
    gui->getAudioEditor()->show( ab, true );
    
    while ( gui->getAudioEditor()->shown() ) Fl::wait();
    
    // handle "cancel" return
    if ( ab->getBeats() == -1 )
    {
      return LUPPP_RETURN_ERROR;
    }
  }
  
  return LUPPP_RETURN_OK;
}


int DiskReader::loadSample( int track, int scene, string path )
{
  /// load the sample
  SndfileHandle infile( path, SFM_READ );
  std::vector<float> buf( infile.frames() );
  infile.read( (float*)&buf[0] , infile.frames() );
  
  LUPPP_NOTE("Loading file with  %i chnls, frames %i,buffer size %i", infile.channels(), infile.frames(), buf.size() );
  
  /// kick stereo channel?
  int chnls = infile.channels();
  if ( chnls > 1 )
  {
    // we're gonna kick all samples that are *not* channel 1
    std::vector<float> tmp( buf.size() / chnls );
    
    LUPPP_NOTE("Non mono file: %i chnls found, old size %i, new size %i ", infile.channels(), buf.size(), tmp.size() );
    
    for(size_t i = 0; i < tmp.size(); i++ )
    {
      tmp.at(i) = buf.at( i * chnls );
    }
    
    // swap the buffers
    buf.swap( tmp );
  }
  
  /// resample?
  if ( infile.samplerate() != gui->samplerate )
  {
    LUPPP_NOTE("%s%i%s%i", "Resampling from ", infile.samplerate(), " to ", gui->samplerate);
    
    float resampleRatio = float(gui->samplerate) / infile.samplerate();
    std::vector<float> resampled( infile.frames() * resampleRatio );
    
    SRC_DATA data;
    data.data_in  = &buf[0];
    data.data_out = &resampled[0];
    
    data.input_frames = infile.frames();
    data.output_frames = infile.frames() * resampleRatio;
    
    data.end_of_input = 0;
    data.src_ratio = resampleRatio;
    
    int ret = src_simple ( &data, SRC_SINC_FASTEST, 1 );
    
    LUPPP_NOTE("%s%i%s%i", "Resampling finished, from ", data.input_frames_used, " to ", data.output_frames_gen );
    
    /// exchange buffers, so buf contains the resampled audio
    buf.swap( resampled );
  }
  
  
  /// create buffer, and set the data
  AudioBuffer* ab = new AudioBuffer();
  ab->setAudioFrames( buf.size() );
  ab->nonRtSetSample( buf );
  
  //cout << "DiskReader::loadSample() " << path << endl;
  
  bool loadableBuffer = false;
  
  if ( infile.frames() > 0 )
  {
    char* basePath = strdup( path.c_str() );
    stringstream base;
    base << dirname( basePath ) << "/audio.cfg";
    
    free( basePath );
    
    /// open audio.cfg, reading whole file
#ifdef DEBUG_STATE
    cout << "loading sample metadata file " << base.str().c_str() << endl;
#endif
    std::ifstream sampleFile( base.str().c_str(), std::ios_base::in|std::ios_base::ate);
    long file_length = sampleFile.tellg();
    if ( file_length > 0 )
    {
      sampleFile.seekg(0, std::ios_base::beg);
      sampleFile.clear();
      char *sampleString = new char[file_length];
      sampleFile.read(sampleString, file_length);
      
      //cout << "Sample file:" << endl << sampleString << endl;
      //cout << "Sample file (parsed):" << endl << cJSON_Parse( sampleString ) << endl;
      
      cJSON* audioJson = cJSON_Parse( sampleString );
      if (!audioJson) {
        LUPPP_ERROR("%s %s","Error in Sample JSON before: ", cJSON_GetErrorPtr() );
        return LUPPP_RETURN_ERROR;
      }
      
      // retrieve sample metadata from sample.cfg using filename as key
      char* tmp = strdup( path.c_str() );
      char* baseName = basename( tmp );
      //cout << "tmp " << tmp << " baseName " << baseName << endl;
      
      cJSON* sample = cJSON_GetObjectItem( audioJson, baseName );
      if ( sample )
      {
        cJSON* beats = cJSON_GetObjectItem( sample, "beats" );
        cJSON* name  = cJSON_GetObjectItem( sample, "name" );
        //cout << "Clip @ " << track << " " << scene << " gets " << beats->valuedouble << " beats."<< endl;
        if ( beats )
        {
          loadableBuffer = true;
          ab->setBeats( beats->valuedouble );
        }
        if ( name )
          gui->getTrack(track)->getClipSelector()->clipName( scene, name->valuestring );
      }
      
      // if we don't find the beats from audio.cfg, show dialog
      if ( loadableBuffer == false )
      {
        cout << "Warning: audio.cfg has no entry for beats." << endl;
        int ret = showAudioEditor( ab );
        if ( ret == LUPPP_RETURN_OK )
        {
          // flag that we can load this sample OK
          loadableBuffer = true;
        }
        else
        {
          delete ab;
        }
      }
      
      cJSON_Delete( audioJson );
      free ( sampleString  );
      free ( tmp );
    }
    else
    {
      // this means there's no audio.cfg file found for the sample: show the user
      // the file, and ask what the intended beat number is, and load the AudioBuffer
      LUPPP_WARN("%s %s","Empty or no audio.cfg found at ",base.str().c_str() );
      int error = showAudioEditor( ab );
      if ( error == LUPPP_RETURN_ERROR )
      {
        LUPPP_WARN("cancel clicked, deleting audiobuffer" );
        delete ab;
      }
    }
    
    if ( loadableBuffer )
    {
      // write audioBuffer to DSP
      EventLooperLoad e = EventLooperLoad( track, scene, ab );
      writeToDspRingbuffer( &e );
    }
    else
    {
      LUPPP_NOTE("AudioBuffer not loaded, missing beats info and dialog was Canceled" );
    }
  }
  
  return LUPPP_RETURN_OK;
  
}

int DiskReader::readSession( std::string path )
{
  cout << "DiskReader::readSession() " << path << endl;
  sessionPath = path;
  
  stringstream s;
  s << path << "/session.luppp";
  
  stringstream samplePath;
  samplePath << path << "/audio/audio.cfg";
  
  cout << "session path: " << s.str() << endl;
  cout << "audio path:  " << samplePath.str() << endl;
  
  // open session, read all
  std::ifstream file( s.str().c_str(), std::ios_base::in|std::ios_base::ate);
  long file_length = file.tellg();
  if ( file_length < 0 )
  {
    // empty file / file no exists:
    LUPPP_ERROR("no session file exists!");
    return LUPPP_RETURN_ERROR;
  }
  
  file.seekg(0, std::ios_base::beg);
  file.clear();
  char *sessionString = new char[file_length];
  file.read(sessionString, file_length);
  
  // create cJSON nodes from strings
  sessionJson = cJSON_Parse( sessionString );
  if (!sessionJson) {
    LUPPP_ERROR("%s %s", "Error in Session JSON before: ", cJSON_GetErrorPtr() );
    return LUPPP_RETURN_ERROR;
  }
  
  
  int tr = readTracks();
  
  int mr = readMaster();
  
  // cleanup
  cJSON_Delete( sessionJson );
  free ( sessionString );
  
  return LUPPP_RETURN_OK;
}

int DiskReader::readMaster()
{
  cJSON* master = cJSON_GetObjectItem( sessionJson, "master");
  if ( master )
  {
    // bpm
    { 
      cJSON* bpm = cJSON_GetObjectItem( master, "bpm");
      LUPPP_NOTE("%s %i","Session: BPM ",bpm->valueint); 
      
      EventTimeBPM e( bpm->valuedouble );
      writeToDspRingbuffer( &e );
    }
    // fader
    { 
      cJSON* fader = cJSON_GetObjectItem( master, "fader");
      EventTrackVol e( -1, fader->valuedouble );
      writeToDspRingbuffer( &e );
    }
    // reverb
    {
      cJSON* reverb = cJSON_GetObjectItem( master, "reverb");
      if ( reverb )
      {
        cJSON* active  = cJSON_GetObjectItem( reverb, "active");
        cJSON* size    = cJSON_GetObjectItem( reverb, "size");
        cJSON* wet     = cJSON_GetObjectItem( reverb, "wet");
        cJSON* damping = cJSON_GetObjectItem( reverb, "damping");
        
        if ( active && size && wet && damping )
        {
          EventFxReverb e(  active->valuedouble, size->valuedouble,
                            wet->valuedouble, damping->valuedouble );
          writeToDspRingbuffer( &e );
        }
      }
      else
      {
#ifdef DEBUG_STATE
        cout << "Session has no reverb element" << endl;
#endif
      }
    }
    // TODO add samplerate to session JSON
    
    // sceneNames
    {
      cJSON* names = cJSON_GetObjectItem( master, "sceneNames");
      if ( names )
      {
        GMasterTrack* master = gui->getMasterTrack();
        Avtk::ClipSelector* clipSelector = master->getClipSelector();
        int nscenes = cJSON_GetArraySize( names );
        for(int s = 0; s < nscenes; s++ )
        {
          cJSON* name = cJSON_GetArrayItem( names, s );
          clipSelector->clipName( s, name->valuestring );
        }
        clipSelector->redraw();
      }
    }
    
  }
  else
  {
    LUPPP_ERROR("%s", "Error getting master from JSON" );
    return LUPPP_RETURN_ERROR;
  }
  
  return LUPPP_RETURN_OK;
}


int DiskReader::readScenes(int t, cJSON* track)
{
  cJSON* clips = cJSON_GetObjectItem( track, "clips");
  if ( clips )
  {
    
    int nClips = cJSON_GetArraySize( clips );
    for(int s = 0; s < nClips; s++ )
    {
      // get metadata for Clip
      cJSON* clip = cJSON_GetArrayItem( clips, s );
      
      if ( !strcmp(clip->valuestring, "") == 0 )
      {
        stringstream sampleFilePath;
        sampleFilePath << sessionPath << "/audio/" << clip->valuestring;
#ifdef DEBUG_STATE
        LUPPP_NOTE << "clip t " << t << " s " << s << " path " << sampleFilePath.str() << endl;
#endif
        // load it, checking for sample.cfg, and using metadata if there
        loadSample( t, s, sampleFilePath.str() );
      }
      
      // FIXME: check GUI ringbuffer after each sample: with resampling it can
      // take quite some time, and the ->GUI ringbuffer can fill up
      handleGuiEvents();
      
    } // nClips loop
  }
  
  return LUPPP_RETURN_OK;
}

int DiskReader::readTracks()
{
  cJSON* tracks = cJSON_GetObjectItem( sessionJson, "tracks");
  if ( tracks )
  {
    int nTracks = cJSON_GetArraySize( tracks );
    for(int t = 0; t < nTracks; t++ )
    {
      cJSON* track = cJSON_GetArrayItem( tracks, t );
      
      if( !track )
      {
        LUPPP_WARN("Track %i has no name track saved.", t);
      }
      else
      {
        readScenes( t, track );
        
        // name
        {
          cJSON* name = cJSON_GetObjectItem( track, "name");
          if( !name )
          {
            LUPPP_WARN("Track %i has no name data saved.", t);
          }
          else
          {
            gui->getTrack(t)->bg.setLabel( name->valuestring );
          }
        }
        // fader
        { 
          cJSON* fader = cJSON_GetObjectItem( track, "fader");
          if( !fader )
          {
            LUPPP_WARN("Track %i has no fader data saved.", t);
          }
          else
          {
            EventTrackVol e( t, fader->valuedouble );
            writeToDspRingbuffer( &e );
          }
        }
        // sends
        { 
          cJSON* send       = cJSON_GetObjectItem( track, "sendAmount");
          cJSON* sendActive = cJSON_GetObjectItem( track, "sendActive");
          
          cJSON* xside      = cJSON_GetObjectItem( track, "xsideAmount");
          cJSON* keyActive  = cJSON_GetObjectItem( track, "keyActive");
          
          if( !send || !sendActive || !xside || !keyActive )
          {
            LUPPP_WARN("Track %i has no send data saved.", t);
          }
          else
          {
            EventTrackSendActive e1( t, SEND_POSTFADER, sendActive->valueint );
            EventTrackSendActive e2( t, SEND_KEY      , keyActive ->valueint );
            
            EventTrackSend e3( t, SEND_XSIDE     , xside->valuedouble );
            EventTrackSend e4( t, SEND_POSTFADER , send->valuedouble  );
            
            
            writeToDspRingbuffer( &e1 );
            writeToDspRingbuffer( &e2 );
            writeToDspRingbuffer( &e3 );
          }
        }
      }// if track
    } // nTracks loop
  }
  else
  {
    LUPPP_ERROR("%s", "Error getting clip" );
    return LUPPP_RETURN_ERROR;
  }
  return LUPPP_RETURN_OK;
}

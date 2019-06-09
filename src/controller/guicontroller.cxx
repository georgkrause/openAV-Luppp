/*
 * Author: Harry van Haaren 2013
 *         harryhaaren@gmail.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "guicontroller.hxx"

#include <iostream>

#include "../jack.hxx"
#include "../event.hxx"
#include "../gridlogic.hxx"

#include "../eventhandler.hxx"

extern Jack* jack;

LooppGUI::LooppGUI() :
	Controller()
{
}

void LooppGUI::masterInputVol(float f)
{
	EventMasterInputVol e( f );
	writeToGuiRingbuffer( &e );
}

void LooppGUI::masterInputTo(int to,float f)
{
	EventMasterInputTo e( (Event::INPUT_TO)to, f );
	writeToGuiRingbuffer( &e );
}

void LooppGUI::masterInputToActive(int to,float f)
{
	EventMasterInputToActive e( (Event::INPUT_TO)to, f );
	writeToGuiRingbuffer( &e );
}

void LooppGUI::masterVolume(float f)
{
	EventMasterVol e( f );
	writeToGuiRingbuffer( &e );
}

void LooppGUI::masterReturnVolume(float f)
{
	printf(" return %f ", f );
	EventMasterReturn e( RETURN_MAIN, f );
	writeToGuiRingbuffer( &e );
}

void LooppGUI::metronomeEnable(bool r)
{
	EventMetronomeActive e( r );
	writeToGuiRingbuffer( &e );
}

void LooppGUI::recordArm(int t, bool r)
{
	EventTrackRecordArm e( t, r );
	writeToGuiRingbuffer( &e );
}

void LooppGUI::trackSend(int t, int send, float r)
{
	EventTrackSend e( t, static_cast<Event::SEND_TYPE>(send), r );
	writeToGuiRingbuffer( &e );
}

void LooppGUI::specialScene(int t, int s)
{
	EventGridSelectNewChosen e( t, s );
	writeToGuiRingbuffer( &e );
}

void LooppGUI::trackSendActive(int t, int send, bool a)
{
	EventTrackSendActive e( t, static_cast<Event::SEND_TYPE>(send), a );
	writeToGuiRingbuffer( &e );
}

void LooppGUI::trackJackSend(int t, float v)
{
	EventTrackJackSend e(t,v);
	writeToGuiRingbuffer(&e);
}

void LooppGUI::trackJackSendActivate(int t, bool a)
{
	EventTrackJackSendActivate e(t,a);
	writeToGuiRingbuffer(&e);
}

void LooppGUI::setSceneState(int t, int clip, GridLogic::State s)
{
	EventGridState e( t, clip, s );
	writeToGuiRingbuffer( &e );
}

void LooppGUI::launchScene( int scene )
{
	EventGridLaunchScene e( scene );
	writeToGuiRingbuffer( &e );
}

void LooppGUI::mute(int t, bool b)
{

}

void LooppGUI::tapTempo( bool b )
{
	EventTimeTempoTap e(b);
	writeToGuiRingbuffer( &e );
}

void LooppGUI::bpm(float bpm)
{
	EventTimeBPM e(bpm);
	writeToGuiRingbuffer( &e );
}

void LooppGUI::volume(int t, float f)
{
	EventTrackVol e( t, f );
	writeToGuiRingbuffer( &e );
}

void LooppGUI::pan(int t, float p)
{
	EventTrackPan e( t, p );
	writeToGuiRingbuffer( &e );
}

void LooppGUI::progress(int t, int s, float f)
{
	EventLooperProgress e( t, f );
	writeToGuiRingbuffer( &e );
}

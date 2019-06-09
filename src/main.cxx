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

#include "config.hxx"
#include "version.hxx"

#include <iostream>
#include <unistd.h>
#include <signal.h>

#include <jack/ringbuffer.h>

#include "gui.hxx"
#include "jack.hxx"
#include "event.hxx"
#include "denormals.hxx"

int signalHanlderInt = 0;

EventBase* processDspMem = 0;
EventBase* processGuiMem = 0;

jack_ringbuffer_t* rbToDsp = 0;
jack_ringbuffer_t* rbToGui = 0;

// global static pointers, for access from EventHandlerGui and EventHandlerDsp
Gui * gui  = nullptr;
Jack* jack = nullptr;


void signalHanlder(int signum)
{
	signalHanlderInt = signum;
}

static void gui_static_loadSession_cb(void* inst)
{
	char* tmp = (char*) inst;
	int sess = gui->getDiskReader()->readSession( tmp );
	if ( sess != LOOPP_RETURN_OK )
		LOOPP_ERROR( "Error loading session" );
}



int main(int argc, char** argv)
{
	bool runTests = false;
	if ( runTests ) {} // remove unused warning if not built with BUILD_TESTS

	for(int i = 0; i < argc; i++) {
		if ( strcmp(argv[i], "-test" ) == 0 ) {
			runTests = true;
		} else if ( strcmp(argv[i], "--version" ) == 0  || strcmp(argv[i], "-v" ) == 0 ) {
			printf("%s\n", GIT_VERSION);
			return 0;
		} else if ( i != 0 ) { // don't try load with the program name!
			// assume filename, try load it
			Fl::repeat_timeout( 1 / 30.f, &gui_static_loadSession_cb, argv[i] );
		}
	}

	LOOPP_NOTE("Git: %s", GIT_VERSION   );

	// setup the environment
	AVOIDDENORMALS();
	signal(SIGINT , signalHanlder);
	signal(SIGTERM, signalHanlder);

	// allocate data to read from
	processDspMem = (EventBase*)malloc( sizeof(EventBase) );
	processGuiMem = (EventBase*)malloc( sizeof(EventBase) );

	rbToDsp = jack_ringbuffer_create( 5000 * sizeof(EventBase));
	rbToGui = jack_ringbuffer_create( 5000 * sizeof(EventBase));


#ifdef BUILD_TESTS
	LOOPP_NOTE("Built with BUILD_TESTS enabled");
	if ( runTests ) {
		// counts failures
		int testResult = 0;

		// setup the testing Gui / JACK: Jack first, then GUI
		gui = new Gui( argv[0] );
		Jack::setup("LooppTEST");

		// test offline functionality
		testResult += gui->getDiskReader()->runTests();
		testResult += gui->getDiskWriter()->runTests();

		// test realtime functionality
		testResult += jack->getGridLogic()->runTests();

		jack->quit();

		delete gui;
		delete jack;
		jack = 0;

		// running tests == quitting after testing finishes
		return testResult;
	}

#endif


	// Create a GUI, check for NSM integration
	gui = new Gui( argv[0] );

	if ( gui->getNsm() ) {
		// the NSM OSC Open message will trigger Jack initialization: necessary
		// to use the right name to create the JACK client.
	} else {
		Jack::setup("Loopp");
		jack->activate();
	}

	gui->show();

    delete gui;

	return 0;
}


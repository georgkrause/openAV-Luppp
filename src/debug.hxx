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

#ifndef LOOPP_DEBUG_HXX
#define LOOPP_DEBUG_HXX

#include <stdio.h>
#include <stdarg.h>

/* Example usage
LOOPP_NOTE( "%s", "MessageHere" );
LOOPP_WARN( "%s", "MessageHere" );
LOOPP_KILL( "%s", "MessageHere" );
*/

enum DEBUG_LEVEL {
	DEBUG_LEVEL_NOTE = 0,
	DEBUG_LEVEL_WARN,
	DEBUG_LEVEL_ERROR,
	DEBUG_LEVEL_TEST
};

void loopp_debug( int warnLevel, const char* name, const char* file, const char* func, int line,
                  const char* format = 0, ... );


#define LOOPP_DSP( format, args... ) loopp_debug( DEBUG_LEVEL_NOTE, " DSP ", "", "", 0, format, ## args )

#define LOOPP_NOTE( format, args... ) loopp_debug( DEBUG_LEVEL_NOTE, NAME, __FILE__, __FUNCTION__, __LINE__, format, ## args )
#define LOOPP_WARN( format, args... ) loopp_debug( DEBUG_LEVEL_WARN, NAME, __FILE__, __FUNCTION__, __LINE__, format, ## args )
#define LOOPP_ERROR( format, args... ) loopp_debug( DEBUG_LEVEL_ERROR, NAME, __FILE__, __FUNCTION__, __LINE__, format, ## args )

// only gets printed if #definde BUILD_TESTS
#define LOOPP_PRINT_TEST( format, args... ) loopp_debug( DEBUG_LEVEL_DEBUG_ONLY, NAME, __FILE__, __FUNCTION__, __LINE__, format, ## args )

#endif


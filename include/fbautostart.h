/*
 * fbautostart.h
 * 
 * This file is part of the Fluxbox Autostart ( fbautostart )
 * Utility.
 *
 * Copyright(c) 2010 Paul Tagliamonte <paultag@ubuntu.com>
 *
 * This file may be licensed under the terms of of the
 * GNU General Public License Version 3 (the ``GPL'').
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the GPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the GPL along with this
 * program. If not, go to http://www.gnu.org/licenses/gpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef _FBAUTOSTART_H
#define _FBAUTOSTART_H ohai

#ifndef _DEBUG_MODE
#define _DEBUG_MODE 0
#endif

#ifndef _ON_BEHALF_OF
#define _ON_BEHALF_OF "FLUXBOX"
#endif


#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <vector>
#include <cstdlib>
#include <string.h>
#include <stdio.h>

#include <sstream>

#include <dot_desktop.h>

/*
 * OK. Here's some Spec text. I accessed this data on
 * October 14th, 2010. 
 *
 * The URL was:
 * http://standards.freedesktop.org/autostart-spec/autostart-spec-latest.html
 *
 *
 * == Autostart Directories ==
 *
 * The Autostart Directories are $XDG_CONFIG_DIRS/autostart as
 * defined in accordance with the "Referencing this specification"
 * section in the "desktop base directory specification".
 *
 * If the same filename is located under multiple Autostart Directories
 * only the file under the most important directory should be used.
 *
 * Example: If $XDG_CONFIG_HOME is not set the Autostart Directory
 *          in the user's home directory is ~/.config/autostart/
 *
 * Example: If $XDG_CONFIG_DIRS is not set the system wide Autostart
 *          Directory is /etc/xdg/autostart/
 *
 * Example: If $XDG_CONFIG_HOME and $XDG_CONFIG_DIRS are not set and
 *          the two files /etc/xdg/autostart/foo.desktop and
 *          ~/.config/autostart/foo.desktop exist then only the file
 *          ~/.config/autostart/foo.desktop will be used because
 *          ~/.config/autostart/ is more important than /etc/xdg/autostart/
 */

#define _DEFAULT_XDG_HOME "~/.config"
#define _DEFAULT_XDG_DIRS "/etc/xdg"
#define _XDG_AUTOSTART_DIR "/autostart/"

void logError( std::string s ) { // [e]: my error line
	std::cerr << "[e]: " << s << std::endl;
}

void logError( int i ) { // [e]: (int) n
	std::cerr << "[e]: (int) " << i << std::endl;
}

void debug( std::string s ) {    // [l]: my debug line
	if ( _DEBUG_MODE ) {
		std::cout << "[l]: " << s << std::endl;
	}
}

void debug( std::vector<std::string> * foo ) {    // [l]: my debug line
	if ( _DEBUG_MODE ) {
		std::cout << "[l]: The following is an array" << std::endl;
		for ( unsigned int i = 0; i < foo->size(); ++i ) {
			std::cout << "[l]:   " << foo->at(i) << std::endl;
		}
	}
}

void debug( size_t foo ) {    // [l]: my debug line
	if ( _DEBUG_MODE ) {
		std::cout << "[l]: " << foo << std::endl;
	}
}

void breakupLine( std::vector<std::string> * locs, std::string lines ) {
	std::string token;
	std::istringstream iss(lines);
	while ( getline(iss, token, ':') ) {
		token.append( _XDG_AUTOSTART_DIR ); // use the xdg autostart dir.
		locs->push_back( token );
	}
}

void fixHomePathing( std::vector<std::string> * locs, std::string home ) {
	for ( unsigned int i = 0; i < locs->size(); ++i ) {
		if ( locs->at(i).substr(0,2) == "~/" ) {
			debug("");
			debug("Fixing a path old / new path follows.");
			debug(locs->at(i));
			locs->at(i).replace(0, 1, home );
			debug(locs->at(i));
		} else {
			debug("");
			debug("Path looks OK. Path follows.");
			debug(locs->at(i));
		}
	}
}

std::vector<std::string> * getConfDirs() {
	std::vector<std::string> * loc = new std::vector<std::string>(); // locations to look

	const char * xdg_home = getenv("XDG_CONFIG_HOME"); // See the spec for why
	const char * xdg_dirs = getenv("XDG_CONFIG_DIRS"); // I'm using what I'm using.
	const char * uzr_home = getenv("HOME");

	if ( uzr_home != NULL ) {
		if ( xdg_dirs != NULL ) {
			debug( "We have an overridden xdg dir. Using it." );
		} else {
			xdg_home = _DEFAULT_XDG_DIRS;
		}

		if ( xdg_home != NULL ) {
			debug( "We have an overridden xdg home. Using it." );
		} else {
			xdg_home = _DEFAULT_XDG_HOME;
		}

		debug("");
		debug( "XDG Home: " );
		debug( xdg_home );
		debug("");

		debug( "XDG Dirs: " );
		debug( xdg_dirs );
		debug("");

		debug( "User Home: " );
		debug( uzr_home );
		debug("");

		breakupLine( loc, std::string( xdg_dirs ) );
		breakupLine( loc, std::string( xdg_home ) );
		debug("Using the following array to find files:");

		debug(loc);

		for ( unsigned int i = 0; i < loc->size(); ++i )
			fixHomePathing( loc, uzr_home );

		debug("Using the following array to find files ( Expanded ): ");
		debug(loc);

		return loc;
	} else {
		logError( "the env var HOME is not set. Panic!" );
		return NULL;
	}
}

std::vector<std::string> * getConfFiles( std::vector<std::string> * dirs ) {
	std::vector<std::string> * files = new std::vector<std::string>();
	for ( unsigned int i = 0; i < dirs->size(); ++i ) {
		DIR           * dp   = NULL;
		struct dirent * dirp = NULL;
		if ((dp = opendir(dirs->at(i).c_str())) == NULL ) {
			if ( errno == ENOENT ) { // E(RROR) NO EXIST ( or whatever )
				debug("");
				debug("Looks like the dir does not exist. Dir follows.");
				debug( dirs->at(i) );
				debug("I'm going to keep this quiet. No big deal. Moving on.");
			} else {
				logError("");
				logError("Oh no! Error opening directory! Directory, then Errorno follows: ");
				logError( dirs->at(i) );
				logError(errno);
			}
		} else {
			debug( "" );
			debug( "Processing Directory:" );
			debug( dirs->at(i) );
			while ((dirp = readdir(dp)) != NULL) { // for every file in the directory
				std::string file(dirp->d_name);
				if ( file != "." && file != ".."  ) { // make sure we don't use . / ..
					// debug(file);
					std::string dees_nutz = dirs->at(i); // howabout deez nuts?
					file = dees_nutz.append(file); // there's a dick joke somewhere in here
					files->push_back( dees_nutz ); // so we have the full path for reading l8r
				} 
			}
			closedir(dp); // done with you. bieeeatch.
		}
	}
	return files;
}

std::vector<dot_desktop *> * loadDesktopFiles( std::vector<std::string> * files ) {
	std::vector<dot_desktop *> * object_array = new std::vector<dot_desktop *>();
	for ( unsigned int i = 0; i < files->size(); ++i ) {
		object_array->push_back(new dot_desktop(files->at(i)));
	}
	return object_array;
}

#endif

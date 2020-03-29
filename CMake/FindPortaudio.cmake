# Try to find PortAudio
# Once done, this will define
#
# PORTAUDIO_FOUND - system has PortAudio
# PORTAUDIO_INCLUDE_DIR - the PortAudio include directories
# PORTAUDIO_LIBRARIES - link these to use PortAudio
#
#
# This file was taken from RakNet 4.082.
# Please see licenses/RakNet license.txt for the underlying license and related copyright.
#
#
#
#  Modified work: Copyright (c) 2016, SLikeSoft UG (haftungsbeschränkt)
#
# This source code was modified by SLikeSoft. Modifications are licensed under the MIT-style
#  license found in the license.txt file in the root directory of this source tree.
#

if(PORTAUDIO_INCLUDE_DIR AND PORTAUDIO_LIBRARIES)
set(PORTAUDIO_FIND_QUIETLY TRUE)
endif(PORTAUDIO_INCLUDE_DIR AND PORTAUDIO_LIBRARIES)

# include dir
find_path(PORTAUDIO_INCLUDE_DIR portaudio.h
	/usr/include/
	/usr/local/include/
)



# Finally the library itself.
find_library(libPortAudio NAMES portaudio)
find_library(libPortAudioCpp NAMES portaudiocpp)
set(PORTAUDIO_LIBRARIES ${libPortAudio} ${libPortAudioCpp})

# Handle the QUIETLY and REQUIRED arguments and set PORTAUDIO_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PortAudio DEFAULT_MSG PORTAUDIO_LIBRARIES PORTAUDIO_INCLUDE_DIR)

mark_as_advanced(PORTAUDIO_LIBRARIES PORTAUDIO_INCLUDE_DIR)

import os
import sys


def is_active():
	return False
	
def get_name():
	return "iSIM"
	

def can_build():

	import sys
	if sys.platform == 'darwin':
		return True

	return False

def get_opts():

	return [
		('ISIMPLATFORM', 'name of the iphone platform', 'iPhoneSimulator'),
		('ISIMPATH', 'the path to iphone toolchain', '/Applications/Xcode.app/Contents/Developer/Platforms/${ISIMPLATFORM}.platform'),
		('ISIMSDK', 'path to the iphone SDK', '$ISIMPATH/Developer/SDKs/${ISIMPLATFORM}7.0.sdk'),
		('game_center', 'Support for game center', 'yes'),
		('store_kit', 'Support for in-app store', 'yes'),
		('ios_gles22_override', 'Force GLES2.0 on iOS', 'yes'),
		('ios_GLES1_override', 'Force legacy GLES (1.1) on iOS', 'no'),
		('ios_appirater', 'Enable Appirater', 'no'),
		('ios_exceptions', 'Use exceptions when compiling on playbook', 'no'),
	]

def get_flags():

	return [
		('lua', 'yes'),
		('tools', 'yes'),
		('nedmalloc', 'no'),
		('webp', 'yes'),
	]



def configure(env):

	env.Append(CPPPATH=['#platform/iphone'])

	env['OBJSUFFIX'] = ".isim.o"
	env['LIBSUFFIX'] = ".isim.a"
	env['PROGSUFFIX'] = ".isim"

	env['ENV']['PATH'] = env['ISIMPATH']+"/Developer/usr/bin/:"+env['ENV']['PATH']

	env['CC'] = '$ISIMPATH/Developer/usr/bin/gcc'
	env['CXX'] = '$ISIMPATH/Developer/usr/bin/g++'
	env['AR'] = 'ar'

	import string
	env['CCFLAGS'] = string.split('-arch i386 -fobjc-abi-version=2 -fobjc-legacy-dispatch -fmessage-length=0 -fpascal-strings -fasm-blocks  -Wall -D__IPHONE_OS_VERSION_MIN_REQUIRED=40100 -isysroot $ISIMSDK -mios-simulator-version-min=4.3 -DCUSTOM_MATRIX_TRANSFORM_H=\\\"build/iphone/matrix4_iphone.h\\\" -DCUSTOM_VECTOR3_TRANSFORM_H=\\\"build/iphone/vector3_iphone.h\\\"')

	env.Append(LINKFLAGS=['-arch', 'i386',
							'-mios-simulator-version-min=4.3',
							'-isysroot', '$ISIMSDK',
							#'-mmacosx-version-min=10.6',
							'-Xlinker',
							'-objc_abi_version',
							'-Xlinker', '2',
							'-framework', 'Foundation',
							'-framework', 'UIKit',
							'-framework', 'IOKit',
							'-framework', 'CoreGraphics',
							'-framework', 'OpenGLES',
							'-framework', 'QuartzCore',
							'-framework', 'AudioToolbox',
							'-framework', 'MediaPlayer',
							'-F$ISIMSDK',
							])

	env.Append(CPPPATH = ['$ISIMSDK/System/Library/Frameworks/OpenGLES.framework/Headers'])

	if (env["target"]=="release"):

		env.Append(CCFLAGS=['-O3', '-ffast-math'])
		env.Append(LINKFLAGS=['-O3', '-ffast-math'])
		env['OBJSUFFIX'] = "_opt"+env['OBJSUFFIX']
		env['LIBSUFFIX'] = "_opt"+env['LIBSUFFIX']

	elif (env["target"]=="debug"):

		env.Append(CCFLAGS=['-DDEBUG', '-D_DEBUG', '-gdwarf-2', '-Wall', '-O0', '-DDEBUG_ENABLED'])
		env.Append(CPPFLAGS=['-DDEBUG_MEMORY_ALLOC'])

	elif (env["target"]=="profile"):

		env.Append(CCFLAGS=['-g','-pg'])
		env.Append(LINKFLAGS=['-pg'])


	env['ENV']['MACOSX_DEPLOYMENT_TARGET'] = '10.6'
	env['ENV']['CODESIGN_ALLOCATE'] = '/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/codesign_allocate'
	env.Append(CPPFLAGS=['-DIPHONE_ENABLED', '-DUNIX_ENABLED', '-DGLES2_ENABLED', '-fno-exceptions'])

	if env['lua'] == "yes":
		env.Append(CCFLAGS=['-DLUA_USE_FLOAT'])


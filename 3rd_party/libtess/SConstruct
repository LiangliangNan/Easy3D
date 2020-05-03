
import sys

glutess_files = [
    "dict.c",
    "geom.c",
    "memalloc.c",
    "mesh.c",
    "normal.c",
    "priorityq.c",
    "priorityq-heap.c",
    "render.c",
    "sweep.c",
    "tess.c",
    "tessellate.c",
    "tessmono.c"
]

ACCEPTABLE_ARCHS = ['x86_64', 'x86']


def get_lib_path( base_name, is_debug, target_arch ):

    # Darwin uses fat binaries, so don't append bit count
    append_bits = True
    if sys.platform=='darwin':
        append_bits = False

    if append_bits:
        if target_arch == 'x86_64':
            base_name += '64'
    
        if target_arch == 'x86':
            base_name += '32'

    if is_debug:
        base_name += '_d'
    return base_name

# Get command line args
debug = int(ARGUMENTS.get( 'debug', 1 ))
arch  = ARGUMENTS.get('arch', 'x86_64')
if arch not in ACCEPTABLE_ARCHS:
    print "Invalid arch.  valid archs are: " + ','.join( ACCEPTABLE_ARCHS )
    sys.exit(1)                                                         

env = Environment( TARGET_ARCH=arch )
if sys.platform == 'win32':
    #env.Append( CPPFLAGS=['-Wall'] )

    if debug:
        env.Append( CPPFLAGS=['/Od',               # opts off
                              '/Zi',               # pdb
                              '/MDd',              # Mulithreaded Debug DLL
                              '/RTC1'              # run time checks
                          ],
                    LINKFLAGS=['/DEBUG' ],         # linker debug info
                )
    else:
        env.Append( CPPFLAGS=['/Ox',               # opt max
                              '/fp:precise',       # precise fp
                              '/d2Zi+',            # undocumented release mode debugging (vs2012)
                              '/MD'                # Multithreaded DLL
                          ] )


else: # Not-Windows case

    env.Append( CPPFLAGS=['-Wall'] )

    if debug:
        env.Append( CPPFLAGS=['-O0',               # opts off
                              '-g' ]               # generate debug symbols
                    )
    else:
        env.Append( CPPFLAGS=['-O3'] )
                              
sys.stdout.write("Build arch:\t\t")
sys.stdout.write( env['TARGET_ARCH'] )
if debug:
  sys.stdout.write(" Debug\n")
else:
  sys.stdout.write(" Release\n")


out_path = get_lib_path( 'glutess', debug, env['TARGET_ARCH'])

env.StaticLibrary(out_path, glutess_files )


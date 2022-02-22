project(
	'Mandelbrot',
	'cpp',
	default_options: [
		'cpp_std=c++14',
		'warning_level=3',
		'buildtype=release'
	]
)

compiler = meson.get_compiler('cpp')
if compiler.get_id() == 'gcc' and compiler.version().version_compare('<=5.0.0')
	error('Must use GCC 5.0.0 or newer')
endif

libpng = compiler.find_library('png')
threading = dependency('threads')

mandelbrotSrcs = [
	'mandelbrot.cxx', 'compute.cxx',    'shade.cxx',
	'pngWriter.cxx',  'argsParser.cxx', 'socket.cxx'
]

mandelbrot = executable('mandelbrot',
	mandelbrotSrcs,
	dependencies: [libpng, threading],
	#install_rpath: '$(ORIGIN)',
	install: true,
	build_by_default: true
)

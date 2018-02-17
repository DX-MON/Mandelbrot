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

libpng = compiler.find_library('png')
threading = dependency('threads')

mandelbrotSrcs = [
	'mandelbrot.cxx', 'compute.cxx',    'shade.cxx',
	'pngWriter.cxx',  'argsParser.cxx', 'pipeStream.cxx'
]

mandelbrot = executable('mandelbrot',
	mandelbrotSrcs,
	dependencies: [libpng, threading],
	#install_rpath: '$(ORIGIN)',
	install: true,
	build_by_default: true
)

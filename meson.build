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

rSON = compiler.find_library('rSON', dirs: meson.current_source_dir() + '/rSON')
rSONSocket = compiler.find_library('rSON.socket', dirs: meson.current_source_dir() + '/rSON')
libpng = compiler.find_library('png')
threading = dependency('threads')

mandelbrotSrcs = [
	'mandelbrot.cxx', 'compute.cxx',    'shade.cxx',
	'pngWriter.cxx',  'argsParser.cxx', 'pipeStream.cxx'
]

mandelbrot = executable('mandelbrot',
	mandelbrotSrcs,
	include_directories: include_directories('rSON'),
	dependencies: [rSON, rSONSocket, libpng, threading],
	install_rpath: '$(ORIGIN)',
	install: true,
	build_by_default: true
)

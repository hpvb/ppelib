pelib_sources = [
	'utils.c',
	'main.c',
	'pelib-error.c',
	gen_src,
	gen_h
]

pelib = shared_library(
	'pelib',
	pelib_sources,
	include_directories: inc,
	install: true,
	version: '0.0.1',
	soversion: 0
)
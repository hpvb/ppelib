project(
	'ppelib', 'c',
	default_options : ['c_std=c11'],
	version : '0.0.1',
	license: 'Apache-2.0',
)

pkg = import('pkgconfig')
lib = library('ppelib')
pkg.generate(
	lib,
	description: 'Portable library for manipulating PE (Portable Executable) files',
)

prog_python = import('python').find_installation('python3', modules: ['jinja2', 'inflection'])
cc = meson.get_compiler('c')

inc = include_directories('include')
subdir('include')
subdir('src')
subdir('test')
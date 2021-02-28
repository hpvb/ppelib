project(
	'ppelib', 'c',
	default_options : ['c_std=c11'],
	version : '0.0.1',
	license: 'Apache-2.0',
)

cc = meson.get_compiler('c')

if get_option('use_clang_fuzzer') == true
	if cc.get_id() != 'clang'
		error('use_clang_fuzzer must use CC=clang')
	endif
	
	add_project_arguments(
		'-fsanitize=fuzzer-no-link,address,undefined',
		language: ['c', 'cpp'],	
	)
	add_project_link_arguments(
		['-fsanitize=fuzzer,address,undefined', '-Wl,--unresolved-symbols=ignore-in-object-files'],
		language: ['c', 'cpp'],
	)
endif

pkg = import('pkgconfig')
lib = library('ppelib')
pkg.generate(
	lib,
	description: 'Portable library for manipulating PE (Portable Executable) files',
)

prog_python = import('python').find_installation('python3', modules: ['jinja2', 'inflection'])

inc = include_directories('include')
subdir('include')
subdir('src')
if get_option('use_clang_fuzzer') == false
	subdir('test')
else
	subdir('fuzz')
endif

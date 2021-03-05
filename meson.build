project(
	'ppelib', 'c',
	default_options: [
		'c_std=c11',
		'warning_level=2',
	],
	version: '0.0.1',
	license: 'Apache-2.0',
)

cc = meson.get_compiler('c')

if get_option('use_clang_fuzzer') == true
	if cc.get_id() != 'clang'
		error('use_clang_fuzzer must use CC=clang')
	endif

	add_project_arguments(
		['-fsanitize=fuzzer-no-link,address,undefined', '-DFUZZ=1'],
		language: ['c', 'cpp'],
	)
	add_project_link_arguments(
		['-fsanitize=fuzzer,address,undefined', '-Wl,--unresolved-symbols=ignore-in-object-files'],
		language: ['c', 'cpp'],
	)
endif

if cc.get_argument_syntax() == 'gcc'
	add_project_arguments(
		['-Wconversion', '-D_POSIX_C_SOURCE=200809L', '-D_GNU_SOURCE=1'],
		language: ['c', 'cpp'],
	)
endif

if cc.get_id() == 'msvc'
	add_project_arguments(
		'-D_CRT_SECURE_NO_WARNINGS=1',
		language: ['c', 'cpp'],
	)
endif

prog_python = import('python').find_installation('python3', modules: ['jinja2', 'yaml'])

inc = include_directories('include')
subdir('generator')
subdir('include')
subdir('src')
if get_option('use_clang_fuzzer') == false
	subdir('test')
else
	subdir('fuzz')
endif

pkg = import('pkgconfig')
pkg.generate(
	ppelib,
	description: 'Portable library for manipulating PE (Portable Executable) files',
)

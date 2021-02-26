project(
	'pelib', 'c',
	version : '0.0.1',
)

prog_python = import('python').find_installation('python3', modules: ['jinja2', 'inflection'])

gen_src = custom_target(
	'generate-files-c',
	input: [
		meson.source_root() + '/generator/templates/pelib-certificate_table.c',
		meson.source_root() + '/generator/templates/pelib-header.c',
		meson.source_root() + '/generator/templates/pelib-section.c',
		meson.source_root() + '/generator/templates/print-field-macro.jinja',
		meson.source_root() + '/generator/generate-files.py',
	],
	output: [
		'pelib-certificate_table.c',
		'pelib-header.c',
		'pelib-section.c',
	],
	command: [prog_python, meson.source_root() + '/generator/generate-files.py', '@OUTDIR@', 'c']
)

inc = include_directories('include')
subdir('include')
subdir('src')
subdir('test')
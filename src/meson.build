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
	version: meson.project_version(),
	soversion: 0
)
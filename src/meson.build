if cc.get_id() != 'msvc'
	if cc.get_argument_syntax() == 'gcc'
		extra_args = ['-fvisibility=hidden']
	else
		if cc.has_argument('-fvisibility=hidden')
			extra_args = ['-fvisibility=hidden']
		endif
	endif
endif

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

pelib = library(
	'pelib',
	pelib_sources,
	c_args : extra_args,
	include_directories: inc,
	install: true,
	version: meson.project_version(),
	soversion: 0
)
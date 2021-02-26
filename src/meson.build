prog_python = import('python').find_installation('python3')

gen_src = custom_target('gen-output',
	input: [
		meson.source_root() + '/generator/templates/pelib-certificate_table.c',
		meson.source_root() + '/generator/templates/pelib-certificate_table.h',
		meson.source_root() + '/generator/templates/pelib-header.c',
		meson.source_root() + '/generator/templates/pelib-header.h',
		meson.source_root() + '/generator/templates/pelib-section.c',
		meson.source_root() + '/generator/templates/pelib-section.h',
		meson.source_root() + '/generator/templates/print-field-macro.jinja',
		meson.source_root() + '/generator/generate-files.py',
	],
	output: [
		'pelib-certificate_table.c',
		'pelib-certificate_table.h',
		'pelib-header.c',
		'pelib-header.h',
		'pelib-section.c',
		'pelib-section.h',
	],
	command: [prog_python, meson.source_root() + '/generator/generate-files.py', '@OUTDIR@'])

pelib_sources = ['utils.c', gen_src]

pelib = shared_library('pelib',
                        pelib_sources,
                        include_directories : inc,
                        install : true)
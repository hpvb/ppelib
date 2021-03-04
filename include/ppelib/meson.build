gen_h = custom_target(
	'generate-files-h',
	input: [ files_public_gen ],
	output: [
		'ppelib-section.h',
		'ppelib-section-lowlevel.h',
		'ppelib-header.h',
		'ppelib-header-lowlevel.h',
	],
	install: true,
	install_dir: get_option('includedir') + '/ppelib',
	command: [prog_python, meson.source_root() + '/generator/generate-public.py', '@OUTDIR@']
)

install_headers(
	'ppelib.h',
	'ppelib-constants.h',
	'ppelib-low-level.h',
	'ppelib-resource-table.h',
	subdir: 'ppelib'
)

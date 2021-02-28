gen_h = custom_target(
	'generate-files-h',
	input: [
		meson.source_root() + '/generator/templates/ppelib-certificate_table.h',
		meson.source_root() + '/generator/templates/ppelib-header.h',
		meson.source_root() + '/generator/templates/ppelib-section.h',
		meson.source_root() + '/generator/templates/print-field-macro.jinja',
		meson.source_root() + '/generator/generate-files.py',
	],
	output: [
		'ppelib-certificate_table.h',
		'ppelib-header.h',
		'ppelib-section.h',
	],
	install: true,
	install_dir: get_option('includedir') + '/ppelib',
	command: [prog_python, meson.source_root() + '/generator/generate-files.py', '@OUTDIR@', 'header']
)

install_headers(
	'ppelib.h',
	'ppelib-constants.h',
	'ppelib-low-level.h',
	'ppelib-resource-table.h',
	subdir: 'ppelib'
)
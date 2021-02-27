gen_h = custom_target(
	'generate-files-h',
	input: [
		meson.source_root() + '/generator/templates/pelib-certificate_table.h',
		meson.source_root() + '/generator/templates/pelib-header.h',
		meson.source_root() + '/generator/templates/pelib-section.h',
		meson.source_root() + '/generator/templates/print-field-macro.jinja',
		meson.source_root() + '/generator/generate-files.py',
	],
	output: [
		'pelib-certificate_table.h',
		'pelib-header.h',
		'pelib-section.h',
	],
	install: true,
	install_dir: get_option('includedir') + '/pelib',
	command: [prog_python, meson.source_root() + '/generator/generate-files.py', '@OUTDIR@', 'header']
)

install_headers(
	'pelib.h',
	'pelib_constants.h',
	subdir: 'pelib'
)
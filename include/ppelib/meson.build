gen_h = custom_target(
	'generate-files-h',
	input: [ files_public_gen ],
	output: [
		'ppelib-coff_symbol-lowlevel.h',
		'ppelib-coff_symbol.h',
		'ppelib-dos_header-lowlevel.h',
		'ppelib-dos_header.h',
		'ppelib-header-lowlevel.h',
		'ppelib-header.h',
		'ppelib-section-lowlevel.h',
		'ppelib-section.h',
		'ppelib-vlv_signature-lowlevel.h',
		'ppelib-vlv_signature.h'
	],
	install: true,
	install_dir: get_option('includedir') + '/ppelib',
	command: [prog_python, meson.source_root() + '/generator/generate-public.py', '@OUTDIR@']
)

install_headers(
	'ppelib.h',
	'ppelib-constants.h',
	'ppelib-data-directory-lowlevel.h',
	'ppelib-data-directory.h',
	'ppelib-low-level.h',
	subdir: 'ppelib'
)

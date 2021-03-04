gen_src = custom_target(
	'generate-files-c',
	input: [ files_private_gen ],
	output: [
		'header_deserialize.c',
		'header_getset.c',
		'header_print.c',
		'header_private.h',
		'header_serialize.c',
		'section_deserialize.c',
		'section_getset.c',
		'section_print.c',
		'section_private.h',
		'section_serialize.c',
	],
	command: [prog_python, meson.source_root() + '/generator/generate-private.py', '@OUTDIR@']
)

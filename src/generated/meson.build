gen_src = custom_target(
	'generate-files-c',
	input: [ files_private_gen ],
	output: [
		'header_getset.c',
		'header_deserialize.c',
		'header_serialize.c',
		'header_print.c',
		'header_private.h'
	],
	command: [prog_python, meson.source_root() + '/generator/generate-private.py', '@OUTDIR@']
)

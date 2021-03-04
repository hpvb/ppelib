if cc.get_id() != 'msvc'
	if cc.get_argument_syntax() == 'gcc'
		extra_args = ['-fvisibility=hidden']
	else
		if cc.has_argument('-fvisibility=hidden')
			extra_args = ['-fvisibility=hidden']
		endif
	endif
else
	extra_args = []
endif

gen_src = custom_target(
	'generate-files-c',
	input: [
		meson.source_root() + '/generator/generator.py',
		meson.source_root() + '/generator/generate-private.py',
		meson.source_root() + '/generator/structures/header.yaml',
		meson.source_root() + '/generator/templates/getset_structure.c',
		meson.source_root() + '/generator/templates/header_deserialize.c',
		meson.source_root() + '/generator/templates/header_serialize.c',
		meson.source_root() + '/generator/templates/print_structure.c',
		meson.source_root() + '/generator/templates/private_header.h'
	],
	output: [
		'header_getset.c',
		'header_deserialize.c',
		'header_serialize.c',
		'header_print.c',
		'header_private.h'
	],
	command: [prog_python, meson.source_root() + '/generator/generate-private.py', '@OUTDIR@']
)


ppelib_sources = [
#	'ppelib-certificates.c',
	'ppelib-error.c',
#	'ppelib-handles.c',
#	'ppelib-headers.c',
#	'ppelib-resource-table.c',
#	'ppelib-sections.c',
	'utils.c',
	gen_src,
]

ppelib = shared_library(
	'ppelib',
	ppelib_sources,
	c_args: extra_args,
	include_directories: inc,
	install: true,
	version: meson.project_version(),
	soversion: 0
)

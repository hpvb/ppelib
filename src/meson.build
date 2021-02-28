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

if cc.get_id() == 'clang'
	extra_args += '-fsanitize=fuzzer'
	extra_args += '-v'
	extra_linker_args = ['-Wl,--whole-archive', '/usr/lib64/clang/11.0.0/lib/linux/libclang_rt.fuzzer-x86_64.a', '-Wl,--no-whole-archive', '-lstdc++', '-pthread', '-lm']
else
	extra_linker_args = []
endif

gen_src = custom_target(
	'generate-files-c',
	input: [
		meson.source_root() + '/generator/templates/ppelib-certificate_table.c',
		meson.source_root() + '/generator/templates/ppelib-header.c',
		meson.source_root() + '/generator/templates/ppelib-section.c',
		meson.source_root() + '/generator/templates/print-field-macro.jinja',
		meson.source_root() + '/generator/generate-files.py',
	],
	output: [
		'ppelib-certificate_table.c',
		'ppelib-header.c',
		'ppelib-section.c',
	],
	command: [prog_python, meson.source_root() + '/generator/generate-files.py', '@OUTDIR@', 'c']
)

ppelib_sources = [
	'ppelib-certificates.c',
	'ppelib-error.c',
	'ppelib-handles.c',
	'ppelib-headers.c',
	'ppelib-resource-table.c',
	'ppelib-sections.c',
	'utils.c',
	gen_src,
	gen_h
]

ppelib = library(
	'ppelib',
	ppelib_sources,
	c_args: extra_args,
	link_args: extra_linker_args,
	include_directories: inc,
	install: true,
	version: meson.project_version(),
	soversion: 0
)
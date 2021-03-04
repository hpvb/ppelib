structures_gen = [
	'structures/header.yaml',
	'structures/section.yaml',
]

files_private_gen = files([
	structures_gen,
	'generator.py',
	'generate-private.py',
	'templates/getset_structure.c',
	'templates/header_deserialize.c',
	'templates/header_serialize.c',
	'templates/print_structure.c',
	'templates/private_header.h',
])

files_public_gen = files([
	structures_gen,
	'generator.py',
	'generate-public.py',
	'templates/public_header_lowlevel.h',
	'templates/public_header.h',
])

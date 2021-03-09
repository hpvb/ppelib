structures_gen = [
	'structures/coff_symbol.yaml',
	'structures/dos_header.yaml',
	'structures/export_directory_table.yaml',
	'structures/header.yaml',
	'structures/import_directory_table.yaml',
	'structures/section.yaml',
	'structures/vlv_signature.yaml',
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
	'templates/structure_deserialize.c',
	'templates/structure_serialize.c',
])

files_public_gen = files([
	structures_gen,
	'generator.py',
	'generate-public.py',
	'templates/public_header_lowlevel.h',
	'templates/public_header.h',
])

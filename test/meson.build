content_roundtrip_files = [ 'content-roundtrip.c', gen_h ]
header_roundtrip_files = [ 'header-roundtrip.c', gen_h ]
print_header_files = [ 'print-header.c', gen_h ]
print_resource_table_files = [ 'print-resource-table.c', gen_h ]
remove_signature_files = [ 'remove-signature.c', gen_h ]

content_roundtrip = executable(
	'content-roundtrip',
	content_roundtrip_files,
	include_directories: inc,
	link_with: ppelib
)

header_roundtrip = executable(
	'header-roundtrip',
	header_roundtrip_files,
	include_directories: inc,
	link_with: ppelib
)

print_header = executable(
	'print-header',
	print_header_files,
	include_directories: inc,
	link_with: ppelib
)

print_resource_table = executable(
	'print-resource-table',
	print_resource_table_files,
	include_directories: inc,
	link_with: ppelib
)

remove_signature = executable(
	'remove-signature',
	remove_signature_files,
	include_directories: inc,
	link_with: ppelib
)
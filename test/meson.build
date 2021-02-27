header_roundtrip_files = [ 'header-roundtrip.c', gen_h ]
content_roundtrip_files = [ 'content-roundtrip.c', gen_h ]
print_header_files = [ 'print-header.c', gen_h ]

header_roundtrip = executable(
	'header-roundtrip',
	header_roundtrip_files,
	include_directories: inc,
	link_with: ppelib
)

content_roundtrip = executable(
	'content-roundtrip',
	content_roundtrip_files,
	include_directories: inc,
	link_with: ppelib
)

print_header = executable(
	'print-header',
	print_header_files,
	include_directories: inc,
	link_with: ppelib
)
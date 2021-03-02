fuzz = executable(
	'fuzz',
	[ 'fuzz.c', gen_h],
	include_directories: inc,
	link_with: ppelib,
)

fuzz_test = executable(
	'fuzz-test',
	[ 'fuzz-test.c', gen_h],
	include_directories: inc,
	link_with: ppelib,
)

test('fuzzer run', fuzz_test,
	timeout: 240,
	args: [meson.source_root() + '/fuzz/corpus', '-max_total_time=120', '-jobs=2']
)
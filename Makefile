make: test FORCE
	ninja -C out

win: win_test FORCE
	ninja.exe -C out

test: FORCE
	ninja -C out tests
	out/tests

win_test: FORCE
	ninja.exe -C out tests
	out/tests.exe

FORCE:

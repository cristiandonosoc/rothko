make: test FORCE
	ninja -C out

test: FORCE
	ninja -C out tests
	out/tests

FORCE:

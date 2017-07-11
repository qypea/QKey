all: verify

verify:
	arduino --verify `pwd`/QKey.ino

upload:
	arduino --upload `pwd`/QKey.ino

.PHONEY: test

all: test

build:
	python setup.py build

install: build
	python setup.py install

test: install
	python ospTutorial.py



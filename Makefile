CC ?= cc
.PHONY: check clean
check:
	mkdir -p build
	$(CC) -O2 -Wall -Wextra -Werror -pedantic -std=c11 -Isrc -o build/catalog-test tests/catalog_test.c src/q2_games_catalog.c
	./build/catalog-test
	python3 -m unittest discover -s tests -v
clean:
	rm -rf build

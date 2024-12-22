CC ?= cc

.PHONY: help
help: ## Display help section
	@ cat $(MAKEFILE_LIST) | grep -e "^[a-zA-Z_\-]*: *.*## *" | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

.PHONY: build
build: *.h *.c ## Build binary file
	$(CC) *.c -o dasm

.PHONY: test
test: build ## Run tests
	./tests/runtests.sh

all:
	@make -C src/


check:
	@./tests/run-all

clean:
	@rm -rf src/*.o
	@rm parsh
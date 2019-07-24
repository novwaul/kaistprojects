.PHONY: run clean

run: classes
	@if [ -d 'classes' ]; then scala -cp classes -i load; fi

compile: classes

classes: src/Main.scala
	@echo 'compile...'
	@rm -rf classes
	@mkdir classes
	@scalac src/*.scala -d classes || rm -rf classes

clean:
	@rm -rf classes
	@echo ''classes' directory is removed'

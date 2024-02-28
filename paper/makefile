.PHONY: main.pdf all clean clean-all

all: main.pdf

main.pdf: main.tex
	 latexmk -pdf -bibtex -shell-escape -file-line-error -f main.tex
	 # pdflatex main.tex 

clean:
	# latexmk -c
	$(RM) -f main.aux
	$(RM) -f main.blg
	$(RM) -f main.lof
	$(RM) -f main.log
	$(RM) -f main.out
	$(RM) -f main.toc
	$(RM) -f main.fls

clean-all: clean
	$(RM) -f main.bbl

game: game.c lilypond.wav unifont-15.1.05.otf
	gcc -ggdb3 -Og `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf -lSDL2_gfx -lSDL2_mixer -lm -o game game.c

unifont-15.1.05.otf:
	wget https://unifoundry.com/pub/unifont/unifont-15.1.05/font-builds/unifont-15.1.05.otf

lilypond.wav: lilypond.midi
	timidity -o lilypond.wav lilypond.midi -Ow

lilypond.midi: lilypond.txt
	lilypond lilypond.txt

clean:
	rm game lilypond.wav lilypond.midi lilypond.pdf

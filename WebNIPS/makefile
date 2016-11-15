TARGET=bin/js/facelib.asm.js
CPP=main violajones
EXP=capture_buffer detect_face track_face recognize_expression

FILES=$(addsuffix .cpp,$(addprefix src/,$(CPP)))
EXPORTS=$(addsuffix ',$(addprefix '_,$(EXP)))

NOOP:=
SPACE:= $(NOOP) $(NOOP)
COMMA:= ,

all:
	emcc $(FILES) -o $(TARGET) -s EXPORTED_FUNCTIONS="[$(subst $(SPACE),$(COMMA),$(EXPORTS))]" -Iinc/

clean:
	rm $(TARGET)

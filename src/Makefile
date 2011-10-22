CC      = ccache gcc34
PROF    = -O2 -g3 -DACK_43 -ggdb3
C_FLAGS =  -Wall $(PROF)
L_FLAGS =  -lcrypt $(PROF) -lz -lm

#various Makefile variables
ECHOCMD = echo -e

L_RED     = \e[1;31m
L_BLUE    = \e[1;34m
L_GREEN   = \e[1;32m
L_WHITE   = \e[1;37m
L_NRM     = \e[0;00m

O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o comm.o const.o \
          db.o fight.o handler.o interp.o engineer.o \
          save.o update.o board.o areasave.o buildings.o \
          buildtab.o build.o write.o buildare.o hash.o areachk.o \
          macros.o lists.o social-edit.o hack.o space.o \
          logs.o pdelete.o wizutil.o brands.o terrain.o web.o \
          ssm.o scheck.o sysdata.o strfuns.o mapper.o email.o \
          act_misc.o mccp.o mxp.o games.o act_skills.o act_alliance.o \
          chrono_wiz.o

ack: $(O_FILES)
	rm -f ack
	chmod g+w $(O_FILES)
	$(CC) $(L_FLAGS) -o ack $(O_FILES)

all:
	@touch chrono_wiz.c
	@$(MAKE) ack

.c.o: ack.h
	@$(ECHOCMD) "-->   Compiling file: $(L_WHITE)$<$(L_NRM)   <--"
	@$(CC) $(C_FLAGS) -c -o $@ $<

scan: scan.o
	rm -f scan
	$(CC) $(L_FLAGS) -o scan scan.o -lcrypt

clean:
	rm -f $(O_FILES) ack ../area/core

purge:
	@ccache -c


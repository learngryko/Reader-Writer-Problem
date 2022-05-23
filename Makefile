all:
	gcc -pthread noStarw.c -o noStarw
	gcc wake.c -o wake

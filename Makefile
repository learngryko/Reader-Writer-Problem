all:
	gcc -pthread writStarw.c -o writStarw
	gcc -pthread readStarw.c -o readStarw
	gcc -pthread noStarw.c -o noStarw

all : select

select : select.cpp
	g++ select.cpp -o select


clean: 
	rm select

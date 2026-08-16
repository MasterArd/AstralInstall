package common

/*
extern void init_c_random();
extern int get_random_number();
*/
import "C"


func Seed() int {
	C.init_c_random()
	randomNum := int(C.get_random_number())
	return randomNum
}
## Copy to ~/.bashrc or any other startup file used  

fy (){
	ret=$(faye $1)
	cd $ret
}

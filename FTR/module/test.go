package module

/*
#include "function.h"
#include <stdio.h>
#include <stdlib.h>
*/
import "C"
import (
	"fmt"
	//"github.com/mahonia"
	"net/http"
	"html/template"
	"strings"
	//"log"
)

func FTRInit(){
	var Data string = "data.txt"
	psData := C.CString(Data)
	var HZ string = "hz.txt"
	psHZ := C.CString(HZ)
	var IdxDat string = "indexDat.txt"
	psIdxDat := C.CString(IdxDat)
	C.FTRInit(psHZ, psData, psIdxDat)
}

func Run(w http.ResponseWriter, psUserQuery string) {
	psInput := C.CString(psUserQuery)
	tmp:=new(*C.char) 
	C.FTR(psInput, 50, tmp)
	t,_:=template.ParseFiles("./src/html/login.gtpl")
	t.Execute(w,nil)
	new:="<strong>"+psUserQuery+"</strong>"
	s:= C.GoString(*tmp)
	fmt.Fprintf(w, strings.Replace(s, psUserQuery, new, -1))
}

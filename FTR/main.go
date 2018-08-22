package main

import(
	"./module"
	"fmt"
	"net/http"
	"html/template"
	"log"
)

func IndexHandler(w http.ResponseWriter, r *http.Request) { 
	fmt.Fprintln(w, "hello world") 
} 


func query(w http.ResponseWriter, r *http.Request) { 
	if r.Method=="GET"{
		t,_:=template.ParseFiles("./src/html/login.gtpl")
		t.Execute(w,nil)
	}else{
		r.ParseForm()
		sUserQuery:=r.Form["userQuery"]
		fmt.Println(sUserQuery[0])
		q:=sUserQuery[0]
		module.Run(w, q)
	}

} 


func main(){
	//module.Run()
	//http.HandleFunc("/", IndexHandler) 
	module.FTRInit()

	http.HandleFunc("/query", query)
	err:=http.ListenAndServe("localhost:8000", nil)

	if err!=nil{
		log.Fatal("ListenAndServe:", err)
	}
}
